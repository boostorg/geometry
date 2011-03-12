// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Tests with-strategies, especially point-in-polygon

#include <geometry_test_common.hpp>

#include <boost/geometry/algorithms/within.hpp>

#include <boost/geometry/strategies/cartesian/point_in_poly_franklin.hpp>
#include <boost/geometry/strategies/cartesian/point_in_poly_crossings_multiply.hpp>
#include <boost/geometry/strategies/agnostic/point_in_poly_winding.hpp>

#include <boost/geometry/strategies/cartesian/side_by_triangle.hpp>


#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include <boost/geometry/domains/gis/io/wkt/wkt.hpp>






template <typename Point, typename Polygon, typename Strategy>
void test_point_in_polygon(std::string const& case_id,
            Point const& point, Polygon const& polygon,
            Strategy const& strategy, std::string const& strategy_id,
            bool expected)
{
    BOOST_CONCEPT_ASSERT( (bg::concept::WithinStrategy<Strategy>) );
    bool detected = bg::within(point, polygon, strategy);

    BOOST_CHECK_MESSAGE(detected == expected,
            "within: " << case_id
            << " strategy: " << strategy_id // typeid(strategy).name() is too long
            << " output expected: " << int(expected)
            << " detected: " << int(detected)
            );
}

template <typename Point, typename Polygon>
void test_geometry(std::string const& case_id, std::string const& wkt_point
            , std::string const& wkt_polygon
            , bool expected
            , std::string const& deviations = ""
            )
{
    Point point;
    Polygon polygon;
    bg::read_wkt(wkt_point, point);
    bg::read_wkt(wkt_polygon, polygon);

    namespace sw = bg::strategy::within;
    test_point_in_polygon(case_id, point, polygon, sw::winding<Point>(),
        "winding", expected);
    test_point_in_polygon(case_id, point, polygon, sw::franklin<Point>(),
        "franklin", boost::contains(deviations, "f") ? !expected : expected);
    test_point_in_polygon(case_id, point, polygon, sw::crossings_multiply<Point>(),
        "cross.mult", boost::contains(deviations, "c") ? !expected : expected);
}



template <typename Point>
void test_all()
{
    typedef bg::model::polygon<Point> polygon;

    std::string const box = "POLYGON((0 0,0 2,2 2,2 0,0 0))";
    std::string const triangle = "POLYGON((0 0,0 4,6 0,0 0))";
    std::string const with_hole = "POLYGON((0 0,0 3,3 3,3 0,0 0),(1 1,2 1,2 2,1 2,1 1))";


    test_geometry<Point, polygon>("b1", "POINT(1 1)", box, true);
    test_geometry<Point, polygon>("b2", "POINT(3 3)", box, false);

    // Test ALL corners (officialy false but some strategies might answer true)
    test_geometry<Point, polygon>("b3a", "POINT(0 0)", box, false, "f");
    test_geometry<Point, polygon>("b3b", "POINT(0 2)", box, false);
    test_geometry<Point, polygon>("b3c", "POINT(2 2)", box, false);
    test_geometry<Point, polygon>("b3d", "POINT(2 0)", box, false);

    // Test ALL sides (officialy false but some strategies might answer true)
    test_geometry<Point, polygon>("b4a", "POINT(0 1)", box, false, "f");
    test_geometry<Point, polygon>("b4b", "POINT(1 2)", box, false, "c");
    test_geometry<Point, polygon>("b4c", "POINT(2 1)", box, false);
    test_geometry<Point, polygon>("b4d", "POINT(1 0)", box, false, "f");


    test_geometry<Point, polygon>("t1", "POINT(1 1)", triangle, true);
    test_geometry<Point, polygon>("t2", "POINT(3 3)", triangle, false);

    test_geometry<Point, polygon>("t3a", "POINT(0 0)", triangle, false, "f");
    test_geometry<Point, polygon>("t3b", "POINT(0 4)", triangle, false, "c");
    test_geometry<Point, polygon>("t3c", "POINT(5 0)", triangle, false, "f");

    test_geometry<Point, polygon>("t4a", "POINT(0 2)", triangle, false, "f");
    test_geometry<Point, polygon>("t4b", "POINT(3 2)", triangle, false);
    test_geometry<Point, polygon>("t4c", "POINT(2 0)", triangle, false, "f");


    test_geometry<Point, polygon>("h1", "POINT(0.5 0.5)", with_hole, true);
    test_geometry<Point, polygon>("h2a", "POINT(1.5 1.5)", with_hole, false);
    test_geometry<Point, polygon>("h2b", "POINT(5 5)", with_hole, false);

    test_geometry<Point, polygon>("h3a", "POINT(1 1)", with_hole, false, "c");
    test_geometry<Point, polygon>("h3b", "POINT(2 2)", with_hole, false, "f");
    test_geometry<Point, polygon>("h3c", "POINT(0 0)", with_hole, false, "f");

    test_geometry<Point, polygon>("h4a", "POINT(1 1.5)", with_hole, false);
    test_geometry<Point, polygon>("h4b", "POINT(1.5 2)", with_hole, false, "f");

    // Lying ON (one of the sides of) interior ring
    test_geometry<Point, polygon>("#77-1", "POINT(6 3.5)", "POLYGON((5 3,5 4,4 4,4 5,3 5,3 6,5 6,5 5,7 5,7 6,8 6,8 5,9 5,9 2,8 2,8 1,7 1,7 2,5 2,5 3),(6 3,8 3,8 4,6 4,6 3))", false);
}


int test_main(int, char* [])
{
    test_all<bg::model::point<float, 2, bg::cs::cartesian> >();
    test_all<bg::model::point<double, 2, bg::cs::cartesian> >();

#if defined(HAVE_TTMATH)
    test_all<bg::model::point<ttmath_big, 2, bg::cs::cartesian> >();
#endif

    return 0;
}
