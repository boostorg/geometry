// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#include <iostream>

#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE test_is_simple
#endif

#include <boost/test/included/unit_test.hpp>


#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/multi/geometries/multi_point.hpp>
#include <boost/geometry/multi/geometries/multi_linestring.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>

#include <boost/geometry/io/wkt/read.hpp>
#include <boost/geometry/io/wkt/write.hpp>
#include <boost/geometry/multi/io/wkt/read.hpp>
#include <boost/geometry/multi/io/wkt/write.hpp>

#include <boost/geometry/algorithms/is_valid.hpp>
#include <boost/geometry/algorithms/is_simple.hpp>

namespace bg = ::boost::geometry;

typedef bg::model::point<double,2,bg::cs::cartesian>  point_type;
typedef bg::model::segment<point_type>                segment_type;
typedef bg::model::linestring<point_type>             linestring_type;
typedef bg::model::multi_linestring<linestring_type>  multi_linestring_type;
// ccw open and closed polygons
typedef bg::model::polygon<point_type,false,false>    open_polygon_type;
typedef bg::model::polygon<point_type,false,true>     closed_polygon_type;

// multi-geometries
typedef bg::model::multi_point<point_type>            multi_point_type;
typedef bg::model::multi_polygon<open_polygon_type>   multi_polygon_type;


template <typename Geometry>
Geometry from_wkt(std::string const& wkt)
{
    Geometry g;
    bg::read_wkt(wkt, g);
    return g;
}



template <typename Geometry>
void test_simple(Geometry const& g, bool simple_geometry)
{
#ifdef GEOMETRY_TEST_DEBUG
    std::cout << "=======" << std::endl;
#endif

    bool simple = bg::is_simple(g);
    BOOST_CHECK(simple == simple_geometry);

#ifdef GEOMETRY_TEST_DEBUG
    std::cout << "Geometry: " << bg::wkt(g) << std::endl;
    std::cout << std::boolalpha;
    std::cout << "is valid : " << bg::is_valid(g) << std::endl;
    std::cout << "is simple: " << simple << std::endl;
    std::cout << "expected result: " << simple_geometry << std::endl;
    std::cout << "=======" << std::endl;
    std::cout << std::endl << std::endl;
    std::cout << std::noboolalpha;
#endif
}

//===========================================================================
//===========================================================================
//===========================================================================

BOOST_AUTO_TEST_CASE( test_is_simple_point )
{
#ifdef GEOMETRY_TEST_DEBUG
    std::cout << std::endl << std::endl;
    std::cout << "************************************" << std::endl;
    std::cout << " is_simple: POINT " << std::endl;
    std::cout << "************************************" << std::endl;
#endif

    typedef point_type G;

    test_simple(from_wkt<G>("POINT(0 0)"), true);
}

BOOST_AUTO_TEST_CASE( test_is_simple_segment )
{
#ifdef GEOMETRY_TEST_DEBUG
    std::cout << std::endl << std::endl;
    std::cout << "************************************" << std::endl;
    std::cout << " is_simple: SEGMENT " << std::endl;
    std::cout << "************************************" << std::endl;
#endif

    typedef segment_type G;

    test_simple(from_wkt<G>("SEGMENT(0 0,0 0)"), false);
    test_simple(from_wkt<G>("SEGMENT(0 0,1 0)"), true);
}

