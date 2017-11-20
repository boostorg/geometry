// Boost.Geometry
// Unit Test

// Copyright (c) 2017 Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "test_formula.hpp"
#include "direct_meridian_cases.hpp"

#include <boost/geometry/formulas/vincenty_direct.hpp>
#include <boost/geometry/formulas/elliptic_meridian_arc_direct.hpp>

void test_all(expected_results const& results)
{
    double const d2r = bg::math::d2r<double>();

    double lat_expectedr = results.lat_expected * d2r;
    double distance = results.distance;

    // WGS84
    bg::srs::spheroid<double> spheroid(6378137.0, 6356752.3142451793);

    bg::formula::result_direct<double> vincenty_result;

    typedef bg::formula::vincenty_direct<double, true, false, false, false> vi_t;
    vincenty_result = vi_t::apply(0.0, 0.0, distance, 0.0, spheroid);

    {
        typedef bg::formula::elliptic_meridian_arc_direct<double, 1> eli;
        double lat = eli::apply(distance, spheroid);
        check_one(lat, lat_expectedr, vincenty_result.lat2, 0.001);
    }
    {
        typedef bg::formula::elliptic_meridian_arc_direct<double, 2> eli;
        double lat = eli::apply(distance, spheroid);
        check_one(lat, lat_expectedr, vincenty_result.lat2, 0.00001);
    }
    {
        typedef bg::formula::elliptic_meridian_arc_direct<double, 3> eli;
        double lat = eli::apply(distance, spheroid);
        check_one(lat, lat_expectedr, vincenty_result.lat2, 0.00000001);
    }
    {
        typedef bg::formula::elliptic_meridian_arc_direct<double, 4> eli;
        double lat = eli::apply(distance, spheroid);
        check_one(lat, lat_expectedr, vincenty_result.lat2, 0.00000001);
    }
    {
        typedef bg::formula::elliptic_meridian_arc_direct<double, 5> eli;
        double lat = eli::apply(distance, spheroid);
        check_one(lat, lat_expectedr, vincenty_result.lat2, 0.00000001);
    }
}

int test_main(int, char*[])
{
    for (size_t i = 0; i < expected_size; ++i)
    {
        test_all(expected[i]);
    }

    return 0;
}
