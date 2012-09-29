// Boost.Geometry Index
//
// R-tree options, algorithms, parameters
//
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_OPTIONS_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_OPTIONS_HPP

#include <limits>

namespace boost { namespace geometry { namespace index {

// InsertTag
struct insert_default_tag {};
struct insert_reinsert_tag {};

// ChooseNextNodeTag
struct choose_by_content_diff_tag {};
struct choose_by_overlap_diff_tag {};

// SplitTag
struct split_default_tag {};
//struct split_kmeans_tag {};

// RedistributeTag
struct linear_tag {};
struct quadratic_tag {};
struct rstar_tag {};

// NodeTag
struct node_d_mem_dynamic_tag {};
struct node_d_mem_static_tag {};
struct node_s_mem_dynamic_tag {};
struct node_s_mem_static_tag {};

// TODO: awulkiew - implement those:
//if ( m_min_elems_per_node < 1 )
//    m_min_elems_per_node = 1;
//if ( m_max_elems_per_node < 2 )
//    m_max_elems_per_node = 2;

/*!
Linear r-tree creation algorithm parameters.

\tparam MaxElements     Maximum number of elements in nodes.
\tparam MinElements     Minimum number of elements in nodes.
*/
template <size_t MaxElements, size_t MinElements>
struct linear
{
    static const size_t max_elements = MaxElements;
    static const size_t min_elements = MinElements;

    size_t get_max_elements() const { return max_elements; }
    size_t get_min_elements() const { return min_elements; }
};

/*!
Quadratic r-tree creation algorithm parameters.

\tparam MaxElements     Maximum number of elements in nodes.
\tparam MinElements     Minimum number of elements in nodes.
*/
template <size_t MaxElements, size_t MinElements>
struct quadratic
{
    static const size_t max_elements = MaxElements;
    static const size_t min_elements = MinElements;

    size_t get_max_elements() const { return max_elements; }
    size_t get_min_elements() const { return min_elements; }
};

namespace options { namespace detail { 

template <size_t MaxElements>
struct default_rstar_reinserted_elements
{
    static const size_t value = (MaxElements * 3) / 10;
};

}} // namespace options::detail

/*!
Quadratic r-tree creation algorithm parameters.

\tparam MaxElements             Maximum number of elements in nodes.
\tparam MinElements             Minimum number of elements in nodes.
\tparam OverlapCostThreshold    The number of leaf node children elements above which
                                nearly minimum overlap cost is calculated instead of minimum
                                overlap cost. If 0 minimum overlap cost is always calculated.
\tparam ReinsertedElements      Number of elements reinserted by forced reinsertions algorithm.
*/
template <size_t MaxElements,
          size_t MinElements,
          size_t OverlapCostThreshold = 0,
          size_t ReinsertedElements = options::detail::default_rstar_reinserted_elements<MaxElements>::value
          >
struct rstar
{
    static const size_t max_elements = MaxElements;
    static const size_t min_elements = MinElements;
    static const size_t overlap_cost_threshold = OverlapCostThreshold;
    static const size_t reinserted_elements = ReinsertedElements;

    size_t get_max_elements() const { return max_elements; }
    size_t get_min_elements() const { return min_elements; }
    size_t get_overlap_cost_threshold() const { return overlap_cost_threshold; }
    size_t get_reinserted_elements() const { return reinserted_elements; }
};

namespace runtime {

class linear
{
public:
    linear(size_t max_elements_, size_t min_elements_)
        : max_elements(max_elements_)
        , min_elements(min_elements_)
    {}

    size_t get_max_elements() const { return max_elements; }
    size_t get_min_elements() const { return min_elements; }

private:
    size_t max_elements;
    size_t min_elements;
};

class quadratic
{
public:
    quadratic(size_t max_elements_, size_t min_elements_)
        : max_elements(max_elements_)
        , min_elements(min_elements_)
    {}

    size_t get_max_elements() const { return max_elements; }
    size_t get_min_elements() const { return min_elements; }

private:
    size_t max_elements;
    size_t min_elements;
};

class rstar
{
public:
    static const size_t default_reinserted_elements = -1;

