// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2016-2017, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#include <iostream>

#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE test_distance_geographic_pl_l
#endif

#include <boost/geometry/core/srs.hpp>
#include <boost/test/included/unit_test.hpp>

#include "test_distance_geo_common.hpp"

typedef bg::cs::geographic<bg::degree> cs_type;
typedef bg::model::point<double, 2, cs_type> point_type;
typedef bg::model::segment<point_type> segment_type;
typedef bg::model::multi_point<point_type> multi_point_type;
typedef bg::model::segment<point_type> segment_type;
typedef bg::model::linestring<point_type> linestring_type;
typedef bg::model::multi_linestring<linestring_type> multi_linestring_type;

namespace services = bg::strategy::distance::services;
typedef bg::default_distance_result<point_type>::type return_type;

typedef bg::srs::spheroid<double> stype;

// Strategies for point-point distance

typedef bg::strategy::distance::andoyer<stype> andoyer_pp;
typedef bg::strategy::distance::thomas<stype> thomas_pp;
typedef bg::strategy::distance::vincenty<stype> vincenty_pp;

// Strategies for point-segment distance

typedef bg::strategy::distance::cross_track_geo<bg::strategy::andoyer, stype, double>
        andoyer_strategy;

typedef bg::strategy::distance::cross_track_geo<bg::strategy::thomas, stype, double>
        thomas_strategy;

typedef bg::strategy::distance::cross_track_geo<bg::strategy::vincenty, stype, double>
        vincenty_strategy;

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

template <typename Strategy_pp, typename Strategy_ps>
void test_distance_point_segment(Strategy_pp const& strategy_pp,
                                 Strategy_ps const& strategy_ps)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "point/segment distance tests" << std::endl;
#endif
    typedef test_distance_of_geometries<point_type, segment_type> tester;

    tester::apply("p-s-01",
                  "POINT(0 0)",
                  "SEGMENT(2 0,3 0)",
                  pp_distance("POINT(0 0)", "POINT(2 0)", strategy_pp),
                  strategy_ps);
    tester::apply("p-s-02",
                  "POINT(2.5 3)",
                  "SEGMENT(2 0,3 0)",
                  pp_distance("POINT(2.5 3)", "POINT(2.49777 0)", strategy_pp),
                  strategy_ps);
    tester::apply("p-s-03",
                  "POINT(2 0)",
                  "SEGMENT(2 0,3 0)",
                  0,
                  strategy_ps);
    tester::apply("p-s-04",
                  "POINT(3 0)",
                  "SEGMENT(2 0,3 0)",
                  0,
                  strategy_ps);
    tester::apply("p-s-06",
                  "POINT(3.5 3)",
                  "SEGMENT(2 0,3 0)",
                  pp_distance("POINT(3 0)", "POINT(3.5 3)", strategy_pp),
                  strategy_ps);
    tester::apply("p-s-07",
                  "POINT(15 80)",
                  "SEGMENT(10 15,30 15)",
                  7204174.8264546748,
                  strategy_ps);
    tester::apply("p-s-08",
                  "POINT(15 10)",
                  "SEGMENT(10 15,30 15)",
                  571412.71247283253,
                  strategy_ps);
    tester::apply("p-s-09",
                  "POINT(5 10)",
                  "SEGMENT(10 15,30 15)",
                  pp_distance("POINT(5 10)", "POINT(10 15)", strategy_pp),
                  strategy_ps);
    tester::apply("p-s-10",
                  "POINT(35 10)",
                  "SEGMENT(10 15,30 15)",
                  pp_distance("POINT(35 10)", "POINT(30 15)", strategy_pp),
                  strategy_ps);
    tester::apply("p-s-11",
                  "POINT(5 10)",
                  "SEGMENT(30 15,10 15)",
                  pp_distance("POINT(5 10)", "POINT(10 15)", strategy_pp),
                  strategy_ps);
    tester::apply("p-s-12",
                  "POINT(35 10)",
                  "SEGMENT(30 15,10 15)",
                  pp_distance("POINT(35 10)", "POINT(30 15)", strategy_pp),
                  strategy_ps);

    tester::apply("p-s-right-up",
                  "POINT(3.5 3)",
                  "SEGMENT(2 2,3 2)",
                  pp_distance("POINT(3 2)", "POINT(3.5 3)", strategy_pp),
                  strategy_ps);

    tester::apply("p-s-left-up",
                  "POINT(1.5 3)",
                  "SEGMENT(2 2,3 2)",
                  pp_distance("POINT(2 2)", "POINT(1.5 3)", strategy_pp),
                  strategy_ps);

    tester::apply("p-s-up-1",
                  "POINT(2 3)",
                  "SEGMENT(2 2,3 2)",
                  pp_distance("POINT(2.0003 2)", "POINT(2 3)", strategy_pp),
                  strategy_ps);

    tester::apply("p-s-up-2",
                  "POINT(3 3)",
                  "SEGMENT(2 2,3 2)",
                  pp_distance("POINT(2.9997 2)", "POINT(3 3)", strategy_pp),
                  strategy_ps);

    tester::apply("p-s-right-down",
                  "POINT(3.5 1)",
                  "SEGMENT(2 2,3 2)",
                  pp_distance("POINT(3 2)", "POINT(3.5 1)", strategy_pp),
                  strategy_ps);

    tester::apply("p-s-left-down",
                  "POINT(1.5 1)",
                  "SEGMENT(2 2,3 2)",
                  pp_distance("POINT(2 2)", "POINT(1.5 1)", strategy_pp),
                  strategy_ps);

    tester::apply("p-s-down-1",
                  "POINT(2 1)",
                  "SEGMENT(2 2,3 2)",
                  pp_distance("POINT(2.0003 2)", "POINT(2 1)", strategy_pp),
                  strategy_ps);

    tester::apply("p-s-down-2",
                  "POINT(3 1)",
                  "SEGMENT(2 2,3 2)",
                  pp_distance("POINT(2.9997 2)", "POINT(3 1)", strategy_pp),
                  strategy_ps);

    tester::apply("p-s-south",
                  "POINT(3 -1)",
                  "SEGMENT(2 -2,3 -2)",
                  pp_distance("POINT(2.9997 -2)", "POINT(3 -1)", strategy_pp),
                  strategy_ps);

    tester::apply("p-s-antimeridian-1",
                  "POINT(3 1)",
                  "SEGMENT(220 2,3 2)",
                  pp_distance("POINT(3 2)", "POINT(3 1)", strategy_pp),
                  strategy_ps);

    tester::apply("p-s-antimeridian-2",
                  "POINT(220 1)",
                  "SEGMENT(220 2,3 2)",
                  pp_distance("POINT(220 2)", "POINT(220 1)", strategy_pp),
                  strategy_ps);

    // equator special case
    tester::apply("p-s-eq1",
                  "POINT(2.5 0)",
                  "SEGMENT(2 0,3 0)",
                  0,
                  strategy_ps);
    tester::apply("p-s-eq2",
                  "POINT(2.5 2)",
                  "SEGMENT(2 0,3 0)",
                  pp_distance("POINT(2.5 0)", "POINT(2.5 2)", strategy_pp),
                  strategy_ps);
    tester::apply("p-s-eq3",
                  "POINT(2 2)",
                  "SEGMENT(2 0,3 0)",
                  pp_distance("POINT(2 0)", "POINT(2 2)", strategy_pp),
                  strategy_ps);
    tester::apply("p-s-eq4",
                  "POINT(3 2)",
                  "SEGMENT(2 0,3 0)",
                  pp_distance("POINT(3 0)", "POINT(3 2)", strategy_pp),
                  strategy_ps);

    // meridian special case
    tester::apply("p-s-mer1",
                  "POINT(2.5 2)",
                  "SEGMENT(2 2,2 4)",
                  pp_distance("POINT(2.5 2)", "POINT(2 2)", strategy_pp),
                  strategy_ps);
    tester::apply("p-s-mer2",
                  "POINT(2.5 3)",
                  "SEGMENT(2 2,2 4)",
                  pp_distance("POINT(2.5 3)", "POINT(2 3)", strategy_pp),
                  strategy_ps);
    tester::apply("p-s-mer3",
                  "POINT(2.5 5)",
                  "SEGMENT(2 2,2 4)",
                  pp_distance("POINT(2.5 5)", "POINT(2 4)", strategy_pp),
                  strategy_ps);


    //Nearly antipodal points not treated correctly in BG so far
