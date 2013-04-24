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
class nearest_query_result_k
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

    inline bool has_enough_neighbors() const
    {
        return m_count <= m_neighbors.size();
    }

    inline distance_type greatest_comparable_distance() const
    {
        // greatest distance is in the first neighbor only
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
        typedef typename index::detail::rtree::container_from_elements_type<
            elements_type,
            std::pair<node_distances_type, typename Allocators::node_pointer>
        >::type active_branch_list_type;

        active_branch_list_type active_branch_list;
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
                if ( m_result.has_enough_neighbors() &&
                     is_node_prunable(m_result.greatest_comparable_distance(), node_dist_data) )
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
        for ( typename active_branch_list_type::const_iterator it = active_branch_list.begin();
              it != active_branch_list.end() ; ++it )
        {
            // if current node is further than furthest neighbor, the rest of nodes also will be further
            if ( m_result.has_enough_neighbors() &&
                 is_node_prunable(m_result.greatest_comparable_distance(), it->first) )
                break;

            rtree::apply_visitor(*this, *(it->second));
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
    static inline bool is_node_prunable(Distance const& greatest_dist, node_distances_type const& d)
    {
        return greatest_dist <= index::detail::cdist_value<node_distances_type>
                                    ::template get<index::detail::to_nearest_tag>(d);
    }

    inline distance_predicates_type const& dist_pred() const
    {
        return nearest_predicate_access::get(m_pred).distance_predicates;
    }

    parameters_type const& m_parameters;
    Translator const& m_translator;

    Predicates m_pred;
    Result & m_result;
};

template <
    typename Value,
    typename Options,
    typename Translator,
    typename Box,
    typename Allocators,
    typename Predicates,
    unsigned NearestPredicateIndex
>
class nearest_query_incremental
    : public rtree::visitor<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag, true>::type
{
public:
    typedef typename Options::parameters_type parameters_type;

    typedef typename rtree::node<Value, parameters_type, Box, Allocators, typename Options::node_tag>::type node;
    typedef typename rtree::internal_node<Value, parameters_type, Box, Allocators, typename Options::node_tag>::type internal_node;
    typedef typename rtree::leaf<Value, parameters_type, Box, Allocators, typename Options::node_tag>::type leaf;

    typedef index::detail::predicates_element<NearestPredicateIndex, Predicates> nearest_predicate_access;
    typedef typename index::detail::distance_predicates_type<typename nearest_predicate_access::type>::type distance_predicates_type;
    typedef typename geometry::default_distance_result<
        typename index::detail::relation<
            typename index::detail::point_relation<distance_predicates_type>::type
        >::value_type,
        typename indexable_type<Translator>::type
    >::type value_distance_type;

    typedef index::detail::distances_calc<distance_predicates_type, Box, index::detail::bounds_tag> node_distances_calc;
    typedef typename node_distances_calc::result_type node_distances_type;
    typedef index::detail::distances_predicates_check<distance_predicates_type, Box, index::detail::bounds_tag> node_distances_predicates_check;
    typedef typename index::detail::cdist_value<node_distances_type>::type node_distance_type;

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

    typedef typename Allocators::size_type size_type;
    typedef typename Allocators::node_pointer node_pointer;

    static const unsigned predicates_len = index::detail::predicates_length<Predicates>::value;

    typedef typename rtree::elements_type<internal_node>::type internal_elements;
    typedef typename internal_elements::const_iterator internal_iterator;
    typedef typename rtree::elements_type<leaf>::type leaf_elements;

    typedef std::pair<node_distances_type, node_pointer> branch_data;
    typedef typename index::detail::rtree::container_from_elements_type<
        internal_elements, branch_data
    >::type active_branch_list_type;
    typedef std::vector<
        std::pair<active_branch_list_type, typename active_branch_list_type::size_type>
    > internal_stack_type;

    inline nearest_query_incremental(Translator const& translator, Predicates const& pred)
        : m_translator(translator)
        , m_pred(pred)
        , current_neighbor((std::numeric_limits<size_type>::max)())

        , next_closest_node_distance((std::numeric_limits<node_distance_type>::max)())
    {
        BOOST_ASSERT_MSG(0 < max_count(), "k must be greather than 0");
    }

    Value const& dereference() const
    {
        return *(neighbors[current_neighbor].second);
    }

    void increment()
    {
        for (;;)
        {
            size_type new_neighbor = current_neighbor == (std::numeric_limits<size_type>::max)() ? 0 : current_neighbor + 1;

            if ( internal_stack.empty() )
            {
                if ( new_neighbor < neighbors.size() )
                    current_neighbor = new_neighbor;
                else
                {
                    current_neighbor = (std::numeric_limits<size_type>::max)();
// TODO - temporary - used to disable the condition above
                    neighbors.clear();
                }

                return;
            }
            else
            {
                active_branch_list_type & branches = internal_stack.back().first;
                typename active_branch_list_type::size_type & current_branch = internal_stack.back().second;

                if ( branches.size() <= current_branch )
                {
                    internal_stack.pop_back();
                    continue;
                }

                // if there are no nodes which can have closer values, set new value
                if ( new_neighbor < neighbors.size() &&
                     // here must be < because otherwise neighbours may be sorted in different order
                     // if there is another value with equal distance
                     neighbors[new_neighbor].first < next_closest_node_distance )
                {
                    current_neighbor = new_neighbor;
                    return;
                }

                // if node is further than the furthest neighbour, following nodes also will be further
                BOOST_ASSERT_MSG(neighbors.size() <= max_count(), "unexpected neighbours count");
                if ( max_count() <= neighbors.size() &&
                     is_node_prunable(neighbors.back().first, branches[current_branch].first) )
                {
                    // stop traversing current level
                    internal_stack.pop_back();
                    continue;
                }
                else
                {
                    // new level - must increment current_branch before traversing of another level (mem reallocation)
                    ++current_branch;
                    rtree::apply_visitor(*this, *(branches[current_branch - 1].second));

                    next_closest_node_distance = calc_closest_node_distance(internal_stack.begin(), internal_stack.end());
                }
            }
        }
    }

    friend bool operator==(nearest_query_incremental const& l, nearest_query_incremental const& r)
    {
        BOOST_ASSERT_MSG(l.current_neighbor != r.current_neighbor ||
                         l.current_neighbor == (std::numeric_limits<size_type>::max)() ||
                         l.neighbors[l.current_neighbor].second == r.neighbors[r.current_neighbor].second,
                         "not corresponding iterators");
        return l.current_neighbor == r.current_neighbor;
    }

    // Put node's elements into the list of active branches if those elements meets predicates
    // and distance predicates(currently not used)
    // and aren't further than found neighbours (if there is enough neighbours)
    inline void operator()(internal_node const& n)
    {
        typedef typename rtree::elements_type<internal_node>::type elements_type;
        elements_type const& elements = rtree::elements(n);

        internal_stack.push_back(std::make_pair(active_branch_list_type(), 0));

        // fill active branch list array of nodes meeting predicates
        for ( typename elements_type::const_iterator it = elements.begin() ; it != elements.end() ; ++it )
        {
            // if current node meets predicates
            // 0 - dummy value
            if ( index::detail::predicates_check<index::detail::bounds_tag, 0, predicates_len>(m_pred, 0, it->first) )
            {
                // calculate node's distance(s) for distance predicate
                node_distances_type node_dist_data = node_distances_calc::apply(dist_pred(), it->first);

                // if current node is further than found neighbors - don't analyze it
                if ( max_count() <= neighbors.size() &&
                     is_node_prunable(neighbors.back().first, node_dist_data) )
                {
                    continue;
                }

                // if current node distance(s) meets distance predicate
                if ( node_distances_predicates_check::apply(dist_pred(), node_dist_data) )
                {
                    // add current node's data into the list
                    internal_stack.back().first.push_back( std::make_pair(node_dist_data, it->second) );
                }
            }
        }

        if ( internal_stack.back().first.empty() )
            internal_stack.pop_back();
        else
            // sort array
            std::sort(internal_stack.back().first.begin(), internal_stack.back().first.end(), abl_less);
    }

    // Put values into the list of neighbours if those values meets predicates
    // and distance predicates(currently not used)
    // and aren't further than already found neighbours (if there is enough neighbours)
    inline void operator()(leaf const& n)
    {
        typedef typename rtree::elements_type<leaf>::type elements_type;
        elements_type const& elements = rtree::elements(n);

        // store distance to the furthest neighbour
        bool not_enough_neighbors = neighbors.size() < max_count();
        value_distance_type greatest_distance = !not_enough_neighbors ? neighbors.back().first : (std::numeric_limits<value_distance_type>::max)();
        
        // search leaf for closest value meeting predicates
        for ( typename elements_type::const_iterator it = elements.begin() ; it != elements.end() ; ++it)
        {
            // if value meets predicates
            if ( index::detail::predicates_check<index::detail::value_tag, 0, predicates_len>(m_pred, *it, m_translator(*it)) )
            {
                // calculate values distance for distance predicate
                value_distances_type distances = value_distances_calc::apply(dist_pred(), m_translator(*it));

                // if distance meets distance predicate
                if ( value_distances_predicates_check::apply(dist_pred(), distances) )
                {
                    typedef typename index::detail::point_relation<distance_predicates_type>::type point_relation_type;
                    typedef typename index::detail::relation<point_relation_type>::tag point_relation_tag;

                    // store value if it's closer than the furthest neighbour
                    value_distance_type dist = index::detail::cdist_value<value_distances_type>
                                                ::template get<point_relation_tag>(distances);

                    // if there is not enough values or current value is closer than furthest neighbour
                    if ( not_enough_neighbors || dist < greatest_distance )
                    {
                        neighbors.push_back(std::make_pair(dist, boost::addressof(*it)));
                    }
                }
            }
        }

        // sort array
        std::sort(neighbors.begin(), neighbors.end(), neighbors_less);
        // remove furthest values
        if ( max_count() < neighbors.size() )
            neighbors.resize(max_count());
    }

private:
    static inline bool abl_less(std::pair<node_distances_type, typename Allocators::node_pointer> const& p1,
                                std::pair<node_distances_type, typename Allocators::node_pointer> const& p2)
    {
        return index::detail::cdist_value<node_distances_type>::template get<index::detail::to_nearest_tag>(p1.first)
             < index::detail::cdist_value<node_distances_type>::template get<index::detail::to_nearest_tag>(p2.first);
    }

    static inline bool neighbors_less(std::pair<value_distance_type, const Value *> const& p1,
                                      std::pair<value_distance_type, const Value *> const& p2)
    {
        return p1.first < p2.first;
    }

    node_distance_type
    calc_closest_node_distance(typename internal_stack_type::const_iterator first,
                               typename internal_stack_type::const_iterator last)
    {
        node_distance_type result = (std::numeric_limits<node_distance_type>::max)();
        for ( ; first != last ; ++first )
        {
            if ( first->first.size() <= first->second )
                continue;

            node_distance_type curr_dist = index::detail::cdist_value<node_distances_type>
                                            ::template get<index::detail::to_nearest_tag>(first->first[first->second].first);
            if ( curr_dist < result )
                result = curr_dist;
        }
        return result;
    }

    template <typename Distance>
    static inline bool is_node_prunable(Distance const& greatest_dist, node_distances_type const& d)
    {
        return greatest_dist <= index::detail::cdist_value<node_distances_type>
                                    ::template get<index::detail::to_nearest_tag>(d);
    }

    inline distance_predicates_type const& dist_pred() const
    {
        return nearest_predicate_access::get(m_pred).distance_predicates;
    }

    inline unsigned max_count() const
    {
        return nearest_predicate_access::get(m_pred).count;
    }

    Translator const& m_translator;

    Predicates m_pred;

    internal_stack_type internal_stack;
    std::vector< std::pair<value_distance_type, const Value *> > neighbors;
    size_type current_neighbor;
    node_distance_type next_closest_node_distance;
};

} // namespace visitors

