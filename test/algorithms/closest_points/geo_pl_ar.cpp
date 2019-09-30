// Boost.Geometry
// Unit Test

// Copyright (c) 2019, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE test_closest_points_geographic_pointlike_areal
#endif

#define BOOST_GEOMETRY_TEST_DEBUG

#include "test_closest_points_common.hpp"

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
//===========================================================================
//===========================================================================

template <typename Point, typename Strategy>
void test_all_pl_ar(Strategy cp_strategy)
{
    test_closest_points_point_polygon_or_ring<Point>(cp_strategy);
    test_closest_points_point_multi_polygon<Point>(cp_strategy);
    //test_closest_points_point_box<Point>(cp_strategy);

    test_closest_points_multi_point_polygon_or_ring<Point>(cp_strategy);
    test_closest_points_multi_point_multi_polygon<Point>(cp_strategy);
    //test_closest_points_multi_point_box<Point>(cp_strategy);

    //test_more_empty_input_pointlike_areal<Point>(cp_strategy);
}

BOOST_AUTO_TEST_CASE( test_all_pointlike_areal )
{
    typedef bg::model::point
            <
                double, 2,
                bg::cs::geographic<bg::degree>
            > geo_point;

    test_all_pl_ar<geo_point>(andoyer_cp());
    test_all_pl_ar<geo_point>(thomas_cp());
    test_all_pl_ar<geo_point>(vincenty_cp());
}


