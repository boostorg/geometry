// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2017, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

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

    // case 1
    tester::apply(//"pb1-1a", "SEGMENT(15 25, 16 25)", box1,
                  "pb1-1a", "SEGMENT(0 0, 0 20)", box1,
                  pp_distance("POINT(0 20)", "POINT(10 20)", strategy_pp),
                  //0,
                  strategy_ps);

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
