// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <string>


#include <geometry_test_common.hpp>

#include <boost/geometry/algorithms/disjoint.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/domains/gis/io/wkt/read_wkt.hpp>
#include <boost/geometry/strategies/strategies.hpp>

#include <test_common/test_point.hpp>

#include <algorithms/test_relate.hpp>


template <typename G1, typename G2>
void test_disjoint(std::string const& id,
            std::string const& wkt1,
            std::string const& wkt2, bool expected)
{
    G1 g1;
    bg::read_wkt(wkt1, g1);

    G2 g2;
    bg::read_wkt(wkt2, g2);

    bool detected = bg::disjoint(g1, g2);
    BOOST_CHECK_MESSAGE(detected == expected,
        "disjoint: " << id
        << " -> Expected: " << expected
        << " detected: " << detected);
}



template <typename P>
void test_all()
{
    typedef bg::model::box<P> box;

    test_disjoint<P, P>("pp1", "point(1 1)", "point(1 1)", false);
    test_disjoint<P, P>("pp2", "point(1 1)", "point(1.001 1)", true);

    // left-right
    test_disjoint<box, box>("bb1", "box(1 1, 2 2)", "box(3 1, 4 2)", true);
    test_disjoint<box, box>("bb2", "box(1 1, 2 2)", "box(2 1, 3 2)", false);
    test_disjoint<box, box>("bb3", "box(1 1, 2 2)", "box(2 2, 3 3)", false);
    test_disjoint<box, box>("bb4", "box(1 1, 2 2)", "box(2.001 2, 3 3)", true);

    // up-down
    test_disjoint<box, box>("bb5", "box(1 1, 2 2)", "box(1 3, 2 4)", true);
    test_disjoint<box, box>("bb6", "box(1 1, 2 2)", "box(1 2, 2 3)", false);
    // right-left
    test_disjoint<box, box>("bb7", "box(1 1, 2 2)", "box(0 1, 1 2)", false);
    test_disjoint<box, box>("bb8", "box(1 1, 2 2)", "box(0 1, 1 2)", false);

    // point-box
    test_disjoint<P, box>("pb1", "point(1 1)", "box(0 0, 2 2)", false);
    test_disjoint<P, box>("pb2", "point(2 2)", "box(0 0, 2 2)", false);
    test_disjoint<P, box>("pb3", "point(2.0001 2)", "box(1 1, 2 2)", true);
    test_disjoint<P, box>("pb4", "point(0.9999 2)", "box(1 1, 2 2)", true);

    // box-point (to test reverse compiling)
    test_disjoint<box, P>("bp1", "box(1 1, 2 2)", "point(2 2)", false);

    // Test triangles for polygons/rings, boxes
    // Note that intersections are tested elsewhere, they don't need
    // thorough test at this place
    typedef bg::model::polygon<P> polygon;
    typedef bg::model::ring<P> ring;

    // Four times same test with other types
    test_disjoint<polygon, polygon>("disjoint_simplex_pp", disjoint_simplex[0], disjoint_simplex[1], true);
    test_disjoint<ring, polygon>("disjoint_simplex_rp", disjoint_simplex[0], disjoint_simplex[1], true);
    test_disjoint<ring, ring>("disjoint_simplex_rr", disjoint_simplex[0], disjoint_simplex[1], true);
    test_disjoint<polygon, ring>("disjoint_simplex_pr", disjoint_simplex[0], disjoint_simplex[1], true);

    // Testing touch
    test_disjoint<polygon, polygon>("touch_simplex_pp", touch_simplex[0], touch_simplex[1], false);

    // Testing overlap (and test compiling with box)
    test_disjoint<polygon, polygon>("overlaps_box_pp", overlaps_box[0], overlaps_box[1], false);
    test_disjoint<box, polygon>("overlaps_box_bp", overlaps_box[0], overlaps_box[1], false);
    test_disjoint<box, ring>("overlaps_box_br", overlaps_box[0], overlaps_box[1], false);
    test_disjoint<polygon, box>("overlaps_box_pb", overlaps_box[1], overlaps_box[0], false);
    test_disjoint<ring, box>("overlaps_box_rb", overlaps_box[1], overlaps_box[0], false);

    // Test if within(a,b) returns false for disjoint
    test_disjoint<ring, ring>("within_simplex_rr1", within_simplex[0], within_simplex[1], false);
    test_disjoint<ring, ring>("within_simplex_rr2", within_simplex[1], within_simplex[0], false);

    // Linear
    typedef bg::model::linestring<P> ls;
    typedef bg::model::segment<P> segment;
    test_disjoint<ls, ls>("ls/ls 1", "linestring(0 0,1 1)", "linestring(1 0,0 1)", false);
    test_disjoint<ls, ls>("ls/ls 2", "linestring(0 0,1 1)", "linestring(1 0,2 1)", true);
    test_disjoint<segment, segment>("s/s 1", "linestring(0 0,1 1)", "linestring(1 0,0 1)", false);
    test_disjoint<segment, segment>("s/s 2", "linestring(0 0,1 1)", "linestring(1 0,2 1)", true);
    // TODO test_disjoint<segment, ls>("s/ls 1", "linestring(0 0,1 1)", "linestring(1 0,0 1)", false);
    // TODO test_disjoint<segment, ls>("s/ls 2", "linestring(0 0,1 1)", "linestring(1 0,2 1)", true);
    // TODO test_disjoint<ls, segment>("ls/s 1", "linestring(0 0,1 1)", "linestring(1 0,0 1)", false);
    // TODO test_disjoint<ls, segment>("ls/s 2", "linestring(0 0,1 1)", "linestring(1 0,2 1)", true);

}

int test_main(int, char* [])
{
    test_all<bg::model::d2::point_xy<float> >();
    test_all<bg::model::d2::point_xy<double> >();

#ifdef HAVE_TTMATH
    test_all<bg::model::d2::point_xy<ttmath_big> >();
#endif

    return 0;
}
