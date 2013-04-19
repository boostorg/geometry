// Boost.Geometry Index
//
// R-tree k nearest neighbour query visitor implementation
//
// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_INDEX_DETAIL_RTREE_VISITORS_NEAREST_QUERY_HPP
#define BOOST_GEOMETRY_INDEX_DETAIL_RTREE_VISITORS_NEAREST_QUERY_HPP

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

// TODO: awulkiew - maby it's a good idea to check if curr_mindist < comp_mindist and then check predicates
// in store() or break store to 2 functions e.g. should_store() and store()
// - well not with this algorithm of storing k-th neighbor

//template <typename Value, typename Translator, typename Point>
//struct nearest_query_result_one
//{
//public:
//    typedef typename geometry::default_distance_result<
//        Point,
//        typename translator::indexable_type<Translator>::type
//    >::type distance_type;
//
//    inline nearest_query_result_one(Value & value)
//        : m_value(value)
//        , m_comp_dist((std::numeric_limits<distance_type>::max)())
//    {}
//
//    inline void store(Value const& val, distance_type const& curr_comp_dist)
//    {
//        if ( curr_comp_dist < m_comp_dist )
//        {
//            m_comp_dist = curr_comp_dist;
//            m_value = val;
//        }
//    }
//
//    inline bool is_comparable_distance_valid() const
//    {
//        return m_comp_dist < (std::numeric_limits<distance_type>::max)();
//    }
//
//    inline distance_type comparable_distance() const
//    {
//        return m_comp_dist;
//    }
//
//    inline size_t finish()
//    {
//        return is_comparable_distance_valid() ? 1 : 0;
//    }
//
//private:
//    Value & m_value;
//    distance_type m_comp_dist;
//};

template <typename Value, typename Translator, typename Point, typename OutIt>
struct nearest_query_result_k
{
public:
    typedef typename geometry::default_distance_result<
        Point,
        typename indexable_type<Translator>::type
    >::type distance_type;

    inline explicit nearest_query_result_k(size_t k, OutIt out_it)
        : m_count(k), m_out_it(out_it)
    {
        BOOST_GEOMETRY_INDEX_ASSERT(0 < m_count, "Number of neighbors should be greater than 0");

        m_neighbors.reserve(m_count);
    }

    inline void store(Value const& val, distance_type const& curr_comp_dist)
    {
        if ( m_neighbors.size() < m_count )
        {
            m_neighbors.push_back(std::make_pair(curr_comp_dist, val));

            if ( m_neighbors.size() == m_count )
                std::make_heap(m_neighbors.begin(), m_neighbors.end(), neighbors_less);
        }
        else
        {
            if ( curr_comp_dist < m_neighbors.front().first )
            {
                std::pop_heap(m_neighbors.begin(), m_neighbors.end(), neighbors_less);
                m_neighbors.back().first = curr_comp_dist;
                m_neighbors.back().second = val;
                std::push_heap(m_neighbors.begin(), m_neighbors.end(), neighbors_less);
            }
        }
    }

    inline bool is_comparable_distance_valid() const
    {
        return m_neighbors.size() == m_count;
    }

    inline distance_type comparable_distance() const
    {
        // smallest distance is in the first neighbor only
        // if there is at least m_count values found
        // this is just for safety reasons since is_comparable_distance_valid() is checked earlier
        // TODO - may be replaced by ASSERT
        return m_neighbors.size() < m_count
            ? (std::numeric_limits<distance_type>::max)()
            : m_neighbors.front().first;
    }

    inline size_t finish()
    {
        typedef typename std::vector< std::pair<distance_type, Value> >::const_iterator neighbors_iterator;
        for ( neighbors_iterator it = m_neighbors.begin() ; it != m_neighbors.end() ; ++it, ++m_out_it )
            *m_out_it = it->second;

        return m_neighbors.size();
    }

private:
    inline static bool neighbors_less(
        std::pair<distance_type, Value> const& p1,
        std::pair<distance_type, Value> const& p2)
    {
        return p1.first < p2.first;
    }

    size_t m_count;
    OutIt m_out_it;

    std::vector< std::pair<distance_type, Value> > m_neighbors;
    distance_type m_biggest_comp_dist;
};

// TODO: awulkiew - add additional pruning before adding nodes to the ABL

template <
    typename Value,
    typename Options,
    typename Translator,
    typename Box,
    typename Allocators,
    typename Predicates,
    unsigned NearestPredicateIndex,
    typename Result
