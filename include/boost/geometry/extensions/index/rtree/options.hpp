// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - rtree variants options
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_OPTIONS_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_OPTIONS_HPP

namespace boost { namespace geometry { namespace index {

// InsertTag
struct insert_tag {};
struct reinsert_tag {};

// ChooseNextNodeTag
struct choose_by_area_diff_tag {};
struct choose_by_overlap_diff_tag {};

// RedistributeTag
struct linear_tag {};
struct quadratic_tag {};
struct rstar_tag {};

// NodeTag
struct default_tag {};

namespace detail { namespace rtree {

template <typename InsertTag, typename ChooseNextNodeTag, typename RedistributeTag, typename NodeTag>
struct options
{
	typedef InsertTag insert_tag;
	typedef ChooseNextNodeTag choose_next_node_tag;
	typedef RedistributeTag redistribute_tag;
	typedef NodeTag node_tag;
};

template <typename Tag>
struct options_type
{
	typedef void type;
};

template <typename InsertTag, typename ChooseNextNodeTag, typename RedistributeTag, typename NodeTag>
struct options_type< options<InsertTag, ChooseNextNodeTag, RedistributeTag, NodeTag> >
{
	typedef options<InsertTag, ChooseNextNodeTag, RedistributeTag, NodeTag> type;
};

template <>
struct options_type<linear_tag>
{
	typedef options<insert_tag, choose_by_area_diff_tag, linear_tag, default_tag> type;
};

template <>
struct options_type<quadratic_tag>
{
	typedef options<insert_tag, choose_by_area_diff_tag, quadratic_tag, default_tag> type;
};

template <>
struct options_type<rstar_tag>
{
	typedef options<reinsert_tag, choose_by_overlap_diff_tag, rstar_tag, default_tag> type;
};

}} // namespace detail::rtree

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_OPTIONS_HPP
