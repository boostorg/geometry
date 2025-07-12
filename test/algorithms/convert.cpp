// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2007-2025 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// This file was modified by Oracle on 2021.
// Modifications copyright (c) 2021, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <algorithms/test_convert.hpp>



template <typename Point1, typename Point2>
void test_mixed_orientation_and_closure()
{
    namespace model = boost::geometry::model;

    // Ring, (counter)clockwise, closed/open
    check<model::ring<Point1, true, true>, model::ring<Point2, true, true>>(
        "POLYGON((1 1,2 2,3 0,1 1))", "POLYGON((1 1,2 2,3 0,1 1))", 4);
    check<model::ring<Point1, true, true>, model::ring<Point2, false, true>>(
        "POLYGON((1 1,2 2,3 0,1 1))", "POLYGON((1 1,3 0,2 2,1 1))", 4);
    check<model::ring<Point1, true, true>, model::ring<Point2, true, false>>(
        "POLYGON((1 1,2 2,3 0,1 1))", "POLYGON((1 1,2 2,3 0))", 3);
    check<model::ring<Point1, true, true>, model::ring<Point2, false, false>>(
        "POLYGON((1 1,2 2,3 0,1 1))", "POLYGON((1 1,3 0,2 2))", 3);

    check<model::ring<Point1, false, true>, model::ring<Point2, true, true>>(
        "POLYGON((1 1,3 0,2 2,1 1))", "POLYGON((1 1,2 2,3 0,1 1))", 4);
    check<model::ring<Point1, false, true>, model::ring<Point2, false, true>>(
        "POLYGON((1 1,3 0,2 2,1 1))", "POLYGON((1 1,3 0,2 2,1 1))", 4);
    check<model::ring<Point1, false, true>, model::ring<Point2, true, false>>(
        "POLYGON((1 1,3 0,2 2,1 1))", "POLYGON((1 1,2 2,3 0))", 3);
    check<model::ring<Point1, false, true>, model::ring<Point2, false, false>>(
        "POLYGON((1 1,3 0,2 2,1 1))", "POLYGON((1 1,3 0,2 2))", 3);

    check<model::ring<Point1, true, false>, model::ring<Point2, true, true>>(
        "POLYGON((1 1,2 2,3 0))", "POLYGON((1 1,2 2,3 0,1 1))", 4);
    check<model::ring<Point1, true, false>, model::ring<Point2, false, true>>(
        "POLYGON((1 1,2 2,3 0))", "POLYGON((1 1,3 0,2 2,1 1))", 4);
    check<model::ring<Point1, true, false>, model::ring<Point2, true, false>>(
        "POLYGON((1 1,2 2,3 0))", "POLYGON((1 1,2 2,3 0))", 3);
    check<model::ring<Point1, true, false>, model::ring<Point2, false, false>>(
        "POLYGON((1 1,2 2,3 0))", "POLYGON((1 1,3 0,2 2))", 3);

    check<model::ring<Point1, false, false>, model::ring<Point2, true, true>>(
        "POLYGON((1 1,3 0,2 2))", "POLYGON((1 1,2 2,3 0,1 1))", 4);
    check<model::ring<Point1, false, false>, model::ring<Point2, false, true>>(
        "POLYGON((1 1,3 0,2 2))", "POLYGON((1 1,3 0,2 2,1 1))", 4);
    check<model::ring<Point1, false, false>, model::ring<Point2, true, false>>(
        "POLYGON((1 1,3 0,2 2))", "POLYGON((1 1,2 2,3 0))", 3);
    check<model::ring<Point1, false, false>, model::ring<Point2, false, false>>(
        "POLYGON((1 1,3 0,2 2))", "POLYGON((1 1,3 0,2 2))", 3);

    // Polygon (using ring underneath, so most combinations can be omitted)
    check<model::polygon<Point1>, model::polygon<Point2, false>>(
        "POLYGON((0 0,0 5,5 5,5 0,0 0),(1 1,3 2,2 4,1 1))",
        "POLYGON((0 0,5 0,5 5,0 5,0 0),(1 1,2 4,3 2,1 1))");

    check<model::polygon<Point1>, model::polygon<Point2, false, false>>(
        "POLYGON((0 0,0 5,5 5,5 0,0 0),(1 1,3 2,2 4,1 1))",
        "POLYGON((0 0,5 0,5 5,0 5,0 0),(1 1,2 4,3 2,1 1))",
        7);

    check<model::polygon<Point1>, model::ring<Point2, false>>("POLYGON((1 1,2 2,3 0,1 1))",
        "POLYGON((1 1,3 0,2 2,1 1))");

    using box1_t = model::box<Point1>;

    check<box1_t, model::ring<Point2, false>>("BOX(0 0,2 2)",
        "POLYGON((0 0,2 0,2 2,0 2,0 0))", 5);

    check<box1_t, model::ring<Point2, true, false>>("BOX(0 0,2 2)",
        "POLYGON((0 0,0 2,2 2,2 0))", 4);

    check<box1_t, model::ring<Point2, false, false>>("BOX(0 0,2 2)",
        "POLYGON((0 0,2 0,2 2,0 2))", 4);

    check<box1_t, model::polygon<Point2, false>>("BOX(0 0,2 2)",
        "POLYGON((0 0,2 0,2 2,0 2,0 0))", 5);
    check<box1_t, model::polygon<Point2, true, false>>("BOX(0 0,2 2)",
        "POLYGON((0 0,0 2,2 2,2 0,0 0))", 4);
    check<box1_t, model::polygon<Point2, false, false>>("BOX(0 0,2 2)",
        "POLYGON((0 0,2 0,2 2,0 2,0 0))", 4);
}

