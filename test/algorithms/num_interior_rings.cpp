// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE test_num_interior_rings
#endif

#include <iostream>

#include <boost/test/included/unit_test.hpp>

#include <boost/variant/variant.hpp>

#include <boost/geometry/algorithms/num_interior_rings.hpp>

#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/io/wkt/wkt.hpp>
#include <boost/geometry/io/dsv/write.hpp>

namespace bg = boost::geometry;


typedef bg::model::point<double, 2, bg::cs::cartesian> point;
typedef bg::model::linestring<point> linestring;
typedef bg::model::segment<point> segment;
typedef bg::model::box<point> box;
typedef bg::model::ring<point, true, true> ring_cw_closed;
typedef bg::model::ring<point, true, false> ring_cw_open;
typedef bg::model::ring<point, false, true> ring_ccw_closed;
typedef bg::model::ring<point, false, false> ring_ccw_open;
typedef bg::model::polygon<point, true, true> polygon_cw_closed;
typedef bg::model::polygon<point, true, false> polygon_cw_open;
typedef bg::model::polygon<point, false, true> polygon_ccw_closed;
typedef bg::model::polygon<point, false, false> polygon_ccw_open;
typedef bg::model::multi_point<point> multi_point;
typedef bg::model::multi_linestring<linestring> multi_linestring;
typedef bg::model::multi_polygon<polygon_cw_closed> multi_polygon_cw_closed;
typedef bg::model::multi_polygon<polygon_cw_open> multi_polygon_cw_open;
typedef bg::model::multi_polygon<polygon_ccw_closed> multi_polygon_ccw_closed;
typedef bg::model::multi_polygon<polygon_ccw_open> multi_polygon_ccw_open;


template <typename Geometry>
struct test_num_interior_rings
{
    static inline void apply(Geometry const& geometry,
                             std::size_t expected)
    {
        std::size_t detected = bg::num_interior_rings(geometry);
        BOOST_CHECK_MESSAGE( detected == expected,
                             "Expected: " << expected
                             << " detected: " << detected
                             << " wkt: " << bg::wkt(geometry) );
    }

    static inline void apply(std::string const& wkt,
                             std::size_t expected)
    {
        Geometry geometry;
        bg::read_wkt(wkt, geometry);
        apply(geometry, expected);
    }
};

BOOST_AUTO_TEST_CASE( test_point )
{
    test_num_interior_rings<point>::apply("POINT(0 0)", 0);
}

BOOST_AUTO_TEST_CASE( test_segment )
{
    test_num_interior_rings<segment>::apply("SEGMENT(0 0,1 1)", 0);
}

BOOST_AUTO_TEST_CASE( test_box )
{
    test_num_interior_rings<box>::apply("BOX(0 0,1 1)", 0);
}

BOOST_AUTO_TEST_CASE( test_linestring )
{
    typedef test_num_interior_rings<linestring> tester;

    tester::apply("LINESTRING()", 0);
    tester::apply("LINESTRING(0 0)", 0);
    tester::apply("LINESTRING(0 0,0 0)", 0);
    tester::apply("LINESTRING(0 0,0 0,1 1)", 0);
    tester::apply("LINESTRING(0 0,0 0,0 0,1 1)", 0);
}

BOOST_AUTO_TEST_CASE( test_multipoint )
{
    typedef test_num_interior_rings<multi_point> tester;

    tester::apply("MULTIPOINT()", 0);
    tester::apply("MULTIPOINT(0 0)", 0);
    tester::apply("MULTIPOINT(0 0,0 0)", 0);
    tester::apply("MULTIPOINT(0 0,0 0,1 1)", 0);
}

BOOST_AUTO_TEST_CASE( test_multilinestring )
{
    typedef test_num_interior_rings<multi_linestring> tester;

    tester::apply("MULTILINESTRING()", 0);
    tester::apply("MULTILINESTRING((),(0 0))", 0);
    tester::apply("MULTILINESTRING((0 0))", 0);
    tester::apply("MULTILINESTRING((0 0,1 0))", 0);
    tester::apply("MULTILINESTRING((),(),(0 0,1 0))", 0);
    tester::apply("MULTILINESTRING((0 0,1 0,0 1),(0 0,1 0,0 1,0 0))", 0);
}

