// Boost.Geometry
// Unit Test

// Copyright (c) 2019, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE test_closest_points_geographic_pointlike_linear
#endif

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
                  "SEGMENT(1 1,0.5 0.5)",
                  "SEGMENT(1 1,0.499962 0.500095)",
                  "SEGMENT(1 1,0.503314 0.496737)",
                  strategy);

    tester::apply("POINT(0 1)",
                  "SEGMENT(0 1,1 0)",
                  "SEGMENT(0 1,0 1)",
                  strategy);

    //default strategy
    tester::apply("POINT(1 1)",
                  "SEGMENT(0 1,1 0)",
                  "SEGMENT(1 1,0.5 0.5)",
                  "SEGMENT(1 1,0.499962 0.500095)",
                  "SEGMENT(1 1,0.503314 0.496737)",
                  strategy, true, true);
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

    tester::apply("POINT(0 1)",
                  "SEGMENT(0 1,1 0)",
                  "SEGMENT(0 1,0 1)",
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
                  "SEGMENT(1 1,0.5 0.5)",
                  "SEGMENT(1 1,0.499962 0.500095)",
                  "SEGMENT(1 1,0.503314 0.496737)",
                  strategy);

    tester::apply("POINT(3 2)",
                  "LINESTRING(3 2, 1 3, 0 3, 0 1, 1 0, 0 0)",
                  "SEGMENT(3 2,3 2)",
                  strategy);

    //default strategy
    tester::apply("POINT(3 2)",
                  "LINESTRING(3 2, 1 3, 0 3, 0 1, 1 0, 0 0)",
                  "SEGMENT(3 2,3 2)",
                  strategy, true, true);
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
                  "SEGMENT(1 1,0.5 0.5)",
                  "SEGMENT(1 1,0.499962 0.500095)",
                  "SEGMENT(1 1,0.503314 0.496737)",
                  strategy);

    tester::apply("POINT(1 3)",
                  "MULTILINESTRING((3 2, 1 3, 0 3, 0 1, 1 0, 0 0),(2 0,3 1,4 0))",
                  "SEGMENT(1 3,1 3)",
                  strategy);

    //default strategy
    tester::apply("POINT(1 3)",
                  "MULTILINESTRING((3 2, 1 3, 0 3, 0 1, 1 0, 0 0),(2 0,3 1,4 0))",
                  "SEGMENT(1 3,1 3)",
                  strategy, true, true);
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
                  "SEGMENT(1 1,0.5 0.5)",
                  "SEGMENT(1 1,0.499962 0.500095)",
                  "SEGMENT(1 1,0.503314 0.496737)",
                  strategy);

    tester::apply("MULTIPOINT((0 1),(1 1),(1 2))",
                  "SEGMENT(0 1,1 0)",
                  "SEGMENT(0 1,0 1)",
                  strategy);

    //default strategy
    tester::apply("MULTIPOINT((0 1),(1 1),(1 2))",
                  "SEGMENT(0 1,1 0)",
                  "SEGMENT(0 1,0 1)",
                  strategy, true, true);
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
                  "LINESTRING(4 2,3 2, 1 3, 0 3, 0 1, 1 0, 0 0)",
                  "SEGMENT(1 1,0.5 0.5)",
                  "SEGMENT(1 1,0.499962 0.500095)",
                  "SEGMENT(1 1,0.503314 0.496737)",
                  strategy);

    tester::apply("MULTIPOINT((3 2),(1 1),(3 0))",
                  "LINESTRING(4 2,3 2, 1 3, 0 3, 0 1, 1 0, 0 0)",
                  "SEGMENT(3 2,3 2)",
                  strategy);

    //default strategy
    tester::apply("MULTIPOINT((3 2),(1 1),(3 0))",
                  "LINESTRING(4 2,3 2, 1 3, 0 3, 0 1, 1 0, 0 0)",
                  "SEGMENT(3 2,3 2)",
                  strategy, true, true);
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
                  "SEGMENT(1 1,0.5 0.5)",
                  "SEGMENT(1 1,0.499962 0.500095)",
                  "SEGMENT(1 1,0.503314 0.496737)",
                  strategy);

    tester::apply("MULTIPOINT((5 0),(1 3),(6 0))",
                  "MULTILINESTRING((3 2, 1 3, 0 3, 0 1, 1 0, 0 0),(2 0,3 1,4 0))",
                  "SEGMENT(1 3,1 3)",
                  strategy);

    //default strategy
    tester::apply("MULTIPOINT((5 0),(1 3),(6 0))",
                  "MULTILINESTRING((3 2, 1 3, 0 3, 0 1, 1 0, 0 0),(2 0,3 1,4 0))",
                  "SEGMENT(1 3,1 3)",
                  strategy, true, true);
}


//===========================================================================
//===========================================================================
//===========================================================================

template <typename Point, typename Strategy>
void test_all_pl_l(Strategy ps_strategy)
{
    test_closest_points_point_segment<Point>(ps_strategy);
    test_closest_points_point_linestring<Point>(ps_strategy);
    test_closest_points_point_multi_linestring<Point>(ps_strategy);

    test_closest_points_multi_point_segment<Point>(ps_strategy);
    test_closest_points_multi_point_linestring<Point>(ps_strategy);
    test_closest_points_multi_point_multi_linestring<Point>(ps_strategy);

    test_more_empty_input_pointlike_linear<Point>(ps_strategy);
}

BOOST_AUTO_TEST_CASE( test_all_pointlike_linear )
{
    test_all_pl_l<car_point>(cartesian_ps());

    test_all_pl_l<sph_point>(spherical_ps());
    test_all_pl_l<sph_point>(spherical_ps(bg::formula::mean_radius
                                          <double>(bg::srs::spheroid<double>())));

    test_all_pl_l<geo_point>(andoyer_ps());
    test_all_pl_l<geo_point>(thomas_ps());
    test_all_pl_l<geo_point>(vincenty_ps());

    typedef bg::srs::spheroid<double> stype;

    test_closest_points_point_segment_diff_spheroid<geo_point>
            (andoyer_ps(stype(5000000,6000000)));
}