BOOST_AUTO_TEST_CASE( test_is_simple_linestring )
{
#ifdef GEOMETRY_TEST_DEBUG
    std::cout << std::endl << std::endl;
    std::cout << "************************************" << std::endl;
    std::cout << " is_simple: LINESTRING " << std::endl;
    std::cout << "************************************" << std::endl;
#endif

    typedef linestring_type G;

    // invalid linestrings
    test_simple(from_wkt<G>("LINESTRING()"), false);
    test_simple(from_wkt<G>("LINESTRING(0 0)"), false);
    test_simple(from_wkt<G>("LINESTRING(0 0,0 0)"), false);

    // valid linestrings with multiple points
    test_simple(from_wkt<G>("LINESTRING(0 0,0 0,1 0)"), false);
    test_simple(from_wkt<G>("LINESTRING(0 0,0 0,1 0,0 0)"), false);
    test_simple(from_wkt<G>("LINESTRING(0 0,0 0,1 0,1 0,1 1,0 0)"), false);
    test_simple(from_wkt<G>("LINESTRING(0 0,1 0,2 0,1 1,1 0,1 -1)"), false);

    // simple open linestrings
    test_simple(from_wkt<G>("LINESTRING(0 0,1 2)"), true);
    test_simple(from_wkt<G>("LINESTRING(0 0,1 2,2 3)"), true);

    // simple closed linestrings
    test_simple(from_wkt<G>("LINESTRING(0 0,1 0,1 1,0 0)"), true);
    test_simple(from_wkt<G>("LINESTRING(0 0,1 0,1 1,0 1,0 0)"), true);

    // non-simple linestrings
    test_simple(from_wkt<G>("LINESTRING(0 0,1 0,0 0)"), false);
    test_simple(from_wkt<G>("LINESTRING(0 0,1 0,2 10,0.5 -1)"), false);
    test_simple(from_wkt<G>("LINESTRING(0 0,1 0,2 1,1 0)"), false);
    test_simple(from_wkt<G>("LINESTRING(0 0,1 0,2 1,0.5 0)"), false);
    test_simple(from_wkt<G>("LINESTRING(0 0,2 0,1 0)"), false);
    test_simple(from_wkt<G>("LINESTRING(0 0,3 0,5 0,1 0)"), false);
    test_simple(from_wkt<G>("LINESTRING(0 0,3 0,5 0,4 0)"), false);
    test_simple(from_wkt<G>("LINESTRING(0 0,3 0,5 0,4 0,2 0)"), false);
    test_simple(from_wkt<G>("LINESTRING(0 0,3 0,2 0,5 0)"), false);
    test_simple(from_wkt<G>("LINESTRING(0 0,2 0,2 2,1 0,0 0)"), false);
}

BOOST_AUTO_TEST_CASE( test_is_simple_multipoint )
{
#ifdef GEOMETRY_TEST_DEBUG
    std::cout << std::endl << std::endl;
    std::cout << "************************************" << std::endl;
    std::cout << " is_simple: MULTIPOINT " << std::endl;
    std::cout << "************************************" << std::endl;
#endif
    typedef multi_point_type G;

    test_simple(from_wkt<G>("MULTIPOINT()"), false);
    test_simple(from_wkt<G>("MULTIPOINT(0 0)"), true);
    test_simple(from_wkt<G>("MULTIPOINT(0 0,1 0,1 1,0 1)"), true);
    test_simple(from_wkt<G>("MULTIPOINT(0 0,1 0,1 1,1 0,0 1)"), false);
}

