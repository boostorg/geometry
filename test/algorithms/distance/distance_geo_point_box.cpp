// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2017, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#include <iostream>

#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE test_distance_geographic_point_box
#endif

#include <boost/range.hpp>
#include <boost/type_traits/is_same.hpp>

#include <boost/test/included/unit_test.hpp>
#include <boost/geometry/util/condition.hpp>
#include <boost/geometry/strategies/strategies.hpp>

#include "test_distance_geo_common.hpp"

typedef bg::cs::geographic<bg::degree> cs_type;
typedef bg::model::point<double, 2, cs_type> point_type;
typedef bg::model::multi_point<point_type> multi_point_type;
typedef bg::model::segment<point_type> segment_type;
typedef bg::model::box<point_type> box_type;

namespace services = bg::strategy::distance::services;
typedef bg::default_distance_result<point_type>::type return_type;

typedef bg::srs::spheroid<double> stype;

// Strategies for point-point distance

typedef bg::strategy::distance::andoyer<stype> andoyer_pp;
typedef bg::strategy::distance::thomas<stype> thomas_pp;
typedef bg::strategy::distance::vincenty<stype> vincenty_pp;

// Strategies for point-segment distance

typedef bg::strategy::distance::geographic_cross_track<bg::strategy::andoyer, stype, double>
        andoyer_ps;

typedef bg::strategy::distance::geographic_cross_track<bg::strategy::thomas, stype, double>
        thomas_ps;

typedef bg::strategy::distance::geographic_cross_track<bg::strategy::vincenty, stype, double>
        vincenty_ps;

// Strategies for point-box distance

typedef bg::strategy::distance::geographic_cross_track_point_box
        <
            bg::strategy::andoyer,
            bg::srs::spheroid<double>,
            double
        > andoyer_pb;

typedef bg::strategy::distance::geographic_cross_track_point_box
        <
            bg::strategy::thomas,
            bg::srs::spheroid<double>,
            double
        > thomas_pb;

typedef bg::strategy::distance::geographic_cross_track_point_box
        <
            bg::strategy::vincenty,
            bg::srs::spheroid<double>,
            double
        > vincenty_pb;

//===========================================================================

template <typename Strategy>
inline bg::default_distance_result<point_type>::type
pp_distance(std::string const& wkt1,
            std::string const& wkt2,
            Strategy const& strategy)
{
    point_type p1, p2;
    bg::read_wkt(wkt1, p1);
    bg::read_wkt(wkt2, p2);
    return bg::distance(p1, p2, strategy);
}

template <typename Strategy>
inline bg::default_distance_result<point_type>::type
ps_distance(std::string const& wkt1,
            std::string const& wkt2,
            Strategy const& strategy)
{
    point_type p;
    segment_type s;
    bg::read_wkt(wkt1, p);
    bg::read_wkt(wkt2, s);
    return bg::distance(p, s, strategy);
}

//===========================================================================
// Cases for relative location of a point wrt to a box
//
//           |         |
//           |    3    |
//           |         |
//           +---------+
//           |         |
//       1   |    5    |   2
//           |         |
//           +---------+
//           |         |
//           |    4    |
//           |         |
//
// and also the following cases
//
//           |         |
//           A         B
//           |         |
//           +----C----+
//           |         |
//           D         E
//           |         |
//           +----F----+
//           |         |
//           G         H
//           |         |
//
// and finally we have the corners
//
//           |         |
//           |         |
//           |         |
//           a---------b
//           |         |
//           |         |
//           |         |
//           c---------d
//           |         |
//           |         |
//           |         |
//
// for each relative position we also have to test the shifted point
// (this is due to the fact that boxes have longitudes in the
// range [-180, 540)
//===========================================================================

template <typename Strategy_pp, typename Strategy_ps, typename Strategy_pb>
void test_distance_point_box(Strategy_pp const& strategy_pp,
                             Strategy_ps const& strategy_ps,
                             Strategy_pb const& strategy_pb)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "point/box distance tests" << std::endl;
