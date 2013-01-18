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

#include <boost/geometry/extensions/index/parameters.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree {

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

template <typename Parameters, typename InsertTag, typename ChooseNextNodeTag, typename SplitTag, typename RedistributeTag, typename NodeTag>
struct options
{
    typedef Parameters parameters_type;
    typedef InsertTag insert_tag;
    typedef ChooseNextNodeTag choose_next_node_tag;
    typedef SplitTag split_tag;
    typedef RedistributeTag redistribute_tag;
    typedef NodeTag node_tag;
};

template <typename Parameters>
struct options_type
{
    // TODO: awulkiew - use static assert
};

template <size_t MaxElements, size_t MinElements>
struct options_type< linear<MaxElements, MinElements> >
{
    typedef options<
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
    typedef options<
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
    typedef options<
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
//    typedef options<
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
    typedef options<
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
    typedef options<
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
    typedef options<
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