    rstar(size_t max_elements_,
          size_t min_elements_,
          size_t overlap_cost_threshold_ = 0,
          size_t reinserted_elements_ = default_reinserted_elements)
        : max_elements(max_elements_)
        , min_elements(min_elements_)
        , overlap_cost_threshold(overlap_cost_threshold_)
        , reinserted_elements(default_reinserted_elements == reinserted_elements_ ? (max_elements_ * 3) / 10 : reinserted_elements_)
    {}

    size_t get_max_elements() const { return max_elements; }
    size_t get_min_elements() const { return min_elements; }
    size_t get_overlap_cost_threshold() const { return overlap_cost_threshold; }
    size_t get_reinserted_elements() const { return reinserted_elements; }

private:
    size_t max_elements;
    size_t min_elements;
    size_t overlap_cost_threshold;
    size_t reinserted_elements;
};

}

namespace options {

template <typename Parameters, typename InsertTag, typename ChooseNextNodeTag, typename SplitTag, typename RedistributeTag, typename NodeTag>
struct rtree
{
    typedef Parameters parameters_type;
    typedef InsertTag insert_tag;
    typedef ChooseNextNodeTag choose_next_node_tag;
    typedef SplitTag split_tag;
    typedef RedistributeTag redistribute_tag;
    typedef NodeTag node_tag;
};

} // namespace options

//template <size_t MaxElements, size_t MinElements>
//struct kmeans
//{
//    static const size_t max_elements = MaxElements;
//    static const size_t min_elements = MinElements;
//};

namespace detail { namespace rtree {

template <typename Parameters>
struct options_type
{
    // TODO: awulkiew - use static assert
};

template <size_t MaxElements, size_t MinElements>
struct options_type< linear<MaxElements, MinElements> >
{
    typedef options::rtree<
        linear<MaxElements, MinElements>,
        insert_default_tag,
        choose_by_content_diff_tag,
        split_default_tag,
        linear_tag,
        node_d_mem_static_tag
    > type;
};

template <size_t MaxElements, size_t MinElements>
struct options_type< quadratic<MaxElements, MinElements> >
{
    typedef options::rtree<
        quadratic<MaxElements, MinElements>,
        insert_default_tag,
        choose_by_content_diff_tag,
        split_default_tag,
        quadratic_tag,
        node_d_mem_static_tag
    > type;
};

template <size_t MaxElements, size_t MinElements, size_t OverlapCostThreshold, size_t ReinsertedElements>
struct options_type< rstar<MaxElements, MinElements, OverlapCostThreshold, ReinsertedElements> >
{
    typedef options::rtree<
        rstar<MaxElements, MinElements, OverlapCostThreshold, ReinsertedElements>,
        insert_reinsert_tag,
        choose_by_overlap_diff_tag,
        split_default_tag,
        rstar_tag,
        node_d_mem_static_tag
    > type;
};

//template <size_t MaxElements, size_t MinElements>
//struct options_type< kmeans<MaxElements, MinElements> >
//{
//    typedef options::rtree<
//        kmeans<MaxElements, MinElements>,
//        insert_default_tag,
//        choose_by_content_diff_tag, // change it?
//        split_kmeans_tag,
//        int, // dummy tag - not used for now
//        node_d_mem_static_tag
//    > type;
//};

template <>
struct options_type< runtime::linear >
{
    typedef options::rtree<
        runtime::linear,
        insert_default_tag,
        choose_by_content_diff_tag,
        split_default_tag,
        linear_tag,
        node_d_mem_dynamic_tag
    > type;
};

template <>
struct options_type< runtime::quadratic >
{
    typedef options::rtree<
        runtime::quadratic,
        insert_default_tag,
        choose_by_content_diff_tag,
        split_default_tag,
        quadratic_tag,
        node_d_mem_dynamic_tag
    > type;
};

template <>
struct options_type< runtime::rstar >
{
    typedef options::rtree<
        runtime::rstar,
        insert_reinsert_tag,
        choose_by_overlap_diff_tag,
        split_default_tag,
        rstar_tag,
        node_d_mem_dynamic_tag
    > type;
};

}} // namespace detail::rtree

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_OPTIONS_HPP
