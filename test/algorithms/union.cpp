// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <string>

//#define BOOST_GEOMETRY_DEBUG_ASSEMBLE
//#define BOOST_GEOMETRY_DEBUG_IDENTIFIER


#include <algorithms/test_union.hpp>
#include <algorithms/test_overlay.hpp>

#include <algorithms/overlay/overlay_cases.hpp>

template <typename P>
void test_all()
{
    typedef boost::geometry::polygon<P> polygon;
    typedef boost::geometry::linear_ring<P> ring;
    typedef boost::geometry::box<P> box;


    test_one<polygon, box, polygon>("box_ring", example_box, example_ring,
        1, 1, 15, 6.38875);

    test_one<polygon, box, polygon>("box_poly", example_box, example_polygon,
        1, 3, 23, 6.30983);

    test_one<polygon, polygon, polygon>("star_ring", example_star, example_ring,
        1, 0, 23, 5.67017141);

    test_one<polygon, polygon, polygon>("star_poly", example_star, example_polygon,
        1, 1, 27,  5.647949);

    test_one<polygon, box, polygon>("box_poly1", example_box,
            "POLYGON((3.4 2,4.1 3,5.3 2.6,5.4 1.2,4.9 0.8,2.9 0.7,2 1.3,2.4 1.7,2.8 1.8,3.4 1.2,3.7 1.6,3.4 2))",
                1, 1, 15, 6.38875);
    test_one<polygon, box, polygon>("box_poly2", example_box,
            "POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2,3.7 1.6,3.4 2,4.1 2.5,5.3 2.5,5.4 1.2,4.9 0.8,2.9 0.7,2 1.3))",
                1, 1, 15, 5.93625);

    // Same as 3, but now as polygon
    // (note, internally, the intersection points is different, so yes,
    // it has to be tested)
    test_one<polygon, polygon, polygon>("box_poly3", "POLYGON((1.5 1.5 , 1.5 2.5 , 4.5 2.5 , 4.5 1.5 , 1.5 1.5))",
            "POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2,3.7 1.6,3.4 2,4.1 2.5,5.3 2.5,5.4 1.2,4.9 0.8,2.9 0.7,2 1.3))",
                1, 1, 15, 5.93625);

    test_one<polygon, box, polygon>("box_poly4", example_box,
            "POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2,3.7 1.6,3.4 2,4.1 2.5,4.5 2.5,4.5 1.2,4.9 0.8,2.9 0.7,2 1.3))",
                1, 1, 15, 4.651245);

    test_one<polygon, box, polygon>("box_poly5", example_box,
            "POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2,3.7 1.6,3.4 2,4.1 2.5,4.5 2.5,4.5 2.3,5.0 2.3,5.0 2.1,4.5 2.1,4.5 1.9,4.0 1.9,4.5 1.2,4.9 0.8,2.9 0.7,2 1.3))",
                1, 1, 21, 4.7191);

    test_one<polygon, box, polygon>("box_poly6", example_box,
            "POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2,3.7 1.6,3.4 2,4.1 2.5,4.5 1.2,2.9 0.7,2 1.3))",
                1, 1, 15, 4.2174);

    test_one<polygon, box, polygon>("box_poly7", example_box,
            "POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2,3.7 1.6,3.4 2,4.0 3.0,5.0 2.0,2.9 0.7,2 1.3))",
                1, 1, 17, 4.270554);

    test_one<polygon, box, polygon>("box_poly8", "box(0 0, 3 3)",
            "POLYGON((2 2, 1 4, 2 4, 3 3, 2 2))",
                1, 0, 8, 10.25);


    // First inside second
    test_one<polygon, polygon, polygon>("first_within_second",
        first_within_second[0], first_within_second[1],
        1, 0, 5, 25.0);

    // Second inside first
    test_one<polygon, polygon, polygon>("seond_within_first",
        first_within_second[1], first_within_second[0],
        1, 0, 5, 25.0);

    // First inside hole of second
    test_one<polygon, polygon, polygon>("first_within_hole_of_second",
        first_within_hole_of_second[0], first_within_hole_of_second[1],
        2, 1, 15, 17.0);

    // forming new hole
    test_one<polygon, polygon, polygon>("new_hole",
        new_hole[0], new_hole[1],
        1, 1, 14, 23.0);

    // side by side
    test_one<polygon, polygon, polygon>("side_side",
        side_side[0], side_side[1], 1, 0, 7, 2.0);

    // identical
    test_one<polygon, polygon, polygon>("identical",
        identical[0], identical[1], 1, 0, 5, 1.0);

    // inside each other, having intersections; holes separate intersections
    test_one<polygon, polygon, polygon>("intersect_holes_intersect",
        intersect_holes_intersect[0], intersect_holes_intersect[1],
        1, 1, 14, 39.75);

    test_one<polygon, polygon, polygon>("intersect_holes_intersect_and_disjoint",
        intersect_holes_intersect_and_disjoint[0], intersect_holes_intersect_and_disjoint[1],
        1, 1, 14, 39.75);

    test_one<polygon, polygon, polygon>("intersect_holes_intersect_and_touch",
        intersect_holes_intersect_and_touch[0], intersect_holes_intersect_and_touch[1],
        1, 1, 14, 39.75);

    test_one<polygon, polygon, polygon>("intersect_holes_new_ring",
        intersect_holes_new_ring[0], intersect_holes_new_ring[1],
        1, 2, 15, 253.8961);

    // inside each other, having intersections but holes are disjoint
    test_one<polygon, polygon, polygon>("intersect_holes_disjoint",
        intersect_holes_disjoint[0],
        intersect_holes_disjoint[1],
        1, 0, 9, 40.0);

    // inside each other, having no intersections but holes are disjoint
    test_one<polygon, polygon, polygon>("within_holes_disjoint",
        within_holes_disjoint[0], within_holes_disjoint[1],
        1, 0, 5, 49.0);

    test_one<polygon, polygon, polygon>("winded",
        winded[0], winded[1],
        1, 5, 30, 114.0);

    test_one<polygon, polygon, polygon>("two_bends",
        two_bends[0], two_bends[1],
        1, 0, 7, 40.0);


    test_one<polygon, polygon, polygon>("equal_holes_disjoint",
        equal_holes_disjoint[0], equal_holes_disjoint[1],
        1, 1, 10, 81 - 3 * 7);

    test_one<polygon, polygon, polygon>("only_hole_intersections1",
        only_hole_intersections[0], only_hole_intersections[1],
        1, 2, 13, 190.9090909);
    test_one<polygon, polygon, polygon>("only_hole_intersections2",
        only_hole_intersections[0], only_hole_intersections[2],
        1, 2, 13, 190.9090909);

    test_one<polygon, polygon, polygon>("intersect_exterior_and_interiors_winded",
        intersect_exterior_and_interiors_winded[0], intersect_exterior_and_interiors_winded[1],
        1, 1, 26, 66.5333333);


    test_one<polygon, polygon, polygon>("simplex_normal",
        simplex_normal[0], simplex_normal[1],
        1, 0, 13, 11.526367);

    test_one<polygon, polygon, polygon>("fitting",
        fitting[0], fitting[1],
        1, 0, 5, 25);

    test_one<polygon, polygon, polygon>("wrapped_a",
        wrapped[0], wrapped[1],
        1, 1, 16, 16);
    test_one<polygon, polygon, polygon>("wrapped_b",
        wrapped[0], wrapped[2],
        1, 1, 16, 16);


    test_one<polygon, polygon, polygon>("9",
                case_9[0], case_9[1], 2, 0, 8, 11);
    test_one<polygon, polygon, polygon>("22",
                case_22[0], case_22[1], 2, 0, 8, 9.5);
    test_one<polygon, polygon, polygon>("25",
                case_25[0], case_25[1], 2, 0, 8, 7);
    test_one<polygon, polygon, polygon>("26",
                case_26[0], case_26[1], 2, 0, 9, 7.5);
    test_one<polygon, polygon, polygon>("31",
                case_31[0], case_31[1], 2, 0, 8, 4.5);
    test_one<polygon, polygon, polygon>("32",
                case_32[0], case_32[1], 2, 0, 8, 4.5);
    test_one<polygon, polygon, polygon>("33",
                case_33[0], case_33[1], 2, 0, 8, 4.5);
    test_one<polygon, polygon, polygon>("40",
                case_40[0], case_40[1], 2, 0, 18, 11);


    /*
    test_one<polygon, polygon, polygon>(102,
        simplex_normal[0], simplex_reversed[1],
        1, 0, 7, X);

    test_one<polygon, polygon, polygon>(103,
        simplex_reversed[0], simplex_normal[1],
        1, 0, 7, 24.0);

    test_one<polygon, polygon, polygon>(104,
        simplex_reversed[0], simplex_reversed[1],
        1, 0, 7, 24.0);

    test_one<polygon, polygon, polygon>(100,
        star_15, comb_15,
        1, 10, 7, 24.0);
    */

    // test some other input/output types

    // 1 input ring
    test_one<polygon, polygon, ring>("identical_pr", identical[0], identical[1], 1, 0, 5, 1.0);
    test_one<polygon, ring, polygon>("identical_rp", identical[0], identical[1], 1, 0, 5, 1.0);

    // 2 input rings
    test_one<polygon, ring, ring>("identical_rr", identical[0], identical[1], 1, 0, 5, 1.0);

    // output is also ring
    test_one<ring, ring, ring>("identical_rrr", identical[0], identical[1], 1, 0, 5, 1.0);

    // "new hole", tested with ring -> the newly formed hole will be omitted
    test_one<ring, ring, ring>("new_hole_discarded", new_hole[0], new_hole[1], 1, 0, 9, 24.0);

    // Isovist (submitted by Brandon during Formal Review)
    {
        std::string tn = string_from_type<typename boost::geometry::coordinate_type<P>::type>::name();
        //std::cout << tn << std::endl;
        test_one<polygon, polygon, polygon>("isovist", isovist[0], isovist[1], 1, 0,

            // Note, the number of resulting points differs per point type AND
            // per operating system (showing this test is quite demanding)
#if defined(_MSC_VER)
            tn == std::string("f") ? 71 : 72,
#else
            tn == std::string("f") ? 71 : 
                tn == std::string("e") ? 75 : 
                    tn == std::string("d") ? 72 : 
                        70,
#endif
            313.3603646,
            0.1);
    }
}


int test_main(int, char* [])
{
    //test_all<boost::geometry::point_xy<float> >();
    test_all<boost::geometry::point_xy<double> >();
    //test_all<boost::geometry::point_xy<long double> >();

#if defined(HAVE_CLN)
    test_all<boost::geometry::point_xy<boost::numeric_adaptor::cln_value_type> >();
#endif

    return 0;
}
