// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2015, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_VALIDITY_FAILURE_TYPE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_VALIDITY_FAILURE_TYPE_HPP


namespace boost { namespace geometry
{


enum validity_failure_type
{
    no_failure = 0,
    failure_few_points = 10,
    failure_wrong_dimension = 11,
    failure_spikes = 12,
    failure_duplicate_points = 13,
    failure_not_closed = 20, // for areal geometries
    failure_self_intersections = 21, // for areal geometries
    failure_wrong_orientation = 22, // for areal geometries
    failure_holes_outside = 23, // for areal geometries
    failure_intersecting_holes = 24, // for areal geometries
    failure_disconnected_interior = 30, // for polygons
    failure_intersecting_interiors = 40 // for multipolygons
};


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_VALIDITY_FAILURE_TYPE_HPP
