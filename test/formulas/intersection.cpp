// Boost.Geometry
// Unit Test

// Copyright (c) 2016 Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "test_formula.hpp"
#include "intersection_cases.hpp"

#include <boost/geometry/formulas/andoyer_inverse.hpp>
#include <boost/geometry/formulas/geographic.hpp>
#include <boost/geometry/formulas/gnomonic_intersection.hpp>
#include <boost/geometry/formulas/sjoberg_intersection.hpp>
#include <boost/geometry/formulas/thomas_direct.hpp>
#include <boost/geometry/formulas/thomas_inverse.hpp>
#include <boost/geometry/formulas/vincenty_direct.hpp>
#include <boost/geometry/formulas/vincenty_inverse.hpp>

void check_inverse(expected_result const& result, expected_result const& expected, expected_result const& reference, double reference_error)
{
    check_one(result.lon, expected.lon, reference.lon, reference_error);
    check_one(result.lat, expected.lat, reference.lat, reference_error);
}

void test_all(expected_results const& results)
{
    double const d2r = bg::math::d2r<double>();
    double const r2d = bg::math::r2d<double>();

    double lona1r = results.p1.lon * d2r;
    double lata1r = results.p1.lat * d2r;
    double lona2r = results.p2.lon * d2r;
    double lata2r = results.p2.lat * d2r;
    double lonb1r = results.q1.lon * d2r;
    double latb1r = results.q1.lat * d2r;
    double lonb2r = results.q2.lon * d2r;
    double latb2r = results.q2.lat * d2r;

    expected_result result;

    // WGS84
    bg::srs::spheroid<double> spheroid(6378137.0, 6356752.3142451793);

    bg::formula::gnomonic_intersection<double, bg::formula::vincenty_inverse, bg::formula::vincenty_direct>
        ::apply(lona1r, lata1r, lona2r, lata2r, lonb1r, latb1r, lonb2r, latb2r, result.lon, result.lat, spheroid);
    result.lon *= r2d;
    result.lat *= r2d;
    check_inverse(result, results.gnomonic_vincenty, results.gnomonic_karney, 0.00000001);

    bg::formula::gnomonic_intersection<double, bg::formula::thomas_inverse, bg::formula::thomas_direct>
        ::apply(lona1r, lata1r, lona2r, lata2r, lonb1r, latb1r, lonb2r, latb2r, result.lon, result.lat, spheroid);
    result.lon *= r2d;
    result.lat *= r2d;
    check_inverse(result, results.gnomonic_thomas, results.gnomonic_karney, 0.0000001);

    bg::formula::sjoberg_intersection<double, bg::formula::vincenty_inverse, 4>
        ::apply(lona1r, lata1r, lona2r, lata2r, lonb1r, latb1r, lonb2r, latb2r, result.lon, result.lat, spheroid);
    result.lon *= r2d;
    result.lat *= r2d;
    check_inverse(result, results.sjoberg_vincenty, results.sjoberg_karney, 0.00000001);

    bg::formula::sjoberg_intersection<double, bg::formula::thomas_inverse, 2>
        ::apply(lona1r, lata1r, lona2r, lata2r, lonb1r, latb1r, lonb2r, latb2r, result.lon, result.lat, spheroid);
    result.lon *= r2d;
    result.lat *= r2d;
    check_inverse(result, results.sjoberg_thomas, results.sjoberg_karney, 0.0000001);

    bg::formula::sjoberg_intersection<double, bg::formula::andoyer_inverse, 1>
        ::apply(lona1r, lata1r, lona2r, lata2r, lonb1r, latb1r, lonb2r, latb2r, result.lon, result.lat, spheroid);
    result.lon *= r2d;
    result.lat *= r2d;
    check_inverse(result, results.sjoberg_andoyer, results.sjoberg_karney, 0.0001);

    typedef bg::model::point<double, 2, bg::cs::geographic<bg::degree> > point_geo;
    typedef bg::model::point<double, 3, bg::cs::cartesian> point_3d;
    point_geo a1(results.p1.lon, results.p1.lat);
    point_geo a2(results.p2.lon, results.p2.lat);
    point_geo b1(results.q1.lon, results.q1.lat);
    point_geo b2(results.q2.lon, results.q2.lat);
    point_3d a1v = bg::formula::geo_to_cart3d<point_3d>(a1, spheroid);
    point_3d a2v = bg::formula::geo_to_cart3d<point_3d>(a2, spheroid);
    point_3d b1v = bg::formula::geo_to_cart3d<point_3d>(b1, spheroid);
    point_3d b2v = bg::formula::geo_to_cart3d<point_3d>(b2, spheroid);
    point_3d resv(0, 0, 0);
    point_geo res(0, 0);

    bg::formula::experimental_elliptic_intersection(a1v, a2v, b1v, b2v, resv, spheroid);
    res = bg::formula::cart3d_to_geo<point_geo>(resv, spheroid);
    result.lon = bg::get<0>(res);
    result.lat = bg::get<1>(res);
    check_inverse(result, results.experimental_elliptic, results.gnomonic_karney, 0.0001);

    bg::formula::great_elliptic_intersection(a1v, a2v, b1v, b2v, resv, spheroid);
    res = bg::formula::cart3d_to_geo<point_geo>(resv, spheroid);
    result.lon = bg::get<0>(res);
    result.lat = bg::get<1>(res);
    check_inverse(result, results.great_elliptic, results.gnomonic_karney, 0.0001);
}

int test_main(int, char*[])
{
    for (size_t i = 0; i < expected_size; ++i)
    {
        test_all(expected[i]);
    }

    return 0;
}
