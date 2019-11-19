// Boost.Geometry
// Unit Test

// Copyright (c) 2019, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE test_closest_points_geographic_areal_areal
#endif

#define BOOST_GEOMETRY_TEST_DEBUG

#include "test_closest_points_common.hpp"

namespace bg = boost::geometry;

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_polygon_or_ring(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "polygon or ring /polygon or ring closest_points tests"
              << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef bg::model::ring<Point> Ring;
    typedef bg::model::polygon<Point> Polygon;

    typedef test_geometry<Ring, Ring, Segment> tester;

    tester::apply("POLYGON((2 2,2 0,0 2,2 2))",
                  "POLYGON((0 0,1 0,0 1,0 0))",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);

    typedef test_geometry<Ring, Polygon, Segment> tester2;

    tester2::apply("POLYGON((2 2,2 0,0 2,2 2))",
                  "POLYGON((0 0,1 0,0 1,0 0)(0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4))",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);

    typedef test_geometry<Polygon, Ring, Segment> tester3;

    tester3::apply("POLYGON((2 2,2 0,0 2,2 2)(1.5 1,1 1.5,1.5 1.5,1.5 1))",
                  "POLYGON((0 0,1 0,0 1,0 0))",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);

    typedef test_geometry<Polygon, Polygon, Segment> tester4;

    tester4::apply("POLYGON((2 2,2 0,0 2,2 2)(1.5 1,1 1.5,1.5 1.5,1.5 1))",
                  "POLYGON((0 0,1 0,0 1,0 0)(0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4))",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_polygon_multi_polygon(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "polygon / multi-polygon closest_points tests"
              << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef bg::model::ring<Point> Ring;
    typedef bg::model::polygon<Point> Polygon;
    typedef bg::model::multi_polygon<Polygon> MultiPolygon;

    typedef test_geometry<Ring, MultiPolygon, Segment> tester;

    tester::apply("POLYGON((2 2,2 0,0 2,2 2))",
                  "MULTIPOLYGON(((0 0,1 0,0 1,0 0)),((0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4)))",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);

    typedef test_geometry<Polygon, MultiPolygon, Segment> tester2;

    tester2::apply("POLYGON((2 2,2 0,0 2,2 2))",
                  "MULTIPOLYGON(((0 0,1 0,0 1,0 0)),((0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4)))",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_multi_polygon_multi_polygon(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "multi-polygon / multi-polygon closest_points tests"
              << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef bg::model::polygon<Point> Polygon;
    typedef bg::model::multi_polygon<Polygon> MultiPolygon;

    typedef test_geometry<MultiPolygon, MultiPolygon, Segment> tester;

    tester::apply("MULTIPOLYGON(((2 2,2 0,0 2,2 2)),((1.5 1,1 1.5,1.5 1.5,1.5 1)))",
                  "MULTIPOLYGON(((0 0,1 0,0 1,0 0)),((0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4)))",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_box_box(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "box / box closest_points tests"
              << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef bg::model::box<Point> Box;

    typedef test_geometry<Box, Box, Segment> tester;

    tester::apply("BOX(10 10,20 20)",
                  "BOX(30 30,40 40)",
                  "SEGMENT(20 20,30 30)",
                  strategy);

    tester::apply("BOX(10 10,20 20)",
                  "BOX(15 30,40 40)",
                  "SEGMENT(15 20,15 30)",
                  strategy, false);

    tester::apply("BOX(15 30,40 40)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(20 30,20 20)",
                  strategy, false);

    tester::apply("BOX(10 10,20 20)",
                  "BOX(5 30,40 40)",
                  "SEGMENT(10 20,10 30)",
                  strategy);

    tester::apply("BOX(10 10,20 20)",
                  "BOX(5 30,15 40)",
                  "SEGMENT(15 20,15 30)",
                  strategy, false);

    tester::apply("BOX(5 30,15 40)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(10 30,10 20)",
                  strategy, false);

    tester::apply("BOX(10 10,20 20)",
                  "BOX(0 30,5 40)",
                  "SEGMENT(10 20,5 30)",
                  strategy);
}

//===========================================================================
//===========================================================================
//===========================================================================

template <typename Point, typename PSStrategy, typename BBStrategy>
void test_all_ar_ar(PSStrategy cp_strategy, BBStrategy bb_strategy)
{
    test_closest_points_polygon_or_ring<Point>(cp_strategy);
    test_closest_points_polygon_multi_polygon<Point>(cp_strategy);
    test_closest_points_multi_polygon_multi_polygon<Point>(cp_strategy);

    test_closest_points_box_box<Point>(bb_strategy);

    //test_more_empty_input_areal_areal<Point>(cp_strategy);
}

BOOST_AUTO_TEST_CASE( test_all_areal_areal )
{
    typedef bg::model::point
            <
                double, 2,
                bg::cs::geographic<bg::degree>
            > geo_point;

    test_all_ar_ar<geo_point>(andoyer_cp(), andoyer_bb());
    test_all_ar_ar<geo_point>(thomas_cp(), thomas_bb());
    test_all_ar_ar<geo_point>(vincenty_cp(), vincenty_bb());
}
