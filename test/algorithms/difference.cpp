// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <string>


#include <algorithms/test_difference.hpp>
#include <algorithms/test_overlay.hpp>
#include <algorithms/overlay/overlay_cases.hpp>

template <typename P>
void test_all()
{
    typedef boost::geometry::polygon<P> polygon;

    test_one<polygon, polygon, polygon>(
            "star_ring", example_star, example_ring,
            5, 22, 1.1901714,
            5, 27, 1.6701714);

    test_one<polygon, polygon, polygon>("two_bends",
        two_bends[0], two_bends[1],
        1, 7, 24.0,
        1, 7, 24.0);

    test_one<polygon, polygon, polygon>("star_comb_15",
        star_15, comb_15,
        28, 150, 189.952883,
        28, 150, 189.952883);

    test_one<polygon, polygon, polygon>("simplex_normal",
        simplex_normal[0], simplex_normal[1],
        1, 7, 5.47363293,
        1, 7, 5.47363293);

    test_one<polygon, polygon, polygon>("fitting",
        fitting[0], fitting[1],
        0, 0, 0, 0, 0, 0);

    test_one<polygon, polygon, polygon>("new_hole",
        new_hole[0], new_hole[1],
        2, 10, 2.0,
        2, 10, 2.0);

    test_one<polygon, polygon, polygon>("intersect_holes_disjoint",
        intersect_holes_disjoint[0], intersect_holes_disjoint[1],
        1, 15, 18.0,
        1, 15, 18.0);

    test_one<polygon, polygon, polygon>("intersect_holes_intersect",
        intersect_holes_intersect[0], intersect_holes_intersect[1],
        1, 14, 18.25,
        1, 14, 18.25);

    test_one<polygon, polygon, polygon>(
            "case4", case_4[0], case_4[1],
            4, 22, 1.1901714,
            4, 27, 1.6701714);

    test_one<polygon, polygon, polygon>(
            "case5", case_5[0], case_5[1],
            4, 22, 1.1901714,
            4, 27, 1.6701714);
}


int test_main(int, char* [])
{
    test_all<boost::geometry::point_xy<double> >();
    return 0;
}
