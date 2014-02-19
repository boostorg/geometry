// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013, 2014.
// Modifications copyright (c) 2013-2014 Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_RELATE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_RELATE_HPP

#include <cstddef>

#include <boost/concept_check.hpp>
#include <boost/range.hpp>

#include <boost/geometry/algorithms/make.hpp>
#include <boost/geometry/algorithms/not_implemented.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/point_order.hpp>
#include <boost/geometry/core/ring_type.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>
#include <boost/geometry/strategies/concepts/within_concept.hpp>
#include <boost/geometry/strategies/default_strategy.hpp>
#include <boost/geometry/strategies/within.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/order_as_direction.hpp>
#include <boost/geometry/views/closeable_view.hpp>
#include <boost/geometry/views/reversible_view.hpp>

#include <boost/geometry/algorithms/detail/relate/point_geometry.hpp>
#include <boost/geometry/algorithms/detail/relate/linear_linear.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail_dispatch { namespace relate {

template <typename Geometry1,
          typename Geometry2,
          typename Tag1 = typename geometry::tag<Geometry1>::type,
          typename Tag2 = typename geometry::tag<Geometry2>::type>
struct relate : not_implemented<Tag1, Tag2>
{};

template <typename Point1, typename Point2>
struct relate<Point1, Point2, point_tag, point_tag>
    : detail::relate::point_point<Point1, Point2>
{};

//template <typename Point, typename Box>
//struct relate<Point, Box, point_tag, box_tag>
//    : detail::relate::point_box<Point, Box>
//{};
//
//template <typename Box, typename Point>
//struct relate<Box, Point, box_tag, point_tag>
//    : detail::relate::box_point<Box, Point>
//{};

template <typename Point, typename Geometry, typename Tag2>
struct relate<Point, Geometry, point_tag, Tag2>
    : detail::relate::point_geometry<Point, Geometry>
{};

template <typename Geometry, typename Point, typename Tag1>
struct relate<Geometry, Point, Tag1, point_tag>
    : detail::relate::geometry_point<Geometry, Point>
{};

template <typename Linestring1, typename Linestring2>
struct relate<Linestring1, Linestring2, linestring_tag, linestring_tag>
    : detail::relate::linear_linear<Linestring1, Linestring2>
{};

template <typename Linestring, typename MultiLinestring>
struct relate<Linestring, MultiLinestring, linestring_tag, multi_linestring_tag>
    : detail::relate::linear_linear<Linestring, MultiLinestring>
{};

template <typename MultiLinestring, typename Linestring>
struct relate<MultiLinestring, Linestring, multi_linestring_tag, linestring_tag>
    : detail::relate::linear_linear<MultiLinestring, Linestring>
{};

template <typename MultiLinestring1, typename MultiLinestring2>
struct relate<MultiLinestring1, MultiLinestring2, multi_linestring_tag, multi_linestring_tag>
    : detail::relate::linear_linear<MultiLinestring1, MultiLinestring2>
{};

}} // namespace detail_dispatch::relate

namespace detail { namespace relate {

template <typename Geometry1, typename Geometry2>
inline result relate(Geometry1 const& geometry1, Geometry2 const& geometry2)
{
    return detail_dispatch::relate::relate<Geometry1, Geometry2>::apply(geometry1, geometry2);
}

}} // namespace detail::relate
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_RELATE_HPP
