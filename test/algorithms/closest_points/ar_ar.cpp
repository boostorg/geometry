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
#include "test_empty_geometry.hpp"

namespace bg = boost::geometry;

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_polygon_or_ring_polygon_or_ring(Strategy const& strategy)
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
                  "SEGMENT(1.5 0.5,1 0)",
                  "SEGMENT(0.50019 1.50021,0 1)",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);

    // rings intersect
    tester::apply("POLYGON((1 1,0.1 0.1,2 1,1 1))",
                  "POLYGON((0 0,1 0,0 1,0 0))",
                  "SEGMENT(0.5 0.5,0.5 0.5)",
                  "SEGMENT(0.500004 0.500053,0.500004 0.500053)",
                  strategy);
    // ring inside ring
    tester::apply("POLYGON((0.1 0.1,0.2 0.1,0.1 0.2,0.1 0.1))",
                  "POLYGON((0 0,1 0,0 1,0 0))",
                  "SEGMENT(0.1 0.1,0.1 0.1)",
                  strategy);

    typedef test_geometry<Ring, Polygon, Segment> tester2;

    tester2::apply("POLYGON((2 2,2 0,0 2,2 2))",
                   "POLYGON((0 0,1 0,0 1,0 0)(0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4))",
                   "SEGMENT(1.5 0.5,1 0)",
                   "SEGMENT(0.50019 1.50021,0 1)",
                   "SEGMENT(1.496909 0.503379,1 0)",
                   strategy);

    // ring-polygon intersect
    tester2::apply("POLYGON((1 1,0.1 0.1,2 1,1 1))",
                  "POLYGON((0 0,1 0,0 1,0 0))",
                  "SEGMENT(0.5 0.5,0.5 0.5)",
                  "SEGMENT(0.500004 0.500053,0.500004 0.500053)",
                  strategy);

    typedef test_geometry<Polygon, Ring, Segment> tester3;

    tester3::apply("POLYGON((2 2,2 0,0 2,2 2)(1.5 1,1 1.5,1.5 1.5,1.5 1))",
                   "POLYGON((0 0,1 0,0 1,0 0))",
                   "SEGMENT(1.5 0.5,1 0)",
                   "SEGMENT(0.50019 1.50021,0 1)",
                   "SEGMENT(1.496909 0.503379,1 0)",
                   strategy);

    // polygon-ring intersect
    tester3::apply("POLYGON((1 1,0.1 0.1,2 1,1 1))",
                   "POLYGON((0 0,1 0,0 1,0 0))",
                   "SEGMENT(0.5 0.5,0.5 0.5)",
                   "SEGMENT(0.500004 0.500053,0.500004 0.500053)",
                   strategy);

    typedef test_geometry<Polygon, Polygon, Segment> tester4;

    tester4::apply("POLYGON((2 2,2 0,0 2,2 2)(1.5 1,1 1.5,1.5 1.5,1.5 1))",
                   "POLYGON((0 0,1 0,0 1,0 0)(0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4))",
                   "SEGMENT(1.5 0.5,1 0)",
                   "SEGMENT(0.50019 1.50021,0 1)",
                   "SEGMENT(1.496909 0.503379,1 0)",
                   strategy);

    // polygons one included in the other
    tester4::apply("POLYGON((10 10,10 0,0 10,10 10)(8 8,4 8,8 4,8 8))",
                   "POLYGON((9 9,9 2,2 9,9 9))",
                   "SEGMENT(9 9,9 9)",
                   strategy);
    tester4::apply("POLYGON((10 10,10 0,0 10,10 10)(8 8,4 8,8 4,8 8))",
                   "POLYGON((9 9,9 2,2 9,9 9)(7 7,6 7,7 6,7 7))",
                   "SEGMENT(9 9,9 9)",
                   strategy);
    //touches(cartesian), intersects(spherical,geographic)
    tester4::apply("POLYGON((10 10,10 0,0 10,10 10)(8 8,4 8,8 4,8 8))",
                   "POLYGON((9 9,9 2,2 9,9 9)(7 7,6 7,7 5,7 7))",
                   "SEGMENT(7 5,7 5)",
                   "SEGMENT(6.99222 5.01561,6.99222 5.01561)",
                   strategy);
    //intersects
    tester4::apply("POLYGON((10 10,10 0,0 10,10 10)(8 8,4 8,8 4,8 8))",
                   "POLYGON((9 9,9 2,2 9,9 9)(7 7,6 7,7 4.5,7 7))",
                   "SEGMENT(6.66667 5.33333,6.66667 5.33333)",
                   "SEGMENT(6.66211 5.34732,6.66211 5.34732)",
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
                  "MULTIPOLYGON(((0 0,1 0,0 1,0 0)),\
                               ((0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4)))",
                  "SEGMENT(1.5 0.5,1 0)",
                  "SEGMENT(0.50019 1.50021,0 1)",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);

    // multipolygon-ring intersect
    tester::apply("POLYGON((1 1,0.1 0.1,2 1,1 1))",
                  "MULTIPOLYGON(((0 0,1 0,0 1,0 0))((2 2,2 0,0 2,2 2)))",
                  "SEGMENT(0.5 0.5,0.5 0.5)",
                  "SEGMENT(0.500004 0.500053,0.500004 0.500053)",
                  strategy);

    typedef test_geometry<Polygon, MultiPolygon, Segment> tester2;

    tester2::apply("POLYGON((2 2,2 0,0 2,2 2))",
                   "MULTIPOLYGON(((0 0,1 0,0 1,0 0)),\
                                ((0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4)))",
                   "SEGMENT(1.5 0.5,1 0)",
                   "SEGMENT(0.50019 1.50021,0 1)",
                   "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);

    // multipolygon-polygon intersect
    tester2::apply("POLYGON((1 1,0.1 0.1,2 1,1 1)(1 0.9,1.02 0.9,1 0.8,1 0.9))",
                   "MULTIPOLYGON(((0 0,1 0,0 1,0 0))((2 2,2 0,0 2,2 2)))",
                   "SEGMENT(0.5 0.5,0.5 0.5)",
                   "SEGMENT(0.500004 0.500053,0.500004 0.500053)",
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

    tester::apply("MULTIPOLYGON(((2 2,2 0,0 2,2 2)),\
                               ((1.5 1,1 1.5,1.5 1.5,1.5 1)))",
                  "MULTIPOLYGON(((0 0,1 0,0 1,0 0)),\
                               ((0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4)))",
                  "SEGMENT(1.5 0.5,1 0)",
                  "SEGMENT(0.50019 1.50021,0 1)",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);

    // multipolygon-multipolygon intersect
    tester::apply("MULTIPOLYGON(((1 1,0.1 0.1,2 1,1 1)),\
                                ((1 0.9,1.02 0.9,1 0.8,1 0.9)))",
                  "MULTIPOLYGON(((0 0,1 0,0 1,0 0))((2 2,2 0,0 2,2 2)))",
                  "SEGMENT(0.5 0.5,0.5 0.5)",
                  "SEGMENT(0.500004 0.500053,0.500004 0.500053)",
                  strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_box_polygon_or_ring(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "box / polygon or ring closest_points tests"
              << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef bg::model::ring<Point> Ring;
    typedef bg::model::polygon<Point> Polygon;
    typedef bg::model::box<Point> Box;

    typedef test_geometry<Box, Ring, Segment> tester;

    tester::apply("BOX(10 10,20 20)",
                  "POLYGON((0 0,2 0,0 2,0 0))",
                  "SEGMENT(10 10,1 1)",
                  "SEGMENT(10 10,0.922834 1.07763)",
                  "SEGMENT(10 10,0.983761 1.0167)",
                  strategy);

    typedef test_geometry<Box, Polygon, Segment> tester2;

    tester2::apply("BOX(10 10,20 20)",
                   "POLYGON((0 0,2 0,0 2,0 0)(0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4))",
                   "SEGMENT(10 10,1 1)",
                   "SEGMENT(10 10,0.922834 1.07763)",
                   "SEGMENT(10 10,0.983761 1.0167)",
                   strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_box_multi_polygon(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "box / multi-polygon closest_points tests"
              << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef bg::model::polygon<Point> Polygon;
    typedef bg::model::multi_polygon<Polygon> MultiPolygon;
    typedef bg::model::box<Point> Box;

    typedef test_geometry<Box, MultiPolygon, Segment> tester;

    tester::apply("BOX(10 10,20 20)",
                  "MULTIPOLYGON(((0 0,2 0,0 2,0 0)),\
                                ((0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4)))",
                  "SEGMENT(10 10,1 1)",
                  "SEGMENT(10 10,0.922834 1.07763)",
                  "SEGMENT(10 10,0.983761 1.0167)",
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
                  "SEGMENT(15 30,15 20)",
                  "SEGMENT(20 30,20 20)",
                  strategy, false);

    tester::apply("BOX(10 10,20 20)",
                  "BOX(5 30,40 40)",
                  "SEGMENT(10 20,10 30)",
                  strategy);

    tester::apply("BOX(10 10,20 20)",
                  "BOX(5 30,15 40)",
                  "SEGMENT(10 20,10 30)",
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

template
<
        typename Point,
        typename PSStrategy,
        typename BBStrategy,
        typename SBStrategy
>
void test_all_ar_ar(PSStrategy ps_strategy,
                    BBStrategy bb_strategy,
                    SBStrategy sb_strategy)
{
    test_closest_points_polygon_or_ring_polygon_or_ring<Point>(ps_strategy);
    test_closest_points_polygon_multi_polygon<Point>(ps_strategy);
    test_closest_points_multi_polygon_multi_polygon<Point>(ps_strategy);

    //test_closest_points_box_polygon_or_ring<Point>(sb_strategy);
    //test_closest_points_box_multi_polygon<Point>(sb_strategy);
    //test_closest_points_box_box<Point>(bb_strategy);

    //test_more_empty_input_areal_areal<Point>(ps_strategy);
}

BOOST_AUTO_TEST_CASE( test_all_areal_areal )
{
    test_all_ar_ar<car_point>(cartesian_ps(), cartesian_bb(), cartesian_ps());

    double radius = bg::formula::mean_radius<double>(bg::srs::spheroid<double>());

    test_all_ar_ar<sph_point>(spherical_ps(), spherical_bb(), spherical_sb());
    test_all_ar_ar<sph_point>(spherical_ps(radius),
                              spherical_bb(radius),
                              spherical_sb(radius));

    test_all_ar_ar<geo_point>(andoyer_ps(), andoyer_bb(), andoyer_sb());
    test_all_ar_ar<geo_point>(thomas_ps(), thomas_bb(), thomas_sb());
    test_all_ar_ar<geo_point>(vincenty_ps(), vincenty_bb(), vincenty_sb());
}

