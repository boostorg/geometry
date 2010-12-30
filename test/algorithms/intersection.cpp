// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <string>

//#define BOOST_GEOMETRY_DEBUG_TRAVERSE
//#define BOOST_GEOMETRY_DEBUG_ASSEMBLE
//#define BOOST_GEOMETRY_DEBUG_IDENTIFIER


#include <algorithms/test_intersection.hpp>
#include <algorithms/test_overlay.hpp>

#include <boost/geometry/geometries/adapted/std_as_linestring.hpp>

#include <test_common/test_point.hpp>
#include <test_common/with_pointer.hpp>
#include <test_geometries/custom_segment.hpp>


template <typename Polygon>
void test_areal()
{
    test_one<Polygon, Polygon, Polygon>("simplex_normal",
        simplex_normal[0], simplex_normal[1],
        1, 7, 5.47363293);
    test_one<Polygon, Polygon, Polygon>("star_ring", example_star, example_ring,
        1, 18, 2.80983);

    test_one<Polygon, Polygon, Polygon>("star_poly", example_star, example_polygon,
        1, 0, // CLN: 23 points, other types: 22 point (one is merged)
        2.5020508);
    test_one<Polygon, Polygon, Polygon>("first_within_second1",
        first_within_second[0], first_within_second[1],
        1, 5, 1.0);

    test_one<Polygon, Polygon, Polygon>("first_within_second2",
        first_within_second[1], first_within_second[0],
        1, 5, 1.0);

    test_one<Polygon, Polygon, Polygon>("first_within_hole_of_second",
            first_within_hole_of_second[0], first_within_hole_of_second[1],
            0, 0, 0.0);

    // Two forming new hole
    test_one<Polygon, Polygon, Polygon>("new_hole",
        new_hole[0], new_hole[1],
        2, 10, 2.0);

    // Two identical
    test_one<Polygon, Polygon, Polygon>("identical",
        identical[0], identical[1],
        1, 5, 1.0);

    // Two, inside each other, having intersections but holes are disjoint
    test_one<Polygon, Polygon, Polygon>("intersect_holes_disjoint",
        intersect_holes_disjoint[0], intersect_holes_disjoint[1],
        1, 15, 18.0);

    // Two, inside each other, having intersections; holes separate intersections
    test_one<Polygon, Polygon, Polygon>("intersect_holes_intersect",
        intersect_holes_intersect[0], intersect_holes_intersect[1],
        1, 14, 18.25);

    test_one<Polygon, Polygon, Polygon>("intersect_holes_intersect_and_disjoint",
        intersect_holes_intersect_and_disjoint[0], intersect_holes_intersect_and_disjoint[1],
        1, 19, 17.25);

    test_one<Polygon, Polygon, Polygon>("intersect_holes_intersect_and_touch",
        intersect_holes_intersect_and_touch[0], intersect_holes_intersect_and_touch[1],
        1, 23, 17.25);

    test_one<Polygon, Polygon, Polygon>("intersect_holes_new_ring",
        intersect_holes_new_ring[0], intersect_holes_new_ring[1],
        2, 23, 122.1039);

    test_one<Polygon, Polygon, Polygon>("winded",
        winded[0], winded[1],
        1, 22, 40.0);

    test_one<Polygon, Polygon, Polygon>("two_bends",
        two_bends[0], two_bends[1],
        1, 7, 24.0);

    test_one<Polygon, Polygon, Polygon>("star_comb_15",
        star_15, comb_15,
        28, 150, 189.952883);

    test_one<Polygon, Polygon, Polygon>("simplex_normal",
        simplex_normal[0], simplex_normal[1],
        1, 7, 5.47363293);

    test_one<Polygon, Polygon, Polygon>("fitting",
        fitting[0], fitting[1],
        0, 0, 0);

    test_one<Polygon, Polygon, Polygon>("dist_zero",
        distance_zero[0], distance_zero[1],
        1, 0 /* f: 4, other: 5 */, 0.29516139, 0.01);

    test_one<Polygon, Polygon, Polygon>("ehd",
        equal_holes_disjoint[0], equal_holes_disjoint[1],
        1, 20, 81 - 2 * 3 * 3 - 3 * 7);

    test_one<Polygon, Polygon, Polygon>("only_hole_intersections1",
        only_hole_intersections[0], only_hole_intersections[1],
        1, 21, 178.090909);
    test_one<Polygon, Polygon, Polygon>("only_hole_intersection2",
        only_hole_intersections[0], only_hole_intersections[2],
        1, 21, 149.090909);

    test_one<Polygon, Polygon, Polygon>("intersect_exterior_and_interiors_winded",
        intersect_exterior_and_interiors_winded[0], intersect_exterior_and_interiors_winded[1],
        1, 14, 25.2166667);

    return;


    test_one<Polygon, Polygon, Polygon>(
            "polygon_pseudo_line",
            "Polygon((0 0,0 4,4 4,4 0,0 0))",
            "Polygon((2 -2,2 -1,2 6,2 -2))",
            5, 22, 1.1901714);


    // Icovist (submitted by Brandon during Formal Review)
    // Test the FORWARD case
    {
        std::string tn = string_from_type<typename bg::coordinate_type<Polygon>::type>::name();
        test_one<Polygon, Polygon, Polygon>("isovist",
            isovist1[0], isovist1[1],
            1,
            tn == std::string("f") ? 19 : tn == std::string("d") ? 21 : 20,
            88.19203,
            tn == std::string("f") ? 0.5 : tn == std::string("d") ? 0.1 : 0.01);
    }


    // Test the REVERSE case - does not give correct results yet
    /*
    test_one<Polygon, Polygon, Polygon>("icovist_r",
        isovist[0], isovist[2],
        1, 4, 0.29516139, 0.01);
    */

}