template <typename OpenRing>
void test_open_ring()
{
    typedef test_num_interior_rings<OpenRing> tester;

    tester::apply("POLYGON(())", 0);
    tester::apply("POLYGON((0 0))", 0);
    tester::apply("POLYGON((0 0,1 0))", 0);
    tester::apply("POLYGON((0 0,1 0,0 1))", 0);
    tester::apply("POLYGON((0 0,0 0,1 0,0 1))", 0);
}

template <typename ClosedRing>
void test_closed_ring()
{
    typedef test_num_interior_rings<ClosedRing> tester;

    tester::apply("POLYGON(())", 0);
    tester::apply("POLYGON((0 0))", 0);
    tester::apply("POLYGON((0 0,0 0))", 0);
    tester::apply("POLYGON((0 0,1 0,0 0))", 0);
    tester::apply("POLYGON((0 0,1 0,0 1,0 0))", 0);
    tester::apply("POLYGON((0 0,1 0,1 0,0 1,0 0))", 0);
}

BOOST_AUTO_TEST_CASE( test_ring )
{
    test_open_ring<ring_ccw_open>();
    test_open_ring<ring_cw_open>();
    test_closed_ring<ring_ccw_closed>();
    test_closed_ring<ring_cw_closed>();
}

template <typename OpenPolygon>
void test_open_polygon()
{
    typedef test_num_interior_rings<OpenPolygon> tester;

    tester::apply("POLYGON(())", 0);
    tester::apply("POLYGON((0 0))", 0);
    tester::apply("POLYGON((0 0,10 0),(0 0))", 1);
    tester::apply("POLYGON((0 0,10 0),(1 1,2 1))", 1);
    tester::apply("POLYGON((0 0,10 0,0 10))", 0);
    tester::apply("POLYGON((0 0,10 0,0 10),())", 1);
    tester::apply("POLYGON((0 0,10 0,0 10),(1 1))", 1);
    tester::apply("POLYGON((0 0,10 0,0 10),(1 1,2 1))", 1);
    tester::apply("POLYGON((0 0,10 0,0 10),(1 1,2 1,1 2))", 1);
    tester::apply("POLYGON((0 0,10 0,10 10,0 10),(1 1,2 1,1 2))", 1);
    tester::apply("POLYGON((0 0,10 0,10 10,0 10),(1 1,2 1,2 2,1 2))", 1);
    tester::apply("POLYGON((0 0,10 0,10 10,0 10),(1 1,2 1,2 2,1 2),(5 5,6 5,6 6,5 6))", 2);
}

template <typename ClosedPolygon>
void test_closed_polygon()
{
    typedef test_num_interior_rings<ClosedPolygon> tester;

    tester::apply("POLYGON(())", 0);
    tester::apply("POLYGON((0 0))", 0);
    tester::apply("POLYGON((0 0,10 0,0 0),(0 0))", 1);
    tester::apply("POLYGON((0 0,10 0,0 0),(1 1,2 1,1 1))", 1);
    tester::apply("POLYGON((0 0,10 0,0 10,0 0))", 0);
    tester::apply("POLYGON((0 0,10 0,0 10,0 0),())", 1);
    tester::apply("POLYGON((0 0,10 0,0 10,0 0),(1 1))", 1);
    tester::apply("POLYGON((0 0,10 0,0 10,0 0),(1 1,2 1,1 1))", 1);
    tester::apply("POLYGON((0 0,10 0,0 10,0 0),(1 1,2 1,1 2,1 1))", 1);
    tester::apply("POLYGON((0 0,10 0,10 10,0 10,0 0),(1 1,2 1,1 2,1 1))", 1);
    tester::apply("POLYGON((0 0,10 0,10 10,0 10,0 0),(1 1,2 1,2 2,1 2,1 1))", 1);
    tester::apply("POLYGON((0 0,10 0,10 10,0 10,0 0),(1 1,2 1,2 2,1 2,1 1),(5 5,6 5,6 6,5 6,5 5))", 2);
}

