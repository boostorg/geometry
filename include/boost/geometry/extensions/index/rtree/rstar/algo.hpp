// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - rtree variants algorithms
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_ALGO_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_ALGO_HPP

namespace boost { namespace geometry { namespace index {

struct with_reinsert_tag {};
struct find_minimum_cost_tag {};
struct rstar_tag {};

namespace detail { namespace rtree {

template <>
struct algo_type<rstar_tag>
{
	typedef algo<with_reinsert_tag, find_minimum_cost_tag, rstar_tag, default_tag> type;
};

}} // namespace detail::rtree

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_ALGO_HPP