template <typename Polygon, typename Box>
void test_areal_clip()
{
    test_one<Polygon, Box, Polygon>("boxring", example_box, example_ring,
        2, 12, 1.09125);
    test_one<Polygon, Polygon, Box>("boxring2", example_ring,example_box,
        2, 12, 1.09125);

    test_one<Polygon, Box, Polygon>("boxpoly", example_box, example_polygon,
        3, 19, 0.840166);

    test_one<Polygon, Box, Polygon>("poly1", example_box,
        "POLYGON((3.4 2,4.1 3,5.3 2.6,5.4 1.2,4.9 0.8,2.9 0.7,2 1.3,2.4 1.7,2.8 1.8,3.4 1.2,3.7 1.6,3.4 2))",
        2, 12, 1.09125);

    test_one<Polygon, Box, Polygon>("clip_poly2", example_box,
        "POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2,3.7 1.6,3.4 2,4.1 2.5,5.3 2.5,5.4 1.2,4.9 0.8,2.9 0.7,2 1.3))",
        2, 12, 1.00375);
    test_one<Polygon, Box, Polygon>("clip_poly3", example_box,
        "POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2,3.7 1.6,3.4 2,4.1 2.5,4.5 2.5,4.5 1.2,4.9 0.8,2.9 0.7,2 1.3))",
        2, 12, 1.00375);
    test_one<Polygon, Box, Polygon>("clip_poly4", example_box,
        "POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2,3.7 1.6,3.4 2,4.1 2.5,4.5 2.5,4.5 2.3,5.0 2.3,5.0 2.1,4.5 2.1,4.5 1.9,4.0 1.9,4.5 1.2,4.9 0.8,2.9 0.7,2 1.3))",
        2, 16, 0.860892);

    test_one<Polygon, Box, Polygon>("clip_poly5", example_box,
            "POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2,3.7 1.6,3.4 2,4.1 2.5,4.5 1.2,2.9 0.7,2 1.3))",
                2, 11, 0.7575961);

    test_one<Polygon, Box, Polygon>("clip_poly6", example_box,
            "POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2,3.7 1.6,3.4 2,4.0 3.0,5.0 2.0,2.9 0.7,2 1.3))",
                2, 13, 1.0744456);

    test_one<Polygon, Box, Polygon>("clip_poly7", "Box(0 0, 3 3)",
            "POLYGON((2 2, 1 4, 2 4, 3 3, 2 2))", 1, 4, 0.75);
}

template <typename Box>
void test_boxes(std::string const& wkt1, std::string const& wkt2, double expected_area, bool expected_result)
{
    Box box1, box2;
    bg::read_wkt(wkt1, box1);
    bg::read_wkt(wkt2, box2);

    Box box_out;
    bool detected = bg::intersection(box1, box2, box_out);
    double area = bg::area(box_out);

    BOOST_CHECK_EQUAL(detected, expected_result);
    if (detected && expected_result)
    {
        BOOST_CHECK_CLOSE(area, expected_area, 0.01);
    }
}