template <typename Point1, typename Point2>
void test_mixed_point_types()
{
    namespace model = boost::geometry::model;

    const std::string point_simplex = "POINT(0 0)";
    const std::string ls_simplex = "LINESTRING(1 1,2 2)";
    const std::string mls_simplex = "MULTILINESTRING((1 1,2 2),(3 3,4 4))";
    const std::string mpoint_simplex = "MULTIPOINT((1 1),(2 2))";

    const std::string box_simplex = "BOX(0 0,1 1)";
    const std::string ring_simplex = "POLYGON((0 0,0 1,1 1,1 0,0 0))";
    const std::string poly_simplex = "POLYGON((0 0,0 5,5 5,5 0,0 0),(2 2,3 2,3 3,2 2))";
    const std::string mpoly_simplex = "MULTIPOLYGON("
            "((0 0,0 5,5 5,5 0,0 0),(2 2,3 2,3 3,2 2)),"
            "((6 6,6 7,7 7,7 6,6 6))"
            ")";

    using segment1_t = model::segment<Point1>;
    using segment2_t = model::segment<Point2>;
    using ls1_t = model::linestring<Point1>;
    using ls2_t = model::linestring<Point2>;

    using box1_t = model::box<Point1>;
    using box2_t = model::box<Point2>;
    using ring1_t = model::ring<Point1>;
    using ring2_t = model::ring<Point2>;
    using poly1_t = model::polygon<Point1>;
    using poly2_t = model::polygon<Point2>;

    using mpoint1_t = model::multi_point<Point1>;
    using mpoint2_t = model::multi_point<Point2>;
    using mls1_t = model::multi_linestring<ls1_t>;
    using mls2_t = model::multi_linestring<ls2_t>;
    using mpoly1_t = model::multi_polygon<poly1_t>;
    using mpoly2_t = model::multi_polygon<poly2_t>;

    check<Point1, Point2>(point_simplex);
    check<box1_t, box2_t>(ring_simplex);
    check<segment1_t, segment2_t>(ls_simplex);
    check<ls1_t, ls2_t>(ls_simplex);
    check<ring1_t, ring2_t>(ring_simplex);
    check<poly1_t, poly2_t>(poly_simplex);
    check<mpoint1_t, mpoint2_t>(mpoint_simplex);
    check<mls1_t, mls2_t>(mls_simplex);
    check<mpoly1_t, mpoly2_t>(mpoly_simplex);

    check<Point1, box2_t>(point_simplex, "POLYGON((0 0,0 0,0 0,0 0,0 0))");
    check<segment1_t, ls2_t>(ls_simplex);
    check<box1_t, ls2_t>(box_simplex, "LINESTRING(0 0,0 1,1 1,1 0,0 0)");
    check<box1_t, ring2_t>(box_simplex, ring_simplex);
    check<box1_t, poly2_t>(box_simplex, ring_simplex);

    check<ring1_t, poly2_t>(ring_simplex);
    check<poly1_t, ring2_t>(ring_simplex);

    // Interior rings are omitted going from polygon to ring
    check<poly1_t, ring2_t>(poly_simplex, "POLYGON((0 0,0 5,5 5,5 0,0 0))");

    // single -> multi
    check<Point1, mpoint2_t>(point_simplex, "MULTIPOINT((0 0))");
    check<ls1_t, mls2_t>(ls_simplex, "MULTILINESTRING((1 1,2 2))");
    check<segment1_t, mls2_t>(ls_simplex, "MULTILINESTRING((1 1,2 2))");

    check<ls1_t, mpoint2_t>(ls_simplex, "MULTIPOINT((1 1),(2 2))");
    check<box1_t, mpoint2_t>(box_simplex, "MULTIPOINT((0 0),(0 1),(1 1),(1 0))");
    check<ring1_t, mpoint2_t>(ring_simplex, "MULTIPOINT((0 0),(0 1),(1 1),(1 0),(0 0))");
    check<poly1_t, mpoint2_t>(ring_simplex, "MULTIPOINT((0 0),(0 1),(1 1),(1 0),(0 0))");
    check<mls1_t, mpoint2_t>(mls_simplex, "MULTIPOINT((1 1),(2 2),(3 3),(4 4))");
    check<mpoly1_t, mpoint2_t>(mpoly_simplex,
        "MULTIPOINT((0 0),(0 5),(5 5),(5 0),(0 0),(2 2),(3 2),(3 3),(2 2),(6 6),(6 7),(7 7),(7 6),(6 6))");

    check<ring1_t, mls2_t>(ring_simplex, "MULTILINESTRING((0 0,0 1,1 1,1 0,0 0))");
    check<poly1_t, mls2_t>(ring_simplex, "MULTILINESTRING((0 0,0 1,1 1,1 0,0 0))");
    check<poly1_t, mls2_t>(poly_simplex,
        "MULTILINESTRING((0 0,0 5,5 5,5 0,0 0),(2 2,3 2,3 3,2 2))");
    check<mpoly1_t, mls2_t>(mpoly_simplex,
        "MULTILINESTRING((0 0,0 5,5 5,5 0,0 0),(2 2,3 2,3 3,2 2),(6 6,6 7,7 7,7 6,6 6))");

    check<box1_t, mpoly2_t>(box_simplex, "MULTIPOLYGON(((0 0,0 1,1 1,1 0,0 0)))");
    check<ring1_t, mpoly2_t>(ring_simplex, "MULTIPOLYGON(((0 0,0 1,1 1,1 0,0 0)))");
    check<poly1_t, mpoly2_t>(ring_simplex, "MULTIPOLYGON(((0 0,0 1,1 1,1 0,0 0)))");
    check<poly1_t, mpoly2_t>(poly_simplex,
        "MULTIPOLYGON(((0 0,0 5,5 5,5 0,0 0),(2 2,3 2,3 3,2 2)))");
}