template <typename Value, typename Options, typename Translator, typename Box, typename Allocators, typename Predicates, unsigned NearestPredicateIndex>
class nearest_query_iterator
{
    typedef visitors::nearest_query_incremental<Value, Options, Translator, Box, Allocators, Predicates, NearestPredicateIndex> visitor_type;
    typedef typename visitor_type::node_pointer node_pointer;

    typedef typename Allocators::allocator_type::template rebind<Value>::other allocator_type;

public:
    typedef std::input_iterator_tag iterator_category;
    typedef const Value value_type;
    typedef Value const& reference;
    typedef typename allocator_type::difference_type difference_type;
    typedef typename allocator_type::const_pointer pointer;

    inline nearest_query_iterator(Translator const& t, Predicates const& p)
        : m_visitor(t, p)
    {}

    inline nearest_query_iterator(node_pointer root, Translator const& t, Predicates const& p)
        : m_visitor(t, p)
    {
        detail::rtree::apply_visitor(m_visitor, *root);
        m_visitor.increment();
    }

    reference operator*() const
    {
        return m_visitor.dereference();
    }

    const value_type * operator->() const
    {
        return boost::addressof(m_visitor.dereference());
    }

    nearest_query_iterator & operator++()
    {
        m_visitor.increment();
        return *this;
    }

    nearest_query_iterator operator++(int)
    {
        nearest_query_iterator temp = *this;
        this->operator++();
        return temp;
    }

    friend bool operator==(nearest_query_iterator const& l, nearest_query_iterator const& r)
    {
        return l.m_visitor == r.m_visitor;
    }

    friend bool operator!=(nearest_query_iterator const& l, nearest_query_iterator const& r)
    {
        return !(l.m_visitor == r.m_visitor);
    }

private:
    visitor_type m_visitor;
};

}} // namespace detail::rtree

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_INDEX_DETAIL_RTREE_VISITORS_NEAREST_QUERY_HPP
