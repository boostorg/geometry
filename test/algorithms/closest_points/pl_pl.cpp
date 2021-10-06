// Boost.Geometry
// Unit Test

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE test_closest_points_pointlike_pointlike
#endif

#include "common.hpp"
//#include "test_empty_geometry.hpp"

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

    tester::apply("POINT(1 1)",
                  "POINT(1 1)",
                  "SEGMENT(1 1,1 1)",
                  strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_point_multi_point(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "point/multi-point closest_points tests" << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef bg::model::multi_point<Point> MultiPoint;
    typedef test_geometry<Point, MultiPoint, Segment> tester;

    tester::apply("POINT(0 0)",
                  "MULTIPOINT((1 1),(2 0),(0 1),(2 1))",
                  "SEGMENT(0 0,0 1)",
                  strategy);

    tester::apply("POINT(2 0)",
                  "MULTIPOINT((1 1),(2 0),(0 1),(2 1))",
                  "SEGMENT(2 0,2 0)",
                  strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_closest_points_multi_point_multi_point(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "multi-point/multi-point closest_points tests" << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef bg::model::multi_point<Point> MultiPoint;
    typedef test_geometry<MultiPoint, MultiPoint, Segment> tester;

    tester::apply("MULTIPOINT((-1 -1),(0 0))",
                  "MULTIPOINT((1 1),(2 0),(0 1),(2 1))",
                  "SEGMENT(0 0,0 1)",
                  strategy);

    tester::apply("MULTIPOINT((1 1),(0 0))",
                  "MULTIPOINT((1 1),(2 0),(0 1),(2 1))",
                  "SEGMENT(1 1,1 1)",
                  strategy);
}

template <typename Point, typename Strategy>
void test_variant(Strategy const& strategy)
{
    typedef Point point_type;
    typedef bg::model::segment<point_type> segment_type;
    typedef bg::model::multi_point<point_type> mp_type;
    typedef boost::variant<point_type, mp_type> variant_type;

    point_type point1;
    bg::read_wkt("POINT(1 3)", point1);

    point_type point2;
    bg::read_wkt("POINT(2 3)", point2);

    mp_type mpoint;
    bg::read_wkt("MULTIPOINT((1 1),(2 0),(0 1),(2 1))", mpoint);

    variant_type v1, v2;

    v1 = point1;
    v2 = point2;
    segment_type res;
    bg::read_wkt("SEGMENT(1 3,2 3)", res);
    compute_result(v1, v2, res, strategy, false);

    v1 = point1;
    v2 = mpoint;
    bg::read_wkt("SEGMENT(1 3,1 1)", res);
    compute_result(v1, v2, res, strategy, false);

}


//===========================================================================
//===========================================================================
//===========================================================================

template <typename Point, typename Strategy>
void test_all_pl_pl(Strategy pp_strategy)
{
    test_closest_points_point_point<Point>(pp_strategy);
    test_closest_points_point_multi_point<Point>(pp_strategy);
    //test_closest_points_multi_point_multi_point<Point>(pp_strategy);

    //test_variant<Point>(pp_strategy);

    //test_more_empty_input_pointlike_pointlike<Point>(pp_strategy);
}

BOOST_AUTO_TEST_CASE( test_all_pointlike_pointlike )
{
    test_all_pl_pl<car_point>(cartesian_pp());

    //test_all_pl_pl<sph_point>(spherical_pp());
    //test_all_pl_pl<sph_point>(spherical_pp(bg::formula::mean_radius
    //                                       <double>(bg::srs::spheroid<double>())));

    //test_all_pl_pl<geo_point>(andoyer_pp());
    //test_all_pl_pl<geo_point>(thomas_pp());
    //test_all_pl_pl<geo_point>(vincenty_pp());
}