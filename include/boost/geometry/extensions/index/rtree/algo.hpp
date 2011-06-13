// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - rtree variants algorithms
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_ALGO_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_ALGO_HPP

namespace boost { namespace geometry { namespace index {

struct default_tag {};
struct analyze_area_tag {};

namespace detail { namespace rtree {

template <typename InsertTag, typename ChooseNextNodeTag, typename RedistributeTag, typename NodeTag>
struct algo
{
	typedef InsertTag insert_tag;
	typedef ChooseNextNodeTag choose_next_node_tag;
	typedef RedistributeTag redistribute_tag;
	typedef NodeTag node_tag;
};

template <typename Tag>
struct algo_type
{
	typedef void type;
};

template <typename InsertTag, typename ChooseNextNodeTag, typename RedistributeTag, typename NodeTag>
struct algo_type< algo<InsertTag, ChooseNextNodeTag, RedistributeTag, NodeTag> >
{
	typedef algo<InsertTag, ChooseNextNodeTag, RedistributeTag, NodeTag> type;
};

}} // namespace detail::rtree

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_ALGO_HPP
