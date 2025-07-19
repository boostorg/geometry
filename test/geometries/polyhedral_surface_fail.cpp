// Boost.Geometry
// Unit Test

// Copyright (c) 2025 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Polyhedral surface tests invalid concepts

#include <iostream>
#include <initializer_list>

#include <geometry_test_common.hpp>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/polyhedral_surface.hpp>

int test_main(int, char* [])
{
    // The following tests should fail

    namespace bg = boost::geometry;

    // Define a 2-dimensional polyhedral surface
    using point2_t = bg::model::point<double, 2, bg::cs::cartesian>;
    using polygon2_t = bg::model::polygon<point2_t>;
    using polyhedral2_t = bg::model::polyhedral_surface<polygon2_t>;

    // Create a 3D polyhedral surface with geographic coordinates
    using point_geo_t = bg::model::point<double, 3, bg::cs::geographic<bg::degree>>;
    using polygon_geo_t = bg::model::polygon<point_geo_t>;
    using polyhedral_geo_t = bg::model::polyhedral_surface<polygon_geo_t>;

    BOOST_CONCEPT_ASSERT( (bg::concepts::PolyhedralSurface<polyhedral2_t>) );
    BOOST_CONCEPT_ASSERT( (bg::concepts::PolyhedralSurface<polyhedral_geo_t>) );

    return 0;
}
