// Boost.Geometry
// Unit Test

// Copyright (c) 2017 Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_VERTEX_LONGITUDE_CASES_HPP
#define BOOST_GEOMETRY_TEST_VERTEX_LONGITUDE_CASES_HPP

struct coordinates
{
    double lon;
    double lat;
};

struct expected_result
{
    double lon;
    double lat;
};

struct expected_results
{
    coordinates p1;
    coordinates p2;
    double andoyer;
    double thomas;
    double vincenty;
    double spherical;
};

#define ND 1000

expected_results expected[] =
{
    {
        { 0, 1 },{ 4, 1 },
        1.979821631,
        1.97982168,
        1.979821604,
        2
    } ,
    {
        { 0, 1 },{ 10, 1 },
        4.949554511,
        4.949554266,
        4.949554076,
        5
    } ,
    {
        { 0, 1 },{ 60, 1 },
        29.69744743,
        29.69734534,
        29.6973442,
        30
    } ,
    /*{//TODO: check low accuracy by thomas formula
        { 0, 1 },{ 90, 1 },
        44.54647609,
        44.32110183,
        44.54607474,
        45
    } ,*/
    {
        { 0, 1 },{ 120, 1 },
        59.39621842,
        59.39499152,
        59.39498928,
        60
    } ,
    {
        { 0, 1 },{ 150, 1 },
        74.24923578,
        74.24518362,
        74.24518154,
        75
    } ,
    {
        { 0, 1 },{ 180, 1 },
        90,
        90,
        90,
        90
    }
};

size_t const expected_size = sizeof(expected) / sizeof(expected_results);

#endif // BOOST_GEOMETRY_TEST_VERTEX_LONGITUDE_CASES_HPP
