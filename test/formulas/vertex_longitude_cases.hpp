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

expected_results expected[] =
{
    { //ascenting segments (wrt pole)
      { 1, 1 },{ 100, 2 },
      34.19259335,
      34.19217968,
      34.19217881,
      34.60255792
    },{
        { 1, 1 },{ 90, 2 },
        26.39840281,
        26.39808799,
        26.39808733,
        26.75585618
    },{
        { 0, 1 },{ 50, 1 },
        24.74783897,
        24.74778199,
        24.74778104,
        25
    },{
        { 0, 1 },{ 50, 1.1 },
        18.93792818,
        18.93781935,
        18.93781865,
        19.16790944
    },{
        { 0, 1 },{ 50, 1.2 },
        13.75469908,
        13.7545509,
        13.75455041,
        13.9656424
    },{
        { 0, 1 },{ 50, 1.3 },
        9.173570329,
        9.173393358,
        9.173393044,
        9.368797077
    },{
        { 0, 1 },{ 50, 1.4 },
        5.144443213,
        5.144244541,
        5.144244368,
        5.327041497
    },{
        { 0, 1 },{ 50, 1.5 },
        1.606488011,
        1.606262516,
        1.606262455,
        1.779116148
    },{
        { 0, 1 },{ 50, 1.6 },
        50,
        50,
        50,
        50
    },{ //descending segment (wrt pole)
        { 50, 1 },{ 0, 1.1 },
        31.06207182,
        31.06218065,
        31.06218135,
        30.83209056
    },{
        { 50, 1 },{ 0, 1.2 },
        36.24530092,
        36.2454491,
        36.24544959,
        36.0343576
    },{
        { 50, 1 },{ 0, 1.3 },
        40.82642967,
        40.82660664,
        40.82660696,
        40.63120292
    },{
        { 50, 1 },{ 0, 1.4 },
        44.85555679,
        44.85575546,
        44.85575563,
        44.6729585
    },{
        { 50, 1 },{ 0, 1.5 },
        48.39351199,
        48.39373748,
        48.39373755,
        48.22088385
    },{ //segments parallel to equator
        { 0, 1 },{ 4, 1 },
        1.979821631,
        1.97982168,
        1.979821604,
        2
    },{
        { 0, 1 },{ 10, 1 },
        4.949554511,
        4.949554266,
        4.949554076,
        5
    },{
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
    },{
        { 0, 1 },{ 150, 1 },
        74.24923578,
        74.24518362,
        74.24518154,
        75
    },{
        { 0, 1 },{ 180, 1 },
        90,
        90,
        90,
        90
    },{ //in equator vertex is any point on segment
        { 1, 0 },{ 10, 0 },
        1,
        1,
        1,
        1
    },{ //meridian
        { 1, 1 },{ 1, 2 },
        1,
        1,
        1,
        1
    },{ //nearly meridian
        { 1, 1 },{ 1.001, 2 },
        1.001,
        1.001,
        1.001,
        1.001
    },{ //vertex is a segment's endpoint
        { 1, 1 },{ 10, 2 },
        10,
        10,
        10,
        10
    },{
        { 10, 1 },{ 1, 2 },
        1,
        1,
        1,
        1
    },{ //South hemisphere, ascending
        { 0, -1 },{ 50, -1.4 },
        5.144443213,
        5.144244541,
        5.144244368,
        5.327041497
    },{ //South hemisphere, descending
        { 50, -1 },{ 0, -1.5 },
        48.39351199,
        48.39373748,
        48.39373755,
        48.22088385
    },{ //South hemisphere, same latitude
        { 0, -1 },{ 50, -1 },
        24.74783897,
        24.74778199,
        24.74778104,
        25
    },{ //Both hemispheres, vertex on the northern
        { 3, 3},{ 5, -5},
        5,
        5,
        5,
        5
    },{
        { 3, 3 },{ 150, -5},
        31.20707204,
        31.2011382,
        31.20113579,
        26.74999989
    },{ //Both hemispheres, vertex on the southern
        { 3, 5},{ 5, -3 },
        3,
        3,
        3,
        3
    },{//TODO: remove assumption first point MUST be closer to equator
       { 150, -3 },{ 3, 5},
        121.792928,
        121.7988618,
        121.7988642,
        126.2500001
    }
    
};

size_t const expected_size = sizeof(expected) / sizeof(expected_results);

#endif // BOOST_GEOMETRY_TEST_VERTEX_LONGITUDE_CASES_HPP
