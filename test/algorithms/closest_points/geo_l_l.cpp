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
                  "SEGMENT(1.496909 0.503379,1 0)",
                  strategy);
}

//===========================================================================
//===========================================================================
//===========================================================================

template <typename Point, typename Strategy>
void test_all_l_l(Strategy cp_strategy)
{
    test_closest_points_segment_segment<Point>(cp_strategy);
    test_closest_points_segment_linestring<Point>(cp_strategy);

    test_more_empty_input_linear_linear<Point>(cp_strategy);
}

BOOST_AUTO_TEST_CASE( test_all_linear_linear )
{
    typedef bg::model::point
            <
                double, 2,
                bg::cs::geographic<bg::degree>
            > geo_point;

    test_all_l_l<geo_point>(andoyer_cp());
    test_all_l_l<geo_point>(thomas_cp());
    test_all_l_l<geo_point>(vincenty_cp());
}
