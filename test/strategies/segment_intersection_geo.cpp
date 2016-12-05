// Boost.Geometry
// Unit Test

// Copyright (c) 2016, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include "segment_intersection_geo.hpp"


template <typename T>
void test_geographic()
{
    typedef bg::model::point<double, 2, bg::cs::geographic<bg::degree> > point_t;
    typedef bg::model::segment<point_t> segment_t;

    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-1 -2, -1 -1)", 'a', 1, "POINT(-1 -1)");
    test_strategies<segment_t, point_t>(
        "SEGMENT(-1 -2, -1 2)", "SEGMENT(-2 -2, 2 2)",
        great_elliptic('i', 1, "POINT(-1 -1.000457053724121)"),
        geodesic_vincenty('i', 1, "POINT(-1 -1.000459099991114)"),
        geodesic_andoyer('i', 1, "POINT(-1 -1.000453510849886)"));
    test_strategies<segment_t, point_t>(
        "SEGMENT(-2 -2, 2 2)", "SEGMENT(-1 -2, -1 2)",
        great_elliptic('i', 1, "POINT(-1 -1.000457053724121)"),
        geodesic_vincenty('i', 1, "POINT(-1 -1.000459099991114)"),
        geodesic_andoyer('i', 1, "POINT(-1 -1.000453510849886)"));

    // crossing   X
    test_strategies<segment_t, point_t>(
        "SEGMENT(-45 -45, 45 45)", "SEGMENT(-45 45, 45 -45)",
        great_elliptic('i', 1, "POINT(0 0)"),
        geodesic_vincenty('i', 1, "POINT(0.00000000138009992627 0.00000000000000636110)"),
        geodesic_andoyer('i', 1, "POINT(-0.0002480157764363963 0.00000000000000636111)"));
    test_strategies<segment_t, point_t>(
        "SEGMENT(-45 -45, 45 45)", "SEGMENT(45 -45, -45 45)",
        great_elliptic('i', 1, "POINT(0 0)"),
        geodesic_vincenty('i', 1, "POINT(0 -0.0000000019543337739)"),
        geodesic_andoyer('i', 1, "POINT(0 0.0003512140328446071)"));
    test_strategies<segment_t, point_t>(
        "SEGMENT(45 45, -45 -45)", "SEGMENT(-45 45, 45 -45)",
        great_elliptic('i', 1, "POINT(0 0)"),
        geodesic_vincenty('i', 1, "POINT(0 0.0000000019543337739)"),
        geodesic_andoyer('i', 1, "POINT(0 -0.0003512140328446071)"));
    test_strategies<segment_t, point_t>(
        "SEGMENT(45 45, -45 -45)", "SEGMENT(45 -45, -45 45)",
        great_elliptic('i', 1, "POINT(0 0)"),
        geodesic_vincenty('i', 1, "POINT(-0.00000000138009992627 -0.00000000000000636110)"),
        geodesic_andoyer('i', 1, "POINT(0.0002480157764363963 -0.00000000000000636111)"));
    // crossing   X
    test_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-1 1, 1 -1)",
        great_elliptic('i', 1, "POINT(0 0)"),
        geodesic_vincenty('i', 1, "POINT(0.00000000006265374667 0)"),
        geodesic_andoyer('i', 1, "POINT(-0.00000557785856154582 0)"));
    test_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(1 -1, -1 1)",
        great_elliptic('i', 1, "POINT(0 0)"),
        geodesic_vincenty('i', 1, "POINT(0.000000000000000596354 -0.0000000000626626779)"),
        geodesic_andoyer('i', 1, "POINT(-0.000000000000000596354 0.0000055787431347552)"));
    test_strategies<segment_t, point_t>(
        "SEGMENT(1 1, -1 -1)", "SEGMENT(-1 1, 1 -1)",
        great_elliptic('i', 1, "POINT(0 0)"),
        geodesic_vincenty('i', 1, "POINT(-0.00000000000000059635 0.0000000000626626779)"),
        geodesic_andoyer('i', 1, "POINT(0.000000000000000596354 -0.000005578743134557)"));
    test_strategies<segment_t, point_t>(
        "SEGMENT(1 1, -1 -1)", "SEGMENT(1 -1, -1 1)",
        great_elliptic('i', 1, "POINT(0 0)"),
        geodesic_vincenty('i', 1, "POINT(-0.00000000006265374 0)"),
        geodesic_andoyer('i', 1, "POINT(0.00000557785856134 0)"));

    // equal
    //   //
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-45 -45, 45 45)", "SEGMENT(-45 -45, 45 45)", 'e', 2, "POINT(-45 -45)", "POINT(45 45)");
    //   //
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-45 -45, 45 45)", "SEGMENT(45 45, -45 -45)", 'e', 2, "POINT(-45 -45)", "POINT(45 45)");

    // starting outside s1
    //    /
    //   |
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-2 -2, -1 -1)", 'a', 1, "POINT(-1 -1)");
    //   /
    //  /|
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-2 -2, 0 0)", 'm', 1, "POINT(0 0)");
    //   /|
    //  / |
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-2 -2, 1 1)", 't', 1, "POINT(1 1)");
    //   |/
    //  /|
    test_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-2 -2, 2 2)",
        great_elliptic('i', 1, "POINT(0 0)"),
        geodesic_vincenty('i', 1, "POINT(0.00000013628420059 0.00000013624239008)"),
        geodesic_thomas('i', 1, "POINT(-0.00004079969079346 -0.00004078714535240)"),
        geodesic_andoyer('i', 1, "POINT(-0.01217344899138908 -0.01216980051876599)"));
    //       ------
    // ------
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-2 0, -1 0)", 'a', 1, "POINT(-1 0)");
    //    ------
    // ------
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-2 0, 0 0)", 'c', 2, "POINT(-1 0)", "POINT(0 0)");
    //    ------
    // ---------
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-2 0, 1 0)", 'c', 2, "POINT(-1 0)", "POINT(1 0)");
    //    ------
    // ------------
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-2 0, 2 0)", 'c', 2, "POINT(-1 0)", "POINT(1 0)");

    // starting at s1
    //  /
    // //
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-1 -1, 0 0)", 'c', 2, "POINT(-1 -1)", "POINT(0 0)");
    //  //
    // //
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-1 -1, 1 1)", 'e', 2, "POINT(-1 -1)", "POINT(1 1)");
    // | /
    // |/
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-1 -1, 2 2)", 'f', 1, "POINT(-1 -1)");
    // ------
    // ---
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-1 0, 0 0)", 'c', 2, "POINT(-1 0)", "POINT(0 0)");
    // ------
    // ------
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-1 0, 1 0)", 'e', 2, "POINT(-1 0)", "POINT(1 0)");
    // ------
    // ---------
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-1 0, 2 0)", 'c', 2, "POINT(-1 0)", "POINT(1 0)");
    
    // starting inside
    //   //
    //  /
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(0 0, 1 1)", 'c', 2, "POINT(0 0)", "POINT(1 1)");
    //   |/
    //   /
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(0 0, 2 2)", 's', 1, "POINT(0 0)");
    // ------
    //    ---
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(0 0, 1 0)", 'c', 2, "POINT(0 0)", "POINT(1 0)");
    // ------
    //    ------
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(0 0, 2 0)", 'c', 2, "POINT(0 0)", "POINT(1 0)");
    
    // starting at p2
    //    |
    //   /
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(1 1, 2 2)", 'a', 1, "POINT(1 1)");
    // ------
    //       ---
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(1 0, 2 0)", 'a', 1, "POINT(1 0)");

    // disjoint, crossing
    //     /
    //  |
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-3 -3, -2 -2)", 'd', 0);
    //     |
    //  /
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(2 2, 3 3)", 'd', 0);
    // disjoint, collinear
    //          ------
    // ------
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-3 0, -2 0)", 'd', 0);
    // ------
    //           ------
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(2 0, 3 0)", 'd', 0);

    // degenerated
    //    /
    // *
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-2 -2, -2 -2)", 'd', 0);
    //    /
    //   *
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-1 -1, -1 -1)", '0', 1, "POINT(-1 -1)");
    //    /
    //   *
    //  /
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(0 0, 0 0)", '0', 1, "POINT(0 0)");
    //    *
    //   /
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(1 1, 1 1)", '0', 1, "POINT(1 1)");
    //       *
    //   /
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(2 2, 2 2)", 'd', 0);
    // similar to above, collinear
    // *   ------
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-2 0, -2 0)", 'd', 0);
    //    *------
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-1 0, -1 0)", '0', 1, "POINT(-1 0)");
    //    ---*---
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(0 0, 0 0)", '0', 1, "POINT(0 0)");
    //    ------*
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(1 0, 1 0)", '0', 1, "POINT(1 0)");
    //    ------   *
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(2 0, 2 0)", 'd', 0);

    // Northern hemisphere
    // ---   ------
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(-3 50, -2 50)", 'd', 0);
    //    ------
    // ---
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(-2 50, -1 50)", 'a', 1, "POINT(-1 50)");
    //  \/
    //  /\                   (avoid multi-line comment)
    test_strategies<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(-2 50, 0 50)",
        great_elliptic('i', 1, "POINT(-0.5 50.0032229484023)"),
        geodesic_vincenty('i', 1, "POINT(-0.4999999996073994 50.00323192256208)"),
        geodesic_thomas('i', 1, "POINT(-0.4999999963998482 50.00323192258598)"),
        geodesic_andoyer('i', 1, "POINT(-0.4999990340391772 50.00323192463806)"));
    //  ________
    // /   _____\            (avoid multi-line comment)
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(-2 50, 1 50)", 't', 1, "POINT(1 50)");
    //  _________
    // /  _____  \           (avoid multi-line comment)
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(-2 50, 2 50)", 'd', 0);
    //  ______
    // /___   \              (avoid multi-line comment)
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(-1 50, 0 50)", 'f', 1, "POINT(-1 50)");
    // ------
    // ------
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(-1 50, 1 50)", 'e', 2, "POINT(-1 50)", "POINT(1 50)");
    //  ________
    // /_____   \            (avoid multi-line comment)
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(-1 50, 2 50)", 'f', 1, "POINT(-1 50)");
    //  ______
    // /   ___\              (avoid multi-line comment)
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(0 50, 1 50)", 't', 1, "POINT(1 50)");
    //  \/
    //  /\                   (avoid multi-line comment)
    test_strategies<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(0 50, 2 50)",
        great_elliptic('i', 1, "POINT(0.5 50.0032229484023)"),
        geodesic_vincenty('i', 1, "POINT(0.4999999996112415 50.00323192256208)"),
        geodesic_thomas('i', 1, "POINT(0.5000000051005989 50.00323192258598)"),
        geodesic_andoyer('i', 1, "POINT(0.5000009655139563 50.00323192463806)"));
    // ------
    //       ---
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(1 50, 2 50)", 'a', 1, "POINT(1 50)");
    // ------   ---
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(2 50, 3 50)", 'd', 0);

    // ___|
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(0 0, 1 0)", "SEGMENT(1 0, 1 1)", 'a', 1, "POINT(1 0)");
    // ___|
    test_all_strategies<segment_t, point_t>(
        "SEGMENT(1 0, 1 1)", "SEGMENT(0 0, 1 0)", 'a', 1, "POINT(1 0)");

    //   |/
    //  /|
    test_strategies<segment_t, point_t>(
        "SEGMENT(10 -1, 20 1)", "SEGMENT(12.5 -1, 12.5 1)",
        great_elliptic('i', 1, "POINT(12.5 -0.50051443471392)"),
        geodesic_vincenty('i', 1, "POINT(12.5 -0.5005177749487734)"),
        geodesic_thomas('i', 1, "POINT(12.5 -0.5005177654827411)"),
        geodesic_andoyer('i', 1, "POINT(12.5 -0.500525380045163)"));
    //   |/
    //  /|
    test_strategies<segment_t, point_t>(
        "SEGMENT(10 -1, 20 1)", "SEGMENT(17.5 -1, 17.5 1)",
        great_elliptic('i', 1, "POINT(17.5 0.50051443471392)"),
        geodesic_vincenty('i', 1, "POINT(17.5 0.5005177748229335)"),
        geodesic_thomas('i', 1, "POINT(17.5 0.5005178030678186)"),
        geodesic_andoyer('i', 1, "POINT(17.5 0.5004949944844279)"));
}

int test_main(int, char* [])
{
    //test_geographic<float>();
    test_geographic<double>();

    return 0;
}
