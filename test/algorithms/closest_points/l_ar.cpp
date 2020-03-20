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
#include "test_empty_geometry.hpp"

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
                  "SEGMENT(1.5 0.5,1 0)",
                  "SEGMENT(0.50019 1.50021,0 1)",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);

    //geometries intersect
    //intersect boundary of ring
    tester::apply("SEGMENT(1 1,0.1 0.1)",
                  "POLYGON((0 0,1 0,0 1,0 0))",
                  "SEGMENT(0.5 0.5,0.5 0.5)",
                  "SEGMENT(0.500004 0.500053,0.500004 0.500053)",
                  strategy);

    //intersect interior of ring
    tester::apply("SEGMENT(0.2 0.2,0.1 0.1)",
                  "POLYGON((0 0,1 0,0 1,0 0))",
                  "SEGMENT(0.2 0.2,0.2 0.2)",
                  strategy);

    typedef test_geometry<Segment, Polygon, Segment> tester2;

    tester2::apply("SEGMENT(2 0,0 2)",
                   "POLYGON((0 0,1 0,0 1,0 0))",
                   "SEGMENT(1.5 0.5,1 0)",
                   "SEGMENT(0.50019 1.50021,0 1)",
                   "SEGMENT(1.496909 0.503379,1 0)",
                   strategy);

    tester2::apply("SEGMENT(2 0,0 2)",
                   "POLYGON((0 0,1 0,0 1,0 0)(0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4))",
                   "SEGMENT(1.5 0.5,1 0)",
                   "SEGMENT(0.50019 1.50021,0 1)",
                   "SEGMENT(1.496909 0.503379,1 0)",
                   strategy);

    //geometries intersect
    //intersect boundary of ring
    tester2::apply("SEGMENT(1 1,0.1 0.1)",
                  "POLYGON((0 0,1 0,0 1,0 0))",
                  "SEGMENT(0.5 0.5,0.5 0.5)",
                  "SEGMENT(0.500004 0.500053,0.500004 0.500053)",
                  strategy);

    //intersect interior of ring
    tester2::apply("SEGMENT(0.2 0.2,0.1 0.1)",
                  "POLYGON((0 0,1 0,0 1,0 0))",
                  "SEGMENT(0.2 0.2,0.2 0.2)",
                  strategy);

    //intersect boundary of interior ring
    tester2::apply("SEGMENT(0.9 0.9,0.1 0.1)",
                  "POLYGON((-1 -1,2 0,0 2,-1 -1)(0 0,0 1,1 0,0 0))",
                   "SEGMENT(0.5 0.5,0.5 0.5)",
                   "SEGMENT(0.500004 0.500053,0.500004 0.500053)",
                  strategy);

    //intersect boundary of exterior ring
    tester2::apply("SEGMENT(0.9 0.9,2 2)",
                   "POLYGON((-1 -1,2 0,0 2,-1 -1)(0 0,0 1,1 0,0 0))",
                   "SEGMENT(1 1,1 1)",
                   "SEGMENT(1.0002 1.00026,1.0002 1.00026)",
                  strategy);

    //intersect interior
    tester2::apply("SEGMENT(0 -0.5,1 -0.5)",
                   "POLYGON((-1 -1,0 2,2 0,-1 -1)(0 0,1 0,0 1,0 0))",
                   "SEGMENT(0.5 -0.5,0.5 -0.5)",
                   "SEGMENT(0.500575 -0.500019,0.500575 -0.500019)",
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
                  "SEGMENT(1.5 0.5,1 0)",
                  "SEGMENT(0.50019 1.50021,0 1)",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);

    //geometries intersect
    //intersect boundary of ring
    tester::apply("SEGMENT(1 1,0.1 0.1)",
                  "MULTIPOLYGON(((0 0,1 0,0 1,0 0)),\
                                ((0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4)))",
                  "SEGMENT(0.5 0.5,0.5 0.5)",
                  "SEGMENT(0.500004 0.500053,0.500004 0.500053)",
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
                  "SEGMENT(1 1,10 10)",
                  "SEGMENT(0.922834 1.07763,10 10)",
                  "SEGMENT(0.983761 1.0167,10 10)",
                  strategy);

    tester::apply("SEGMENT(5 0,5 15)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(5 15,10 15)",
                  "SEGMENT(5 15,10 15.0547)",
                  "SEGMENT(5 15,10 15.055)",
                  strategy);

    tester::apply("SEGMENT(0 5,15 5)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(15 5,15 10)",
                  "SEGMENT(9.98098 5.03848,10 10)",
                  strategy);

    tester::apply("SEGMENT(0 5,20 5)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(20 5,20 10)",
                  "SEGMENT(10 5.07673,10 10)",
                  "SEGMENT(10 5.07725,10 10)",
                  strategy);

    tester::apply("SEGMENT(10 5, 20 6)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(20 6,20 10)",
                  strategy);

    tester::apply("SEGMENT(0 0, 0 10)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(0 10,10 10)",
                  "SEGMENT(0 10,10 10.1511)",
                  "SEGMENT(0 10,10 10.1521)",
                  strategy);

    tester::apply("SEGMENT(0 5, 125 5)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(20 5,20 10)",
                  "SEGMENT(20.2581 7.98472,20 10)",
                  "SEGMENT(20.2533 8.02139,20 10)",
                  strategy);

    // Test segments above box
    tester::apply("SEGMENT(0 25, 9 25)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(9 25,10 20)",
                  strategy);
    tester::apply("SEGMENT(0 25, 10 25)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(10 25,10 20)",
                  strategy);
    tester::apply("SEGMENT(0 25, 11 25)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(11 25,11 20)",
                  strategy);
    tester::apply("SEGMENT(0 25, 22 25)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(10 25,10 20)",
                  "SEGMENT(20.3808 25.1117,20 20)",
                  "SEGMENT(20.3787 25.1124,20 20)",
                  strategy);

    // Segments left-right of box
    tester::apply("SEGMENT(0 5,9 5)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(9 5,10 10)",
                  strategy);
    tester::apply("SEGMENT(21 10, 21 15)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(21 15,20 15)",
                  "SEGMENT(21 15,20 15.0022)",
                  strategy);


    //Segments on corners of box
    //left-top corner
    //generic
    tester::apply("SEGMENT(9 19.5, 11 21)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(9.88 20.16,10 20)",
                  "SEGMENT(9.8677 20.1571,10 20)",
                  strategy);
    //degenerate
    tester::apply("SEGMENT(9 19, 11 21)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(9 19,9 19)",
                  "SEGMENT(9.99514 20.0043,10 20)",
                  strategy);
    //left-bottom corner
    //generic
    tester::apply("SEGMENT(8.5 11, 11 9)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(9.90244 9.87805,10 10)",
                  "SEGMENT(9.90395 9.88218,10 10)",
                  strategy);
    //right-top corner
    //generic
    tester::apply("SEGMENT(19 21, 21 19.5)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(20.12 20.16,20 20)",
                  "SEGMENT(20.1323 20.1571,20 20)",
                  strategy);
    //degenerate
