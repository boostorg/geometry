// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.
// Copyright (c) 2024 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2020-2025.
// Modifications copyright (c) 2020-2025 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_CORE_POINT_TYPE_HPP
#define BOOST_GEOMETRY_CORE_POINT_TYPE_HPP


#include <boost/range/value_type.hpp>

#include <boost/geometry/core/geometry_types.hpp>
#include <boost/geometry/core/ring_type.hpp>
#include <boost/geometry/core/static_assert.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/util/sequence.hpp>
#include <boost/geometry/util/type_traits_std.hpp>


namespace boost { namespace geometry
{

namespace traits
{

/*!
\brief Traits class indicating the type of contained points
\ingroup traits
\par Geometries:
    - all geometries except point
\par Specializations should provide:
    - typedef P type (where P should fulfil the Point concept)
\tparam Geometry geometry
*/
template <typename Geometry>
struct point_type
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE(
        "Not implemented for this Geometry type.",
        Geometry);
};


} // namespace traits


#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{

template <typename Tag, typename Geometry>
struct point_type
{
    // Default: call traits to get point type
    using type = std::remove_const_t
        <
            typename traits::point_type<Geometry>::type
        >;
};


// Specialization for point: the point itself
template <typename Point>
struct point_type<point_tag, Point>
{
    using type = Point;
};


// Specializations for linestring/ring, via boost::range
template <typename Linestring>
struct point_type<linestring_tag, Linestring>
{
    using type = typename boost::range_value<Linestring>::type;
};


template <typename Ring>
struct point_type<ring_tag, Ring>
{
    using type = typename boost::range_value<Ring>::type;
};

// Specialization for PolyhedralSurface: the point-type is the point-type of its polygon type
template <typename PolyhedralSurface>
struct point_type<polyhedral_surface_tag, PolyhedralSurface>
{
    using type = typename point_type
        <
            polygon_tag,
            typename boost::range_value<PolyhedralSurface>::type
        >::type;
};

// Specialization for polygon: the point-type is the point-type of its rings
template <typename Polygon>
struct point_type<polygon_tag, Polygon>
{
    using type = typename point_type
        <
            ring_tag,
            typename ring_type<polygon_tag, Polygon>::type
        >::type;
};


template <typename MultiPoint>
struct point_type<multi_point_tag, MultiPoint>
{
    using type = typename boost::range_value
        <
            MultiPoint
        >::type;
};


template <typename MultiLinestring>
struct point_type<multi_linestring_tag, MultiLinestring>
{
    using type = typename point_type
        <
            linestring_tag,
            typename boost::range_value<MultiLinestring>::type
        >::type;
};


template <typename MultiPolygon>
struct point_type<multi_polygon_tag, MultiPolygon>
{
    using type = typename point_type
        <
            polygon_tag,
            typename boost::range_value<MultiPolygon>::type
        >::type;
};


template <typename DynamicGeometry>
struct point_type<dynamic_geometry_tag, DynamicGeometry>
{
    using geometry_t = typename util::sequence_front
        <
            typename traits::geometry_types<DynamicGeometry>::type
        >::type;
    using type = typename point_type
        <
            tag_t<geometry_t>,
            util::remove_cptrref_t<geometry_t>
        >::type;
};


template <typename GeometryCollection>
struct point_type<geometry_collection_tag, GeometryCollection>
{
    using geometry_t = typename util::sequence_front
        <
            typename traits::geometry_types<GeometryCollection>::type
        >::type;
    using type = typename point_type
        <
            tag_t<geometry_t>,
            util::remove_cptrref_t<geometry_t>
        >::type;
};


} // namespace core_dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
\brief \brief_meta{type, point_type, \meta_geometry_type}
\tparam Geometry \tparam_geometry
\ingroup core

\qbk{[include reference/core/point_type.qbk]}
*/
template <typename Geometry>
struct point_type
{
    using type = typename core_dispatch::point_type
        <
            tag_t<Geometry>,
            util::remove_cptrref_t<Geometry>
        >::type;
};


template <typename Geometry>
using point_type_t = typename point_type<Geometry>::type;


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_CORE_POINT_TYPE_HPP