template <typename P>
void test_all()
{
    typedef bg::model::linestring<P> linestring;
    typedef bg::model::polygon<P> polygon;
    typedef bg::model::box<P> box;
    typedef bg::model::segment<P> segment;

    typedef bg::model::polygon<P, false> polygon_ccw;
    typedef bg::model::polygon<P, true, false> polygon_open;
    typedef bg::model::polygon<P, false, false> polygon_ccw_open;

    std::string clip = "box(2 2,8 8)";

    // Test polygons clockwise and counter clockwise
    test_areal<polygon>();
    test_areal<polygon_ccw>();
    test_areal<polygon_open>();
    test_areal<polygon_ccw_open>();

    test_areal_clip<polygon, box>();
    test_areal_clip<polygon_ccw, box>();

#if defined(TEST_FAIL_DIFFERENT_ORIENTATIONS)
    // Should NOT compile
    // NOTE: this can probably be relaxed later on.        
    test_one<polygon, polygon_ccw, polygon>("simplex_normal",
        simplex_normal[0], simplex_normal[1],
        1, 7, 5.47363293);
    // Output ccw, nyi (should be just reversing afterwards)
    test_one<polygon, polygon, polygon_ccw>("simplex_normal",
        simplex_normal[0], simplex_normal[1],
        1, 7, 5.47363293);
#endif

       // Basic check: box/linestring, is clipping OK? should compile in any order
    test_one<linestring, linestring, box>("llb", "LINESTRING(0 0,10 10)", clip, 1, 2, sqrt(2.0 * 6.0 * 6.0));
    test_one<linestring, box, linestring>("lbl", clip, "LINESTRING(0 0,10 10)", 1, 2, sqrt(2.0 * 6.0 * 6.0));

    // Box/segment
    test_one<linestring, segment, box>("lsb", "LINESTRING(0 0,10 10)", clip, 1, 2, sqrt(2.0 * 6.0 * 6.0));
    test_one<linestring, box, segment>("lbs", clip, "LINESTRING(0 0,10 10)", 1, 2, sqrt(2.0 * 6.0 * 6.0));

    // Completely inside
    test_one<linestring, linestring, box>("llbi", "LINESTRING(3 3,7 7)", clip, 1, 2, sqrt(2.0 * 4.0 * 4.0));

    // Completely outside
    test_one<linestring, linestring, box>("llbo", "LINESTRING(9 9,10 10)", clip, 0, 0, 0);

    // Touching with point (-> output linestring with ONE point)
    //std::cout << "Note: the output line is degenerate! Might be removed!" << std::endl;
    test_one<linestring, linestring, box>("llb_touch", "LINESTRING(8 8,10 10)", clip, 1, 1, 0.0);

    // Along border
    test_one<linestring, linestring, box>("llb_along", "LINESTRING(2 2,2 8)", clip, 1, 2, 6);

    // Outputting two lines (because of 3-4-5 constructions (0.3,0.4,0.5)
    // which occur 4 times, the length is expected to be 2.0)
    test_one<linestring, linestring, box>("llb_2", "LINESTRING(1.7 1.6,2.3 2.4,2.9 1.6,3.5 2.4,4.1 1.6)", clip, 2, 6, 4 * 0.5);
    



    // linear
    test_one<P, linestring, linestring>("llp1", "LINESTRING(0 0,1 1)", "LINESTRING(0 1,1 0)", 1, 1, 0);
    test_one<P, segment, segment>("ssp1", "LINESTRING(0 0,1 1)", "LINESTRING(0 1,1 0)", 1, 1, 0);
    test_one<P, linestring, linestring>("llp2", "LINESTRING(0 0,1 1)", "LINESTRING(0 0,2 2)", 2, 2, 0);

    // polygons outputing points
    //test_one<P, polygon, polygon>("ppp1", simplex_normal[0], simplex_normal[1], 1, 7, 5.47363293);

    test_boxes<box>("box(2 2,8 8)", "box(4 4,10 10)", 16, true);
    test_boxes<box>("box(2 2,8 7)", "box(4 4,10 10)", 12, true);
    test_boxes<box>("box(2 2,8 7)", "box(14 4,20 10)", 0, false);
    test_boxes<box>("box(2 2,4 4)", "box(4 4,8 8)", 0, true);


    /*
    test_one<polygon, box, polygon>(99, "box(115041.10 471900.10, 118334.60 474523.40)",
            "POLYGON ((115483.40 474533.40, 116549.40 474059.20, 117199.90 473762.50, 117204.90 473659.50, 118339.40 472796.90, 118334.50 472757.90, 118315.10 472604.00, 118344.60 472520.90, 118277.90 472419.10, 118071.40 472536.80, 118071.40 472536.80, 117943.10 472287.70, 117744.90 472248.40, 117708.00 472034.50, 117481.90 472056.90, 117481.90 472056.90, 117272.30 471890.10, 117077.90 472161.20, 116146.60 473054.50, 115031.10 473603.30, 115483.40 474533.40))",
                1, 26, 3727690.74);
    */

}

void test_pointer_version()
{
    std::vector<test::test_point_xy*> ln;
    test::test_point_xy* p;
    p = new test::test_point_xy; p->x = 0; p->y = 0; ln.push_back(p);
    p = new test::test_point_xy; p->x = 10; p->y = 10; ln.push_back(p);

    bg::model::box<bg::model::d2::point_xy<double> > box;
    bg::assign(box, 2, 2, 8, 8);

    typedef bg::model::linestring<bg::model::d2::point_xy<double> > output_type;
    std::vector<output_type> clip;
    bg::intersection_inserter<output_type>(box, ln, std::back_inserter(clip));

    double length = 0;
    int n = 0;
    for (std::vector<output_type>::const_iterator it = clip.begin();
            it != clip.end(); ++it)
    {
        length += bg::length(*it);
        n += bg::num_points(*it);
    }

    BOOST_CHECK_EQUAL(clip.size(), 1u);
    BOOST_CHECK_EQUAL(n, 2);
    BOOST_CHECK_CLOSE(length, sqrt(2.0 * 6.0 * 6.0), 0.001);


    for (unsigned int i = 0; i < ln.size(); i++)
    {
        delete ln[i];
    }
}




int test_main(int, char* [])
{
    //test_all<bg::model::d2::point_xy<float> >();
    test_all<bg::model::d2::point_xy<double> >();

#if defined(HAVE_TTMATH)
    test_all<bg::model::d2::point_xy<ttmath_big> >();
#endif

    //test_pointer_version();
    return 0;
}

