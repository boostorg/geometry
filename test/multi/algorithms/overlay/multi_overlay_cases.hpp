// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_MULTI_OVERLAY_CASES_HPP
#define BOOST_GEOMETRY_TEST_MULTI_OVERLAY_CASES_HPP


#include <string>


static std::string case_multi_simplex[2] = {
        "MULTIPOLYGON(((0 1,2 5,5 3,0 1)),((1 1,5 2,5 0,1 1)))",
        "MULTIPOLYGON(((3 0,0 3,4 5,3 0)))" };

// To mix multi/single 
static std::string case_single_simplex = "POLYGON((3 0,0 3,4 5,3 0))";

static std::string case_multi_no_ip[2] = {
        "MULTIPOLYGON(((4 1,0 7,7 9,4 1)),((8 1,6 3,10 4,8 1)),((12 6,10 7,13 8,12 6)))",
        "MULTIPOLYGON(((14 4,8 8,15 10,14 4)),((15 3,18 9,20 2,15 3)),((3 4,1 7,5 7,3 4)))" };

static std::string case_multi_2[2] = {
        "MULTIPOLYGON(((4 3,2 7,10 9,4 3)),((8 1,6 3,10 4,8 1)),((12 6,10 7,13 8,12 6)))",
        "MULTIPOLYGON(((14 4,8 8,15 10,14 4)),((15 3,18 9,20 2,15 3)),((5 5,4 7,7 7,5 5)))" };



#endif // BOOST_GEOMETRY_TEST_MULTI_OVERLAY_CASES_HPP
