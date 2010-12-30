// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//#define BOOST_GEOMETRY_CHECK_WITH_POSTGIS

//#define BOOST_GEOMETRY_DEBUG_ASSEMBLE


#include <iostream>
#include <string>


#include <algorithms/test_difference.hpp>
#include <algorithms/test_overlay.hpp>
#include <algorithms/overlay/overlay_cases.hpp>


template <typename P>
void test_all()
{
    typedef bg::model::polygon<P> polygon;

    test_one<polygon, polygon, polygon>(
            "star_ring", example_star, example_ring,
            5, 22, 1.1901714,
            5, 27, 1.6701714);

    test_one<polygon, polygon, polygon>("two_bends",
        two_bends[0], two_bends[1],
        1, 7, 8.0,
        1, 7, 8.0);

    test_one<polygon, polygon, polygon>("star_comb_15",
        star_15, comb_15,
        30, 150, 227.658275102812,
        30, 150, 480.485775259312);

    test_one<polygon, polygon, polygon>("simplex_normal",
        simplex_normal[0], simplex_normal[1],
        3, 3, 2.52636706856656,
        3, 3, 3.52636706856656);

    test_one<polygon, polygon, polygon>("new_hole",
        new_hole[0], new_hole[1],
        1, 10, 7.0,
        1, 10, 14.0);

    test_one<polygon, polygon, polygon>("fitting",
        fitting[0], fitting[1],
        1, 0, 21.0,
        1, 0, 4.0);

    test_one<polygon, polygon, polygon>("crossed",
        crossed[0], crossed[1],
        1, 0, 19.5,
        1, 0, 2.5);
    test_one<polygon, polygon, polygon>("first_within_second",
        first_within_second[1], first_within_second[0],
        1, 1, 24,
        0, 0, 0);

    test_one<polygon, polygon, polygon>("intersect_holes_disjoint",
        intersect_holes_disjoint[0], intersect_holes_disjoint[1],
        2, 15, 16.0,
        2, 15, 6.0);

    test_one<polygon, polygon, polygon>("intersect_holes_intersect",
        intersect_holes_intersect[0], intersect_holes_intersect[1],
        2, 14, 15.75,
        2, 14, 5.75);

    test_one<polygon, polygon, polygon>(
            "case4", case_4[0], case_4[1],
            6, 22, 2.77878787878788,
            4, 27, 4.77878787878788);

    test_one<polygon, polygon, polygon>(
            "case5", case_5[0], case_5[1],
            8, 22, 2.43452380952381,
            7, 27, 3.18452380952381);

    /***
    Experimental (cut), does not work: 
    test_one<polygon, polygon, polygon>(
            "polygon_pseudo_line",
            "POLYGON((0 0,0 4,4 4,4 0,0 0))",
            "POLYGON((2 -2,2 -1,2 6,2 -2))",
            5, 22, 1.1901714,
            5, 27, 1.6701714);
    ***/
}


int test_main(int, char* [])
{
    test_all<bg::model::d2::point_xy<double> >();
    return 0;
}
