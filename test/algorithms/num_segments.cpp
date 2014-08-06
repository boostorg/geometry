// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE test_num_segments
#endif

#include <iostream>

#include <boost/test/included/unit_test.hpp>

#include <boost/variant/variant.hpp>

#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/io/wkt/wkt.hpp>
#include <boost/geometry/algorithms/num_segments.hpp>

namespace bg = boost::geometry;


typedef bg::model::point<double,2,bg::cs::cartesian> point;
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
struct test_num_segments
{
    static inline void apply(Geometry const& geometry,
                             std::size_t expected_closed,
                             std::size_t expected_open)
    {
        std::size_t detected = bg::num_segments(geometry);
        BOOST_CHECK_MESSAGE( detected == expected_closed,
                             "Expected: " << expected_closed
                             << " detected: " << detected
                             << " wkt: " << bg::wkt(geometry) );

        detected = bg::num_segments(geometry, true);
        BOOST_CHECK_MESSAGE( detected == expected_open,
                             "Expected (add for open): " << expected_open
                             << " detected (add for open): " << detected
                             << " wkt: " << bg::wkt(geometry) );

    }

    static inline void apply(Geometry const& geometry,
                             std::size_t expected_closed)
    {
        apply(geometry, expected_closed, expected_closed);
    }

    static inline void apply(std::string const& wkt,
                             std::size_t expected_closed,
                             std::size_t expected_open)
    {
        Geometry geometry;
        bg::read_wkt(wkt, geometry);
        apply(geometry, expected_closed, expected_open);
    }

    static inline void apply(std::string const& wkt,
                             std::size_t expected_closed)
    {
        apply(wkt, expected_closed, expected_closed);
    }
};

BOOST_AUTO_TEST_CASE( test_point )
{
    test_num_segments<point>::apply("POINT(0 0)", 0);
}

BOOST_AUTO_TEST_CASE( test_segment )
{
    test_num_segments<segment>::apply("SEGMENT(0 0,1 1)", 1);
}

BOOST_AUTO_TEST_CASE( test_box )
{
    test_num_segments<box>::apply("BOX(0 0,1 1)", 4);
}

BOOST_AUTO_TEST_CASE( test_linestring )
{
    typedef test_num_segments<linestring> tester;

    tester::apply("LINESTRING()", 0);
    tester::apply("LINESTRING(0 0)", 0);
    tester::apply("LINESTRING(0 0,0 0)", 1);
    tester::apply("LINESTRING(0 0,0 0,1 1)", 2);
    tester::apply("LINESTRING(0 0,0 0,0 0,1 1)", 3);
}

BOOST_AUTO_TEST_CASE( test_multipoint )
{
    typedef test_num_segments<multi_point> tester;

    tester::apply("MULTIPOINT()", 0);
    tester::apply("MULTIPOINT(0 0)", 0);
    tester::apply("MULTIPOINT(0 0,0 0)", 0);
    tester::apply("MULTIPOINT(0 0,0 0,1 1)", 0);
}

BOOST_AUTO_TEST_CASE( test_multilinestring )
{
    typedef test_num_segments<multi_linestring> tester;

    tester::apply("MULTILINESTRING()", 0);
    tester::apply("MULTILINESTRING((),(0 0))", 0);
    tester::apply("MULTILINESTRING((0 0))", 0);
    tester::apply("MULTILINESTRING((0 0,1 0))", 1);
    tester::apply("MULTILINESTRING((),(),(0 0,1 0))", 1);
    tester::apply("MULTILINESTRING((0 0,1 0,0 1),(0 0,1 0,0 1,0 0))", 5);
}

template <typename OpenRing>
void test_open_ring()
{
    typedef test_num_segments<OpenRing> tester;

    tester::apply("POLYGON(())", 0);
    tester::apply("POLYGON((0 0))", 0);
    tester::apply("POLYGON((0 0,1 0))", 1, 2);
    tester::apply("POLYGON((0 0,1 0,0 1))", 2, 3);
    tester::apply("POLYGON((0 0,0 0,1 0,0 1))", 3, 4);
}

template <typename ClosedRing>
void test_closed_ring()
{
    typedef test_num_segments<ClosedRing> tester;

    tester::apply("POLYGON(())", 0);
    tester::apply("POLYGON((0 0))", 0);
    tester::apply("POLYGON((0 0,0 0))", 1);
    tester::apply("POLYGON((0 0,1 0,0 0))", 2);
    tester::apply("POLYGON((0 0,1 0,0 1,0 0))", 3);
    tester::apply("POLYGON((0 0,1 0,1 0,0 1,0 0))", 4);
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
    typedef test_num_segments<OpenPolygon> tester;

    tester::apply("POLYGON(())", 0);
    tester::apply("POLYGON((0 0))", 0);
    tester::apply("POLYGON((0 0,10 0),(0 0))", 1, 2);
    tester::apply("POLYGON((0 0,10 0),(1 1,2 1))", 2, 4);
    tester::apply("POLYGON((0 0,10 0,0 10))", 2, 3);
    tester::apply("POLYGON((0 0,10 0,0 10),())", 2, 3);
    tester::apply("POLYGON((0 0,10 0,0 10),(1 1))", 2, 3);
    tester::apply("POLYGON((0 0,10 0,0 10),(1 1,2 1))", 3, 5);
    tester::apply("POLYGON((0 0,10 0,0 10),(1 1,2 1,1 2))", 4, 6);
    tester::apply("POLYGON((0 0,10 0,10 10,0 10),(1 1,2 1,1 2))", 5, 7);
    tester::apply("POLYGON((0 0,10 0,10 10,0 10),(1 1,2 1,2 2,1 2))", 6, 8);
    tester::apply("POLYGON((0 0,10 0,10 10,0 10),(1 1,2 1,2 2,1 2),(5 5,6 5,6 6,5 6))", 9, 12);
}

