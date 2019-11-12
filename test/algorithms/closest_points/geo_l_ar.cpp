// Boost.Geometry
// Unit Test

// Copyright (c) 2019, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE test_closest_points_geographic_linear_areal
#endif

#define BOOST_GEOMETRY_TEST_DEBUG

#include "test_closest_points_common.hpp"

namespace bg = boost::geometry;

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_segment_polygon_or_ring(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "segment/polygon or ring closest_points tests" << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef bg::model::ring<Point> Ring;
    typedef bg::model::polygon<Point> Polygon;
    typedef test_geometry<Segment, Ring, Segment> tester;

    tester::apply("SEGMENT(2 0,0 2)",
                  "POLYGON((0 0,1 0,0 1,0 0))",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);

    typedef test_geometry<Segment, Polygon, Segment> tester2;

    tester2::apply("SEGMENT(2 0,0 2)",
                   "POLYGON((0 0,1 0,0 1,0 0))",
                   "SEGMENT(1.496909 0.503379,1 0)",
                   strategy);

    tester2::apply("SEGMENT(2 0,0 2)",
                   "POLYGON((0 0,1 0,0 1,0 0)(0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4))",
                   "SEGMENT(1.496909 0.503379,1 0)",
                   strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_segment_multi_polygon(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "segment/multi-polygon or ring closest_points tests" << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef bg::model::polygon<Point> Polygon;
    typedef bg::model::multi_polygon<Polygon> MultiPolygon;
    typedef test_geometry<Segment, MultiPolygon, Segment> tester;

    tester::apply("SEGMENT(2 0,0 2)",
                  "MULTIPOLYGON(((0 0,1 0,0 1,0 0)),\
                                ((0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4)))",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_segment_box(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "segment/box closest_points tests" << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef bg::model::box<Point> Box;
    typedef test_geometry<Segment, Box, Segment> tester;

    tester::apply("SEGMENT(2 0,0 2)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(0.983761 1.0167,10 10)",
                  strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_linestring_polygon_or_ring(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "linestring/polygon or ring closest_points tests" << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef bg::model::linestring<Point> Linestring;
    typedef bg::model::ring<Point> Ring;
    typedef bg::model::polygon<Point> Polygon;
    typedef test_geometry<Linestring, Ring, Segment> tester;

    tester::apply("LINESTRING(2 0,0 2)",
                  "POLYGON((0 0,1 0,0 1,0 0))",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);

    typedef test_geometry<Linestring, Polygon, Segment> tester2;

    tester2::apply("LINESTRING(2 0,0 2)",
                   "POLYGON((0 0,1 0,0 1,0 0))",
                   "SEGMENT(1.496909 0.503379,1 0)",
                   strategy);

    tester2::apply("LINESTRING(2 0,0 2)",
                   "POLYGON((0 0,1 0,0 1,0 0)(0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4))",
                   "SEGMENT(1.496909 0.503379,1 0)",
                   strategy);
}


//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_linestring_multi_polygon(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "linestring/multi-polygon closest_points tests" << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef bg::model::linestring<Point> Linestring;
    typedef bg::model::polygon<Point> Polygon;
    typedef bg::model::multi_polygon<Polygon> MultiPolygon;
    typedef test_geometry<Linestring, MultiPolygon, Segment> tester;

    tester::apply("LINESTRING(2 0,0 2)",
                  "MULTIPOLYGON(((0 0,1 0,0 1,0 0)),\
                                ((0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4)))",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_multi_linestring_polygon_or_ring(Strategy
                                                          const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "multi-linestring/polygon or ring closest_points tests"
              << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef bg::model::linestring<Point> Linestring;
    typedef bg::model::multi_linestring<Linestring> MultiLinestring;
    typedef bg::model::ring<Point> Ring;
    typedef bg::model::polygon<Point> Polygon;
    typedef test_geometry<MultiLinestring, Ring, Segment> tester;

    tester::apply("MULTILINESTRING((2 0,0 2)(0 3,3 0))",
                  "POLYGON((0 0,1 0,0 1,0 0))",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);

    typedef test_geometry<MultiLinestring, Polygon, Segment> tester2;

    tester2::apply("MULTILINESTRING((2 0,0 2)(0 3,3 0))",
                   "POLYGON((0 0,1 0,0 1,0 0))",
                   "SEGMENT(1.496909 0.503379,1 0)",
                   strategy);

    tester2::apply("MULTILINESTRING((2 0,0 2)(0 3,3 0))",
                   "POLYGON((0 0,1 0,0 1,0 0)(0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4))",
                   "SEGMENT(1.496909 0.503379,1 0)",
                   strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_multi_linestring_multi_polygon(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "multi-linestring/polygon or ring closest_points tests"
              << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef bg::model::linestring<Point> Linestring;
    typedef bg::model::multi_linestring<Linestring> MultiLinestring;
    typedef bg::model::polygon<Point> Polygon;
    typedef bg::model::multi_polygon<Polygon> MultiPolygon;

    typedef test_geometry<MultiLinestring, MultiPolygon, Segment> tester;


    tester::apply("MULTILINESTRING((2 0,0 2)(0 3,3 0))",
                  "MULTIPOLYGON(((0 0,1 0,0 1,0 0)),\
                  ((0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4)))",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);
}

//===========================================================================
//===========================================================================
//===========================================================================

template <typename Point, typename PSStrategy, typename PBStrategy>
void test_all_l_ar(PSStrategy cp_strategy, PBStrategy sb_strategy)
{
    test_closest_points_segment_polygon_or_ring<Point>(cp_strategy);
    test_closest_points_segment_multi_polygon<Point>(cp_strategy);
    test_closest_points_segment_box<Point>(sb_strategy);

    test_closest_points_linestring_polygon_or_ring<Point>(cp_strategy);
    test_closest_points_linestring_multi_polygon<Point>(cp_strategy);
    //test_closest_points_linestring_box<Point>(cp_strategy);

    test_closest_points_multi_linestring_polygon_or_ring<Point>(cp_strategy);
    test_closest_points_multi_linestring_multi_polygon<Point>(cp_strategy);
    //test_closest_points_multi_linestring_box<Point>(cp_strategy);

    //test_more_empty_input_pointlike_areal<Point>(cp_strategy);
}

BOOST_AUTO_TEST_CASE( test_all_linear_areal )
{
    typedef bg::model::point
            <
                double, 2,
                bg::cs::geographic<bg::degree>
            > geo_point;

    test_all_l_ar<geo_point>(andoyer_cp(), andoyer_sb());
    test_all_l_ar<geo_point>(thomas_cp(), thomas_sb());
    test_all_l_ar<geo_point>(vincenty_cp(), vincenty_sb());
}
