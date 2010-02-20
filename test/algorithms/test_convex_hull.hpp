// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_CONVEX_HULL_HPP
#define BOOST_GEOMETRY_TEST_CONVEX_HULL_HPP

#include <geometry_test_common.hpp>

#include <boost/geometry/algorithms/convex_hull.hpp>
#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/num_points.hpp>

#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>
#include <boost/geometry/extensions/gis/io/wkt/write_wkt.hpp>

#include <boost/geometry/geometries/polygon.hpp>


template <typename Geometry, typename Hull>
void test_convex_hull(Geometry const& geometry, Hull const& hull,
                      std::size_t size_original, std::size_t size_hull,
                      double expected_area, bool reverse)
{

    std::size_t n = boost::geometry::num_points(hull);

    BOOST_CHECK_MESSAGE(n == size_hull,
        "convex hull: " << boost::geometry::wkt(geometry)
        << " -> " << boost::geometry::wkt(hull)
        << " type "
        << (typeid(typename boost::geometry::coordinate_type<Hull>::type).name())
        << " -> Expected: " << size_hull
        << " detected: " << n);


    BOOST_CHECK(boost::geometry::num_points(geometry) == size_original);

    double ah = boost::geometry::area(hull);
    if (reverse)
    {
        ah = -ah;
    }

//std::cout << "Area: " << boost::geometry::area(geometry) << std::endl;
//std::cout << boost::geometry::wkt(hull) << std::endl;

    BOOST_CHECK_CLOSE(ah, expected_area, 0.001);
}

template <typename Geometry, bool Clockwise>
void test_geometry_order(std::string const& wkt,
                      std::size_t size_original, std::size_t size_hull,
                      double expected_area)
{
    Geometry geometry;
    boost::geometry::read_wkt(wkt, geometry);

    boost::geometry::polygon
        <
            typename boost::geometry::point_type<Geometry>::type,
            std::vector,
            std::vector,
            Clockwise
        > hull;

    // Test version with output iterator
    convex_hull_inserter(geometry, std::back_inserter(hull.outer()));
    test_convex_hull(geometry, hull,
        size_original, size_hull, expected_area, ! Clockwise);

    // Test version with ring as output
    boost::geometry::clear(hull);
    boost::geometry::convex_hull(geometry, hull.outer());
    test_convex_hull(geometry, hull, size_original, size_hull, expected_area, false);

    // Test version with polygon as output
    boost::geometry::clear(hull);
    boost::geometry::convex_hull(geometry, hull);
    test_convex_hull(geometry, hull, size_original, size_hull, expected_area, false);

    // Test version with strategy
    boost::geometry::clear(hull);
    boost::geometry::strategy::convex_hull::graham_andrew
        <
            Geometry,
            typename boost::geometry::point_type<Geometry>::type
        > graham;
    boost::geometry::convex_hull(geometry, hull.outer(), graham);
    test_convex_hull(geometry, hull, size_original, size_hull, expected_area, false);

    // Test version with output iterator and strategy
    boost::geometry::clear(hull);
    boost::geometry::convex_hull_inserter(geometry, std::back_inserter(hull.outer()), graham);
    test_convex_hull(geometry, hull, size_original, size_hull, expected_area, ! Clockwise);
}

template <typename Geometry>
void test_geometry(std::string const& wkt,
                      std::size_t size_original, std::size_t size_hull,
                      double expected_area)
{
    test_geometry_order<Geometry, true>(wkt, size_original, size_hull, expected_area);
    test_geometry_order<Geometry, false>(wkt, size_original, size_hull, expected_area);
}


#endif
