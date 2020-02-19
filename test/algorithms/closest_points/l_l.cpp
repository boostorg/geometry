// Boost.Geometry
// Unit Test

// Copyright (c) 2019, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE test_closest_points_geographic_linear_linear
#endif

#define BOOST_GEOMETRY_TEST_DEBUG

#include "test_closest_points_common.hpp"
#include "test_empty_geometry.hpp"

namespace bg = boost::geometry;

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_segment_segment(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "point/segment closest_points tests" << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef test_geometry<Segment, Segment, Segment> tester;

    tester::apply("SEGMENT(0 2,2 0)",
                  "SEGMENT(0 1,1 0)",
                  "SEGMENT(0.5 1.5,0 1)",
                  "SEGMENT(0.50019 1.50021,0 1)",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_segment_linestring(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "segment/linestring closest_points tests" << std::endl;
#endif

    typedef bg::model::linestring<Point> Linestring;
    typedef bg::model::segment<Point> Segment;
    typedef test_geometry<Segment, Linestring, Segment> tester;

    tester::apply("SEGMENT(0 2,2 0)",
                  "LINESTRING(-1 0,0 1,1 0,-1 -2)",
                  "SEGMENT(0.5 1.5,0 1)",
                  "SEGMENT(0.50019 1.50021,0 1)",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_linestring_linestring(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "linestring/linestring closest_points tests" << std::endl;
#endif

    typedef bg::model::linestring<Point> Linestring;
    typedef bg::model::segment<Point> Segment;
    typedef test_geometry<Linestring, Linestring, Segment> tester;

    tester::apply("LINESTRING(0 2,2 0,3 0,4 0,5 0)",
                  "LINESTRING(-1 0,0 1,1 0,-1 -2)",
                  "SEGMENT(0.5 1.5,0 1)",
                  "SEGMENT(0.50019 1.50021,0 1)",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_segment_multi_linestring(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "segment/multi-linestring closest_points tests" << std::endl;
#endif

    typedef bg::model::linestring<Point> Linestring;
    typedef bg::model::multi_linestring<Linestring> MultiLinestring;
    typedef bg::model::segment<Point> Segment;
    typedef test_geometry<Segment, MultiLinestring, Segment> tester;

    tester::apply("SEGMENT(0 2,2 0)",
                  "MULTILINESTRING((-1 0,0 1,1 0,-1 -2)(0 4,4 0,5 0))",
                  "SEGMENT(0.5 1.5,0 1)",
                  "SEGMENT(0.50019 1.50021,0 1)",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_linestring_multi_linestring(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "linestring /multi-linestring closest_points tests" << std::endl;
#endif

    typedef bg::model::linestring<Point> Linestring;
    typedef bg::model::multi_linestring<Linestring> MultiLinestring;
    typedef bg::model::segment<Point> Segment;
    typedef test_geometry<Linestring, MultiLinestring, Segment> tester;

    tester::apply("LINESTRING(0 2,2 0,2 -1)",
                  "MULTILINESTRING((-1 0,0 1,1 0,-1 -2)(0 4,4 0,5 0))",
                  "SEGMENT(0.5 1.5,0 1)",
                  "SEGMENT(0.50019 1.50021,0 1)",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_multi_linestring_multi_linestring(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "multi-linestring /multi-linestring closest_points tests" << std::endl;
#endif

    typedef bg::model::linestring<Point> Linestring;
    typedef bg::model::multi_linestring<Linestring> MultiLinestring;
    typedef bg::model::segment<Point> Segment;
    typedef test_geometry<MultiLinestring, MultiLinestring, Segment> tester;

    tester::apply("MULTILINESTRING((0 2,2 0,2 -1)(0 2,-1 2))",
                  "MULTILINESTRING((-1 0,0 1,1 0,-1 -2)(0 4,4 0,5 0))",
                  "SEGMENT(0.5 1.5,0 1)",
                  "SEGMENT(0.50019 1.50021,0 1)",
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);
}


//===========================================================================
//===========================================================================
//===========================================================================

template <typename Point, typename Strategy>
void test_all_l_l(Strategy ps_strategy)
{
    test_closest_points_segment_segment<Point>(ps_strategy);
    test_closest_points_segment_linestring<Point>(ps_strategy);
    test_closest_points_linestring_linestring<Point>(ps_strategy);

    test_closest_points_segment_multi_linestring<Point>(ps_strategy);
    test_closest_points_linestring_multi_linestring<Point>(ps_strategy);
    test_closest_points_multi_linestring_multi_linestring<Point>(ps_strategy);

    test_more_empty_input_linear_linear<Point>(ps_strategy);
}

BOOST_AUTO_TEST_CASE( test_all_linear_linear )
{
    test_all_l_l<car_point>(cartesian_ps());

    double radius = bg::formula::mean_radius<double>(bg::srs::spheroid<double>());

    test_all_l_l<sph_point>(spherical_ps());
    test_all_l_l<sph_point>(spherical_ps(radius));

    test_all_l_l<geo_point>(andoyer_ps());
    test_all_l_l<geo_point>(thomas_ps());
    test_all_l_l<geo_point>(vincenty_ps());

}