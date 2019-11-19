// Boost.Geometry
// Unit Test

// Copyright (c) 2019, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE test_closest_points_geographic_pointlike_linear
#endif

#define BOOST_GEOMETRY_TEST_DEBUG

#include "test_closest_points_common.hpp"
#include "test_empty_geometry.hpp"

namespace bg = boost::geometry;

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_point_segment(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "point/segment closest_points tests" << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef test_geometry<Point, Segment, Segment> tester;

    tester::apply("POINT(1 1)",
                  "SEGMENT(0 1,1 0)",
                  "SEGMENT(1 1,0.503314 0.496737)",
                  strategy);
}

//===========================================================================

template <typename Point>
void test_closest_points_point_segment()
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "point/segment closest_points tests default strategy"
              << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef test_geometry<Point, Segment, Segment> tester;

    tester::apply("POINT(1 1)",
                  "SEGMENT(0 1,1 0)",
                  "SEGMENT(1 1,0.503314 0.496737)");
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_point_segment_diff_spheroid(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "point/segment closest_points tests non-default spheroid"
              << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef test_geometry<Point, Segment, Segment> tester;

    tester::apply("POINT(1 1)",
                  "SEGMENT(0 1,1 0)",
                  "SEGMENT(1 1,0.31099 0.67425)",
                  strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_point_linestring(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "point/linestring closest_points tests" << std::endl;
#endif

    typedef bg::model::linestring<Point> Linestring;
    typedef bg::model::segment<Point> Segment;
    typedef test_geometry<Point, Linestring, Segment> tester;

    tester::apply("POINT(1 1)",
                  "LINESTRING(3 2, 1 3, 0 3, 0 1, 1 0, 0 0)",
                  "SEGMENT(1 1,0.503314 0.496737)",
                  strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_point_multi_linestring(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "point/multi_linestring closest_points tests" << std::endl;
#endif

    typedef bg::model::linestring<Point> Linestring;
    typedef bg::model::multi_linestring<Linestring> MultiLinestring;
    typedef bg::model::segment<Point> Segment;
    typedef test_geometry<Point, MultiLinestring, Segment> tester;

    tester::apply("POINT(1 1)",
                  "MULTILINESTRING((3 2, 1 3, 0 3, 0 1, 1 0, 0 0),(2 0,3 1,4 0))",
                  "SEGMENT(1 1,0.503314 0.496737)",
                  strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_multi_point_segment(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "multi point/segment closest_points tests" << std::endl;
#endif

    typedef bg::model::multi_point<Point> MultiPoint;
    typedef bg::model::segment<Point> Segment;
    typedef test_geometry<MultiPoint, Segment, Segment> tester;

    tester::apply("MULTIPOINT((2 0),(1 1),(1 2))",
                  "SEGMENT(0 1,1 0)",
                  "SEGMENT(1 1,0.503314 0.496737)",
                  strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_multi_point_linestring(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "multi point/linestring closest_points tests" << std::endl;
#endif

    typedef bg::model::multi_point<Point> MultiPoint;
    typedef bg::model::linestring<Point> Linestring;
    typedef bg::model::segment<Point> Segment;
    typedef test_geometry<MultiPoint, Linestring, Segment> tester;

    tester::apply("MULTIPOINT((2 0),(1 1),(3 0))",
                  "LINESTRING(3 2, 1 3, 0 3, 0 1, 1 0, 0 0)",
                  "SEGMENT(1 1,0.503314 0.496737)",
                  strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_multi_point_multi_linestring(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "multi point/multi linestring closest_points tests" << std::endl;
#endif

    typedef bg::model::multi_point<Point> MultiPoint;
    typedef bg::model::linestring<Point> Linestring;
    typedef bg::model::multi_linestring<Linestring> MultiLinestring;
    typedef bg::model::segment<Point> Segment;
    typedef test_geometry<MultiPoint, MultiLinestring, Segment> tester;

    tester::apply("MULTIPOINT((5 0),(1 1),(6 0))",
                  "MULTILINESTRING((3 2, 1 3, 0 3, 0 1, 1 0, 0 0),(2 0,3 1,4 0))",
                  "SEGMENT(1 1,0.503314 0.496737)",
                  strategy);
}

//===========================================================================
//===========================================================================
//===========================================================================

template <typename Point, typename Strategy>
void test_all_pl_l(Strategy cp_strategy)
{
    test_closest_points_point_segment<Point>();
    test_closest_points_point_segment<Point>(cp_strategy);
    test_closest_points_point_linestring<Point>(cp_strategy);
    test_closest_points_point_multi_linestring<Point>(cp_strategy);

    test_closest_points_multi_point_segment<Point>(cp_strategy);
    test_closest_points_multi_point_linestring<Point>(cp_strategy);
    test_closest_points_multi_point_multi_linestring<Point>(cp_strategy);

    test_more_empty_input_pointlike_linear<Point>(cp_strategy);
}

BOOST_AUTO_TEST_CASE( test_all_pointlike_linear )
{
    typedef bg::model::point
            <
                double, 2,
                bg::cs::geographic<bg::degree>
            > geo_point;

    test_all_pl_l<geo_point>(andoyer_cp());
    test_all_pl_l<geo_point>(thomas_cp());
    test_all_pl_l<geo_point>(vincenty_cp());

    typedef bg::srs::spheroid<double> stype;

    test_closest_points_point_segment_diff_spheroid<geo_point>
            (andoyer_cp(stype(5000000,6000000)));
}