template <typename Point1, typename Point2>
void test_mixed_point_types_3d()
{
    namespace model = boost::geometry::model;
    check<Point1, Point2>("POINT(1 2 3)");

    check<model::segment<Point1>, model::segment<Point2>>(
        "LINESTRING(1 2 3,4 5 6)");

    check<model::linestring<Point1>, model::linestring<Point2>>(
        "LINESTRING(1 2 3,4 5 6,7 8 9)");

    check<model::segment<Point1>, model::linestring<Point2>>(
        "LINESTRING(1 2 3,4 5 6)", "LINESTRING(1 2 3,4 5 6)", 2);
}

template <typename Point1, typename Point2>
void test_mixed_types()
{
    test_mixed_point_types<Point1, Point2>();
    test_mixed_point_types<Point2, Point1>();
    test_mixed_orientation_and_closure<Point1, Point2>();
    test_mixed_orientation_and_closure<Point2, Point1>();
}

template <typename Point1, typename Point2>
void test_mixed_types_3d()
{
    test_mixed_point_types_3d<Point1, Point2>();
    test_mixed_point_types_3d<Point2, Point1>();
}

void test_array()
{
    int a[2] = {1, 2};
    int b[2];
    bg::convert(a, b);
    BOOST_CHECK_EQUAL(b[0], 1);
    BOOST_CHECK_EQUAL(b[1], 2);
}

int test_main(int, char* [])
{
    test_mixed_types
        <
            bg::model::point<int, 2, bg::cs::cartesian>,
            bg::model::point<double, 2, bg::cs::cartesian>
        >();
    test_mixed_types
        <
            boost::tuple<float, float>,
            bg::model::point<float, 2, bg::cs::cartesian>
        >();
    test_mixed_types_3d
        <
            boost::tuple<double, double, double>,
            bg::model::point<double, 3, bg::cs::cartesian>
        >();

    test_array();
    return 0;
}
