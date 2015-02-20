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
    failure_holes_outside = 30, // for (multi-)polygons
    failure_nested_holes = 31, // for (multi-)polygons
    failure_disconnected_interior = 32, // for (multi-)polygons
    failure_intersecting_interiors = 40, // for multi-polygons
    failure_wrong_corner_order = 50 // for boxes
};


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_VALIDITY_FAILURE_TYPE_HPP