#endif
    typedef test_distance_of_geometries<point_type, box_type> tester;

    std::string const box1 = "BOX(10 10,20 20)";

    // case 1
    tester::apply("pb1-1a", "POINT(5 25)", box1,
                  pp_distance("POINT(5 25)", "POINT(10 20)", strategy_pp),
                  strategy_pb);

    // case 1
    tester::apply("pb1-1b", "POINT(3 12)", box1,
                  ps_distance("POINT(3 12)", "SEGMENT(10 10,10 20)", strategy_ps),
                  strategy_pb);

    // case 1
    tester::apply("pb1-1c", "POINT(3 17)", box1,
                  ps_distance("POINT(3 17)", "SEGMENT(10 10,10 20)", strategy_ps),
                  strategy_pb);

    // case 1
    tester::apply("pb1-1d", "POINT(5 4)", box1,
                  pp_distance("POINT(5 4)", "POINT(10 10)", strategy_pp),
                  strategy_pb);

    // case 1
    tester::apply("pb1-1e", "POINT(-100 20)", box1,
                  pp_distance("POINT(-100 20)", "POINT(10 20)", strategy_pp),
                  strategy_pb);

    // case 1
    tester::apply("pb1-1g", "POINT(-100 10)", box1,
                  ps_distance("POINT(-100 10)", "SEGMENT(10 10,10 20)", strategy_ps),
                  strategy_pb);

    // case 2
    tester::apply("pb1-2a", "POINT(31 25)", box1,
                  pp_distance("POINT(31 25)", "POINT(20 20)", strategy_pp),
                  strategy_pb);

    // case 2
    tester::apply("pb1-2b", "POINT(23 17)", box1,
                  ps_distance("POINT(23 17)", "SEGMENT(20 10,20 20)", strategy_ps),
                  strategy_pb);

    // case 2
    tester::apply("pb1-2c", "POINT(29 3)", box1,
                  pp_distance("POINT(29 3)", "POINT(20 10)", strategy_pp),
                  strategy_pb);

    // case 2
    tester::apply("pb1-2d", "POINT(131 65)", box1,
                  pp_distance("POINT(131 65)", "POINT(20 20)", strategy_pp),
                  strategy_pb);

    // case 2
    tester::apply("pb1-2e", "POINT(110 10)", box1,
                  ps_distance("POINT(110 10)", "SEGMENT(20 10,20 20)", strategy_ps),
                  strategy_pb);

    // case 2
    tester::apply("pb1-2f", "POINT(150 20)", box1,
                  pp_distance("POINT(150 20)", "POINT(20 20)", strategy_pp),
                  strategy_pb);

    // case 3
    tester::apply("pb1-3a", "POINT(11 25)", box1,
                  pp_distance("POINT(11 25)", "POINT(11 20)", strategy_pp),
                  strategy_pb);

    // case 3
    tester::apply("pb1-3b", "POINT(15 25)", box1,
                  pp_distance("POINT(15 25)", "POINT(15 20)", strategy_pp),
                  strategy_pb);

    // case 3
    tester::apply("pb1-3c", "POINT(18 25)", box1,
                  pp_distance("POINT(18 25)", "POINT(18 20)", strategy_pp),
                  strategy_pb);

    // case 4
    tester::apply("pb1-4a", "POINT(13 4)", box1,
                  pp_distance("POINT(13 4)", "POINT(13 10)", strategy_pp),
                  strategy_pb);

    // case 4
    tester::apply("pb1-4b", "POINT(19 4)", box1,
                  pp_distance("POINT(19 4)", "POINT(19 10)", strategy_pp),
                  strategy_pb);

    // case 5
    tester::apply("pb1-5", "POINT(15 14)", box1, 0, strategy_pb);

    // case A
    tester::apply("pb1-A", "POINT(10 28)", box1,
                  pp_distance("POINT(10 28)", "POINT(10 20)", strategy_pp),
                  strategy_pb);

    // case B
    tester::apply("pb1-B", "POINT(20 28)", box1,
                  pp_distance("POINT(20 28)", "POINT(20 20)", strategy_pp),
                  strategy_pb);


    // case C
    tester::apply("pb1-C", "POINT(14 20)", box1, 0, strategy_pb);

    // case D
    tester::apply("pb1-D", "POINT(10 17)", box1, 0, strategy_pb);

    // case E
    tester::apply("pb1-E", "POINT(20 11)", box1, 0, strategy_pb);

    // case F
    tester::apply("pb1-F", "POINT(19 10)", box1, 0, strategy_pb);

    // case G
    tester::apply("pb1-G", "POINT(10 -40)", box1,
                  pp_distance("POINT(10 -40)", "POINT(10 10)", strategy_pp),
                  strategy_pb);

    // case H
    tester::apply("pb1-H", "POINT(20 -50)", box1,
                  pp_distance("POINT(20 -50)", "POINT(20 10)", strategy_pp),
                  strategy_pb);

    // case a
    tester::apply("pb1-a", "POINT(10 20)", box1, 0, strategy_pb);
    // case b
    tester::apply("pb1-b", "POINT(20 20)", box1, 0, strategy_pb);
    // case c
    tester::apply("pb1-c", "POINT(10 10)", box1, 0, strategy_pb);
    // case d
    tester::apply("pb1-d", "POINT(20 10)", box1, 0, strategy_pb);


    std::string const box2 = "BOX(170 -60,400 80)";

    // case 1 - point is closer to western meridian
    tester::apply("pb2-1a", "POINT(160 0)", box2,
                  ps_distance("POINT(160 0)", "SEGMENT(170 -60,170 80)", strategy_ps),
                  strategy_pb);

    // case 1 - point is closer to eastern meridian
    tester::apply("pb2-1b", "POINT(50 0)", box2,
                  ps_distance("POINT(50 0)", "SEGMENT(40 -60,40 80)", strategy_ps),
                  strategy_pb);

    // case 3 - equivalent point POINT(390 85) is above the box
    tester::apply("pb2-3", "POINT(30 85)", box2,
                  pp_distance("POINT(30 85)", "POINT(30 80)", strategy_pp),
                  strategy_pb);

    // case 4 - equivalent point POINT(390 -75) is below the box
    tester::apply("pb2-4", "POINT(30 -75)", box2,
                  pp_distance("POINT(30 -75)", "POINT(30 -60)", strategy_pp),
                  strategy_pb);

    // case 5 - equivalent point POINT(390 0) is inside box
    tester::apply("pb2-5", "POINT(30 0)", box2, 0, strategy_pb);


    std::string const box3 = "BOX(-150 -50,-40 70)";

    // case 1 - point is closer to western meridian
    tester::apply("pb3-1a", "POINT(-170 10)", box3,
                  ps_distance("POINT(-170 10)", "SEGMENT(-150 -50,-150 70)", strategy_ps),
                  strategy_pb);

    // case 2 - point is closer to eastern meridian
    tester::apply("pb3-2a", "POINT(5 10)", box3,
                  ps_distance("POINT(5 10)", "SEGMENT(-40 -50,-40 70)", strategy_ps),
                  strategy_pb);

    // case 2 - point is closer to western meridian
    tester::apply("pb3-2a", "POINT(160 10)", box3,
                  ps_distance("POINT(160 10)", "SEGMENT(-150 -50,-150 70)", strategy_ps),
                  strategy_pb);

    // case 2 - point is at equal distance from eastern and western meridian
    tester::apply("pb3-2c1", "POINT(85 20)", box3,
                  ps_distance("POINT(85 20)", "SEGMENT(-150 -50,-150 70)", strategy_ps),
                  strategy_pb);

    // case 2 - point is at equal distance from eastern and western meridian
    tester::apply("pb3-2c2", "POINT(85 20)", box3,
                  ps_distance("POINT(85 20)", "SEGMENT(-40 -50,-40 70)", strategy_ps),
                  strategy_pb);

    // box that is symmetric wrt the prime meridian
    std::string const box4 = "BOX(-75 -45,75 65)";

    // case 1 - point is closer to western meridian
    tester::apply("pb4-1a", "POINT(-100 10)", box4,
                  ps_distance("POINT(-100 10)", "SEGMENT(-75 -45,-75 65)", strategy_ps),
                  strategy_pb);

    // case 2 - point is closer to eastern meridian
    tester::apply("pb4-2a", "POINT(90 15)", box4,
                  ps_distance("POINT(90 15)", "SEGMENT(75 -45,75 65)", strategy_ps),
                  strategy_pb);

    // case 2 - point is at equal distance from eastern and western meridian
    tester::apply("pb4-2c1", "POINT(-180 20)", box4,
                  ps_distance("POINT(-180 20)", "SEGMENT(-75 -45,-75 65)", strategy_ps),
                  strategy_pb);

    // case 2 - point is at equal distance from eastern and western meridian
    tester::apply("pb4-2c2", "POINT(-180 20)", box4,
                  ps_distance("POINT(-180 20)", "SEGMENT(75 -45,75 65)", strategy_ps),
                  strategy_pb);
}

