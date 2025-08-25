// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iterator>
#include <string>
#include <vector>

#include <boost/test/included/test_exec_monitor.hpp>
#include <boost/test/included/unit_test.hpp>
#include <boost/test/tools/floating_point_comparison.hpp>
#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/io/wkt/read.hpp>
#include <boost/geometry/extensions/gis/io/wkb/write_wkb.hpp>
#include <boost/geometry/extensions/gis/io/wkb/utility.hpp>

#include <boost/geometry/io/wkt/wkt.hpp>

#include <boost/geometry/multi/algorithms/equals.hpp>
#include <boost/geometry/multi/geometries/multi_geometries.hpp> 
#include <boost/geometry/multi/io/wkt/wkt.hpp> 

#include <boost/geometry/extensions/multi/gis/io/wkb/write_wkb.hpp>

#include <boost/range/algorithm_ext/push_back.hpp>
#include <boost/assign/list_of.hpp>

#include <boost/algorithm/string.hpp>

namespace bg = boost::geometry;

namespace { // anonymous

template <typename Geometry, bool IsEqual>
void test_geometry_equals(Geometry const& geometry, std::string const& wkbhex)
{
    std::string wkb_out;
    bg::write_wkb(geometry, std::back_inserter(wkb_out));

    std::string hex_out;
    BOOST_CHECK( bg::wkb2hex(wkb_out.begin(), wkb_out.end(), hex_out) );
    
    boost::algorithm::to_lower(hex_out);
    
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
            "0101000000000000000000f03f0000000000000040"
            );
    }

    {
        point3d_type point(1.0, 2.0, 3.0);

        test_geometry_equals<point3d_type, true>
            (
            point,
            "01e9030000000000000000f03f00000000000000400000000000000840"
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
            "010200000003000000000000000000f03f00000000000000400000000000000040000000000000084000000000000010400000000000001440"
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
            "01ea03000003000000000000000000f03f00000000000000400000000000000840000000000000004000000000000008400000000000001040000000000000104000000000000014400000000000001840"
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
            "01eb0300000100000005000000000000000000494000000000000049400000000000001440000000000000494000000000000059400000000000002440000000000000594000000000000059400000000000001440000000000000594000000000000049400000000000002440000000000000494000000000000049400000000000001440"
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
            "0103000000020000000500000000000000008041400000000000002440000000000080464000000000008046400000000000002e40000000000000444000000000000024400000000000003440000000000080414000000000000024400400000000000000000034400000000000003e40000000000080414000000000008041400000000000003e40000000000000344000000000000034400000000000003e40"
            );
    }

    //
    // Multi Geometries
    //
    
    typedef bg::model::multi_point<point_type> multipoint_type;
    typedef bg::model::multi_linestring<linestring_type> multilinestring_type;
    typedef bg::model::multi_polygon<polygon_type> multipolygon_type;

    //
    // MultiPoint
    //
    
    {
        multipoint_type multipoint;
        
        bg::append(multipoint, 
            boost::assign::list_of
            (point_type(1.234, 5.678))
            );

        test_geometry_equals<multipoint_type, true>
            (
            multipoint, 
            "01040000000100000001010000005839b4c876bef33f83c0caa145b61640"
            );
    }
    
    {
        multipoint_type multipoint;
        
        bg::append(multipoint, 
            boost::assign::list_of
            (point_type(1.234, 5.678))
            (point_type(10.1112, 13.1415))
            );

        test_geometry_equals<multipoint_type, true>
            (
            multipoint, 
            "01040000000200000001010000005839b4c876bef33f83c0caa145b61640010100000062a1d634ef3824409cc420b072482a40"
            );
    }
    
    //
    // MultiLineString
    //
    
    {
        multilinestring_type multilinestring;
        
        // Create linestrings (append does not do this automatically)
        multilinestring.resize(1);
        
        bg::append(multilinestring[0], 
            boost::assign::list_of
            (point_type(1.234, 5.678))
            (point_type(9.1011, 10.1112))
            (point_type(13.1415, 16.1718))
            );

        test_geometry_equals<multilinestring_type, true>
            (
            multilinestring, 
            "0105000000010000000102000000030000005839b4c876bef33f83c0caa145b616404f401361c333224062a1d634ef3824409cc420b072482a40eb73b515fb2b3040"
            );
    }
    
    {
        multilinestring_type multilinestring;
        
        // Create linestrings (append does not do this automatically)
        multilinestring.resize(2);
        
        bg::append(multilinestring[0], 
            boost::assign::list_of
            (point_type(1.234, 5.678))
            (point_type(9.1011, 10.1112))
            (point_type(13.1415, 16.1718))
            );
            
        bg::append(multilinestring[1], 
            boost::assign::list_of
            (point_type(19.2, 21.22))
            (point_type(23.24, 25.26))
            );

        test_geometry_equals<multilinestring_type, true>
            (
            multilinestring, 
"0105000000020000000102000000030000005839b4c876bef33f83c0caa145b616404f401361c333224062a1d634ef3824409cc420b072482a40eb73b515fb2b30400102000000020000003333333333333340b81e85eb513835403d0ad7a3703d3740c3f5285c8f423940"
            );
    }
    
    //
    // MultiPolygon
    //
    
    {
        multipolygon_type multipolygon;
        
        // Create polygons (append does not do this automatically)
        multipolygon.resize(1);
        
        bg::append(multipolygon[0], 
            boost::assign::list_of
            (point_type(100, 200))
            (point_type(200, 200))
            (point_type(200, 400))
            (point_type(100, 400))
            (point_type(100, 200))
            );

        test_geometry_equals<multipolygon_type, true>
            (
            multipolygon, 
"010600000001000000010300000001000000050000000000000000005940000000000000694000000000000069400000000000006940000000000000694000000000000079400000000000005940000000000000794000000000000059400000000000006940"
            );
    }
    
    {
        multipolygon_type multipolygon;
        
        // Create polygons (append does not do this automatically)
        multipolygon.resize(1);
        // Create an interior ring (append does not do this automatically)
        multipolygon[0].inners().resize(1);
        
        bg::append(multipolygon[0], 
            boost::assign::list_of
            (point_type(35, 10))
            (point_type(10, 20))
            (point_type(15, 40))
            (point_type(45, 45))
            (point_type(35, 10))
            );

        bg::append(multipolygon[0], 
            boost::assign::list_of
            (point_type(20, 30))
            (point_type(35, 35))
            (point_type(30, 20))
            (point_type(20, 30)),
            0
            );
        
        test_geometry_equals<multipolygon_type, true>
            (
            multipolygon, 
"0106000000010000000103000000020000000500000000000000008041400000000000002440000000000000244000000000000034400000000000002e40000000000000444000000000008046400000000000804640000000000080414000000000000024400400000000000000000034400000000000003e40000000000080414000000000008041400000000000003e40000000000000344000000000000034400000000000003e40"
            );
    }
    
    return 0;
}
