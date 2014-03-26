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

#include <boost/geometry/algorithms/detail/relate/result.hpp>

#include <boost/geometry/algorithms/detail/relate/point_point.hpp>
#include <boost/geometry/algorithms/detail/relate/point_geometry.hpp>
#include <boost/geometry/algorithms/detail/relate/linear_linear.hpp>
#include <boost/geometry/algorithms/detail/relate/linear_areal.hpp>

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

template <typename Point, typename MultiPoint>
struct relate<Point, MultiPoint, point_tag, multi_point_tag>
    : detail::relate::point_multipoint<Point, MultiPoint>
{};

template <typename MultiPoint, typename Point>
struct relate<MultiPoint, Point, multi_point_tag, point_tag>
    : detail::relate::multipoint_point<MultiPoint, Point>
{};

template <typename MultiPoint1, typename MultiPoint2>
struct relate<MultiPoint1, MultiPoint2, multi_point_tag, multi_point_tag>
    : detail::relate::multipoint_multipoint<MultiPoint1, MultiPoint2>
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

template <typename Linestring, typename Polygon>
struct relate<Linestring, Polygon, linestring_tag, polygon_tag>
    : detail::relate::linear_areal<Linestring, Polygon>
{};
template <typename Linestring, typename Ring>
struct relate<Linestring, Ring, linestring_tag, ring_tag>
    : detail::relate::linear_areal<Linestring, Ring>
{};

template <typename Polygon, typename Linestring>
struct relate<Polygon, Linestring, polygon_tag, linestring_tag>
    : detail::relate::areal_linear<Polygon, Linestring>
{};
template <typename Ring, typename Linestring>
struct relate<Ring, Linestring, ring_tag, linestring_tag>
    : detail::relate::areal_linear<Ring, Linestring>
{};

template <typename Linestring, typename MultiPolygon>
struct relate<Linestring, MultiPolygon, linestring_tag, multi_polygon_tag>
    : detail::relate::linear_areal<Linestring, MultiPolygon>
{};

template <typename MultiPolygon, typename Linestring>
struct relate<MultiPolygon, Linestring, multi_polygon_tag, linestring_tag>
    : detail::relate::areal_linear<MultiPolygon, Linestring>
{};

template <typename MultiLinestring, typename Polygon>
struct relate<MultiLinestring, Polygon, multi_linestring_tag, polygon_tag>
    : detail::relate::linear_areal<MultiLinestring, Polygon>
{};
template <typename MultiLinestring, typename Ring>
struct relate<MultiLinestring, Ring, multi_linestring_tag, ring_tag>
    : detail::relate::linear_areal<MultiLinestring, Ring>
{};

template <typename Polygon, typename MultiLinestring>
struct relate<Polygon, MultiLinestring, polygon_tag, multi_linestring_tag>
    : detail::relate::areal_linear<Polygon, MultiLinestring>
{};
template <typename Ring, typename MultiLinestring>
struct relate<Ring, MultiLinestring, ring_tag, multi_linestring_tag>
    : detail::relate::areal_linear<Ring, MultiLinestring>
{};

template <typename MultiLinestring, typename MultiPolygon>
struct relate<MultiLinestring, MultiPolygon, multi_linestring_tag, multi_polygon_tag>
    : detail::relate::linear_areal<MultiLinestring, MultiPolygon>
{};

template <typename MultiPolygon, typename MultiLinestring>
struct relate<MultiPolygon, MultiLinestring, multi_polygon_tag, multi_linestring_tag>
    : detail::relate::areal_linear<MultiPolygon, MultiLinestring>
{};

}} // namespace detail_dispatch::relate

namespace detail { namespace relate {

template <typename Geometry1, typename Geometry2>
struct interruption_enabled
{
    static const bool value =
        detail_dispatch::relate::relate<Geometry1, Geometry2>::interruption_enabled;
};

template <typename Geometry1, typename Geometry2, typename Result>
struct result_handler_type
    : not_implemented<Result>
{};

template <typename Geometry1, typename Geometry2>
struct result_handler_type<Geometry1, Geometry2, matrix9>
{
    typedef matrix_handler<matrix9> type;
};

template <typename Geometry1, typename Geometry2>
struct result_handler_type<Geometry1, Geometry2, mask9>
{
    typedef mask_handler
        <
            mask9,
            interruption_enabled
                <
                    Geometry1,
                    Geometry2
                >::value
        > type;
};

template <typename Geometry1, typename Geometry2, typename Head, typename Tail>
struct result_handler_type<Geometry1, Geometry2, boost::tuples::cons<Head, Tail> >
{
    typedef mask_handler
        <
            boost::tuples::cons<Head, Tail>,
            interruption_enabled
                <
                    Geometry1,
                    Geometry2
                >::value
        > type;
};

template <typename Geometry1, typename Geometry2,
          char II, char IB, char IE,
          char BI, char BB, char BE,
          char EI, char EB, char EE>
struct result_handler_type<Geometry1, Geometry2, static_mask<II, IB, IE, BI, BB, BE, EI, EB, EE> >
{
    typedef static_mask_handler
        <
            static_mask<II, IB, IE, BI, BB, BE, EI, EB, EE>,
            interruption_enabled
                <
                    Geometry1,
                    Geometry2
                >::value
        > type;
};

template <typename MatrixOrMask, typename Geometry1, typename Geometry2>
inline
typename result_handler_type
    <
        Geometry1,
        Geometry2,
        MatrixOrMask
    >::type::result_type
relate(Geometry1 const& geometry1,
       Geometry2 const& geometry2,
       MatrixOrMask const& matrix_or_mask = MatrixOrMask())
{
    typedef typename result_handler_type
        <
            Geometry1,
            Geometry2,
            MatrixOrMask
        >::type handler_type;

    handler_type handler(matrix_or_mask);
    detail_dispatch::relate::relate<Geometry1, Geometry2>::apply(geometry1, geometry2, handler);
    return handler.result();
}

}} // namespace detail::relate
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_RELATE_HPP
