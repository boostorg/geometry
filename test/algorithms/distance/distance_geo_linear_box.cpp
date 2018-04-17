// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2017, 2018 Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#define BOOST_GEOMETRY_TEST_DEBUG

#include <iostream>

#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE test_distance_geographic_linear_box
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
            stype,
            double
        > andoyer_pb;

typedef bg::strategy::distance::geographic_cross_track_point_box
        <
            bg::strategy::thomas,
            stype,
            double
        > thomas_pb;

typedef bg::strategy::distance::geographic_cross_track_point_box
        <
            bg::strategy::vincenty,
            stype,
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


template <typename Strategy_pp, typename Strategy_ps, typename Strategy_pb>
void test_distance_segment_box(Strategy_pp const& strategy_pp,
                               Strategy_ps const& strategy_ps,
                               Strategy_pb const& strategy_pb)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "segment/box distance tests" << std::endl;
#endif
    typedef test_distance_of_geometries<segment_type, box_type> tester;

    std::string const box_north = "BOX(10 10,20 20)";

    tester::apply("sb1-1a", "SEGMENT(0 0, 0 20)", box_north,
                  pp_distance("POINT(0 20)", "POINT(10 20)", strategy_pp),
                  strategy_ps);
    //segment with slope
    tester::apply("sb1-1b", "SEGMENT(10 5, 20 6)", box_north,
                  pp_distance("POINT(20 6)", "POINT(20 10)", strategy_pp),
                  strategy_ps);
    tester::apply("sb1-2", "SEGMENT(0 0, 0 10)", box_north,
                  ps_distance("POINT(0 10)", "SEGMENT(10 10,10 20)", strategy_ps),
                  strategy_ps);
    tester::apply("sb1-3", "SEGMENT(0 0, 0 15)", box_north,
                  ps_distance("POINT(0 15)", "SEGMENT(10 10,10 20)", strategy_ps),
                  strategy_ps);
    tester::apply("sb1-4", "SEGMENT(0 0, 0 25)", box_north,
                  ps_distance("POINT(10 20)", "SEGMENT(0 0,0 25)", strategy_ps),
                  strategy_ps);
    tester::apply("sb1-5", "SEGMENT(0 10, 0 25)", box_north,
                  ps_distance("POINT(10 20)", "SEGMENT(0 0,0 25)", strategy_ps),
                  strategy_ps);

    tester::apply("sb2-2", "SEGMENT(0 5, 15 5)", box_north,
                  ps_distance("POINT(10 10)", "SEGMENT(0 5,15 5)", strategy_ps),
                  strategy_ps);
    tester::apply("sb2-3a", "SEGMENT(0 5, 20 5)", box_north,
                  ps_distance("POINT(10 10)", "SEGMENT(0 5,20 5)", strategy_ps),
                  strategy_ps);

    // Test segments below box
    tester::apply("test_b1", "SEGMENT(0 5, 9 5)", box_north,
                  ps_distance("POINT(10 10)", "SEGMENT(0 5, 9 5)", strategy_ps),
                  strategy_ps);
    tester::apply("test_b2", "SEGMENT(0 5, 10 5)", box_north,
                  ps_distance("POINT(10 10)", "SEGMENT(0 5, 10 5)", strategy_ps),
                  strategy_ps);
    tester::apply("test_b3", "SEGMENT(0 5, 11 5)", box_north,
                  ps_distance("POINT(10 10)", "SEGMENT(0 5, 11 5)", strategy_ps),
                  strategy_ps);
    tester::apply("test_b4", "SEGMENT(0 5, 20 5)", box_north,
                  ps_distance("POINT(10 10)", "SEGMENT(0 5,20 5)", strategy_ps),
                  strategy_ps);
    tester::apply("test_b5", "SEGMENT(0 5, 22 5)", box_north,
                  ps_distance("POINT(11 10)", "SEGMENT(0 5,22 5)", strategy_ps),
                  strategy_ps);
    tester::apply("test_b6", "SEGMENT(10 5, 20 5)", box_north,
                  ps_distance("POINT(15 10)", "SEGMENT(10 5,20 5)", strategy_ps),
                  strategy_ps);
    tester::apply("test_b7", "SEGMENT(10 5, 22 5)", box_north,
                  ps_distance("POINT(16 10)", "SEGMENT(10 5,22 5)", strategy_ps),
                  strategy_ps);
    tester::apply("test_b8", "SEGMENT(12 5, 22 5)", box_north,
                  ps_distance("POINT(17 10)", "SEGMENT(12 5,22 5)", strategy_ps),
                  strategy_ps);
    tester::apply("test_b9", "SEGMENT(18 5, 22 5)", box_north,
                  ps_distance("POINT(20 10)", "SEGMENT(18 5,22 5)", strategy_ps),
                  strategy_ps);
    tester::apply("test_b10", "SEGMENT(18 5, 24 5)", box_north,
                  ps_distance("POINT(20 10)", "SEGMENT(18 5,24 5)", strategy_ps),
                  strategy_ps);
    tester::apply("test_b11", "SEGMENT(20 5, 24 5)", box_north,
                  ps_distance("POINT(20 10)", "SEGMENT(20 5,24 5)", strategy_ps),
                  strategy_ps);
    tester::apply("test_b12", "SEGMENT(22 5, 24 5)", box_north,
                  ps_distance("POINT(20 10)", "SEGMENT(22 5,24 5)", strategy_ps),
                  strategy_ps);
    tester::apply("test_b13", "SEGMENT(0 5, 125 5)", box_north,
                  ps_distance("POINT(20 10)", "SEGMENT(0 5, 125 5)", strategy_ps),
                  strategy_ps);

    // Test segments above box
    tester::apply("test_a1", "SEGMENT(0 25, 9 25)", box_north,
                  ps_distance("POINT(10 20)", "SEGMENT(0 25, 9 25)", strategy_ps),
                  strategy_ps);
    tester::apply("test_a2", "SEGMENT(0 25, 10 25)", box_north,
                  ps_distance("POINT(10 20)", "SEGMENT(0 25, 10 25)", strategy_ps),
                  strategy_ps);
    tester::apply("test_a3", "SEGMENT(0 25, 11 25)", box_north,
                  ps_distance("POINT(11 20)", "SEGMENT(0 25, 11 25)", strategy_ps),
                  strategy_ps);
    tester::apply("test_a4", "SEGMENT(0 25, 20 25)", box_north,
                  ps_distance("POINT(20 20)", "SEGMENT(0 25, 20 25)", strategy_ps),
                  strategy_ps);
    tester::apply("test_a5", "SEGMENT(0 25, 22 25)", box_north,
                  ps_distance("POINT(20 20)", "SEGMENT(0 25, 22 25)", strategy_ps),
                  strategy_ps);
    tester::apply("test_a6", "SEGMENT(10 25, 20 25)", box_north,
                  ps_distance("POINT(20 20)", "SEGMENT(10 25, 20 25)", strategy_ps),
                  strategy_ps);
    tester::apply("test_a7", "SEGMENT(10 25, 22 25)", box_north,
                  ps_distance("POINT(10 20)", "SEGMENT(10 25, 22 25)", strategy_ps),
                  strategy_ps);
    tester::apply("test_a8", "SEGMENT(12 25, 22 25)", box_north,
                  ps_distance("POINT(12 20)", "SEGMENT(12 25, 22 25)", strategy_ps),
                  strategy_ps);
    tester::apply("test_a9", "SEGMENT(18 25, 22 25)", box_north,
                  ps_distance("POINT(18 20)", "SEGMENT(18 25, 22 25)", strategy_ps),
                  strategy_ps);
    tester::apply("test_a10", "SEGMENT(18 25, 24 25)", box_north,
                  ps_distance("POINT(18 20)", "SEGMENT(18 25, 24 25)", strategy_ps),
                  strategy_ps);
    tester::apply("test_a11", "SEGMENT(20 25, 24 25)", box_north,
                  ps_distance("POINT(20 20)", "SEGMENT(20 25, 24 25)", strategy_ps),
                  strategy_ps);
    tester::apply("test_a12", "SEGMENT(22 25, 24 25)", box_north,
                  ps_distance("POINT(20 20)", "SEGMENT(22 25, 24 25)", strategy_ps),
                  strategy_ps);

    // Segments left-right of box
    tester::apply("test_l1", "SEGMENT(0 5, 9 5)", box_north,
                  ps_distance("POINT(10 10)", "SEGMENT(0 5, 9 5)", strategy_ps),
                  strategy_ps);
    tester::apply("test_l2", "SEGMENT(0 10, 9 10)", box_north,
                  ps_distance("POINT(9 10)", "SEGMENT(10 10, 10 20)", strategy_ps),
                  strategy_ps);
    tester::apply("test_l3", "SEGMENT(0 10, 9 15)", box_north,
                  ps_distance("POINT(9 15)", "SEGMENT(10 10, 10 20)", strategy_ps),
                  strategy_ps);
    tester::apply("test_l4", "SEGMENT(0 10, 0 15)", box_north,
                  ps_distance("POINT(0 15)", "SEGMENT(10 10, 10 20)", strategy_ps),
                  strategy_ps);
    tester::apply("test_l5", "SEGMENT(1 10, 0 15)", box_north,
                  ps_distance("POINT(1 10)", "SEGMENT(10 10, 10 20)", strategy_ps),
                  strategy_ps);
    tester::apply("test_l6", "SEGMENT(0 20, 9 21)", box_north,
                  ps_distance("POINT(9 21)", "SEGMENT(10 10, 10 20)", strategy_ps),
                  strategy_ps);
    tester::apply("test_r1", "SEGMENT(21 5, 29 5)", box_north,
                  ps_distance("POINT(20 10)", "SEGMENT(21 5, 29 5)", strategy_ps),
                  strategy_ps);
    tester::apply("test_r2", "SEGMENT(21 10, 29 10)", box_north,
                  ps_distance("POINT(21 10)", "SEGMENT(20 10, 20 20)", strategy_ps),
                  strategy_ps);
    tester::apply("test_r3", "SEGMENT(21 10, 29 15)", box_north,
                  ps_distance("POINT(21 10)", "SEGMENT(20 10, 20 20)", strategy_ps),
                  strategy_ps);
    tester::apply("test_r4", "SEGMENT(21 10, 21 15)", box_north,
                  ps_distance("POINT(21 15)", "SEGMENT(20 10, 20 20)", strategy_ps),
                  strategy_ps);
    tester::apply("test_r5", "SEGMENT(21 10, 22 15)", box_north,
                  ps_distance("POINT(21 10)", "SEGMENT(20 10, 20 20)", strategy_ps),
                  strategy_ps);
    tester::apply("test_r6", "SEGMENT(29 20, 21 21)", box_north,
                  ps_distance("POINT(21 21)", "SEGMENT(20 10, 20 20)", strategy_ps),
                  strategy_ps);

    //Segments on corners of box
    //left-top corner
    //generic
    tester::apply("test_c1", "SEGMENT(9 19.5, 11 21)", box_north,
                  ps_distance("POINT(10 20)", "SEGMENT(9 19.5, 11 21)", strategy_ps),
                  strategy_ps);
    //degenerate
    tester::apply("test_c2", "SEGMENT(9 19, 11 21)", box_north,
                  ps_distance("POINT(10 20)", "SEGMENT(9 19, 11 21)", strategy_ps),
                  strategy_ps);
    //left-bottom corner
    //generic
    tester::apply("test_c3", "SEGMENT(8.5 11, 11 9)", box_north,
                  ps_distance("POINT(10 10)", "SEGMENT(8.5 11, 11 9)", strategy_ps),
                  strategy_ps);
    //degenerate
    tester::apply("test_c4", "SEGMENT(9 11, 11 9)", box_north,
                  0,
                  strategy_ps);
    //right-top corner
    //generic
    tester::apply("test_c5", "SEGMENT(19 21, 21 19.5)", box_north,
                  ps_distance("POINT(20 20)", "SEGMENT(19 21, 21 19.5)", strategy_ps),
                  strategy_ps);
    //degenerate
    tester::apply("test_c6", "SEGMENT(19 21, 21 19)", box_north,
                  ps_distance("POINT(20 20)", "SEGMENT(19 21, 21 19)", strategy_ps),
                  strategy_ps);
    //right-bottom corner
    //generic
    tester::apply("test_c7", "SEGMENT(19 9, 21 10.5)", box_north,
                  ps_distance("POINT(20 10)", "SEGMENT(19 9, 21 10.5)", strategy_ps),
                  strategy_ps);
    tester::apply("test_c7", "SEGMENT(19 9, 21 11)", box_north,
                  0,
                  strategy_ps);

    //Segment and box on different hemispheres
    std::string const box_south = "BOX(10 -20,20 -10)";

    tester::apply("test_ns1", "SEGMENT(10 20, 15 30)", box_south,
                  ps_distance("POINT(10 -10)", "SEGMENT(10 20, 15 30)", strategy_ps),
                  strategy_ps);
    tester::apply("test_ns2", "SEGMENT(0 10, 12 10)", box_south,
                  pp_distance("POINT(12 10)", "POINT(12 -10)", strategy_pp),
                  strategy_ps);
    tester::apply("test_ns3", "SEGMENT(10 10, 20 10)", box_south,
                  pp_distance("POINT(10 10)", "POINT(10 -10)", strategy_pp),
                  strategy_ps);
    tester::apply("test_ns4", "SEGMENT(0 -10, 12 -10)", box_north,
                  pp_distance("POINT(12 10)", "POINT(12 -10)", strategy_pp),
                  strategy_ps);
    tester::apply("test_ns5", "SEGMENT(10 -10, 20 -10)", box_north,
                  pp_distance("POINT(10 -10)", "POINT(10 10)", strategy_pp),
                  strategy_ps);

    //Box crossing equator
    std::string const box_crossing_eq = "BOX(10 -10,20 10)";

    tester::apply("test_cr1", "SEGMENT(10 20, 15 30)", box_crossing_eq,
                  pp_distance("POINT(10 10)", "POINT(10 20)", strategy_pp),
                  strategy_ps);
    tester::apply("test_cr2", "SEGMENT(10 -20, 15 -30)", box_crossing_eq,
                  pp_distance("POINT(10 10)", "POINT(10 20)", strategy_pp),
                  strategy_ps);

    //Box crossing prime meridian

    std::string const box_crossing_mer = "BOX(-10 10,15 20)";

    tester::apply("test_cr3", "SEGMENT(-5 25, 10 30)", box_crossing_mer,
                  pp_distance("POINT(-5 25)", "POINT(-5 20)", strategy_pp),
                  strategy_ps);
    tester::apply("test_cr4", "SEGMENT(-5 5, 10 7)", box_crossing_mer,
                  pp_distance("POINT(10 7)", "POINT(10 10)", strategy_pp),
                  strategy_ps);
    tester::apply("test_cr5", "SEGMENT(-5 5, 10 5)", box_crossing_mer,
                  ps_distance("POINT(2.5 10)", "SEGMENT(-5 5, 10 5)", strategy_ps),
                  strategy_ps);


    //Geometries in south hemisphere
    tester::apply("test_south1", "SEGMENT(10 -30, 15 -30)", box_south,
                  ps_distance("POINT(10 -20)", "SEGMENT(10 -30, 15 -30)", strategy_ps),
                  strategy_ps);


}

//===========================================================================
//===========================================================================
//===========================================================================

BOOST_AUTO_TEST_CASE( test_all_point_segment )
{
    test_distance_segment_box(vincenty_pp(), vincenty_ps(), vincenty_pb());
    test_distance_segment_box(thomas_pp(), thomas_ps(), thomas_pb());
    test_distance_segment_box(andoyer_pp(), andoyer_ps(), andoyer_pb());
}