>
class nearest_query
    : public rtree::visitor<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag, true>::type
{
public:
    typedef typename Options::parameters_type parameters_type;

    typedef typename rtree::node<Value, parameters_type, Box, Allocators, typename Options::node_tag>::type node;
    typedef typename rtree::internal_node<Value, parameters_type, Box, Allocators, typename Options::node_tag>::type internal_node;
    typedef typename rtree::leaf<Value, parameters_type, Box, Allocators, typename Options::node_tag>::type leaf;

    typedef index::detail::predicates_element<NearestPredicateIndex, Predicates> nearest_predicate_access;
    typedef typename index::detail::distance_predicates_type<typename nearest_predicate_access::type>::type distance_predicates_type;

    typedef index::detail::distances_calc<distance_predicates_type, Box, index::detail::bounds_tag> node_distances_calc;
    typedef typename node_distances_calc::result_type node_distances_type;
    typedef index::detail::distances_predicates_check<distance_predicates_type, Box, index::detail::bounds_tag> node_distances_predicates_check;

    typedef index::detail::distances_calc<
        distance_predicates_type,
        typename indexable_type<Translator>::type,
        index::detail::value_tag
    > value_distances_calc;
    typedef typename value_distances_calc::result_type value_distances_type;
    typedef index::detail::distances_predicates_check<
        distance_predicates_type,
        typename indexable_type<Translator>::type,
        index::detail::value_tag
    > value_distances_predicates_check;

    inline distance_predicates_type const& dist_pred() const
    {
        return nearest_predicate_access::get(m_pred).distance_predicates;
    }

    static const unsigned predicates_len = index::detail::predicates_length<Predicates>::value;

    inline nearest_query(parameters_type const& parameters, Translator const& translator, Predicates const& pred, Result & r)
        : m_parameters(parameters), m_translator(translator)
        , m_pred(pred)
        , m_result(r)
    {}

    //TODO: awulkiew - consider this approach: store one, global vector of active branches, add branches only if mindist is ok

    inline void operator()(internal_node const& n)
    {
        typedef typename rtree::elements_type<internal_node>::type elements_type;

        // array of active nodes
        typename index::detail::rtree::container_from_elements_type<
            elements_type,
            std::pair<node_distances_type, typename Allocators::node_pointer>
        >::type active_branch_list;
        active_branch_list.reserve(m_parameters.get_max_elements());
        
        elements_type const& elements = rtree::elements(n);

        // fill array of nodes meeting predicates
        for (typename elements_type::const_iterator it = elements.begin();
            it != elements.end(); ++it)
        {
            // if current node meets predicates
            // 0 - dummy value
            if ( index::detail::predicates_check<index::detail::bounds_tag, 0, predicates_len>(m_pred, 0, it->first) )
            {
                // calculate node's distance(s) for distance predicate
                node_distances_type node_dist_data = node_distances_calc::apply(dist_pred(), it->first);

                // TODO: awulkiew - consider at first calculating near distance only,
                //                  comparing it with m_result.comparable_distance if it's valid,
                //                  after that calculate the rest of distances and check predicates

                // if current node is further than found neighbors - don't analyze it
                if ( m_result.is_comparable_distance_valid() &&
                     is_node_prunable(m_result.comparable_distance(), node_dist_data) )
                {
                    continue;
                }

                // if current node distance(s) meets distance predicate
                if ( node_distances_predicates_check::apply(dist_pred(), node_dist_data) )
                {
                    // add current node's data into the list
                    active_branch_list.push_back( std::make_pair(node_dist_data, it->second) );
                }
            }
        }

        // if there aren't any nodes in ABL - return
        if ( active_branch_list.empty() )
            return;
        
        // sort array
        std::sort(active_branch_list.begin(), active_branch_list.end(), abl_less);

        // recursively visit nodes
        for ( size_t i = 0 ;; ++i )
        {
            // prune nodes
            prune_nodes(active_branch_list);

            if ( active_branch_list.size() <= i )
                break;

            rtree::apply_visitor(*this, *active_branch_list[i].second);
        }
    }

    inline void operator()(leaf const& n)
    {
        typedef typename rtree::elements_type<leaf>::type elements_type;
        elements_type const& elements = rtree::elements(n);
        
        // search leaf for closest value meeting predicates
        for (typename elements_type::const_iterator it = elements.begin();
            it != elements.end(); ++it)
        {
            // if value meets predicates
            if ( index::detail::predicates_check<index::detail::value_tag, 0, predicates_len>(m_pred, *it, m_translator(*it)) )
            {
                // calculate values distance for distance predicate
                value_distances_type distances = value_distances_calc::apply(dist_pred(), m_translator(*it));

                // TODO: awulkiew - consider at first calculating point relation distance only,
                //                  comparing it with m_result.comparable_distance if it's valid,
                //                  after that calculate the rest of distances and check predicates

                // if distance meets distance predicate
                if ( value_distances_predicates_check::apply(dist_pred(), distances) )
                {
                    typedef typename index::detail::point_relation<distance_predicates_type>::type point_relation_type;
                    typedef typename index::detail::relation<point_relation_type>::tag point_relation_tag;

                    // store value
                    m_result.store(
                        *it,
                        index::detail::cdist_value<value_distances_type>
                            ::template get<point_relation_tag>(distances)
                    );
                }
            }
        }
    }

private:
    template <typename ActiveBranchList>
    inline void prune_nodes(ActiveBranchList & abl) const
    {
        // if some value were found
        if ( m_result.is_comparable_distance_valid() )
        {
            // prune if box's mindist is further than value's mindist
            while ( !abl.empty() &&
                    is_node_prunable(m_result.comparable_distance(), abl.back().first) )
            {
                abl.pop_back();
            }
        }
    }

    static inline bool abl_less(
        std::pair<node_distances_type, typename Allocators::node_pointer> const& p1,
        std::pair<node_distances_type, typename Allocators::node_pointer> const& p2)
    {
        return index::detail::cdist_value<node_distances_type>
                ::template get<index::detail::to_nearest_tag>(p1.first)
            < index::detail::cdist_value<node_distances_type>
                ::template get<index::detail::to_nearest_tag>(p2.first);
    }

    template <typename Distance>
    static inline bool is_node_prunable(Distance const& smallest_dist, node_distances_type const& d)
    {
        return smallest_dist
            < index::detail::cdist_value<node_distances_type>
                ::template get<index::detail::to_nearest_tag>(d);
    }

    parameters_type const& m_parameters;
    Translator const& m_translator;

    Predicates m_pred;
    Result & m_result;
};

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_INDEX_DETAIL_RTREE_VISITORS_NEAREST_QUERY_HPP