//    tester::apply("SEGMENT(19 21, 21 19)",
//                  "BOX(10 10,20 20)",
//                  "SEGMENT(19 21,19 21)",
//                  "SEGMENT(20.0049 20.0043,20 20)",
//                  strategy);
    //right-bottom corner
    //generic
    tester::apply("SEGMENT(19 9, 21 10.5)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(20.12 9.84,20 10)",
                  "SEGMENT(20.1197 9.84293,20 10)",
                  strategy);
    //Segment and box on different hemispheres (for non-cartesian)
    tester::apply("SEGMENT(10 20, 15 30)",
                  "BOX(10 -20,20 -10)",
                  "SEGMENT(10 20,10 -10)",
                  strategy);
    tester::apply("SEGMENT(0 10, 12 10)",
                  "BOX(10 -20,20 -10)",
                  "SEGMENT(12 10,12 -10)",
                  strategy);
    tester::apply("SEGMENT(10 10, 20 10)",
                  "BOX(10 -20,20 -10)",
                  "SEGMENT(10 10,10 -10)",
                  strategy);
    tester::apply("SEGMENT(0 -10, 12 -10)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(12 -10,12 10)",
                  strategy);
    tester::apply("SEGMENT(10 -10, 20 -10)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(20 -10,20 10)",
                  "SEGMENT(10 -10,10 10)",
                  strategy);
    //Box crossing equator
    tester::apply("SEGMENT(10 20, 15 30)",
                  "BOX(10 -10,20 10)",
                  "SEGMENT(10 20,10 10)",
                  strategy);
    tester::apply("SEGMENT(10 -20, 15 -30)",
                  "BOX(10 -10,20 10)",
                  "SEGMENT(10 -20,10 -10)",
                  strategy);

    //Box crossing prime meridian
    tester::apply("SEGMENT(-5 25, 10 30)",
                  "BOX(-10 10,15 20)",
                  "SEGMENT(-5 25,-5 20)",
                  strategy);
    tester::apply("SEGMENT(-5 5, 10 7)",
                  "BOX(-10 10,15 20)",
                  "SEGMENT(10 7,10 10)",
                  strategy);
    tester::apply("SEGMENT(-5 5, 10 5)",
                  "BOX(-10 10,15 20)",
                  "SEGMENT(10 5,10 10)",
                  "SEGMENT(10 5.04321,10 10)",
                  strategy);
    //Geometries in south hemisphere
    tester::apply("SEGMENT(10 -30, 15 -30)",
                  "BOX(10 -20,20 -10)",
                  "SEGMENT(15 -30,15 -20)",
                  "SEGMENT(10 -30,10 -20)",
                  strategy);
    //Segments in boxes corner
    tester::apply("SEGMENT(17 21, 25 20)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(20.0769 20.6154,20 20)",
                  "SEGMENT(20.0886 20.6593,20 20)",
                  strategy);
    tester::apply("SEGMENT(17 21, 0 20)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(9.96552 20.5862,10 20)",
                  "SEGMENT(9.9552 20.7908,10 20)",
                  strategy);
    tester::apply("SEGMENT(17 5, 0 10)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(9.20382 7.29299,10 10)",
                  "SEGMENT(9.20532 7.38795,10 10)",
                  "SEGMENT(9.21495 7.38567,10 10)",
                  strategy);
    tester::apply("SEGMENT(17 5, 25 9)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(21.4 7.2,20 10)",
                  "SEGMENT(21.4147 7.23283,20 10)",
                  "SEGMENT(21.3999 7.22557,20 10)",
                  strategy);

    //intersection
    tester::apply("SEGMENT(15 15,15 25)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(15 15,15 15)",
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
                  "SEGMENT(1.5 0.5,1 0)",
                  "SEGMENT(0.50019 1.50021,0 1)",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);

    //geometries intersect
    //intersect boundary of ring
    tester::apply("LINESTRING(1 1,0.1 0.1,0.2 0.1)",
                  "POLYGON((0 0,1 0,0 1,0 0))",
                  "SEGMENT(0.5 0.5,0.5 0.5)",
                  "SEGMENT(0.500004 0.500053,0.500004 0.500053)",
                  strategy);

    //intersect interior of ring
    tester::apply("LINESTRING(0.2 0.2,0.1 0.1,0.2 0.1)",
                  "POLYGON((0 0,1 0,0 1,0 0))",
                  "SEGMENT(0.2 0.2,0.2 0.2)",
                  strategy);

    typedef test_geometry<Linestring, Polygon, Segment> tester2;

    tester2::apply("LINESTRING(2 0,0 2)",
                   "POLYGON((0 0,1 0,0 1,0 0))",
                   "SEGMENT(1.5 0.5,1 0)",
                   "SEGMENT(0.50019 1.50021,0 1)",
                   "SEGMENT(1.496909 0.503379,1 0)",
                   strategy);

    tester2::apply("LINESTRING(2 0,0 2)",
                   "POLYGON((0 0,1 0,0 1,0 0)(0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4))",
                   "SEGMENT(1.5 0.5,1 0)",
                   "SEGMENT(0.50019 1.50021,0 1)",
                   "SEGMENT(1.496909 0.503379,1 0)",
                   strategy);

    //geometries intersect
    //intersect boundary of ring
    tester2::apply("LINESTRING(1 1,0.1 0.1,0.1 0.2)",
                   "POLYGON((0 0,1 0,0 1,0 0))",
                   "SEGMENT(0.5 0.5,0.5 0.5)",
                   "SEGMENT(0.500004 0.500053,0.500004 0.500053)",
                   strategy);

    //intersect interior of ring
    tester2::apply("LINESTRING(0.2 0.2,0.1 0.1,0.1 0.2)",
                   "POLYGON((0 0,1 0,0 1,0 0))",
                   "SEGMENT(0.2 0.2,0.2 0.2)",
                   strategy);

    //intersect boundary of interior ring
    tester2::apply("LINESTRING(0.9 0.9,0.1 0.1,0.1 0.2)",
                   "POLYGON((-1 -1,2 0,0 2,-1 -1)(0 0,0 1,1 0,0 0))",
                   "SEGMENT(0.5 0.5,0.5 0.5)",
                   "SEGMENT(0.500004 0.500053,0.500004 0.500053)",
                   strategy);

    //intersect boundary of exterior ring
    tester2::apply("LINESTRING(0.9 0.9,2 2,1 2)",
                   "POLYGON((-1 -1,2 0,0 2,-1 -1)(0 0,0 1,1 0,0 0))",
                   "SEGMENT(1 1,1 1)",
                   "SEGMENT(1.0002 1.00026,1.0002 1.00026)",
                   strategy);

    //intersect interior of polygon
    tester2::apply("LINESTRING(0 -0.5,1 -0.5,1.1 -0.5)",
                   "POLYGON((-1 -1,0 2,2 0,-1 -1)(0 0,1 0,0 1,0 0))",
                   "SEGMENT(0.5 -0.5,0.5 -0.5)",
                   "SEGMENT(0.500575 -0.500019,0.500575 -0.500019)",
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
                  "SEGMENT(1.5 0.5,1 0)",
                  "SEGMENT(0.50019 1.50021,0 1)",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);

    //geometries intersect
    //intersect boundary of ring
    tester::apply("LINESTRING(1 1,0.1 0.1,0.1 0.2)",
                  "MULTIPOLYGON(((0 0,1 0,0 1,0 0)),\
                                ((0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4)))",
                  "SEGMENT(0.5 0.5,0.5 0.5)",
                  "SEGMENT(0.500004 0.500053,0.500004 0.500053)",
                  strategy);

    //intersect boundary of ring
    tester::apply("LINESTRING(1 1,0.1 0.1,0.1 0.2)",
                  "MULTIPOLYGON(((0 0,1 0,0 1,0 0)),\
                                ((0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4)))",
                  "SEGMENT(0.5 0.5,0.5 0.5)",
                  "SEGMENT(0.500004 0.500053,0.500004 0.500053)",
                  strategy);

    //intersect interior of polygon
    tester::apply("LINESTRING(0 -0.5,1 -0.5,1.1 -0.5)",
                  "MULTIPOLYGON(((3 0,3 3,4 3,3 0)),\
                                ((-1 -1,0 2,2 0,-1 -1)(0 0,1 0,0 1,0 0)))",
                  "SEGMENT(0.5 -0.5,0.5 -0.5)",
                  "SEGMENT(0.500575 -0.500019,0.500575 -0.500019)",
                  strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_linestring_box(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "linestring/box closest_points tests" << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef bg::model::linestring<Point> Linestring;
    typedef bg::model::box<Point> Box;
    typedef test_geometry<Linestring, Box, Segment> tester;

    tester::apply("LINESTRING(2 0,0 2,0 0,-1 -1)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(1 1,10 10)",
                  "SEGMENT(0.922834 1.07763,10 10)",
                  "SEGMENT(0.983761 1.0167,10 10)",
                  strategy);

    //intersection
    tester::apply("LINESTRING(15 15,15 25,20 25)",
                  "BOX(10 10,20 20)",
                  "SEGMENT(15 15,15 15)",
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
                  "SEGMENT(1.5 0.5,1 0)",
                  "SEGMENT(0.50019 1.50021,0 1)",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);

    //geometries intersect
    //intersect boundary of ring
    tester::apply("MULTILINESTRING((1 1,0.1 0.1,0.2 0.1)(1 1,2 2,3 3))",
                  "POLYGON((0 0,1 0,0 1,0 0))",
                  "SEGMENT(0.5 0.5,0.5 0.5)",
                  "SEGMENT(0.500004 0.500053,0.500004 0.500053)",
                  strategy);

    //intersect interior of ring
    tester::apply("MULTILINESTRING((0.2 0.2,0.1 0.1,0.2 0.1)(1 1,2 2,3 3))",
                  "POLYGON((0 0,1 0,0 1,0 0))",
                  "SEGMENT(0.2 0.2,0.2 0.2)",
                  strategy);

    typedef test_geometry<MultiLinestring, Polygon, Segment> tester2;

    tester2::apply("MULTILINESTRING((2 0,0 2)(0 3,3 0))",
                   "POLYGON((0 0,1 0,0 1,0 0))",
                   "SEGMENT(1.5 0.5,1 0)",
                   "SEGMENT(0.50019 1.50021,0 1)",
                   "SEGMENT(1.496909 0.503379,1 0)",
                   strategy);

    tester2::apply("MULTILINESTRING((2 0,0 2)(0 3,3 0))",
                   "POLYGON((0 0,1 0,0 1,0 0)(0.4 0.4,0.4 0.1,0.1 0.4,0.4 0.4))",
                   "SEGMENT(1.5 0.5,1 0)",
                   "SEGMENT(0.50019 1.50021,0 1)",
                   "SEGMENT(1.496909 0.503379,1 0)",
                   strategy);

    //geometries intersect
    //intersect boundary of ring
    tester2::apply("MULTILINESTRING((1 1,2 2,3 4)(1 1,0.1 0.1,0.1 0.2))",
                   "POLYGON((0 0,1 0,0 1,0 0))",
                   "SEGMENT(0.5 0.5,0.5 0.5)",
                   "SEGMENT(0.500004 0.500053,0.500004 0.500053)",
                   strategy);

    //intersect interior of ring
    tester2::apply("MULTILINESTRING((1 1,2 2,3 4)(0.2 0.2,0.1 0.1,0.1 0.2))",
                   "POLYGON((0 0,1 0,0 1,0 0))",
                   "SEGMENT(0.2 0.2,0.2 0.2)",
                   strategy);

    //intersect boundary of interior ring
    tester2::apply("MULTILINESTRING((2 2,3 4,4 4)(0.9 0.9,0.1 0.1,0.1 0.2))",
                   "POLYGON((-1 -1,2 0,0 2,-1 -1)(0 0,0 1,1 0,0 0))",
                   "SEGMENT(0.5 0.5,0.5 0.5)",
                   "SEGMENT(0.500004 0.500053,0.500004 0.500053)",
                   strategy);

    //intersect boundary of exterior ring
    tester2::apply("MULTILINESTRING((2 2,3 4,4 4)(0.9 0.9,2 2,1 2))",
                   "POLYGON((-1 -1,2 0,0 2,-1 -1)(0 0,0 1,1 0,0 0))",
                   "SEGMENT(1 1,1 1)",
                   "SEGMENT(1.0002 1.00026,1.0002 1.00026)",
                   strategy);

    //intersect interior of polygon
    tester2::apply("MULTILINESTRING((2 2,3 4,4 4)(0 -0.5,1 -0.5,1.1 -0.5))",
                   "POLYGON((-1 -1,0 2,2 0,-1 -1)(0 0,1 0,0 1,0 0))",
                   "SEGMENT(0.5 -0.5,0.5 -0.5)",
                   "SEGMENT(0.500575 -0.500019,0.500575 -0.500019)",
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
                  "SEGMENT(1.5 0.5,1 0)",
                  "SEGMENT(0.50019 1.50021,0 1)",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_multi_linestring_box(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "multi-linestring/polygon or ring closest_points tests"
              << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef bg::model::linestring<Point> Linestring;
    typedef bg::model::multi_linestring<Linestring> MultiLinestring;
    typedef bg::model::box<Point> Box;

    typedef test_geometry<MultiLinestring, Box, Segment> tester;

    tester::apply("MULTILINESTRING((2 0,0 2,0 0,-1 -1)(-1 0,-1 2,0 5))",
                  "BOX(10 10,20 20)",
                  "SEGMENT(0 5,10 10)",
                  strategy);
}

//===========================================================================
//===========================================================================
//===========================================================================

template <typename Point, typename PSStrategy, typename PBStrategy>
void test_all_l_ar(PSStrategy ps_strategy, PBStrategy sb_strategy)
{
    test_closest_points_segment_polygon_or_ring<Point>(ps_strategy);
    test_closest_points_segment_multi_polygon<Point>(ps_strategy);
    test_closest_points_segment_box<Point>(sb_strategy);

    test_closest_points_linestring_polygon_or_ring<Point>(ps_strategy);
    test_closest_points_linestring_multi_polygon<Point>(ps_strategy);
    //test_closest_points_linestring_box<Point>(sb_strategy);

    test_closest_points_multi_linestring_polygon_or_ring<Point>(ps_strategy);
    //test_closest_points_multi_linestring_multi_polygon<Point>(ps_strategy);
    //test_closest_points_multi_linestring_box<Point>(sb_strategy);

    //test_more_empty_input_pointlike_areal<Point>(ps_strategy);

}

BOOST_AUTO_TEST_CASE( test_all_linear_areal )
{
    test_all_l_ar<car_point>(cartesian_ps(), cartesian_sb());

    double radius = bg::formula::mean_radius<double>(bg::srs::spheroid<double>());

    test_all_l_ar<sph_point>(spherical_ps(), spherical_sb());
    test_all_l_ar<sph_point>(spherical_ps(radius), spherical_sb(radius));

    test_all_l_ar<geo_point>(andoyer_ps(), andoyer_sb());
    test_all_l_ar<geo_point>(thomas_ps(), thomas_sb());
    test_all_l_ar<geo_point>(vincenty_ps(), vincenty_sb());
}
