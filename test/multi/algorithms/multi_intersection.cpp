// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <string>


#include <algorithms/test_intersection.hpp>
#include <algorithms/test_overlay.hpp>
#include <multi/algorithms/overlay/multi_overlay_cases.hpp>

#include <boost/geometry/multi/algorithms/correct.hpp>
#include <boost/geometry/multi/algorithms/intersection.hpp>

#include <boost/geometry/multi/geometries/multi_point.hpp>
#include <boost/geometry/multi/geometries/multi_linestring.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>

#include <boost/geometry/extensions/gis/io/wkt/read_wkt_multi.hpp>

template <typename Ring, typename Polygon, typename MultiPolygon>
void test_areal()
{
    test_one<Polygon, MultiPolygon, MultiPolygon>("simplex_multi",
        case_multi_simplex[0], case_multi_simplex[1],
        2, 12, 6.42);

    test_one<Polygon, MultiPolygon, MultiPolygon>("case_multi_no_ip",
        case_multi_no_ip[0], case_multi_no_ip[1],
        2, 8, 8.5);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_multi_2",
        case_multi_2[0], case_multi_2[1],
        3, 12, 5.9);

    test_one<Polygon, MultiPolygon, Polygon>("simplex_multi_mp_p",
        case_multi_simplex[0], case_single_simplex,
        2, 12, 6.42);

    test_one<Polygon, Ring, MultiPolygon>("simplex_multi_r_mp",
        case_single_simplex, case_multi_simplex[0],
        2, 12, 6.42);
    test_one<Ring, MultiPolygon, Polygon>("simplex_multi_mp_r",
        case_multi_simplex[0], case_single_simplex,
        2, 12, 6.42);
}

template <typename Polygon, typename MultiPolygon, typename Box>
void test_areal_clip()
{
    static std::string const clip = "POLYGON((1 1,4 4))";
    test_one<Polygon, Box, MultiPolygon>("simplex_multi_mp_b", clip, case_multi_simplex[0],
        2, 11, 6.791666);
    test_one<Polygon, MultiPolygon, Box>("simplex_multi_b_mp", case_multi_simplex[0], clip,
        2, 11, 6.791666);
}

template <typename LineString, typename MultiLineString, typename Box>
void test_linear()
{
    typedef boost::geometry::point_type<MultiLineString>::type point;
    test_one<point, MultiLineString, MultiLineString>("case_multi_ml_ml_1",
        "MULTILINESTRING((0 0,1 1))", "MULTILINESTRING((0 1,1 0))",
        1, 1, 0);
    test_one<point, MultiLineString, MultiLineString>("case_multi_ml_ml_2",
        "MULTILINESTRING((0 0,1 1),(0.5 0,1.5 1))", "MULTILINESTRING((0 1,1 0),(0.5 1,1.5 0))",
        4, 4, 0);

    test_one<point, LineString, MultiLineString>("case_multi_l_ml",
        "LINESTRING(0 0,1 1)", "MULTILINESTRING((0 1,1 0),(0.5 1,1.5 0))",
        2, 2, 0);
    test_one<point, MultiLineString, LineString>("case_multi_ml_l",
        "MULTILINESTRING((0 1,1 0),(0.5 1,1.5 0))", "LINESTRING(0 0,1 1)", 
        2, 2, 0);

    test_one<LineString, MultiLineString, Box>("case_multi_ml_b",
        "MULTILINESTRING((0 0,3 3)(1 0,4 3))", "POLYGON((1 1,3 2))",
        2, 4, 2 * std::sqrt(2.0));
    test_one<LineString, Box, MultiLineString>("case_multi_b_ml",
        "POLYGON((1 1,3 2))", "MULTILINESTRING((0 0,3 3)(1 0,4 3))", 
        2, 4, 2 * std::sqrt(2.0));
}


template <typename P>
void test_all()
{
    namespace bg = boost::geometry;


    typedef bg::box<P> box;
    typedef bg::linear_ring<P> ring;
    typedef bg::polygon<P> polygon;
    typedef bg::multi_polygon<polygon> multi_polygon;
    test_areal<ring, polygon, multi_polygon>();

    typedef bg::linear_ring<P, std::vector, false> ring_ccw;
    typedef bg::polygon<P, std::vector, std::vector, false> polygon_ccw;
    typedef bg::multi_polygon<polygon_ccw> multi_polygon_ccw;
    test_areal<ring_ccw, polygon_ccw, multi_polygon_ccw>();

    // multi/box: was NOT implemented, next step TODO.
    test_areal_clip<polygon, multi_polygon, box>();
    test_areal_clip<polygon_ccw, multi_polygon_ccw, box>();

    typedef bg::linestring<P> linestring;
    typedef bg::multi_linestring<linestring> multi_linestring;

    test_linear<linestring, multi_linestring, box>();


    // linear

}


int test_main(int, char* [])
{
    test_all<boost::geometry::point_xy<double> >();

    return 0;
}
