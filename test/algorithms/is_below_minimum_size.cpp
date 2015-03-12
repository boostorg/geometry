// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2015, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#include <iostream>

#include <boost/variant.hpp>

#include <geometry_test_common.hpp>

#include <boost/geometry/algorithms/is_below_minimum_size.hpp>
#include <boost/geometry/algorithms/detail/throw_below_minimum_size.hpp>
#include <boost/geometry/geometries/geometries.hpp>

#include <boost/geometry/io/wkt/wkt.hpp>

template <std::size_t D, typename T = double>
struct box_dD
{
    typedef boost::geometry::model::box
        <
            boost::geometry::model::point<T, D, boost::geometry::cs::cartesian>
        > type;
};

template <typename Geometry>
inline void test_one(Geometry const& geometry, bool expected)
{
    bool detected = bg::is_below_minimum_size(geometry);
    BOOST_CHECK_EQUAL(expected, detected);
}

template <typename Geometry>
inline void test_one(std::string const& wkt, bool expected)
{
    Geometry geometry;
    bg::read_wkt(wkt, geometry);
    test_one<Geometry>(geometry, expected);
}


template <typename Geometry>
inline void test_throw_exception(Geometry const& geometry)
{
    bg::detail::throw_below_minimum_size(geometry);
}

template <typename Geometry>
inline void test_catch_exception(Geometry const& geometry)
{
    try
    {
        test_throw_exception(geometry);
    }
    catch (bg::below_minimum_size_exception const&)
    {
    }
}



