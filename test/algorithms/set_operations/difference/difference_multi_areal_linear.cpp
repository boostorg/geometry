// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2010-2015 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <string>

//#define BOOST_GEOMETRY_CHECK_WITH_SQLSERVER

//#define BOOST_GEOMETRY_DEBUG_SEGMENT_IDENTIFIER
//#define BOOST_GEOMETRY_DEBUG_FOLLOW
//#define BOOST_GEOMETRY_DEBUG_TRAVERSE


#include "test_difference.hpp"
#include <algorithms/test_overlay.hpp>
#include <algorithms/overlay/multi_overlay_cases.hpp>

#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/multi_linestring.hpp>
#include <boost/geometry/geometries/multi_polygon.hpp>

#include <boost/geometry/io/wkt/read.hpp>


template <typename MultiPolygon, typename MultiLineString>
void test_areal_linear()
{
    typedef typename boost::range_value<MultiPolygon>::type polygon;
    typedef typename boost::range_value<MultiLineString>::type linestring;
    typedef typename bg::point_type<polygon>::type point;
    typedef bg::model::ring<point> ring;

    test_one_lp<linestring, MultiLineString, MultiPolygon>("first_crossing_between_polygons",
        "MULTILINESTRING((180 30,180 60))",
        "MULTIPOLYGON(((160 60,160 45,200 45,200 60,160 60)),"
        "((160 30,200 0,200 30,180 45,160 30)))", 0, 0, 0.0);

    test_one_lp<linestring, MultiLineString, MultiPolygon>("touching_hole_between_polygons",
        "MULTILINESTRING((20 0,0 0,0 15),"
        "(40 0,20 0,20 15,20 30,0 30,20 45,0 45,0 60,20 60,20 30,40 30,60 30,"
        "60 15,40 15,40 30,20 45,40 60,60 60),(20 15,40 0,60 15),"
        "(40 30,60 45,40 45),(60 30,60 45))",
        "MULTIPOLYGON(((40 60,60 45,60 60,40 60)),((40 60,20 60,20 45,40 60)),"
        "((60 45,40 45,40 30,60 15,60 30,60 45)),"
        "((40 30,20 45,0 30,0 0,40 0,40 30),(40 30,20 15,20 30,40 30)))",
        3, 8, 100.0);

    test_one_lp<linestring, MultiLineString, MultiPolygon>("touch_while_inside_another_polygon",
        "MULTILINESTRING((6 4,6 8))",
        "MULTIPOLYGON(((0 0,8 0,8 8,0 8,0 0),(2 2,2 6,6 4,2 2)),"
        "((3 3,6 4,3 5,3 3)))", 0, 0, 0.0);
    // Leave a touching hole and shell while following another polygon's boundary.
    for (auto const& area : {
        "MULTIPOLYGON(((80 90,60 75,80 60,80 90)),"
        "((60 75,40 90,0 60,40 30,60 45,60 75),(60 75,40 60,40 75,60 75)))",
        "MULTIPOLYGON(((60 75,40 90,0 60,40 30,60 45,60 75),"
        "(60 75,40 60,40 75,60 75)),((80 90,60 75,80 60,80 90)))"})
    {
        test_one_lp<linestring, MultiLineString, MultiPolygon>("leave_touching_boundaries",
            "MULTILINESTRING((80 60,60 75,60 90))", area, 1, 2, 15.0);
        test_one_lp<linestring, MultiLineString, MultiPolygon>("enter_touching_boundaries",
            "MULTILINESTRING((60 90,60 75,80 60))", area, 1, 2, 15.0);
    }

    test_one_lp<linestring, MultiLineString, MultiPolygon>("cross_while_covered",
        "MULTILINESTRING((4 4,4 8))",
        "MULTIPOLYGON(((0 0,8 0,8 8,0 8,0 0),(2 2,2 6,6 6,6 2,2 2)),"
        "((4 4,6 4,4 6,4 4)))", 0, 0, 0.0);

    test_one_lp<linestring, MultiLineString, MultiPolygon>("touch_while_following_boundary",
        "MULTILINESTRING((180 60,160 75,140 75,120 75))",
        "MULTIPOLYGON(((160 75,120 75,140 60,160 60,160 75)),"
        "((140 75,140 90,120 90,140 75)))", 1, 2, 25.0);

    test_one_lp<linestring, linestring, MultiPolygon>("case_mp_ls_1", "LINESTRING(2 0,2 5)", case_multi_simplex[0], 2, 4, 1.30);
    test_one_lp<linestring, MultiLineString, polygon>("case_p_mls_1", "MULTILINESTRING((2 0,2 5),(3 0,3 5))", case_single_simplex, 3, 6, 2.5);
    test_one_lp<linestring, MultiLineString, MultiPolygon>("case_mp_mls_1", "MULTILINESTRING((2 0,2 5),(3 0,3 5))", case_multi_simplex[0], 5, 10, 3.1666667);
    test_one_lp<linestring, MultiLineString, ring>("case_r_mls_1", "MULTILINESTRING((2 0,2 5),(3 0,3 5))", case_single_simplex, 3, 6, 2.5);

    // Collinear cases, with multiple turn points at the same location
    test_one_lp<linestring, linestring, MultiPolygon>("case_mp_ls_2a", "LINESTRING(1 0,1 1,2 1,2 0)", "MULTIPOLYGON(((0 0,0 1,1 1,1 0,0 0)),((1 1,1 2,2 2,2 1,1 1)))", 1, 2, 1.0);
    test_one_lp<linestring, linestring, MultiPolygon>("case_mp_ls_2b", "LINESTRING(1 0,1 1,2 1,2 0)", "MULTIPOLYGON(((1 1,1 2,2 2,2 1,1 1)),((0 0,0 1,1 1,1 0,0 0)))", 1, 2, 1.0);

    test_one_lp<linestring, linestring, MultiPolygon>("case_mp_ls_3",
            "LINESTRING(6 6,6 7,7 7,7 6,8 6,8 7,9 7,9 6)",
            "MULTIPOLYGON(((5 7,5 8,6 8,6 7,5 7)),((6 6,6 7,7 7,7 6,6 6)),((8 8,9 8,9 7,8 7,7 7,7 8,8 8)))", 2, 5, 3.0);

    return;

    // TODO: this case contains collinearities and should still be solved
    test_one_lp<linestring, linestring, MultiPolygon>("case_mp_ls_4",
            "LINESTRING(0 5,0 6,1 6,1 5,2 5,2 6,3 6,3 5,3 4,3 3,2 3,2 4,1 4,1 3,0 3,0 4)",
            "MULTIPOLYGON(((0 2,0 3,1 2,0 2)),((2 5,3 6,3 5,2 5)),((1 5,1 6,2 6,2 5,1 5)),((2 3,2 4,3 4,2 3)),((0 3,1 4,1 3,0 3)),((4 3,3 3,3 5,4 5,4 4,4 3)))", 5, 11, 6.0);
}


template <typename P>
void test_all()
{
    typedef bg::model::polygon<P> polygon;
    typedef bg::model::linestring<P> linestring;
    typedef bg::model::multi_polygon<polygon> multi_polygon;
    typedef bg::model::multi_linestring<linestring> multi_linestring;
    test_areal_linear<multi_polygon, multi_linestring>();
}


int test_main(int, char* [])
{
    test_all<bg::model::d2::point_xy<double> >();

#if ! defined(BOOST_GEOMETRY_TEST_ONLY_ONE_TYPE)
    test_all<bg::model::d2::point_xy<float> >();
#endif

    return 0;
}
