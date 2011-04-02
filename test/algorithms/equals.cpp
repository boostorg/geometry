// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <algorithms/test_equals.hpp>

#include <boost/geometry/geometries/geometries.hpp>




template <typename P>
void test_all()
{
    typedef bg::model::box<P> box;
    typedef bg::model::ring<P> ring;
    typedef bg::model::polygon<P> polygon;
    typedef bg::model::linestring<P> linestring;


    test_geometry<P, P>("p1", "POINT(1 1)", "POINT(1 1)", true);
    test_geometry<P, P>("p2", "POINT(1 1)", "POINT(1 2)", false);
    test_geometry<box, box>("b1", "BOX(1 1,2 2)", "BOX(1 2,2 2)", false);
    test_geometry<box, box>("b1", "BOX(1 2,3 4)", "BOX(1 2,3 4)", true);


    std::string case_p1 = "POLYGON((0 0,0 2,2 2,0 0))";

    // Completely equal
    test_geometry<ring, ring>("poly_eq", case_p1, case_p1, true);

    // Shifted
    test_geometry<ring, ring>("poly_sh", "POLYGON((2 2,0 0,0 2,2 2))", case_p1, true);
    test_geometry<polygon, polygon>("poly_sh2", case_p1, "POLYGON((0 2,2 2,0 0,0 2))", true);

    // Extra coordinate
    test_geometry<ring, ring>("poly_extra", case_p1, "POLYGON((0 0,0 2,2 2,1 1,0 0))", true);
    // Degenerate points
    test_geometry<ring, ring>("poly_degenerate", "POLYGON((0 0,0 2,2 2,2 2,0 0))", "POLYGON((0 0,0 2,0 2,2 2,0 0))", true);

    // Two different bends, same area, unequal
    test_geometry<ring, ring>("poly_bends",
        "POLYGON((4 0,5 3,8 4,7 7,4 8,0 4,4 0))",
        "POLYGON((4 0,7 1,8 4,5 5,4 8,0 4,4 0))", false);

    // Unequal (but same area)
    test_geometry<ring, ring>("poly_uneq", case_p1, "POLYGON((1 1,1 3,3 3,1 1))", false);

    // One having hole
    test_geometry<polygon, polygon>("poly_hole", "POLYGON((0 0,0 4,4 4,0 0))", "POLYGON((0 0,0 4,4 4,0 0),(1 1,2 1,2 2,1 2,1 1))", false);

    // Both having holes
    test_geometry<polygon, polygon>("poly_holes",
            "POLYGON((0 0,0 4,4 4,0 0),(1 1,2 1,2 2,1 2,1 1))",
            "POLYGON((0 0,0 4,4 4,0 0),(1 1,2 1,2 2,1 2,1 1))", true);

    // Both having holes, outer equal, inner not equal
    test_geometry<polygon, polygon>("poly_uneq_holes",
            "POLYGON((0 0,0 4,4 4,0 0),(1 1,2 1,2 2,1 2,1 1))",
            "POLYGON((0 0,0 4,4 4,0 0),(2 2,3 2,3 3,2 3,2 2))", false);

    // Both having 2 holes, equal but in different order
    test_geometry<polygon, polygon>("poly_holes_diff_order",
            "POLYGON((0 0,0 4,4 4,0 0),(1 1,2 1,2 2,1 2,1 1),(2 2,3 2,3 3,2 3,2 2))",
            "POLYGON((0 0,0 4,4 4,0 0),(2 2,3 2,3 3,2 3,2 2),(1 1,2 1,2 2,1 2,1 1))", true);

    // Both having 3 holes, equal but in different order
    test_geometry<polygon, polygon>("poly_holes_diff_order_3",
            "POLYGON((0 0,0 10,10 10,0 0),(1 1,2 1,2 2,1 2,1 1),(4 1,5 1,5 2,4 2,4 1),(2 2,3 2,3 3,2 3,2 2))",
            "POLYGON((0 0,0 10,10 10,0 0),(4 1,5 1,5 2,4 2,4 1),(2 2,3 2,3 3,2 3,2 2),(1 1,2 1,2 2,1 2,1 1))", true);

    // polygon/ring vv
    test_geometry<polygon, ring>("poly_sh2_pr", case_p1, case_p1, true);
    test_geometry<ring, polygon>("poly_sh2_rp", case_p1, case_p1, true);

    // box/ring/poly
    test_geometry<box, ring>("boxring1", "BOX(1 1,2 2)", "POLYGON((1 1,1 2,2 2,2 1,1 1))", true);
    test_geometry<ring, box>("boxring2", "POLYGON((1 1,1 2,2 2,2 1,1 1))", "BOX(1 1,2 2)", true);
    test_geometry<box, polygon>("boxpoly1", "BOX(1 1,2 2)", "POLYGON((1 1,1 2,2 2,2 1,1 1))", true);
    test_geometry<polygon, box>("boxpoly2", "POLYGON((1 1,1 2,2 2,2 1,1 1))", "BOX(1 1,2 2)", true);

    test_geometry<polygon, box>("boxpoly2", "POLYGON((1 1,1 2,2 2,2 1,1 1))", "BOX(1 1,2 3)", false);

    // linestring/linestring
    // simplex
    test_geometry<linestring, linestring>("ls1", "LINESTRING(1 1,2 2)", "LINESTRING(1 1,2 2)", true);

    // REVERSE linestring
    // Is this equal? To be decided.
    //test_geometry<linestring, linestring>("ls1", "LINESTRING(1 1,2 2)", "LINESTRING(2 2,1 1)", true);

}




int test_main( int , char* [] )
{
    test_all<bg::model::d2::point_xy<int> >();
    test_all<bg::model::d2::point_xy<double> >();

#if defined(HAVE_TTMATH)
    test_all<bg::model::d2::point_xy<ttmath_big> >();
#endif

    return 0;
}
