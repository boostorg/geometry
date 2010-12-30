// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Alfredo Correa 2010
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <geometry_test_common.hpp>


#include<boost/geometry/geometry.hpp>
#include<boost/geometry/geometries/cartesian3d.hpp>
#include<boost/geometry/geometries/adapted/boost_array_cartesian.hpp>
#include<boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include<boost/geometry/geometries/adapted/tuple_cartesian.hpp>
#include<iostream>


int test_main(int, char* [])
{
    bg::model::point<double, 3, bg::cs::cartesian> p1(1,2,3);
    double p2[3] = {4,5,6};
    boost::tuple<double, double, double> p3(7,8,9);
    boost::array<double, 3> p4 = {{10,11,12}};
    std::clog << bg::distance(p1, p2) << std::endl;
    std::clog << bg::distance(p2, p3) << std::endl;
    std::clog << bg::distance(p3, p4) << std::endl;
    return 0;
}

