// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2017, Oracle and/or its affiliates.

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

    std::string const box1 = "BOX(10 10,20 20)";

    tester::apply("sb1-1", "SEGMENT(0 0, 0 20)", box1,
                  pp_distance("POINT(0 20)", "POINT(10 20)", strategy_pp),
                  strategy_ps);
    tester::apply("sb1-2", "SEGMENT(0 0, 0 10)", box1,
                  ps_distance("POINT(0 10)", "SEGMENT(10 10,10 20)", strategy_ps),
                  strategy_ps);
    tester::apply("sb1-3", "SEGMENT(0 0, 0 15)", box1,
                  ps_distance("POINT(0 15)", "SEGMENT(10 10,10 20)", strategy_ps),
                  strategy_ps);
    tester::apply("sb1-4", "SEGMENT(0 0, 0 25)", box1,
                  ps_distance("POINT(10 20)", "SEGMENT(0 0,0 25)", strategy_ps),
                  strategy_ps);
    tester::apply("sb1-5", "SEGMENT(0 10, 0 25)", box1,
                  ps_distance("POINT(10 20)", "SEGMENT(0 0,0 25)", strategy_ps),
                  strategy_ps);

    tester::apply("sb2-2", "SEGMENT(0 5, 15 5)", box1,
                  ps_distance("POINT(10 10)", "SEGMENT(0 5,15 5)", strategy_ps),
                  strategy_ps);
    tester::apply("sb2-3a", "SEGMENT(0 5, 20 5)", box1,
                  ps_distance("POINT(10 10)", "SEGMENT(0 5,20 5)", strategy_ps),
                  strategy_ps);

    tester::apply("test1", "SEGMENT(0 5, 9 5)", box1,
                  ps_distance("POINT(10 10)", "SEGMENT(0 5, 9 5)", strategy_ps),
                  strategy_ps);
    tester::apply("test2", "SEGMENT(0 5, 10 5)", box1,
                  ps_distance("POINT(10 10)", "SEGMENT(0 5, 10 5)", strategy_ps),
                  strategy_ps);
    tester::apply("test3", "SEGMENT(0 5, 11 5)", box1,
                  ps_distance("POINT(10 10)", "SEGMENT(0 5, 11 5)", strategy_ps),
                  strategy_ps);
    tester::apply("test4", "SEGMENT(0 5, 20 5)", box1,
                  ps_distance("POINT(10 10)", "SEGMENT(0 5,20 5)", strategy_ps),
                  strategy_ps);
    tester::apply("test5", "SEGMENT(0 5, 22 5)", box1,
                  ps_distance("POINT(11 10)", "SEGMENT(0 5,22 5)", strategy_ps),
                  strategy_ps);
    tester::apply("test6", "SEGMENT(10 5, 20 5)", box1,
                  ps_distance("POINT(15 10)", "SEGMENT(10 5,20 5)", strategy_ps),
                  strategy_ps);
    tester::apply("test7", "SEGMENT(10 5, 22 5)", box1,
                  ps_distance("POINT(16 10)", "SEGMENT(10 5,22 5)", strategy_ps),
                  strategy_ps);
    tester::apply("test8", "SEGMENT(12 5, 22 5)", box1,
                  ps_distance("POINT(17 10)", "SEGMENT(12 5,22 5)", strategy_ps),
                  strategy_ps);
    tester::apply("test9", "SEGMENT(18 5, 22 5)", box1,
                  ps_distance("POINT(20 10)", "SEGMENT(18 5,22 5)", strategy_ps),
                  strategy_ps);
    tester::apply("test10", "SEGMENT(18 5, 24 5)", box1,
                  ps_distance("POINT(20 10)", "SEGMENT(18 5,24 5)", strategy_ps),
                  strategy_ps);
    tester::apply("test11", "SEGMENT(20 5, 24 5)", box1,
                  ps_distance("POINT(20 10)", "SEGMENT(20 5,24 5)", strategy_ps),
                  strategy_ps);
    tester::apply("test12", "SEGMENT(22 5, 24 5)", box1,
                  ps_distance("POINT(20 10)", "SEGMENT(22 5,24 5)", strategy_ps),
                  strategy_ps);
    /*
    tester::apply("sb2-3b", "SEGMENT(0 5, 25 5)", box1,
                  ps_distance("POINT(15 10)", "SEGMENT(0 5,25 5)", strategy_ps),
                  strategy_ps);
    tester::apply("sb2-3c", "SEGMENT(0 5, 26 5)", box1,
                  ps_distance("POINT(13 10)", "SEGMENT(0 5,26 5)", strategy_ps),
                  strategy_ps);
    */
    //tester::apply("sb2-3", "SEGMENT(0 5, 25 6)", box1,
    //              ps_distance("POINT(12.5 10)", "SEGMENT(0 5,25 6)", strategy_ps),
    //              //pp_distance("POINT(10 10)", "POINT(10 5)", strategy_pp),
    //              strategy_ps);

}

//===========================================================================
//===========================================================================
//===========================================================================

BOOST_AUTO_TEST_CASE( test_all_point_segment )
{
    test_distance_segment_box(vincenty_pp(), vincenty_ps(), vincenty_pb());
    //test_distance_segment_box(thomas_pp(), thomas_ps(), thomas_pb());
    test_distance_segment_box(andoyer_pp(), andoyer_ps(), andoyer_pb());
}
