// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2017, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE test_distance_geographic_pl_l
#endif

//#include <boost/geometry/core/srs.hpp>
#include <boost/test/included/unit_test.hpp>

#include "test_distance_geo_common.hpp"

typedef bg::cs::geographic<bg::degree> cs_type;
typedef bg::model::point<double, 2, cs_type> point_type;
typedef bg::model::multi_point<point_type> multi_point_type;

namespace services = bg::strategy::distance::services;
typedef bg::default_distance_result<point_type>::type return_type;

typedef bg::srs::spheroid<double> stype;

// Strategies for point-point distance

typedef bg::strategy::distance::andoyer<stype> andoyer;
typedef bg::strategy::distance::thomas<stype> thomas;
typedef bg::strategy::distance::vincenty<stype> vincenty;

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

//===========================================================================

template <typename Strategy>
void test_distance_multipoint_point(Strategy const& strategy)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "multipoint/point distance tests" << std::endl;
#endif
    typedef test_distance_of_geometries<multi_point_type, point_type> tester;

    tester::apply("mp-p-01",
                  "MULTIPOINT(1 1,1 2,2 3)",
                  "POINT(0 0)",
                  pp_distance("POINT(0 0)","POINT(1 1)",strategy),
                  strategy);

    tester::apply("mp-p-01",
                  "MULTIPOINT(0 0,0 2,2 0,2 2)",
                  "POINT(1.1 1.1)",
                  pp_distance("POINT(1.1 1.1)","POINT(2 2)",strategy),
                  strategy);
}

//===========================================================================

template <typename Point, typename Strategy>
void test_empty_input_pointlike_linear(Strategy const& strategy)
{
#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "testing on empty inputs... " << std::flush;
#endif
    bg::model::multi_point<Point> multipoint_empty;
    Point point_empty;

    Point point = from_wkt<Point>("POINT(0 0)");

    // 1st geometry is empty
    test_empty_input(multipoint_empty, point, strategy);

    // 2nd geometry is empty
    test_empty_input(point, multipoint_empty, strategy);

    // both geometries are empty
    test_empty_input(multipoint_empty, point_empty, strategy);

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << "done!" << std::endl;
#endif
}

//===========================================================================
//===========================================================================
//===========================================================================

BOOST_AUTO_TEST_CASE( test_all_point_segment )
{
    test_distance_multipoint_point(vincenty());
    test_distance_multipoint_point(thomas());
    test_distance_multipoint_point(andoyer());

    test_empty_input_pointlike_linear<point_type>(vincenty());
    test_empty_input_pointlike_linear<point_type>(thomas());
    test_empty_input_pointlike_linear<point_type>(andoyer());
}
