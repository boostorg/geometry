// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2010 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <string>

// #define BOOST_GEOMETRY_DEBUG_ASSEMBLE
#define BOOST_GEOMETRY_CHECK_WITH_SQLSERVER

#include <algorithms/test_difference.hpp>
#include <algorithms/test_overlay.hpp>
#include <multi/algorithms/overlay/multi_overlay_cases.hpp>

#include <boost/geometry/multi/algorithms/correct.hpp>
#include <boost/geometry/multi/algorithms/intersection.hpp>
#include <boost/geometry/multi/algorithms/within.hpp> // only for testing #77

#include <boost/geometry/multi/geometries/multi_point.hpp>
#include <boost/geometry/multi/geometries/multi_linestring.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>

#include <boost/geometry/domains/gis/io/wkt/read_wkt_multi.hpp>

template <typename Ring, typename Polygon, typename MultiPolygon>
void test_areal()
{
    test_one<Polygon, MultiPolygon, MultiPolygon>("simplex_multi",
            case_multi_simplex[0], case_multi_simplex[1],
            5, 12, 5.58, 4, 12, 2.58);

    test_one<Polygon, MultiPolygon, MultiPolygon>("case_multi_no_ip",
            case_multi_no_ip[0], case_multi_no_ip[1],
            2, 12, 24.0, 2, 12, 34.0);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_multi_2",
            case_multi_2[0], case_multi_2[1],
            2, 15, 19.6, 2, 13, 33.6);

    test_one<Polygon, MultiPolygon, Polygon>("simplex_multi_mp_p",
            case_multi_simplex[0], case_single_simplex,
            5, 22, 5.58, 4, 17, 2.58);
    test_one<Polygon, Ring, MultiPolygon>("simplex_multi_r_mp",
            case_single_simplex, case_multi_simplex[0],
            4, 17, 2.58, 5, 22, 5.58);
    test_one<Ring, MultiPolygon, Polygon>("simplex_multi_mp_r",
            case_multi_simplex[0], case_single_simplex,
            5, 22, 5.58, 4, 17, 2.58);

    // Constructed cases for multi/touch/equal/etc
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_61_multi",
            case_61_multi[0], case_61_multi[1],
            2, 10, 2, 2, 10, 2);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_62_multi",
            case_62_multi[0], case_62_multi[1],
            0, 0, 0, 1, 5, 1);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_63_multi",
            case_63_multi[0], case_63_multi[1],
            0, 0, 0, 1, 5, 1);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_64_multi",
        case_64_multi[0], case_64_multi[1],
            1, 1, 1, 1, 1, 1);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_65_multi",
        case_65_multi[0], case_65_multi[1],
            0, 0, 0, 2, 10, 3);
    /* TODO: fix
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_72_multi",
        case_72_multi[0], case_72_multi[1],
            3, 13, 1.65, 3, 17, 6.15);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_77_multi",
        case_77_multi[0], case_77_multi[1],
            6, 31, 7, 6, 36, 13);
    */

    test_one<Polygon, MultiPolygon, MultiPolygon>("case_78_multi",
        case_78_multi[0], case_78_multi[1],
            1, 1, 1.0, 1, 1, 1.0);

    /* TODO: fix
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_101_multi",
        case_101_multi[0], case_101_multi[1],
            5, 23, 4.75, 5, 40, 12.75);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_102_multi",
        case_102_multi[0], case_102_multi[1],
            2, 8, 0.75, 6, 25, 3.75);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_107_multi",
        case_107_multi[0], case_107_multi[1],
            2, 11, 2.25, 3, 14, 3.0);
    */
    /*
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_1",
        case_recursive_boxes_1[0], case_recursive_boxes_1[1],
            1, 1, 1, 1, 1, 1);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_2",
        case_recursive_boxes_2[0], case_recursive_boxes_2[1],
            1, 1, 1, 1, 1, 1);

     test_one<Polygon, MultiPolygon, MultiPolygon>("case_recursive_boxes_3",
         case_recursive_boxes_3[0], case_recursive_boxes_3[1],
            1, 1, 1, 1, 1, 1);
*/
}


template <typename P>
void test_all()
{
    typedef bg::model::box<P> box;
    typedef bg::model::ring<P> ring;
    typedef bg::model::polygon<P> polygon;
    typedef bg::model::multi_polygon<polygon> multi_polygon;
    test_areal<ring, polygon, multi_polygon>();
}


int test_main(int, char* [])
{
    test_all<bg::model::d2::point_xy<double> >();

#ifdef HAVE_TTMATH
    //test_all<bg::model::d2::point_xy<ttmath_big> >();
#endif

    return 0;
}
