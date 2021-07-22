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

#include <boost/geometry/index/detail/predicates.hpp>
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
struct priority_dequeue : index::detail::priority_dequeue<T, std::vector<T>, Comp>
{
    priority_dequeue() = default;
    //void reserve(typename std::vector<T>::size_type n)
    //{
    //    this->c.reserve(n);
    //}
    //void clear()
    //{
    //    this->c.clear();
    //}
};

template <typename T, typename Comp>
struct priority_queue : std::priority_queue<T, std::vector<T>, Comp>
{
    priority_queue() = default;
    //void reserve(typename std::vector<T>::size_type n)
    //{
    //    this->c.reserve(n);
    //}
    void clear()
    {
        this->c.clear();
    }
};

template <typename DistanceType, typename Value>
class distance_query_result
{
    using neighbor_data = std::pair<DistanceType, const Value *>;
    using neighbors_type = std::vector<neighbor_data>;
    using size_type = typename neighbors_type::size_type;

public:
    inline distance_query_result(size_type k)
        : m_count(k)
    {
        m_neighbors.reserve(m_count);
    }

    // NOTE: Do not call if max_count() == 0
    inline void store(DistanceType const& distance, const Value * value_ptr)
    {
        if (m_neighbors.size() < m_count)
        {
            m_neighbors.push_back(std::make_pair(distance, value_ptr));

            if (m_neighbors.size() == m_count)
            {
                std::make_heap(m_neighbors.begin(), m_neighbors.end(), pair_first_less());
            }
        }
        else if (distance < m_neighbors.front().first)
        {
            std::pop_heap(m_neighbors.begin(), m_neighbors.end(), pair_first_less());
            m_neighbors.back().first = distance;
            m_neighbors.back().second = value_ptr;
            std::push_heap(m_neighbors.begin(), m_neighbors.end(), pair_first_less());
        }
    }

    // NOTE: Do not call if max_count() == 0
    inline bool ignore_branch(DistanceType const& distance) const
    {
        return m_neighbors.size() == m_count
            && m_neighbors.front().first <= distance;
    }

    template <typename OutIt>
    inline size_type finish(OutIt out_it) const
    {
        for (auto const& p : m_neighbors)
        {
            *out_it = *(p.second);
            ++out_it;
        }

        return m_neighbors.size();
    }

    size_type max_count() const
    {
        return m_count;
    }

private:
    size_type m_count;
    neighbors_type m_neighbors;
};

template <typename MembersHolder, typename Predicates>
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

    typedef index::detail::predicates_element
        <
            index::detail::predicates_find_distance<Predicates>::value, Predicates
        > nearest_predicate_access;
    typedef typename nearest_predicate_access::type nearest_predicate_type;
    typedef typename indexable_type<translator_type>::type indexable_type;

    typedef index::detail::calculate_distance<nearest_predicate_type, indexable_type, strategy_type, value_tag> calculate_value_distance;
    typedef index::detail::calculate_distance<nearest_predicate_type, box_type, strategy_type, bounds_tag> calculate_node_distance;
    typedef typename calculate_value_distance::result_type value_distance_type;
    typedef typename calculate_node_distance::result_type node_distance_type;

    typedef typename allocators_type::size_type size_type;
    typedef typename allocators_type::node_pointer node_pointer;

    struct branch_data
    {
        branch_data(node_distance_type d, size_type l, node_pointer p)
            : distance(d), level(l), ptr(p)
        {}

        node_distance_type distance;
        size_type level;
        node_pointer ptr;
    };

    struct branch_data_comp
    {
        bool operator()(branch_data const& b1, branch_data const& b2) const
        {
            return b1.distance > b2.distance || (b1.distance == b2.distance && b1.level < b2.level);
        }
    };

    inline distance_query(parameters_type const& parameters, translator_type const& translator, Predicates const& pred)
        : m_tr(translator)
        , m_pred(pred)
        , m_result(nearest_predicate_access::get(m_pred).count)        
        , m_strategy(index::detail::get_strategy(parameters))
    {
        //m_branches.reserve(parameters.get_max_elements() * levels_count); ?
    }

    template <typename OutIter>
    size_type apply(node_pointer root, OutIter out_it)
    {
        if (m_result.max_count() <= 0)
        {
            return 0;
        }

        m_level = 1;
        rtree::apply_visitor(*this, *root);

        for (;;)
        {
            if (m_branches.empty()
                || m_result.ignore_branch(m_branches.top().distance))
            {
                break;
            }

            node_pointer ptr = m_branches.top().ptr;
            m_level = m_branches.top().level + 1;
            m_branches.pop();
            rtree::apply_visitor(*this, *ptr);
        }

        return m_result.finish(out_it);
    }

    inline void operator()(internal_node const& n)
    {
        namespace id = index::detail;

        // fill array of nodes meeting predicates
        for (auto const& p : rtree::elements(n))
        {
            node_distance_type node_distance; // for distance predicate

            // if current node meets predicates (0 is dummy value)
            if (id::predicates_check<id::bounds_tag>(m_pred, 0, p.first, m_strategy)
                // and if distance is ok
                && calculate_node_distance::apply(predicate(), p.first, m_strategy, node_distance)
                // and if current node is closer than the furthest neighbor
                && ! m_result.ignore_branch(node_distance))
            {
                // add current node's data into the list
                m_branches.push(branch_data(node_distance, m_level, p.second));
            }
        }
    }

    inline void operator()(leaf const& n)
    {
        namespace id = index::detail;

        // search leaf for closest value meeting predicates
        for (auto const& v : rtree::elements(n))
        {
            value_distance_type value_distance; // for distance predicate

            // if value meets predicates
            if (id::predicates_check<id::value_tag>(m_pred, v, m_tr(v), m_strategy)
                // and if distance is ok
                && calculate_value_distance::apply(predicate(), m_tr(v), m_strategy, value_distance))
            {
                // store value
                m_result.store(value_distance, boost::addressof(v));
            }
        }
    }

