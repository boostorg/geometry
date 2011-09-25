// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree k nearest neighbors query
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_NEAREST_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_NEAREST_HPP

#include <boost/geometry/extensions/index/rtree/distance_predicates.hpp>

#include <boost/geometry/extensions/index/rtree/node/node.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

// TODO: awulkiew - maby it's a good idea to check if curr_mindist < comp_mindist and then check predicates
// in store() or break store to 2 functions e.g. should_store() and store()
// - well not with this algorithm of storing k-th neighbor

template <typename Value, typename Translator, typename Point>
struct nearest_one
{
public:
    typedef typename Translator::indexable_type indexable_type;
    typedef typename geometry::default_distance_result<Point, indexable_type>::type distance_type;

    inline nearest_one()
        : m_comp_dist(std::numeric_limits<distance_type>::max())
    {}

    inline void store(Value const& val, distance_type const& curr_comp_dist)
    {
        if ( curr_comp_dist < m_comp_dist )
        {
            m_comp_dist = curr_comp_dist;
            m_value = val;
        }
    }

    inline bool is_comparable_distance_valid() const
    {
        return m_comp_dist < std::numeric_limits<distance_type>::max();
    }

    inline distance_type comparable_distance() const
    {
        return m_comp_dist;
    }

    inline size_t get(Value & v)
    {
        v = m_value;
        return is_comparable_distance_valid() ? 1 : 0;
    }

private:
    Value m_value;
    distance_type m_comp_dist;
};

template <typename Value, typename Translator, typename Point>
struct nearest_k
{
public:
    typedef typename Translator::indexable_type indexable_type;
    typedef typename geometry::default_distance_result<Point, indexable_type>::type distance_type;

    inline explicit nearest_k(size_t k)
        : m_count(k)
    {
        // TEMP?
        m_neighbors.reserve(m_count + 1);
    }

    inline void store(Value const& val, distance_type const& curr_comp_dist)
    {
        m_neighbors.push_back(std::make_pair(curr_comp_dist, val));
        std::sort(m_neighbors.begin(), m_neighbors.end(), neighbors_less);

        if ( m_count < m_neighbors.size() )
            m_neighbors.pop_back();

        // TODO: awulkiew - test other methods:
        // heap, manual inserting
        // don't sort if size < k ?
        // check the furthest distance at the first place, before push_back()
    }

    inline bool is_comparable_distance_valid() const
    {
        return m_neighbors.size() == m_count;
    }

    inline distance_type comparable_distance() const
    {
        return m_neighbors.size() < m_count ?
            std::numeric_limits<distance_type>::max() :
            m_neighbors.back().first;
    }

    template <typename OutIter>
    inline size_t get(OutIter & out_it)
    {
        typedef typename std::vector< std::pair<distance_type, Value> >::const_iterator neighbors_iterator;
        for ( neighbors_iterator it = m_neighbors.begin() ; it != m_neighbors.end() ; ++it )
            *out_it = it->second;

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
    std::vector< std::pair<distance_type, Value> > m_neighbors;
};

template <
    typename Value,
    typename Options,
    typename Translator,
    typename Box,
    typename DistancePredicate,
    typename Predicates,
    typename Result
>
class nearest
    : public rtree::visitor<Value, typename Options::parameters_type, Box, typename Options::node_tag, true>::type
    , index::nonassignable
{
public:
    typedef typename rtree::node<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type node;
    typedef typename rtree::internal_node<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type internal_node;
    typedef typename rtree::leaf<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type leaf;

    typedef index::detail::distance_calc<DistancePredicate, Box, rtree::node_distance_predicate_tag> node_distance_calc;
    typedef typename node_distance_calc::result_type node_distance_type;
    typedef index::detail::distance_predicate_check<DistancePredicate, Box, rtree::node_distance_predicate_tag> node_distance_predicate_check;
    typedef rtree::distance_less<DistancePredicate, Box> node_distance_less;

    typedef index::detail::distance_calc<DistancePredicate, typename Translator::indexable_type, rtree::value_distance_predicate_tag> value_distance_calc;
    typedef typename value_distance_calc::result_type value_distance_type;
    typedef index::detail::distance_predicate_check<DistancePredicate, typename Translator::indexable_type, rtree::value_distance_predicate_tag> value_distance_predicate_check;

    inline nearest(Translator const& t, DistancePredicate const& dist_pred, Predicates const& pred, Result & r)
        : m_tr(t), m_dist_pred(dist_pred), m_pred(pred)
        , m_result(r)
    {}

    //TODO: awulkiew - check this approach: store one, global vector of active branches, add branches only if mindist is ok

    inline void operator()(internal_node const& n)
    {
        // array of active nodes
        index::pushable_array<
            std::pair<node_distance_type, const node *>,
            Options::parameters_type::max_elements
        > active_branch_list;

        typedef typename rtree::elements_type<internal_node>::type elements_type;
        elements_type const& elements = rtree::elements(n);

        // fill array of nodes meeting predicates
        for (typename elements_type::const_iterator it = elements.begin();
            it != elements.end(); ++it)
        {
            // if current node meets predicates
            if ( index::predicates_check<rtree::node_predicates_tag>(m_pred, it->first) )
            {
                // calculate node's distance(s) for distance predicate
                node_distance_type node_dist_data = node_distance_calc::apply(m_dist_pred, it->first);

                // if current node distance(s) meets distance predicate
                if ( node_distance_predicate_check::apply(m_dist_pred, node_dist_data) )
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
        std::sort(active_branch_list.begin(), active_branch_list.end(), node_distance_less());

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
            if ( index::predicates_check<rtree::value_predicates_tag>(m_pred, m_tr(*it)) )
            {
                // calculate values distance for distance predicate
                value_distance_type dist = value_distance_calc::apply(m_dist_pred, m_tr(*it));

                // if distance meets distance predicate
                if ( value_distance_predicate_check::apply(m_dist_pred, dist) )
                {
                    // store value
                    m_result.store(*it, dist);
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
                    distance_node_pruning_check<DistancePredicate, Box>
                        ::apply(m_result.comparable_distance(), abl.back().first) )
            {
                abl.pop_back();
            }
        }
    }

    Translator const& m_tr;
    DistancePredicate const& m_dist_pred;
    Predicates const& m_pred;

    Result & m_result;
};

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_NEAREST_HPP
