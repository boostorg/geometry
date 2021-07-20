// Boost.Geometry Index
//
// R-tree distance (knn, path, etc. ) query visitor implementation
//
// Copyright (c) 2011-2014 Adam Wulkiewicz, Lodz, Poland.
//
// This file was modified by Oracle on 2019-2021.
// Modifications copyright (c) 2019-2021 Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_INDEX_DETAIL_RTREE_VISITORS_DISTANCE_QUERY_HPP
#define BOOST_GEOMETRY_INDEX_DETAIL_RTREE_VISITORS_DISTANCE_QUERY_HPP

#include <queue>

#include <boost/geometry/index/detail/priority_dequeue.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {


struct pair_first_less
{
    template <typename First, typename Second>
    inline bool operator()(std::pair<First, Second> const& p1,
                           std::pair<First, Second> const& p2) const
    {
        return p1.first < p2.first;
    }
};

struct pair_first_greater
{
    template <typename First, typename Second>
    inline bool operator()(std::pair<First, Second> const& p1,
                           std::pair<First, Second> const& p2) const
    {
        return p1.first > p2.first;
    }
};

template <typename T, typename Comp>
struct priority_queue : std::priority_queue<T, std::vector<T>, Comp>
{
    priority_queue() = default;
    void clear()
    {
        this->c.clear();
    }
};


template <typename Value, typename Translator, typename DistanceType, typename OutIt>
class distance_query_result
{
public:
    typedef DistanceType distance_type;

