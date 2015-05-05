// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iterator>
#include <string>
#include <vector>

#include <iostream>

#include <boost/test/included/test_exec_monitor.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
#include <boost/cstdint.hpp>
#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/io/wkt/read.hpp>
#include <boost/geometry/extensions/gis/io/wkb/write_wkb.hpp>
#include <boost/geometry/extensions/gis/io/wkb/utility.hpp>

#include <boost/geometry/io/wkt/wkt.hpp>

#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/assign/list_of.hpp>

namespace bg = boost::geometry;

namespace { // anonymous

template <typename Geometry, bool IsEqual>
void test_geometry_equals(Geometry const& geometry, std::string const& wkbhex)
{
    std::string wkb_out;
    bg::write_wkb(geometry, std::back_inserter(wkb_out));

    std::string hex_out;
    BOOST_CHECK( bg::wkb2hex(wkb_out.begin(), wkb_out.end(), hex_out) );

    BOOST_CHECK_EQUAL( wkbhex, hex_out);
}

} // namespace anonymous

int test_main(int, char* [])
{
    typedef bg::model::point<double, 2, bg::cs::cartesian> point_type;
    typedef bg::model::point<double, 3, bg::cs::cartesian> point3d_type;
    typedef bg::model::linestring<point_type> linestring_type;
    typedef bg::model::linestring<point3d_type> linestring3d_type;
    typedef bg::model::polygon<point_type> polygon_type;
    typedef bg::model::polygon<point3d_type> polygon3d_type;

    //
    // POINT
    //

    {
        point_type point(1.0, 2.0);

        test_geometry_equals<point_type, true>
            (
            point,
            "0101000000000000000000F03F0000000000000040"
            );
    }

    {
        point3d_type point(1.0, 2.0, 3.0);

        test_geometry_equals<point3d_type, true>
            (
            point,
            "01E9030000000000000000F03F00000000000000400000000000000840"
            );
    }

    //
    // LINESTRING
    //

    {
        linestring_type linestring;

        bg::append(linestring, 
            boost::assign::list_of
            (point_type(1.0, 2.0))
            (point_type(2.0, 3.0))
            (point_type(4.0, 5.0))
            );

        test_geometry_equals<linestring_type, true>
            (
            linestring,
            "010200000003000000000000000000F03F00000000000000400000000000000040000000000000084000000000000010400000000000001440"
            );
    }

    {
        linestring3d_type linestring;

        bg::append(linestring, 
            boost::assign::list_of
            (point3d_type(1.0, 2.0, 3.0))
            (point3d_type(2.0, 3.0, 4.0))
            (point3d_type(4.0, 5.0, 6.0))
            );

        test_geometry_equals<linestring3d_type, true>
            (
            linestring,
            "01EA03000003000000000000000000F03F00000000000000400000000000000840000000000000004000000000000008400000000000001040000000000000104000000000000014400000000000001840"
            );
    }

    //
    // POLYGON
    //

    {
        polygon_type polygon;

        bg::append(polygon, 
            boost::assign::list_of
            (point_type(50.0, 50.0))
            (point_type(50.0, 100.0))
            (point_type(100.0, 100.0))
            (point_type(100.0, 50.0))
            (point_type(50.0, 50.0))
            );

        test_geometry_equals<polygon_type, true>
            (
            polygon,
            "010300000001000000050000000000000000004940000000000000494000000000000049400000000000005940000000000000594000000000000059400000000000005940000000000000494000000000000049400000000000004940"
            );
    }

    {
        polygon3d_type polygon;

        bg::append(polygon, 
            boost::assign::list_of
            (point3d_type(50.0, 50.0, 5.0))
            (point3d_type(50.0, 100.0, 10.0))
            (point3d_type(100.0, 100.0, 5.0))
            (point3d_type(100.0, 50.0, 10.0))
            (point3d_type(50.0, 50.0, 5.0))
            );

        test_geometry_equals<polygon3d_type, true>
            (
            polygon,
            "01EB0300000100000005000000000000000000494000000000000049400000000000001440000000000000494000000000000059400000000000002440000000000000594000000000000059400000000000001440000000000000594000000000000049400000000000002440000000000000494000000000000049400000000000001440"
            );
    }

    {
        polygon_type polygon;

        bg::append(polygon, 
            boost::assign::list_of
            (point_type(35, 10))
            (point_type(45, 45))
            (point_type(15, 40))
            (point_type(10, 20))
            (point_type(35, 10))
            );

        // Create an interior ring (append does not do this automatically)
        boost::geometry::interior_rings(polygon).resize(1);

        bg::append(polygon, 
            boost::assign::list_of
            (point_type(20, 30))
            (point_type(35, 35))
            (point_type(30, 20))
            (point_type(20, 30))
            , 0);

        test_geometry_equals<polygon_type, true>
            (
            polygon,
            "0103000000020000000500000000000000008041400000000000002440000000000080464000000000008046400000000000002E40000000000000444000000000000024400000000000003440000000000080414000000000000024400400000000000000000034400000000000003E40000000000080414000000000008041400000000000003E40000000000000344000000000000034400000000000003E40"
            );
    }

    return 0;
}
