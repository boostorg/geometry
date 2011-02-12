// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_CORE_TAGS_HPP
#define BOOST_GEOMETRY_CORE_TAGS_HPP


namespace boost { namespace geometry
{

// Tags defining strategies linked to coordinate systems


/// Tag indicating Cartesian coordinate system family (cartesian,epsg)
struct cartesian_tag {};

/// Tag indicating Geographic coordinate system family (geographic)
struct geographic_tag {};

/// Tag indicating Spherical coordinate system family (spherical,celestial,...)
struct spherical_tag {};


// Tags defining tag hierarchy

/// For single-geometries (point, linestring, polygon, box, ring, segment)
struct single_tag {};


/// For multiple-geometries (multi_point, multi_linestring, multi_polygon)
struct multi_tag {};

// Tags defining geometry types


/// "default" tag
struct geometry_not_recognized_tag {};

/// OGC Point identifying tag
struct point_tag : single_tag {};

/// OGC Linestring identifying tag
struct linestring_tag : single_tag  {};

/// OGC Polygon identifying tag
struct polygon_tag : single_tag  {};

/// Convenience (linear) ring identifying tag
struct ring_tag : single_tag  {};

/// Convenience 2D or 3D box (mbr) identifying tag
struct box_tag : single_tag  {};

/// Convenience segment (2-points) identifying tag
struct segment_tag : single_tag  {};



}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_CORE_TAGS_HPP
