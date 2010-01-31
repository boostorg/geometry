// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Point Example - showing different type of points

#include <iostream>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>
#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include <boost/geometry/geometries/cartesian2d.hpp>


int main()
{
    using namespace boost::geometry;

    // GGL contains several point types:
    // 1: it's own generic type
    point<double, 2, cs::cartesian> pt1;

    // 2: it's own type targetted to Cartesian (x,y) coordinates
    point_2d pt2;

    // 3: it supports Boost tuple's (by including the headerfile)
    boost::tuple<double, double> pt3;

    // 4: it supports normal arrays
    double pt4[2];

    // 5: there are more variants, and you can create your own.
    //    (see therefore the custom_point example)

    // All these types are handled the same way. We show here
    // assigning them and calculating distances.
    assign(pt1, 1, 1);
    assign(pt2, 2, 2);
    assign(pt3, 3, 3);
    assign(pt4, 4, 4);

    double d1 = distance(pt1, pt2);
    double d2 = distance(pt3, pt4);
    std::cout << "Distances: " << d1 << " and " << d2 << std::endl;

    // (in case you didn't note, distances can be calculated
    //  from points with different point-types)


    // Several ways of construction and setting point values
    // 1: default, empty constructor, causing no initialization at all
    point_2d p1;

    // 2: as shown above, assign
    point_2d p2;
    assign(p2, 1, 1);

    // 3: using "set" function
    //    set uses the concepts behind, such that it can be applied for
    //    every point-type (like assign)
    point_2d p3;
    set<0>(p3, 1);
    set<1>(p3, 1);
    // set<2>(p3, 1); //will result in compile-error


    // 3: for any point type, and other geometry objects:
    //    there is the "make" object generator
    //    (this one requires to specify the point-type).
    point_2d p4 = make<point_2d>(1,1);


    // 5: for the point_2d type only: constructor with two values
    point_2d p5(1,1);

    // 6: for boost tuples you can of course use make_tuple


    // Some ways of getting point values

    // 1: using the "get" function following the concepts behind
    std::cout << get<0>(p2) << "," << get<1>(p2) << std::endl;

    // 2: for point-2d only
    std::cout << p2.x() << "," << p2.y() << std::endl;

    // 3: using boost-tuples you of course can boost-tuple-methods
    std::cout << pt3.get<0>() << "," << pt3.get<1>() << std::endl;

    // 4: GGL supports various output formats, e.g. DSV
    //    (delimiter separated values)
    std::cout << dsv(pt3) << std::endl;


    // Other examples show other types of points, geometries and more algorithms

    return 0;
}