BOOST_AUTO_TEST_CASE( test_polygon )
{
    test_open_polygon<polygon_ccw_open>();
    test_open_polygon<polygon_cw_open>();
    test_closed_polygon<polygon_ccw_closed>();
    test_closed_polygon<polygon_cw_closed>();
}

template <typename OpenMultiPolygon>
void test_open_multipolygon()
{
    typedef test_num_interior_rings<OpenMultiPolygon> tester;

    tester::apply("MULTIPOLYGON(((0 0,10 0,10 10,0 10),(1 1,2 1,1 2)))", 1);
    tester::apply("MULTIPOLYGON(((0 0,10 0,10 10,0 10),(1 1,2 1,2 2,1 2),(5 5,6 5,6 6,5 6)))", 2);
    tester::apply("MULTIPOLYGON(((0 0,10 0,10 10,0 10),(1 1,2 1,1 2)),((100 100,110 100,110 110),(101 101,102 101,102 102)))", 2);
    tester::apply("MULTIPOLYGON(((0 0,10 0,10 10,0 10),(1 1,2 1,2 2,1 2),(5 5,6 5,6 6,5 6)),((100 100,110 100,110 110),(101 101,102 101,102 102),(105 105,106 105,106 106,105 106)))", 4);
}

template <typename ClosedMultiPolygon>
void test_closed_multipolygon()
{
    typedef test_num_interior_rings<ClosedMultiPolygon> tester;

    tester::apply("MULTIPOLYGON(((0 0,10 0,10 10,0 10,0 0),(1 1,2 1,1 2,1 1)))", 1);
    tester::apply("MULTIPOLYGON(((0 0,10 0,10 10,0 10,0 0),(1 1,2 1,2 2,1 2,1 1),(5 5,6 5,6 6,5 6,5 5)))", 2);
    tester::apply("MULTIPOLYGON(((0 0,10 0,10 10,0 10,0 0),(1 1,2 1,1 2,1 1)),((100 100,110 100,110 110,100 100),(101 101,102 101,102 102,101 101)))", 2);
    tester::apply("MULTIPOLYGON(((0 0,10 0,10 10,0 10,0 0),(1 1,2 1,2 2,1 2,1 1),(5 5,6 5,6 6,5 6,5 5)),((100 100,110 100,110 110,100 100),(101 101,102 101,102 102,101 101),(105 105,106 105,106 106,105 106,105 105)))", 4);
}

BOOST_AUTO_TEST_CASE( test_multipolygon )
{
    test_open_multipolygon<multi_polygon_ccw_open>();
    test_open_multipolygon<multi_polygon_cw_open>();
    test_closed_multipolygon<multi_polygon_ccw_closed>();
    test_closed_multipolygon<multi_polygon_cw_closed>();
}

BOOST_AUTO_TEST_CASE( test_variant )
{
    typedef boost::variant
        <
            linestring, polygon_cw_open, polygon_cw_closed
        > variant_geometry_type;

    typedef test_num_interior_rings<variant_geometry_type> tester;

    linestring ls;
    bg::read_wkt("LINESTRING(0 0,1 1,2 2)", ls);

    polygon_cw_open p_open;
    bg::read_wkt("POLYGON((0 0,0 10,10 10,10 0),(1 1,9 1,9 9,1 9))", p_open);

    polygon_cw_closed p_closed;
    bg::read_wkt("POLYGON((0 0,0 10,10 10,10 0,0 0),(1 1,2 1,1 2,1 1),(5 5,6 5,6 6,5 6,5 5))", p_closed);

    variant_geometry_type variant_geometry;

    variant_geometry = ls;
    tester::apply(variant_geometry, 0);

    variant_geometry = p_open;
    tester::apply(variant_geometry, 1);

    variant_geometry = p_closed;
    tester::apply(variant_geometry, 2);
}
