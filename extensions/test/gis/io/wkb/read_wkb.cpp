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
#include <boost/cstdint.hpp>
#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/io/wkt/read.hpp>
#include <boost/geometry/extensions/gis/io/wkb/read_wkb.hpp>
#include <boost/geometry/extensions/gis/io/wkb/utility.hpp>

#include <boost/geometry/io/wkt/wkt.hpp>
#include <boost/geometry/io/wkt/write.hpp>
#include <boost/assign/list_of.hpp>

#include <boost/geometry/multi/algorithms/equals.hpp>
#include <boost/geometry/multi/geometries/multi_geometries.hpp> 
#include <boost/geometry/multi/io/wkt/wkt.hpp> 

#include <boost/geometry/extensions/multi/gis/io/wkb/read_wkb.hpp>

namespace bg = boost::geometry;

namespace { // anonymous

typedef std::vector<boost::uint8_t> byte_vector;

template <typename Geometry>
void test_geometry_wrong_wkb(std::string const& wkbhex, std::string const& wkt)
{
    byte_vector wkb;
    BOOST_CHECK( bg::hex2wkb(wkbhex, std::back_inserter(wkb)) );
    Geometry g_wkb;
    BOOST_MESSAGE("read_wkb: " << bg::read_wkb(wkb.begin(), wkb.end(), g_wkb));
}

template <typename Geometry>
void test_geometry_parse_failure(std::string const& wkbhex, std::string const& wkt)
{
    byte_vector wkb;
    BOOST_CHECK( bg::hex2wkb(wkbhex, std::back_inserter(wkb)) );
    Geometry g_wkb;
    BOOST_CHECK( bg::read_wkb(wkb.begin(), wkb.end(), g_wkb) == false );
}

template <typename Geometry, bool IsEqual>
void test_geometry_equals_old(std::string const& wkbhex, std::string const& wkt)
{
    byte_vector wkb;
    BOOST_CHECK( bg::hex2wkb(wkbhex, std::back_inserter(wkb)) );
    Geometry g_wkb;
    BOOST_CHECK( bg::read_wkb(wkb.begin(), wkb.end(), g_wkb) );

    Geometry g_expected;
    bg::read_wkt(wkt, g_expected);

    BOOST_CHECK( bg::equals(g_wkb, g_expected) == IsEqual );
}

template <typename Geometry, bool IsEqual>
void test_geometry_equals(Geometry const& g_expected, std::string const& wkbhex)
{
    byte_vector wkb;
    BOOST_CHECK( bg::hex2wkb(wkbhex, std::back_inserter(wkb)));
        
    Geometry g_wkb;
    bg::read_wkb(wkb.begin(), wkb.end(), g_wkb);

    BOOST_CHECK_MESSAGE( bg::equals(g_wkb, g_expected) == IsEqual,
        "{bg::equals(g_wkb, g_expected) == IsEqual} failed for" <<
        " EXPECTED: " << bg::wkt(g_expected) <<
        ", ACTUAL  : " << bg::wkt(g_wkb) );
}

//template <typename P, bool Result>
//void test_polygon_wkt(std::string const& wkt)
//{
//    typedef bg::model::linestring<P> linestring_type;
//    typedef bg::model::polygon<linestring_type> polygon_type;
//
//    polygon_type poly;
//    bg::read_wkb(wkb, poly);
//}

} // namespace anonymous

