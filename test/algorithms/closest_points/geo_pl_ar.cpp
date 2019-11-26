// Boost.Geometry
// Unit Test

// Copyright (c) 2019, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE test_closest_points_geographic_pointlike_areal
#endif

#include "test_closest_points_common.hpp"
#include "test_empty_geometry.hpp"

namespace bg = boost::geometry;

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_point_polygon_or_ring(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "point/polygon or ring closest_points tests" << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef bg::model::ring<Point> Ring;
    typedef bg::model::polygon<Point> Polygon;
    typedef test_geometry<Point, Ring, Segment> tester;

    tester::apply("POINT(1 1)",
                  "POLYGON((0 0,1 0,0 1,0 0))",
                  "SEGMENT(1 1,0.503314 0.496737)",
                  strategy);

    typedef test_geometry<Point, Polygon, Segment> tester2;

    tester2::apply("POINT(1 1)",
                   "POLYGON((0 0,1 0,0 1,0 0))",
                   "SEGMENT(1 1,0.503314 0.496737)",
                   strategy);

    tester2::apply("POINT(1 1)",
                   "POLYGON((0 0,1 0,0 1,0 0)(0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4))",
                   "SEGMENT(1 1,0.503314 0.496737)",
                   strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_point_box(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "point/box closest_points tests" << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef bg::model::box<Point> Box;
    typedef test_geometry<Point, Box, Segment> tester;

    //all 8 possible placements of point around box
    tester::apply("POINT(5 25)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(5 25,10 20)",
                  strategy);

    tester::apply("POINT(15 25)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(15 25,15 20)",
                  strategy);

    tester::apply("POINT(25 25)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(25 25,20 20)",
                  strategy);

    tester::apply("POINT(5 15)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(5 15,10 15.055045985544346)",
                  strategy);

    tester::apply("POINT(25 15)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(25 15,20 15.055045985544346)",
                  strategy);

    tester::apply("POINT(5 5)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(5 5,10 10)",
                  strategy);

    tester::apply("POINT(15 5)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(15 5,15 10)",
                  strategy);

    tester::apply("POINT(25 5)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(25 5,20 10)",
                  strategy);

    //inside
    tester::apply("POINT(15 15)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(0 0,0 0)",
                  strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_point_multi_polygon(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "point/multi-polygon closest_points tests" << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef bg::model::polygon<Point> Polygon;
    typedef bg::model::multi_polygon<Polygon> MultiPolygon;
    typedef test_geometry<Point, MultiPolygon, Segment> tester;

    tester::apply("POINT(1 1)",
                  "MULTIPOLYGON(((0 0,1 0,0 1,0 0)),\
                                ((0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4)))",
                  "SEGMENT(1 1,0.503314 0.496737)",
                  strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_multi_point_polygon_or_ring(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "multi-point/polygon or ring closest_points tests" << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef bg::model::multi_point<Point> MultiPoint;
    typedef bg::model::ring<Point> Ring;
    typedef bg::model::polygon<Point> Polygon;
    typedef test_geometry<MultiPoint, Ring, Segment> tester;

    tester::apply("MULTIPOINT((2 1),(1 1))",
                  "POLYGON((0 0,1 0,0 1,0 0))",
                  "SEGMENT(1 1,0.503314 0.496737)",
                  strategy);

    typedef test_geometry<MultiPoint, Polygon, Segment> tester2;

    tester2::apply("MULTIPOINT((2 1),(1 1))",
                   "POLYGON((0 0,1 0,0 1,0 0))",
                   "SEGMENT(1 1,0.503314 0.496737)",
                   strategy);

    tester2::apply("MULTIPOINT((2 1),(1 1))",
                   "POLYGON((0 0,1 0,0 1,0 0)(0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4))",
                   "SEGMENT(1 1,0.503314 0.496737)",
                   strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_multi_point_multi_polygon(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "multi-point/multi-polygon closest_points tests" << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef bg::model::multi_point<Point> MultiPoint;
    typedef bg::model::polygon<Point> Polygon;
    typedef bg::model::multi_polygon<Polygon> MultiPolygon;
    typedef test_geometry<MultiPoint, MultiPolygon, Segment> tester;

    tester::apply("MULTIPOINT((2 1),(1 1))",
                  "MULTIPOLYGON(((0 0,1 0,0 1,0 0)),\
                                ((0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4)))",
                  "SEGMENT(1 1,0.503314 0.496737)",
                  strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_multi_point_box(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "multi-point/box closest_points tests" << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef bg::model::box<Point> Box;
    typedef bg::model::multi_point<Point> MultiPoint;
    typedef test_geometry<MultiPoint, Box, Segment> tester;

    tester::apply("MULTIPOINT(5 25,0 0,0 20)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(5 25,10 20)",
                  strategy);
}

//===========================================================================
//===========================================================================
//===========================================================================

template <typename Point, typename PSStrategy, typename PBStrategy>
void test_all_pl_ar(PSStrategy cp_strategy, PBStrategy pb_strategy)
{
    test_closest_points_point_polygon_or_ring<Point>(cp_strategy);
    test_closest_points_point_multi_polygon<Point>(cp_strategy);
    //test_closest_points_point_box<Point>(pb_strategy);

    test_closest_points_multi_point_polygon_or_ring<Point>(cp_strategy);
    test_closest_points_multi_point_multi_polygon<Point>(cp_strategy);
    //test_closest_points_multi_point_box<Point>(pb_strategy);

    test_more_empty_input_pointlike_areal<Point>(cp_strategy);
}

BOOST_AUTO_TEST_CASE( test_all_pointlike_areal )
{
    typedef bg::model::point
            <
                double, 2,
                bg::cs::geographic<bg::degree>
            > geo_point;

    test_all_pl_ar<geo_point>(andoyer_ps(), andoyer_pb());
    test_all_pl_ar<geo_point>(thomas_ps(), thomas_pb());
    test_all_pl_ar<geo_point>(vincenty_ps(), vincenty_pb());
}


