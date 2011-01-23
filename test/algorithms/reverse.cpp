// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <algorithms/test_reverse.hpp>
#include <boost/geometry/geometries/geometries.hpp>

#include <test_common/test_point.hpp>


template <typename Point>
void test_all()
{
    // Simplex
    test_geometry<bg::model::linestring<Point> >(
        "LINESTRING(0 0,1 1)",
        "LINESTRING(1 1,0 0)");

    // Three points, middle should stay the same
    test_geometry<bg::model::linestring<Point> >(
        "LINESTRING(0 0,1 1,2 2)",
        "LINESTRING(2 2,1 1,0 0)");

    // Four points
    test_geometry<bg::model::linestring<Point> >(
        "LINESTRING(0 0,1 1,2 2,3 3)",
        "LINESTRING(3 3,2 2,1 1,0 0)");

    // Polygon with holes
    test_geometry<bg::model::polygon<Point> >(
        "POLYGON((4 0,8 2,8 7,4 9,0 7,0 2,2 1,4 0),(7 3,7 6,1 6,1 3,4 3,7 3))",
        "POLYGON((4 0,2 1,0 2,0 7,4 9,8 7,8 2,4 0),(7 3,4 3,1 3,1 6,7 6,7 3))");

    // Check compilation
    test_geometry<Point>("POINT(0 0)", "POINT(0 0)");

    test_geometry<bg::model::ring<Point> >(
        "POLYGON((4 0,8 2,8 7,4 9,0 7,0 2,2 1,4 0))",
        "POLYGON((4 0,2 1,0 2,0 7,4 9,8 7,8 2,4 0))");
}

int test_main(int, char* [])
{
    test_all<bg::model::d2::point_xy<int> >();
    test_all<bg::model::d2::point_xy<double> >();

    return 0;
}
