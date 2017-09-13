// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2014-2017, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE test_is_valid_geo
#endif

#include <limits>
#include <iostream>

#include <boost/test/included/unit_test.hpp>

#include "test_is_valid.hpp"

#include <boost/geometry/core/coordinate_type.hpp>

#include <boost/geometry/algorithms/correct.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/algorithms/reverse.hpp>

BOOST_AUTO_TEST_CASE( test_is_valid_geo_polygon )
{
    typedef bg::model::point<double, 2, bg::cs::geographic<bg::degree> > pt;
    typedef bg::model::polygon<pt, false> G;

    typedef validity_tester_geo_areal<false> tester;
    typedef test_valid<tester, G> test;

    test::apply("p01", "POLYGON((-1  -1, 1  -1, 1  1, -1  1, -1  -1),(-0.5 -0.5, -0.5 0.5, 0.0 0.0, -0.5 -0.5),(0.0 0.0, 0.5 0.5, 0.5 -0.5, 0.0 0.0))", true);
}