BOOST_AUTO_TEST_CASE( test_is_simple_multilinestring )
{
#ifdef GEOMETRY_TEST_DEBUG
    std::cout << std::endl << std::endl;
    std::cout << "************************************" << std::endl;
    std::cout << " is_simple: MULTILINESTRING " << std::endl;
    std::cout << "************************************" << std::endl;
#endif

    typedef multi_linestring_type G;

    // empty multilinestring
    test_simple(from_wkt<G>("MULTILINESTRING()"), false);

    // multilinestrings with empty linestrings
    test_simple(from_wkt<G>("MULTILINESTRING(())"), false);
    test_simple(from_wkt<G>("MULTILINESTRING((),(),())"), false);
    test_simple(from_wkt<G>("MULTILINESTRING((),(0 1,1 0))"), false);

    // multilinestrings with 1-point linestrings
    test_simple(from_wkt<G>("MULTILINESTRING((0 0),(0 1,1 0))"), false);
    test_simple(from_wkt<G>("MULTILINESTRING((0 0,0 0),(0 1,1 0))"), false);
    test_simple(from_wkt<G>("MULTILINESTRING((0 0),(1 0))"), false);
    test_simple(from_wkt<G>("MULTILINESTRING((0 0,0 0),(1 0,1 0))"), false);
    test_simple(from_wkt<G>("MULTILINESTRING((0 0),(0 0))"), false);

    // multilinestrings with linestrings with spikes
    test_simple(from_wkt<G>("MULTILINESTRING((0 0,1 0,0 0),(5 0))"), false);
    test_simple(from_wkt<G>("MULTILINESTRING((0 0,1 0,0 0),(5 0,1 0,4 1))"),
                false);
    test_simple(from_wkt<G>("MULTILINESTRING((0 0,1 0,0 0),(5 0,1 0,4 0))"),
                false);
    test_simple(from_wkt<G>("MULTILINESTRING((0 0,1 0,0 0),(1 0,2 0))"),
                false);


    // simple multilinestrings
    test_simple(from_wkt<G>("MULTILINESTRING((0 0,1 1),(1 1,1 0))"), true);
    test_simple(from_wkt<G>("MULTILINESTRING((0 0,1 1),(1 1,1 0),(0 1,1 1))"),
                true);
    test_simple(from_wkt<G>("MULTILINESTRING((0 0,2 2),(0 0,1 0,2 0,2 2))"), true);
    test_simple(from_wkt<G>("MULTILINESTRING((0 0,2 2),(2 2,2 0,1 0,0 0))"), true);
    test_simple(from_wkt<G>("MULTILINESTRING((0 0,1 0),(0 0,-1 0),\
                            (1 0,2 0))"),
                true);
    test_simple(from_wkt<G>("MULTILINESTRING((0 0,1 0),(-1 0,0 0),\
                            (2 0,1 0))"),
                true);
    test_simple(from_wkt<G>("MULTILINESTRING((0 0,1 0,1 1,0 1,0 0),(-1 0,0 0))"),
                true);

    // non-simple multilinestrings
    test_simple(from_wkt<G>("MULTILINESTRING((0 0,2 2),(0 0,2 2))"), false);
    test_simple(from_wkt<G>("MULTILINESTRING((0 0,2 2),(2 2,0 0))"), false);
    test_simple(from_wkt<G>("MULTILINESTRING((0 0,2 2),\
                            (0 0,1 0,1 1,2 0,2 2))"),
                false);
    test_simple(from_wkt<G>("MULTILINESTRING((0 0,1 1,2 2),\
                            (0 0,1 0,1 1,2 0,2 2))"),
                false);
    test_simple(from_wkt<G>("MULTILINESTRING((0 0,1 1,2 2),(2 2,0 0))"), false);
    test_simple(from_wkt<G>("MULTILINESTRING((0 0,1 1),(0 1,1 0))"), false);
    test_simple(from_wkt<G>("MULTILINESTRING((0 0,2 0),(1 0,0 1))"), false);
    test_simple(from_wkt<G>("MULTILINESTRING((0 0,1 1),(1 1,1 0),\
                             (1 1,0 1,0.5,0.5))"),
                false);
    test_simple(from_wkt<G>("MULTILINESTRING((0 0,1 0,1 1,0 1,0 0),(1 0,1 -1))"),
                false);
}


