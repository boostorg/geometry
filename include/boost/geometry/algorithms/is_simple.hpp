// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_IS_SIMPLE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_IS_SIMPLE_HPP

#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/algorithms/is_valid.hpp>
#include <boost/geometry/algorithms/detail/is_simple/pointlike.hpp>
#include <boost/geometry/algorithms/detail/is_simple/linear.hpp>

#include <boost/geometry/algorithms/dispatch/is_simple.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


// A segment is always simple.
// A segment is a curve.
// A curve is simple if it does not pass through the same point twice,
// with the possible exception of its two endpoints
//
// Reference: OGC 06-103r4 (§6.1.6.1)
template <typename Segment>
struct is_simple<Segment, segment_tag>
    : dispatch::is_valid<Segment, segment_tag>
{};


// A box is always simple
// A box is a Polygon, and it satisfies the conditions for Polygon validity.
//
// Reference (for polygon validity): OGC 06-103r4 (§6.1.11.1)
template <typename Box>
struct is_simple<Box, box_tag>
    : dispatch::is_valid<Box, box_tag>
{};


// A Ring is a Polygon.
// A Polygon is always a simple geometric object provided that it is valid.
//
// Reference (for polygon validity): OGC 06-103r4 (§6.1.11.1)
template <typename Ring>
struct is_simple<Ring, ring_tag>
    : dispatch::is_valid<Ring, ring_tag>
{};


// A Polygon is always a simple geometric object provided that it is valid.
//
// Reference (for validity of Polygons): OGC 06-103r4 (§6.1.11.1)
template <typename Polygon>
struct is_simple<Polygon, polygon_tag>
    : dispatch::is_valid<Polygon, polygon_tag>
{};



// Not clear what the definition is.
// Right now we check that each element is simple (in fact valid), and
// that the MultiPolygon is also valid.
//
// Reference (for validity of MultiPolygons): OGC 06-103r4 (§6.1.14)
template <typename MultiPolygon>
struct is_simple<MultiPolygon, multi_polygon_tag>
    : dispatch::is_valid<MultiPolygon, multi_polygon_tag>
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH



template <typename Geometry>
inline bool is_simple(Geometry const& g)
{
    return dispatch::is_simple<Geometry>::apply(g);
}



}} // namespace boost::geometry



#endif // BOOST_GEOMETRY_ALGORITHMS_IS_SIMPLE_HPP
