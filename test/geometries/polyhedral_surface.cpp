// Boost.Geometry
// Unit Test

// Copyright (c) 2025 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Polyhedral surface tests

#include <iostream>
#include <initializer_list>

#include <geometry_test_common.hpp>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/polyhedral_surface.hpp>

int test_main(int, char* [])
{
    namespace bg = boost::geometry;
    using point_t = bg::model::point<double, 3, bg::cs::cartesian>;
    using polygon_t = bg::model::polygon<point_t>;
    using polyhedral_t = bg::model::polyhedral_surface<polygon_t>;
    using const_polyhedral_t = bg::model::polyhedral_surface<polygon_t> const;

    // intializing an empty polyhedral surface (default constructor)
    polyhedral_t polyhedral0;
    BOOST_CHECK_EQUAL(boost::size(polyhedral0), 0);

    // creating a polyhderal surface using standard initiallized list
    polyhedral_t polyhedral1 = {{{{0, 0, 0}, {0, 1, 0}, {1, 1, 0}, {1, 0, 0}, {0, 0, 0}}},\
                                {{{0, 0, 0}, {0, 1, 0}, {0, 1, 1}, {0, 0, 1}, {0, 0, 0}}},\
                                {{{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}, {0, 0, 0}}},\
                                {{{1, 1, 1}, {1, 0, 1}, {0, 0, 1}, {0, 1, 1}, {1, 1, 1}}},\
                                {{{1, 1, 1}, {1, 0, 1}, {1, 0, 0}, {1, 1, 0}, {1, 1, 1}}},\
                                {{{1, 1, 1}, {1, 1, 0}, {0, 1, 0}, {0, 1, 1}, {1, 1, 1}}}};

    // Modify polyhedral1 by adding a new polygon
    polygon_t new_polygon = {{{0, 0, 1}, {1, 0, 1}, {1, 1, 1}, {0, 1, 1}, {0, 0, 1}}};
    polyhedral1.push_back(new_polygon);
    // Modify polyhedral1 by removing the last polygon
    polyhedral1.pop_back();
    // Modify polyhedral1 by changing the first point of the first polygon
    polyhedral1[0].outer()[0] = point_t(0, 2, 0);

    // Test that the polyhedral surface has 6 polygons
    BOOST_CHECK_EQUAL(boost::size(polyhedral1), 6);

    // Test that each polygon has 5 points
    for (const auto& polygon : polyhedral1)
    {
        BOOST_CHECK_EQUAL(boost::size(polygon.outer()), 5);
    }

    // clear polyhedral surface
    bg::clear(polyhedral1);
    BOOST_CHECK_EQUAL(boost::size(polyhedral1), 0);

    // read/write polyhedral surface wkt
    polyhedral_t polyhedral2;
    std::string wkt = "POLYHEDRALSURFACE(((0 0 0,0 1 0,1 1 0,1 0 0,0 0 0)),((0 0 0,0 1 0,0 1 1,0 0 1,0 0 0)))";
    bg::read_wkt(wkt, polyhedral2);
    std::ostringstream oss;
    oss << bg::wkt(polyhedral2);
    BOOST_CHECK_EQUAL(oss.str(), wkt);

    // Custom polyhedral surface
    std::initializer_list<polygon_t> IL = {{{{0, 0, 0}, {0, 1, 0}, {1, 1, 0}, {1, 0, 0}, {0, 0, 0}}}};
    bg::model::polyhedral_surface<polygon_t> ps1(IL);
    std::ostringstream out;
    out << bg::wkt(ps1);
    BOOST_CHECK_EQUAL(out.str(), "POLYHEDRALSURFACE(((0 0 0,0 1 0,1 1 0,1 0 0,0 0 0)))");
    // Test that the polyhedral surface has 1 polygon
    BOOST_CHECK_EQUAL(boost::size(ps1), 1);

    // Test concepts
    BOOST_CONCEPT_ASSERT( (bg::concepts::PolyhedralSurface<polyhedral_t>) );
    BOOST_CONCEPT_ASSERT( (bg::concepts::ConstPolyhedralSurface<const_polyhedral_t>) );

    return 0;
}