#if 0
BOOST_AUTO_TEST_CASE( test_is_simple_rest )
{
    typedef open_polygon_type op;
    typedef closed_polygon_type cp;
    typedef multi_polygon_type mpl;

    test_simple(from_wkt<op>("POLYGON(())"), true);
    test_simple(from_wkt<op>("POLYGON((),())"), true);
    test_simple(from_wkt<op>("POLYGON((0 0,1 0,1 1),())"), true);
    test_simple(from_wkt<op>("POLYGON((0 0,1 0,1 0,1 1),())"), true);
    test_simple(from_wkt<op>("POLYGON((0 0,1 0))"), false);
    test_simple(from_wkt<op>("POLYGON((0 0,2 0,0.5 0,0.5 1))"), false);
    test_simple(from_wkt<op>("POLYGON((0 0,2 0,0.5 0,0.5 0))"), false);
    test_simple(from_wkt<op>("POLYGON((0 0,1 0,1 1,1 0.5))"), false);
    test_simple(from_wkt<op>("POLYGON((0 0,1 0,1 1))"), true);
    test_simple(from_wkt<op>("POLYGON((0 0,1 0,2 1,2 2,1 3))"), true);
    test_simple(from_wkt<op>("POLYGON((0 0,2 0,4 1,1 0))"), false);
    test_simple(from_wkt<op>("POLYGON((0 0,1 0,3 1,-1 2,3 3,3 4,0 4))"), false);
    test_simple(from_wkt<op>("POLYGON((0 0,1 0,3 1,0 2,3 3,3 4,0 4))"), false);
    test_simple(from_wkt<op>("POLYGON((0 0,10 0,10 10,0 10),\
                             (1 1,1 2,2 2,2 1))"), false);

    test_simple(from_wkt<cp>("POLYGON(())"), true);
    test_simple(from_wkt<cp>("POLYGON((),())"), true);
    test_simple(from_wkt<cp>("POLYGON((0 0,1 0,1 1,0 0),())"), true);
    test_simple(from_wkt<cp>("POLYGON((0 0,1 0,1 0,1 1,0 0),())"), true);
    test_simple(from_wkt<cp>("POLYGON((0 0,1 0,0 0))"), false);
    test_simple(from_wkt<cp>("POLYGON((0 0,2 0,0.5 0,0.5 1,0 0))"), false);
    test_simple(from_wkt<cp>("POLYGON((0 0,2 0,0.5 0,0.5 0,0 0))"), false);
    test_simple(from_wkt<cp>("POLYGON((0 0,1 0,1 1,1 0.5,0 0))"), false);
    test_simple(from_wkt<cp>("POLYGON((0 0,1 0,1 1,0 0))"), true);
    test_simple(from_wkt<cp>("POLYGON((0 0,1 0,2 1,2 2,1 3,0 0))"), true);
    test_simple(from_wkt<cp>("POLYGON((0 0,2 0,4 1,1 0,0 0))"), false);
    test_simple(from_wkt<cp>("POLYGON((0 0,1 0,3 1,-1 2,3 3,3 4,0 4,0 0))"),
                false);
    test_simple(from_wkt<cp>("POLYGON((0 0,1 0,3 1,0 2,3 3,3 4,0 4,0 0))"),
                false);
    test_simple(from_wkt<cp>("POLYGON((0 0,10 0,10 10,0 10,0 0),\
                              (1 1,1 2,2 2,2 1,1 1))"), false);


    test_simple(from_wkt<mpl>("MULTIPOLYGON()"), true);
    test_simple(from_wkt<mpl>("MULTIPOLYGON( ((),()) )"), true);
    test_simple(from_wkt<mpl>("MULTIPOLYGON( (()),(()) )"), true);
    test_simple(from_wkt<mpl>("MULTIPOLYGON( ((),()),(()) )"), true);
    test_simple(from_wkt<mpl>("MULTIPOLYGON( ((0 0),()),(()) )"), true);
    test_simple(from_wkt<mpl>("MULTIPOLYGON( ((0 0),()),((1 1)) )"), true);
#ifdef GEOMETRY_TEST_INCLUDE_FAILING_TESTS
    //    test_simple(from_wkt<mpl>("MULTIPOLYGON( ((0 0),()),((0 0)) )"), false);
#endif

    test_simple(from_wkt<mpl>("MULTIPOLYGON(((0 0,1 0,2 1,2 2,1 3)),\
                              ((10 0,11 0,11 1)))"), true);

    test_simple(from_wkt<mpl>("MULTIPOLYGON(((0 0,1 0,1 0,2 1,2 2,1 3)),\
                              ((10 0,11 0,11 1,11 1)))"), true);

    test_simple(from_wkt<mpl>("MULTIPOLYGON(((0 0,1 0,3 1,0 2,3 3,3 4,0 4)),\
                              ((10 0,11 0,11 1)))"), false);
}
#endif