/*
    // very small distances to segment
    tester::apply("p-s-07",
                  "POINT(90 1e-3)",
                  "SEGMENT(0.5 0,175.5 0)",
                  110.57427582147321,
                  //1e-3 * d2r * 6335439.3272858784,
                  //to_comparable(strategy, 1e-3 * d2r * strategy.radius()),
                  strategy_ps);
    tester::apply("p-s-08",
                  "POINT(90 1e-4)",
                  "SEGMENT(0.5 0,175.5 0)",
                  6176.4469462110519,
                  //1e-4 * d2r * 6335439.3272858784,
                  //1e-4 * d2r * strategy.radius(),
                  //to_comparable(strategy, 1e-4 * d2r * strategy.radius()),
                  strategy_ps);
    tester::apply("p-s-09",
                  "POINT(90 1e-5)",
                  "SEGMENT(0.5 0,175.5 0)",
                  4917.5121192812849,
                  //1e-5 * d2r * strategy.radius(),
                  //to_comparable(strategy, 1e-5 * d2r * strategy.radius()),
                  strategy_ps);
    tester::apply("p-s-10",
                  "POINT(90 1e-6)",
                  "SEGMENT(0.5 0,175.5 0)",
                  4997.0354014473833,
                  //1e-6 * d2r * strategy.radius(),
                  //to_comparable(strategy, 1e-6 * d2r * strategy.radius()),
                  strategy_ps);
    tester::apply("p-s-11",
                  "POINT(90 1e-7)",
                  "SEGMENT(0.5 0,175.5 0)",
                  109854.20732250962,
                  //1e-7 * d2r * strategy.radius(),
                  //to_comparable(strategy, 1e-7 * d2r * strategy.radius()),
                  strategy_ps);
    tester::apply("p-s-12",
                  "POINT(90 1e-8)",
                  "SEGMENT(0.5 0,175.5 0)",
                  1372504.9365672076,
                  //1e-8 * d2r * strategy.radius(),
                  //to_comparable(strategy, 1e-8 * d2r * strategy.radius()),
                  strategy_ps);
*/
}

//===========================================================================
//===========================================================================
//===========================================================================

BOOST_AUTO_TEST_CASE( test_all_point_segment )
{
    test_distance_point_segment(vincenty_pp(), vincenty_strategy());
    test_distance_point_segment(thomas_pp(), thomas_strategy());
    test_distance_point_segment(andoyer_pp(), andoyer_strategy());
}
