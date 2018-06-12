// Boost.Geometry
// Unit Test

// Copyright (c) 2018 Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_DIRECT_MERIDIAN_CASES_HPP
#define BOOST_GEOMETRY_TEST_DIRECT_MERIDIAN_CASES_HPP

struct expected_results
{
    double lat_expected;
    double distance;
};

expected_results expected[] =
{
    { 0.90435537782, 100000},
    { 1.8087062846, 200000},
    { 2.7130880778385226826, 300000},
    { 3.6174296804714929365, 400000},
    { 4.521753233678117212, 500000},
    { 5.4260542596891729872, 600000},
    { 6.3303283037366426811, 700000},
    { 7.234570938597034484, 800000},
    { 8.1386639105161684427, 900000},
    { 9.0428195432854963087, 1000000},
    { 89.982400761362015373, 10000000},
    { 90, 10001965}
};

size_t const expected_size = sizeof(expected) / sizeof(expected_results);

#endif // BOOST_GEOMETRY_TEST_DIRECT_MERIDIAN_CASES_HPP