template <typename Strategy_pp, typename Strategy_ps, typename Strategy_pb>
void test_distance_point_deg_box(Strategy_pp const& strategy_pp,
                                 Strategy_ps const& strategy_ps,
                                 Strategy_pb const& strategy_pb)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "point/box distance tests" << std::endl;
#endif
    typedef test_distance_of_geometries<point_type, box_type> tester;

    //box degenerates to a meridian segment
    std::string const box1 = "BOX(0 10,0 20)";

    tester::apply("pbd1", "POINT(1 10)", box1,
                  ps_distance("POINT(1 10)", "SEGMENT(0 10, 0 20)", strategy_ps),
                  strategy_pb);
    tester::apply("pbd2", "POINT(1 5)", box1,
                  ps_distance("POINT(1 5)", "SEGMENT(0 10, 0 20)", strategy_ps),
                  strategy_pb);
    tester::apply("pbd3", "POINT(1 15)", box1,
                  ps_distance("POINT(1 15)", "SEGMENT(0 10, 0 20)", strategy_ps),
                  strategy_pb);
    tester::apply("pbd4", "POINT(1 25)", box1,
                  ps_distance("POINT(1 25)", "SEGMENT(0 10, 0 20)", strategy_ps),
                  strategy_pb);

    //box degenerates to a horizontal line; that is not a geodesic segment
    std::string const box2 = "BOX(10 10,20 10)";

    tester::apply("pbd5", "POINT(15 15)", box2,
                  pp_distance("POINT(15 15)", "POINT(15 10)", strategy_pp),
                  strategy_pb);
    tester::apply("pbd6", "POINT(5 15)", box2,
                  pp_distance("POINT(5 15)", "POINT(10 10)", strategy_pp),
                  strategy_pb);
    tester::apply("pbd7", "POINT(25 15)", box2,
                  pp_distance("POINT(25 15)", "POINT(20 10)", strategy_pp),
                  strategy_pb);

    //box degenerates to a point
    std::string const box3 = "BOX(0 10,0 10)";

    tester::apply("pbd8", "POINT(1 11)", box3,
                  pp_distance("POINT(1 11)", "POINT(0 10)", strategy_pp),
                  strategy_pb);
}