int test_main(int, char* [])
{
    typedef bg::model::point<double,2,bg::cs::cartesian> point;
    typedef bg::model::linestring<point> linestring;
    typedef bg::model::segment<point> segment;
    typedef bg::model::box<point> box;
    typedef bg::model::ring<point> ring;
    typedef bg::model::polygon<point> polygon;
    typedef bg::model::multi_point<point> multi_point;
    typedef bg::model::multi_linestring<linestring> multi_linestring;
    typedef bg::model::multi_polygon<polygon> multi_polygon;

    // open geometries
    typedef bg::model::ring<point, true, false> open_ring;
    typedef bg::model::polygon<point, true, false> open_polygon;
    typedef bg::model::multi_polygon<open_polygon> open_multi_polygon;

    test_one<point>("POINT(0 0)", false);
    test_one<linestring>("LINESTRING()", true);
    test_one<linestring>("LINESTRING(0 0)", true);
    test_one<linestring>("LINESTRING(0 0,0 0)", false);
    test_one<linestring>("LINESTRING(0 0,1 1)", false);
    test_one<segment>("LINESTRING(0 0,1 1)", false);
    test_one<box>("POLYGON((0 0,10 10))", false);
    test_one<box_dD<3>::type>("BOX(0 0 0,1 1 1)", false);
    test_one<box_dD<4>::type>("BOX(0 0 0 0,1 1 1 1)", false);
    test_one<box_dD<5>::type>("BOX(0 0 0 0 0,1 1 1 1 1)", false);
    test_one<ring>("POLYGON(())", true);
    test_one<ring>("POLYGON((0 0))", true);
    test_one<ring>("POLYGON((0 0,1 1))", true);
    test_one<ring>("POLYGON((0 0,1 1,0 1))", true);
    test_one<ring>("POLYGON((0 0,1 1,0 1,0 0))", false);
    test_one<polygon>("POLYGON(())", true);
    test_one<polygon>("POLYGON((0 0))", true);
    test_one<polygon>("POLYGON((0 0,10 10))", true);
    test_one<polygon>("POLYGON((0 0,10 10,0 10))", true);
    test_one<polygon>("POLYGON((0 0,10 10,0 10,0 0))", false);
    test_one<polygon>("POLYGON((),(4 4,6 4,6 6,4 6,4 4))", true);
    test_one<polygon>("POLYGON((0 0),(4 4,6 4,6 6,4 6,4 4))", true);
    test_one<polygon>("POLYGON((0 0,0 10),(4 4,6 4,6 6,4 6,4 4))", true);
    test_one<polygon>("POLYGON((0 0,0 10,10 10),(4 4,6 4,6 6,4 6,4 4))", true);
    test_one<polygon>("POLYGON((0 0,0 10,10 10,10 0),(4 4,6 4,6 6,4 6,4 4))", false);
    test_one<polygon>("POLYGON((0 0,0 10,10 10,10 0,0 0),(4 4,6 4,6 6,4 6,4 4))", false);
    test_one<polygon>("POLYGON((0 0,0 10,10 10,10 0,0 0),())", true);
    test_one<polygon>("POLYGON((0 0,0 10,10 10,10 0,0 0),(4 4))", true);
    test_one<polygon>("POLYGON((0 0,0 10,10 10,10 0,0 0),(4 4,6 4))", true);
    test_one<polygon>("POLYGON((0 0,0 10,10 10,10 0,0 0),(4 4,6 4,6 6))", true);
    test_one<polygon>("POLYGON((0 0,0 10,10 10,10 0,0 0),(4 4,6 4,6 6,4 6))", false);
    test_one<multi_point>("MULTIPOINT()", true);
    test_one<multi_point>("MULTIPOINT(0 0)", false);
    test_one<multi_point>("MULTIPOINT(0 0,1 1)", false);
    test_one<multi_linestring>("MULTILINESTRING()", true);
    test_one<multi_linestring>("MULTILINESTRING((0 0),(2 2,3 3,4 4))", true);
    test_one<multi_linestring>("MULTILINESTRING((0 0,1 1),(2 2))", true);
    test_one<multi_linestring>("MULTILINESTRING((0 0,1 1),())", true);
    test_one<multi_linestring>("MULTILINESTRING((0 0),())", true);
    test_one<multi_linestring>("MULTILINESTRING((0 0,1 1),(2 2,3 3,4 4))", false);
    test_one<multi_polygon>("MULTIPOLYGON()", true);
    test_one<multi_polygon>("MULTIPOLYGON(((0 0,0 10,0 0)),((0 10,1 10,1 9,0 10)))", true);
    test_one<multi_polygon>("MULTIPOLYGON(((0 0,0 10,10 10,10 0,0 0)),((0 10,1 10,1 9)))", true);
    test_one<multi_polygon>("MULTIPOLYGON(((0 0,0 10,10 10,10 0,0 0)),((0 10,1 10,1 9,0 10)))", false);

    // test open geometries
    test_one<open_ring>("POLYGON(())", true);
    test_one<open_ring>("POLYGON((0 0))", true);
    test_one<open_ring>("POLYGON((0 0,1 1))", true);
    test_one<open_ring>("POLYGON((0 0,1 1,0 1))", false);
    test_one<open_ring>("POLYGON((0 0,1 1,0 1,0 0))", false);
    test_one<open_polygon>("POLYGON((0 0,10 10))", true);
    test_one<open_polygon>("POLYGON((0 0,10 10,0 10))", false);
    test_one<open_polygon>("POLYGON((0 0,10 10,0 10),(1 1))", true);
    test_one<open_multi_polygon>("MULTIPOLYGON(((0 0,0 10)),((0 10,1 10,1 9)))", true);
    test_one<open_multi_polygon>("MULTIPOLYGON(((0 0,0 10,10 10,10 0)),((0 10,1 10)))", true);
    test_one<open_multi_polygon>("MULTIPOLYGON(((0 0,0 10,10 10,10 0)),((0 10,1 10,1 9)))", false);
    test_one<open_multi_polygon>("MULTIPOLYGON(((0 0,0 10,10 10,10 0,0 0)),((0 10,1 10,1 9,0 10)))", false);

    // test variant geometry
    typedef boost::variant
        <
            multi_point, multi_linestring, open_polygon
        > variant_type;

    variant_type v;
    multi_point mp;
    multi_linestring mls;
    open_polygon open_poly;
    bg::read_wkt("MULTIPOINT(0 0)", mp);
    bg::read_wkt("MULTILINESTRING((0 0,1 1),(2 2))", mls);
    bg::read_wkt("POLYGON((0 0,20 20,20 0),(5 4,8 4,8 5))", open_poly);
    v = mp;
    test_one(v, false);
    v = mls;
    test_one(v, true);
    v = open_poly;
    test_one(v, false);

    // test catching the exception
    test_throw_exception(mp);
    test_catch_exception(mls);
    test_throw_exception(open_poly);

    return 0;
}