template <typename ClosedPolygon>
void test_closed_polygon()
{
    typedef test_num_segments<ClosedPolygon> tester;

    tester::apply("POLYGON(())", 0);
    tester::apply("POLYGON((0 0))", 0);
    tester::apply("POLYGON((0 0,10 0,0 0),(0 0))", 2);
    tester::apply("POLYGON((0 0,10 0,0 0),(1 1,2 1,1 1))", 4);
    tester::apply("POLYGON((0 0,10 0,0 10,0 0))", 3);
    tester::apply("POLYGON((0 0,10 0,0 10,0 0),())", 3);
    tester::apply("POLYGON((0 0,10 0,0 10,0 0),(1 1))", 3);
    tester::apply("POLYGON((0 0,10 0,0 10,0 0),(1 1,2 1,1 1))", 5);
    tester::apply("POLYGON((0 0,10 0,0 10,0 0),(1 1,2 1,1 2,1 1))", 6);
    tester::apply("POLYGON((0 0,10 0,10 10,0 10,0 0),(1 1,2 1,1 2,1 1))", 7);
    tester::apply("POLYGON((0 0,10 0,10 10,0 10,0 0),(1 1,2 1,2 2,1 2,1 1))", 8);
    tester::apply("POLYGON((0 0,10 0,10 10,0 10,0 0),(1 1,2 1,2 2,1 2,1 1),(5 5,6 5,6 6,5 6,5 5))", 12);
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
    typedef test_num_segments<OpenMultiPolygon> tester;

    tester::apply("MULTIPOLYGON(((0 0,10 0,10 10,0 10),(1 1,2 1,1 2)))", 5, 7);
    tester::apply("MULTIPOLYGON(((0 0,10 0,10 10,0 10),(1 1,2 1,2 2,1 2),(5 5,6 5,6 6,5 6)))", 9, 12);
    tester::apply("MULTIPOLYGON(((0 0,10 0,10 10,0 10),(1 1,2 1,1 2)),((100 100,110 100,110 110),(101 101,102 101,102 102)))", 9, 13);
    tester::apply("MULTIPOLYGON(((0 0,10 0,10 10,0 10),(1 1,2 1,2 2,1 2),(5 5,6 5,6 6,5 6)),((100 100,110 100,110 110),(101 101,102 101,102 102),(105 105,106 105,106 106,105 106)))", 16, 22);
}

template <typename ClosedMultiPolygon>
void test_closed_multipolygon()
{
    typedef test_num_segments<ClosedMultiPolygon> tester;

    tester::apply("MULTIPOLYGON(((0 0,10 0,10 10,0 10,0 0),(1 1,2 1,1 2,1 1)))", 7);
    tester::apply("MULTIPOLYGON(((0 0,10 0,10 10,0 10,0 0),(1 1,2 1,2 2,1 2,1 1),(5 5,6 5,6 6,5 6,5 5)))", 12);
    tester::apply("MULTIPOLYGON(((0 0,10 0,10 10,0 10,0 0),(1 1,2 1,1 2,1 1)),((100 100,110 100,110 110,100 100),(101 101,102 101,102 102,101 101)))", 13);
    tester::apply("MULTIPOLYGON(((0 0,10 0,10 10,0 10,0 0),(1 1,2 1,2 2,1 2,1 1),(5 5,6 5,6 6,5 6,5 5)),((100 100,110 100,110 110,100 100),(101 101,102 101,102 102,101 101),(105 105,106 105,106 106,105 106,105 105)))", 22);
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

    typedef test_num_segments<variant_geometry_type> tester;

    linestring ls;
    bg::read_wkt("LINESTRING(0 0,1 1,2 2)", ls);

    polygon_cw_open p_open;
    bg::read_wkt("POLYGON((0 0,0 1,1 0))", p_open);

    polygon_cw_closed p_closed;
    bg::read_wkt("POLYGON((0 0,0 1,1 1,1 0,0 0))", p_closed);

    variant_geometry_type variant_geometry;

    variant_geometry = ls;
    tester::apply(variant_geometry, 2);

    variant_geometry = p_open;
    tester::apply(variant_geometry, 2, 3);

    variant_geometry = p_closed;
    tester::apply(variant_geometry, 4);
}