int test_main(int, char* [])
{
    {
        typedef bg::model::point<double, 2, bg::cs::cartesian> point_type;
        typedef bg::model::point<double, 3, bg::cs::cartesian> point3d_type;

        //
        // POINT
        //

        test_geometry_equals_old<point_type, true>(
            "01010000005839B4C876BEF33F83C0CAA145B61640", "POINT (1.234 5.678)");

        test_geometry_equals_old<point3d_type, true>(
            "01E90300005839B4C876BEF33F83C0CAA145B616404F401361C3332240", "POINT(1.234 5.678 9.1011)");

        //     // XYZ - POINT(1.234 5.678 99) - Z coordinate ignored
        //     test_geometry_equals<point_type, true>(
        //         "01010000805839B4C876BEF33F83C0CAA145B616400000000000C05840", "POINT(1.234 5.678)");
        // 
        //     // SRID=32632;POINT(1.234 5.678) - PostGIS EWKT
        //     test_geometry_equals<point_type, false>(
        //         "0101000020787F00005839B4C876BEF33F83C0CAA145B61640", "POINT (1.234 5.678)");
        // 
        //     // SRID=4326;POINT(1.234 5.678 99) - PostGIS EWKT
        //     test_geometry_equals<point_type, false>(
        //         "01010000A0E61000005839B4C876BEF33F83C0CAA145B616400000000000C05840", "POINT(1.234 5.678)");
        // 
        //     // POINTM(1.234 5.678 99) - XYM with M compound ignored
        //     test_geometry_equals<point_type, true>(
        //         "01010000405839B4C876BEF33F83C0CAA145B616400000000000C05840", "POINT (1.234 5.678)");
        // 
        //     // SRID=32632;POINTM(1.234 5.678 99)
        //     test_geometry_equals<point_type, false>(
        //         "0101000060787F00005839B4C876BEF33F83C0CAA145B616400000000000C05840", "POINT (1.234 5.678)");
        // 
        //     // POINT(1.234 5.678 15 79) - XYZM - Z and M compounds ignored
        //     test_geometry_equals<point_type, true>(
        //         "01010000C05839B4C876BEF33F83C0CAA145B616400000000000002E400000000000C05340",
        //         "POINT (1.234 5.678)");
        // 
        //     // SRID=4326;POINT(1.234 5.678 15 79) - XYZM + SRID
        //     test_geometry_equals<point_type, false>(
        //         "01010000E0E61000005839B4C876BEF33F83C0CAA145B616400000000000002E400000000000C05340",
        //         "POINT (1.234 5.678)");

    }
    
    typedef bg::model::point<double, 2, bg::cs::cartesian> point_type;
    typedef bg::model::point<double, 3, bg::cs::cartesian> point3d_type;

    typedef bg::model::linestring<point_type> linestring_type;
    //typedef bg::model::linestring<point3d_type> linestring3d_type;

    typedef bg::model::polygon<point_type> polygon_type;
    //typedef bg::model::polygon<point3d_type> polygon3d_type;

    {

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

        {
            point3d_type point(1.234, 5.678, 99.0);

            test_geometry_equals<point3d_type, true>
                (
                point, 
                "01e90300005839b4c876bef33f83c0caa145b616400000000000c05840"
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
            linestring_type linestring;

            bg::append(linestring, 
                boost::assign::list_of
                (point_type(1.234, 5.678))
                (point_type(9.1011, 10.1112))
                (point_type(13.1415, 16.1718))
                );

            test_geometry_equals<linestring_type, true>
                (
                linestring, 
                "0102000000030000005839B4C876BEF33F83C0CAA145B616404F401361C333224062A1D634EF3824409CC420B072482A40EB73B515FB2B3040"
                );

            test_geometry_equals<linestring_type, false>
                (
                linestring, 
                "0102000080030000005839B4C876BEF33F83C0CAA145B616400000000000C058404F401361C333224062A1D634EF3824400000000000C058409CC420B072482A40EB73B515FB2B30400000000000C05840"
                );
        }

//         {
//             linestring3d_type linestring;
// 
//             bg::append(linestring, 
//                 boost::assign::list_of
//                 (point3d_type(1.0, 2.0, 3.0))
//                 (point3d_type(2.0, 3.0, 4.0))
//                 (point3d_type(4.0, 5.0, 6.0))
//                 );
// 
//             test_geometry_equals<linestring3d_type, true>
//                 (
//                 linestring, 
//                 "01EA03000003000000000000000000F03F00000000000000400000000000000840000000000000004000000000000008400000000000001040000000000000104000000000000014400000000000001840"
//                 );
//         }

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
            polygon_type polygon;

            bg::append(polygon, 
                boost::assign::list_of
                (point_type(100.0, 200.0))
                (point_type(200.0, 200.0))
                (point_type(200.0, 400.0))
                (point_type(100.0, 400.0))
                (point_type(100.0, 200.0))
                );

            test_geometry_equals<polygon_type, true>
                (
                polygon, 
                "010300000001000000050000000000000000005940000000000000694000000000000069400000000000006940000000000000694000000000000079400000000000005940000000000000794000000000000059400000000000006940"
                );
        }
        
//         {
//             polygon3d_type polygon;
// 
//             bg::append(polygon, boost::assign::list_of(point3d_type(50.0, 50.0, 5.0))(point3d_type(50.0, 100.0, 10.0))(point3d_type(100.0, 100.0, 5.0))(point3d_type(100.0, 50.0, 10.0))(point3d_type(50.0, 50.0, 5.0)));
// 
//             test_geometry_equals<polygon3d_type, true>
//                 (
//                 polygon, 
//                 "01EB0300000100000005000000000000000000494000000000000049400000000000001440000000000000494000000000000059400000000000002440000000000000594000000000000059400000000000001440000000000000594000000000000049400000000000002440000000000000494000000000000049400000000000001440"
//                 );
//         }

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
            polygon.inners().resize(1);
            
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