template <typename Strategy_pp, typename Strategy_ps, typename Strategy_pb>
void test_distance_multipoint_box(Strategy_pp const& strategy_pp,
                                  Strategy_ps const& strategy_ps,
                                  Strategy_pb const& strategy_pb)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "multipoint/box distance tests" << std::endl;
#endif
    typedef test_distance_of_geometries<multi_point_type, box_type> tester;

    std::string const box1 = "BOX(10 10,20 20)";

    tester::apply("mpb1-1a", "MULTIPOINT(5 25,25 26)", box1,
                  pp_distance("POINT(5 25)", "POINT(10 20)", strategy_pp),
                  strategy_pb);

    tester::apply("mpb1-2e", "MULTIPOINT(110 10,110 9,110 0)", box1,
                  ps_distance("POINT(110 10)", "SEGMENT(20 10,20 20)", strategy_ps),
                  strategy_pb);
}

//===========================================================================
//===========================================================================
//===========================================================================

BOOST_AUTO_TEST_CASE( test_all_point_segment )
{
    test_distance_point_box(vincenty_pp(), vincenty_ps(), vincenty_pb());
    test_distance_point_box(thomas_pp(), thomas_ps(), thomas_pb());
    test_distance_point_box(andoyer_pp(), andoyer_ps(), andoyer_pb());

    test_distance_point_deg_box(vincenty_pp(), vincenty_ps(), vincenty_pb());
    test_distance_point_deg_box(thomas_pp(), thomas_ps(), thomas_pb());
    test_distance_point_deg_box(andoyer_pp(), andoyer_ps(), andoyer_pb());

    test_distance_multipoint_box(vincenty_pp(), vincenty_ps(), vincenty_pb());
    test_distance_multipoint_box(thomas_pp(), thomas_ps(), thomas_pb());
    test_distance_multipoint_box(andoyer_pp(), andoyer_ps(), andoyer_pb());
}