private:
    nearest_predicate_type const& predicate() const
    {
        return nearest_predicate_access::get(m_pred);
    }

    translator_type const& m_tr;

    Predicates m_pred;

    distance_query_result<value_distance_type, value_type> m_result;
    priority_queue<branch_data, branch_data_comp> m_branches;
    size_type m_level;

    strategy_type m_strategy;
};

template <typename MembersHolder, typename Predicates>
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

    typedef index::detail::predicates_element
        <
            index::detail::predicates_find_distance<Predicates>::value, Predicates
        > nearest_predicate_access;
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
    using neighbors_type = priority_dequeue<neighbor_data, pair_first_greater>;

    inline distance_query_incremental()
        : m_tr(nullptr)
//        , m_pred()
        , m_neighbors_count(0)
        , m_neighbor_ptr(nullptr)
//        , m_strategy_type()
    {}

    inline distance_query_incremental(parameters_type const& params, translator_type const& translator, Predicates const& pred)
        : m_tr(::boost::addressof(translator))
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
                if (ignore_branch_or_value(closest_branch.first))
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

        // fill active branch list array of nodes meeting predicates
        for (auto const& p : rtree::elements(n))
        {
            node_distance_type node_distance; // for distance predicate

            // if current node meets predicates (0 is dummy value)
            if (id::predicates_check<id::bounds_tag>(m_pred, 0, p.first, m_strategy)
                // and if distance is ok
                && calculate_node_distance::apply(predicate(), p.first, m_strategy, node_distance)
                // and if current node is closer than the furthest neighbor
                && ! ignore_branch_or_value(node_distance))
            {
                // add current node into the queue
                m_branches.push(std::make_pair(node_distance, p.second));
            }
        }
    }

    // Put values into the list of neighbours if those values meets predicates
    // and distance predicates(currently not used)
    // and aren't further than already found neighbours (if there is enough neighbours)
    inline void operator()(leaf const& n)
    {
        namespace id = index::detail;

        // search leaf for closest value meeting predicates
        for (auto const& v : rtree::elements(n))
        {
            value_distance_type value_distance; // for distance predicate

            // if value meets predicates
            if (id::predicates_check<id::value_tag>(m_pred, v, (*m_tr)(v), m_strategy)
                // and if distance is ok
                && calculate_value_distance::apply(predicate(), (*m_tr)(v), m_strategy, value_distance)
                // and if current value is closer than the furthest neighbor
                && ! ignore_branch_or_value(value_distance))
            {
                // add current value into the queue
                m_neighbors.push(std::make_pair(value_distance, boost::addressof(v)));

                // remove unneeded value
                if (m_neighbors_count + m_neighbors.size() > max_count())
                {
                    m_neighbors.pop_bottom();
                }
            }
        }
    }

private:
    template <typename Distance>
    inline bool ignore_branch_or_value(Distance const& distance)
    {
        return m_neighbors_count + m_neighbors.size() == max_count()
            && (m_neighbors.empty() || m_neighbors.bottom().first <= distance);
    }

    inline std::size_t max_count() const
    {
        return nearest_predicate_access::get(m_pred).count;
    }

    nearest_predicate_type const& predicate() const
    {
        return nearest_predicate_access::get(m_pred);
    }

    const translator_type * m_tr;

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
