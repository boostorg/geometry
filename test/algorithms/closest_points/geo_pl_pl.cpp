// Boost.Geometry
// Unit Test

// Copyright (c) 2019, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE test_closest_points_geographic_pointlike_pointlike
#endif

#define BOOST_GEOMETRY_TEST_DEBUG

#include "test_closest_points_common.hpp"

namespace bg = boost::geometry;

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_point_point(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "point/point closest_points tests" << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef test_geometry<Point, Point, Segment> tester;

    tester::apply("POINT(1 1)",
                  "POINT(0 0)",
                  "SEGMENT(1 1,0 0)",
                  strategy);
}


//===========================================================================
//===========================================================================
//===========================================================================

template <typename Point, typename Strategy>
void test_all_pl_pl(Strategy cp_strategy)
{
    //test_closest_points_point_point<Point>(cp_strategy);
    //test_closest_points_point_multi_point<Point>(cp_strategy);
    //test_closest_points_multi_point_multi_point<Point>(cp_strategy);

    //test_more_empty_input_pointlike_pointlike<Point>(cp_strategy);
}

BOOST_AUTO_TEST_CASE( test_all_pointlike_pointlike )
{
    typedef bg::model::point
            <
                double, 2,
                bg::cs::geographic<bg::degree>
            > geo_point;

    test_all_pl_pl<geo_point>(andoyer_cp());
    test_all_pl_pl<geo_point>(thomas_cp());
    test_all_pl_pl<geo_point>(vincenty_cp());
}