    inline explicit distance_query_result(size_t k, OutIt out_it)
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
                std::make_heap(m_neighbors.begin(), m_neighbors.end(), pair_first_less());
        }
        else
        {
            if ( curr_comp_dist < m_neighbors.front().first )
            {
                std::pop_heap(m_neighbors.begin(), m_neighbors.end(), pair_first_less());
                m_neighbors.back().first = curr_comp_dist;
                m_neighbors.back().second = val;
                std::push_heap(m_neighbors.begin(), m_neighbors.end(), pair_first_less());
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
    size_t m_count;
    OutIt m_out_it;

    std::vector< std::pair<distance_type, Value> > m_neighbors;
};

template
<
    typename MembersHolder,
    typename Predicates,
    std::size_t DistancePredicateIndex,
    typename OutIter
>
class distance_query
    : public MembersHolder::visitor_const
{
public:
    typedef typename MembersHolder::value_type value_type;
    typedef typename MembersHolder::box_type box_type;
    typedef typename MembersHolder::parameters_type parameters_type;
    typedef typename MembersHolder::translator_type translator_type;
    typedef typename MembersHolder::allocators_type allocators_type;

    typedef typename index::detail::strategy_type<parameters_type>::type strategy_type;

    typedef typename MembersHolder::node node;
    typedef typename MembersHolder::internal_node internal_node;
    typedef typename MembersHolder::leaf leaf;

    typedef index::detail::predicates_element<DistancePredicateIndex, Predicates> nearest_predicate_access;
    typedef typename nearest_predicate_access::type nearest_predicate_type;
    typedef typename indexable_type<translator_type>::type indexable_type;

    typedef index::detail::calculate_distance<nearest_predicate_type, indexable_type, strategy_type, value_tag> calculate_value_distance;
    typedef index::detail::calculate_distance<nearest_predicate_type, box_type, strategy_type, bounds_tag> calculate_node_distance;
    typedef typename calculate_value_distance::result_type value_distance_type;
    typedef typename calculate_node_distance::result_type node_distance_type;

    inline distance_query(parameters_type const& parameters, translator_type const& translator, Predicates const& pred, OutIter out_it)
        : m_parameters(parameters), m_translator(translator)
        , m_pred(pred)
        , m_result(nearest_predicate_access::get(m_pred).count, out_it)
        , m_strategy(index::detail::get_strategy(parameters))
    {}

    inline void operator()(internal_node const& n)
    {
        namespace id = index::detail;

        // array of active nodes
        using active_branch_list_type = typename index::detail::rtree::container_from_elements_type
            <
                typename rtree::elements_type<internal_node>::type,
                std::pair<node_distance_type, typename allocators_type::node_pointer>
            >::type;

        active_branch_list_type active_branch_list;
        active_branch_list.reserve(m_parameters.get_max_elements());
        
        auto const& elements = rtree::elements(n);

        // fill array of nodes meeting predicates
        for (auto it = elements.begin(); it != elements.end(); ++it)
        {
            // if current node meets predicates
            // 0 - dummy value
            if (id::predicates_check<id::bounds_tag>(m_pred, 0, it->first, m_strategy) )
            {
                // calculate node's distance(s) for distance predicate
                node_distance_type node_distance;
                // if distance isn't ok - move to the next node
                if ( !calculate_node_distance::apply(predicate(), it->first,
                                                     m_strategy, node_distance) )
                {
                    continue;
                }

                // if current node is further than found neighbors - don't analyze it
                if ( m_result.has_enough_neighbors() &&
                     is_node_prunable(m_result.greatest_comparable_distance(), node_distance) )
                {
                    continue;
                }

                // add current node's data into the list
                active_branch_list.push_back( std::make_pair(node_distance, it->second) );
            }
        }

        // if there aren't any nodes in ABL - return
        if ( active_branch_list.empty() )
            return;
        
        // sort array
        std::sort(active_branch_list.begin(), active_branch_list.end(), pair_first_less());

        // recursively visit nodes
        for (auto it = active_branch_list.begin(); it != active_branch_list.end() ; ++it)
        {
            // if current node is further than furthest neighbor, the rest of nodes also will be further
            if ( m_result.has_enough_neighbors() &&
                 is_node_prunable(m_result.greatest_comparable_distance(), it->first) )
                break;

            rtree::apply_visitor(*this, *(it->second));
        }

        // ALTERNATIVE VERSION - use heap instead of sorted container
        // It seems to be faster for greater MaxElements and slower otherwise
        // CONSIDER: using one global container/heap for active branches
        //           instead of a sorted container per level
        //           This would also change the way how branches are traversed!
        //           The same may be applied to the iterative version which btw suffers
        //           from the copying of the whole containers on resize of the ABLs container

        //// make a heap
        //std::make_heap(active_branch_list.begin(), active_branch_list.end(), pair_first_greater());

        //// recursively visit nodes
        //while ( !active_branch_list.empty() )
        //{
        //    //if current node is further than furthest neighbor, the rest of nodes also will be further
        //    if ( m_result.has_enough_neighbors()
        //      && is_node_prunable(m_result.greatest_comparable_distance(), active_branch_list.front().first) )
        //    {
        //        break;
        //    }

        //    rtree::apply_visitor(*this, *(active_branch_list.front().second));

        //    std::pop_heap(active_branch_list.begin(), active_branch_list.end(), pair_first_greater());
        //    active_branch_list.pop_back();
        //}
    }

    inline void operator()(leaf const& n)
    {
        namespace id = index::detail;

        auto const& elements = rtree::elements(n);
        
        // search leaf for closest value meeting predicates
        for (auto it = elements.begin(); it != elements.end(); ++it)
        {
            // if value meets predicates
            if (id::predicates_check<id::value_tag>(m_pred, *it, m_translator(*it), m_strategy))
            {
                // calculate values distance for distance predicate
                value_distance_type value_distance;
                // if distance is ok
                if ( calculate_value_distance::apply(predicate(), m_translator(*it),
                                                     m_strategy, value_distance) )
                {
                    // store value
                    m_result.store(*it, value_distance);
                }
            }
        }
    }

    inline size_t finish()
    {
        return m_result.finish();
    }

private:
    template <typename Distance>
    static inline bool is_node_prunable(Distance const& greatest_dist, node_distance_type const& d)
    {
        return greatest_dist <= d;
    }

    nearest_predicate_type const& predicate() const
    {
        return nearest_predicate_access::get(m_pred);
    }

    parameters_type const& m_parameters;
    translator_type const& m_translator;

    Predicates m_pred;
    distance_query_result<value_type, translator_type, value_distance_type, OutIter> m_result;

    strategy_type m_strategy;
};

template <
    typename MembersHolder,
    typename Predicates,
    std::size_t DistancePredicateIndex
>
class distance_query_incremental
    : public MembersHolder::visitor_const
{
public:
    typedef typename MembersHolder::value_type value_type;
    typedef typename MembersHolder::box_type box_type;
    typedef typename MembersHolder::parameters_type parameters_type;
    typedef typename MembersHolder::translator_type translator_type;
    typedef typename MembersHolder::allocators_type allocators_type;

    typedef typename index::detail::strategy_type<parameters_type>::type strategy_type;

    typedef typename MembersHolder::node node;
    typedef typename MembersHolder::internal_node internal_node;
    typedef typename MembersHolder::leaf leaf;

    typedef index::detail::predicates_element<DistancePredicateIndex, Predicates> nearest_predicate_access;
    typedef typename nearest_predicate_access::type nearest_predicate_type;
    typedef typename indexable_type<translator_type>::type indexable_type;
    
    typedef index::detail::calculate_distance<nearest_predicate_type, indexable_type, strategy_type, value_tag> calculate_value_distance;
    typedef index::detail::calculate_distance<nearest_predicate_type, box_type, strategy_type, bounds_tag> calculate_node_distance;
    typedef typename calculate_value_distance::result_type value_distance_type;
    typedef typename calculate_node_distance::result_type node_distance_type;

    typedef typename allocators_type::size_type size_type;
    typedef typename allocators_type::const_reference const_reference;
    typedef typename allocators_type::node_pointer node_pointer;

    typedef typename rtree::elements_type<internal_node>::type internal_elements;
    typedef typename internal_elements::const_iterator internal_iterator;
    typedef typename rtree::elements_type<leaf>::type leaf_elements;

    using branch_data = std::pair<node_distance_type, node_pointer>;
    using branches_type = priority_queue<branch_data, pair_first_greater>;
    using neighbor_data = std::pair<value_distance_type, const value_type *>;
    using neighbors_type = priority_dequeue<neighbor_data, std::vector<neighbor_data>, pair_first_greater>;

    inline distance_query_incremental()
        : m_translator(nullptr)
//        , m_pred()
        , m_neighbors_count(0)
        , m_neighbor_ptr(nullptr)
//        , m_strategy_type()
    {}

    inline distance_query_incremental(parameters_type const& params, translator_type const& translator, Predicates const& pred)
        : m_translator(::boost::addressof(translator))
        , m_pred(pred)
        , m_neighbors_count(0)
        , m_neighbor_ptr(nullptr)
        , m_strategy(index::detail::get_strategy(params))
    {}

    const_reference dereference() const
    {
        return *m_neighbor_ptr;
    }

    void initialize(node_pointer root)
    {
        if (0 < max_count())
        {
            rtree::apply_visitor(*this, *root);
            increment();
        }
    }

    void increment()
    {
        for (;;)
        {
            if (m_branches.empty())
            {
                // there exists a next closest neighbor so we can increment
                if (! m_neighbors.empty())
                {
                    m_neighbor_ptr = m_neighbors.top().second;
                    ++m_neighbors_count;
                    m_neighbors.pop_top();
                }
                else
                {
                    // there aren't any neighbors left, end
                    m_neighbor_ptr = nullptr;
                    m_neighbors_count = max_count();
                }

                return;
            }
            else
            {
                branch_data const& closest_branch = m_branches.top();

                // if next neighbor is closer or as close as the closest branch, set next neighbor
                if (! m_neighbors.empty() && m_neighbors.top().first <= closest_branch.first )
                {
                    m_neighbor_ptr = m_neighbors.top().second;
                    ++m_neighbors_count;
                    m_neighbors.pop_top();
                    return;
                }

                BOOST_GEOMETRY_INDEX_ASSERT(m_neighbors_count + m_neighbors.size() <= max_count(), "unexpected neighbors count");

                // if there is enough neighbors and there is no closer branch
                if (m_neighbors_count + m_neighbors.size() == max_count()
                    && (m_neighbors.empty() || m_neighbors.bottom().first <= closest_branch.first))
                {
                    m_branches.clear();
                    continue;
                }
                else
                {
                    node_pointer ptr = closest_branch.second;
                    m_branches.pop();

                    rtree::apply_visitor(*this, *ptr);
                }
            }
        }
    }

    bool is_end() const
    {
        return m_neighbor_ptr == nullptr;
    }

    friend bool operator==(distance_query_incremental const& l, distance_query_incremental const& r)
    {
        return l.m_neighbors_count == r.m_neighbors_count;
    }

    // Put node's elements into the list of active branches if those elements meets predicates
    // and distance predicates(currently not used)
    // and aren't further than found neighbours (if there is enough neighbours)
    inline void operator()(internal_node const& n)
    {
        namespace id = index::detail;

        auto const& elements = rtree::elements(n);

        // fill active branch list array of nodes meeting predicates
        for (auto it = elements.begin() ; it != elements.end() ; ++it)
        {
            // if current node doesn't meet predicates
            // 0 - dummy value
            if (! id::predicates_check<id::bounds_tag>(m_pred, 0, it->first, m_strategy))
            {
                continue;
            }

            // calculate node's distance(s) for distance predicate
            node_distance_type node_distance;
            // if distance isn't ok
            if (! calculate_node_distance::apply(predicate(), it->first, m_strategy, node_distance))
            {
                continue;
            }

            // if there is enough neighbors and there is no closer branch
            if (m_neighbors_count + m_neighbors.size() == max_count()
                && (m_neighbors.empty() || m_neighbors.bottom().first <= node_distance))
            {
                continue;
            }

            // add current node into the queue
            m_branches.push(std::make_pair(node_distance, it->second));
        }
    }

    // Put values into the list of neighbours if those values meets predicates
    // and distance predicates(currently not used)
    // and aren't further than already found neighbours (if there is enough neighbours)
    inline void operator()(leaf const& n)
    {
        namespace id = index::detail;

        auto const& elements = rtree::elements(n);

        // search leaf for closest value meeting predicates
        for (auto it = elements.begin() ; it != elements.end() ; ++it)
        {
            // if value doesn't meet predicates
            if (! id::predicates_check<id::value_tag>(m_pred, *it, (*m_translator)(*it), m_strategy))
            {
                continue;
            }
                
            // calculate values distance for distance predicate
            value_distance_type value_distance;
            // if distance isn't ok
            if (! calculate_value_distance::apply(predicate(), (*m_translator)(*it),
                                                  m_strategy, value_distance))
            {
                continue;
            }

            // if there is enough neighbors and there is no closer neighbor
            if (m_neighbors_count + m_neighbors.size() == max_count()
                && (m_neighbors.empty() || m_neighbors.bottom().first <= value_distance))
            {
                continue;
            }

            // add current value into the queue
            m_neighbors.push(std::make_pair(value_distance, boost::addressof(*it)));
            if (m_neighbors_count + m_neighbors.size() > max_count())
            {
                m_neighbors.pop_bottom();
            }
        }
    }

private:
    inline std::size_t max_count() const
    {
        return nearest_predicate_access::get(m_pred).count;
    }

    nearest_predicate_type const& predicate() const
    {
        return nearest_predicate_access::get(m_pred);
    }

    const translator_type * m_translator;

    Predicates m_pred;
    
    branches_type m_branches;
    neighbors_type m_neighbors;
    size_type m_neighbors_count;
    const value_type * m_neighbor_ptr;

    strategy_type m_strategy;
};

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_INDEX_DETAIL_RTREE_VISITORS_DISTANCE_QUERY_HPP
