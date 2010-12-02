// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_CARTESIAN3D_HPP
#define BOOST_GEOMETRY_GEOMETRIES_CARTESIAN3D_HPP

// Predeclare common Cartesian 3D points for convenience

#include <boost/geometry/geometries/geometries.hpp>

namespace boost { namespace geometry
{

namespace model { namespace d3
{

// Todo: create point_xyz in between to enable methods .x(), .y(), .z()
// the same way as d2::point has

typedef model::point<double, 3, cs::cartesian> point;
typedef model::linestring<point> linestring;
typedef model::linear_ring<point> ring;
typedef model::polygon<point> polygon;
typedef model::box<point> box;

}} // namespace model::d3


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_GEOMETRIES_CARTESIAN3D_HPP
