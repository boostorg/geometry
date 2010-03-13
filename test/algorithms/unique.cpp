// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <algorithms/test_unique.hpp>
#include <boost/geometry/geometries/geometries.hpp>



template <typename Point>
void test_all()
{
    test_geometry<boost::geometry::linestring<Point> >(
        "LINESTRING(0 0,1 1)",
        "LINESTRING(0 0,1 1)");

    test_geometry<boost::geometry::linestring<Point> >(
        "LINESTRING(0 0,1 1,1 1)",
        "LINESTRING(0 0,1 1)");

    test_geometry<boost::geometry::linestring<Point> >(
        "LINESTRING(0 0,0 0,1 1)",
        "LINESTRING(0 0,1 1)");

	// Consecutive points
    test_geometry<boost::geometry::linestring<Point> >(
        "LINESTRING(0 0,0 0,0 0,0 0,1 1,1 1,1 1)",
        "LINESTRING(0 0,1 1)");

	// Other types
    test_geometry<boost::geometry::linear_ring<Point> >(
        "POLYGON((0 0,0 1,1 1,1 1,1 1,1 0,0 0,0 0))",
        "POLYGON((0 0,0 1,1 1,1 0,0 0))");

    test_geometry<boost::geometry::polygon<Point> >(
        "POLYGON((0 0,0 1,1 1,1 1,1 1,1 0,0 0,0 0))",
        "POLYGON((0 0,0 1,1 1,1 0,0 0))");
}



int test_main(int, char* [])
{
    test_all<boost::geometry::point_xy<int> >();
    test_all<boost::geometry::point_xy<double> >();

    return 0;
}
