// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

#include <iostream>

#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE test_is_valid
#endif

#include <boost/test/included/unit_test.hpp>


#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/multi/geometries/multi_point.hpp>
#include <boost/geometry/multi/geometries/multi_linestring.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>

#include <boost/geometry/io/wkt/read.hpp>
#include <boost/geometry/io/wkt/write.hpp>
#include <boost/geometry/multi/io/wkt/read.hpp>
#include <boost/geometry/multi/io/wkt/write.hpp>

#include <boost/geometry/algorithms/ogc/is_valid.hpp>

namespace bg = ::boost::geometry;

typedef bg::model::point<double,2,bg::cs::cartesian>  point_type;
typedef bg::model::segment<point_type>                segment_type;
typedef bg::model::box<point_type>                    box_type;
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



template<typename Segment>
Segment make_segment(double x1, double y1, double x2, double y2)
{
    typename boost::geometry::point_type<Segment>::type p(x1, y1), q(x2, y2);
    return Segment(p, q);
}


template<typename Box>
Box make_box(double x1, double y1, double x2, double y2)
{
    typename boost::geometry::point_type<Box>::type p(x1, y1), q(x2, y2);
    return Box(p, q);
}


template <typename Geometry>
void test_valid(Geometry const& g, bool expected_result)
{
#ifdef GEOMETRY_TEST_DEBUG
    std::cout << "=======" << std::endl;
#endif

    bool valid = bg::ogc::is_valid(g);
    BOOST_CHECK( valid == expected_result );

#ifdef GEOMETRY_TEST_DEBUG
    std::cout << "Geometry: " << bg::wkt(g) << std::endl;
    std::cout << std::boolalpha;
    std::cout << "is valid:? " << valid << std::endl;
    std::cout << "expected result: " << expected_result << std::endl;
    std::cout << "=======" << std::endl;
    std::cout << std::endl << std::endl;
    std::cout << std::noboolalpha;
#endif
}

//===========================================================================
//===========================================================================
//===========================================================================

BOOST_AUTO_TEST_CASE( test_is_valid_point )
{
#ifdef GEOMETRY_TEST_DEBUG
    std::cout << std::endl << std::endl;
    std::cout << "************************************" << std::endl;
    std::cout << " is_valid: POINT " << std::endl;
    std::cout << "************************************" << std::endl;
#endif

    typedef point_type G;

    test_valid(from_wkt<G>("POINT(0 0)"), true);
}

BOOST_AUTO_TEST_CASE( test_is_valid_segment )
{
#ifdef GEOMETRY_TEST_DEBUG
    std::cout << std::endl << std::endl;
    std::cout << "************************************" << std::endl;
    std::cout << " is_valid: SEGMENT " << std::endl;
    std::cout << "************************************" << std::endl;
#endif

    typedef segment_type G;

    test_valid(make_segment<G>(0, 0, 0, 0), false);
    test_valid(make_segment<G>(0, 0, 1, 0), true);
}

BOOST_AUTO_TEST_CASE( test_is_valid_box )
{
#ifdef GEOMETRY_TEST_DEBUG
    std::cout << std::endl << std::endl;
    std::cout << "************************************" << std::endl;
    std::cout << " is_valid: BOX " << std::endl;
    std::cout << "************************************" << std::endl;
#endif

    typedef box_type G;

    // boxes where the max corner and below and/or to the left of min corner
    test_valid(make_box<G>(0, 0, -1, 0), false);
    test_valid(make_box<G>(0, 0, 0, -1), false);
    test_valid(make_box<G>(0, 0, -1, -1), false);

    // boxes of zero area; they are not 2-dimensional, so invalid
    test_valid(make_box<G>(0, 0, 0, 0), false);
    test_valid(make_box<G>(0, 0, 1, 0), false);
    test_valid(make_box<G>(0, 0, 0, 1), false);

    test_valid(make_box<G>(0, 0, 1, 1), true);
}

BOOST_AUTO_TEST_CASE( test_is_valid_linestring )
{
#ifdef GEOMETRY_TEST_DEBUG
    std::cout << std::endl << std::endl;
    std::cout << "************************************" << std::endl;
    std::cout << " is_valid: LINESTRING " << std::endl;
    std::cout << "************************************" << std::endl;
#endif

    typedef linestring_type G;

    // empty linestring
    test_valid(from_wkt<G>("LINESTRING()"), false);

    // 1-point linestrings
    test_valid(from_wkt<G>("LINESTRING(0 0)"), false);
    test_valid(from_wkt<G>("LINESTRING(0 0,0 0)"), false);
    test_valid(from_wkt<G>("LINESTRING(0 0,0 0,0 0)"), false);

    // 2-point linestrings
    test_valid(from_wkt<G>("LINESTRING(0 0,1 2)"), true);
    test_valid(from_wkt<G>("LINESTRING(0 0,1 2,1 2)"), true);
    test_valid(from_wkt<G>("LINESTRING(0 0,0 0,1 2,1 2)"), true);
    test_valid(from_wkt<G>("LINESTRING(0 0,0 0,0 0,1 2,1 2)"), true);

    // 3-point linestrings
    test_valid(from_wkt<G>("LINESTRING(0 0,1 0,2 10)"), true);
    test_valid(from_wkt<G>("LINESTRING(0 0,1 0,2 10,0 0)"), true);
    test_valid(from_wkt<G>("LINESTRING(0 0,10 0,10 10,5 0)"), true);
    test_valid(from_wkt<G>("LINESTRING(0 0,10 0,10 10,5 0,4 0)"), true);
    test_valid(from_wkt<G>("LINESTRING(0 0,10 0,10 10,5 0,4 0,3 0)"), true);
    test_valid(from_wkt<G>("LINESTRING(0 0,10 0,10 10,5 0,4 0,-1 0)"), true);
    test_valid(from_wkt<G>("LINESTRING(0 0,1 0,1 1,-1 1,-1 0,0 0)"), true);

    // should this be valid? we have two overlapping segments
    test_valid(from_wkt<G>("LINESTRING(0 0,1 0,1 1,-1 1,-1 0,0.5 0)"), true);

    // linestrings with spikes
    static const bool accept_spikes = false;
    test_valid(from_wkt<G>("LINESTRING(0 0,1 2,0 0)"), accept_spikes);
    test_valid(from_wkt<G>("LINESTRING(0 0,1 2,1 2,0 0)"), accept_spikes);
    test_valid(from_wkt<G>("LINESTRING(0 0,0 0,1 2,1 2,0 0)"), accept_spikes);
    test_valid(from_wkt<G>("LINESTRING(0 0,0 0,0 0,1 2,1 2,0 0,0 0)"),
               accept_spikes);
    test_valid(from_wkt<G>("LINESTRING(0 0,10 0,5 0)"), accept_spikes);    
    test_valid(from_wkt<G>("LINESTRING(0 0,10 0,10 10,5 0,0 0)"), accept_spikes);
    test_valid(from_wkt<G>("LINESTRING(0 0,10 0,10 10,5 0,4 0,6 0)"),
               accept_spikes);
    test_valid(from_wkt<G>("LINESTRING(0 0,1 0,1 1,5 5,4 4)"), accept_spikes);
    test_valid(from_wkt<G>("LINESTRING(0 0,1 0,1 1,5 5,4 4,6 6)"), accept_spikes);
    test_valid(from_wkt<G>("LINESTRING(0 0,1 0,1 1,5 5,4 4,4 0)"), accept_spikes);
}

BOOST_AUTO_TEST_CASE( test_is_valid_multipoint )
{
#ifdef GEOMETRY_TEST_DEBUG
    std::cout << std::endl << std::endl;
    std::cout << "************************************" << std::endl;
    std::cout << " is_valid: MULTIPOINT " << std::endl;
    std::cout << "************************************" << std::endl;
#endif
    typedef multi_point_type G;

    test_valid(from_wkt<G>("MULTIPOINT()"), false);
    test_valid(from_wkt<G>("MULTIPOINT(0 0,0 0)"), true);
    test_valid(from_wkt<G>("MULTIPOINT(0 0,1 0,1 1,0 1)"), true);
    test_valid(from_wkt<G>("MULTIPOINT(0 0,1 0,1 1,1 0,0 1)"), true);
}

BOOST_AUTO_TEST_CASE( test_is_valid_multilinestring )
{
#ifdef GEOMETRY_TEST_DEBUG
    std::cout << std::endl << std::endl;
    std::cout << "************************************" << std::endl;
    std::cout << " is_valid: MULTILINESTRING " << std::endl;
    std::cout << "************************************" << std::endl;
#endif

    typedef multi_linestring_type G;

    // empty multilinestring
    test_valid(from_wkt<G>("MULTILINESTRING()"), false);

    // multilinestring with empty linestring(s)
    test_valid(from_wkt<G>("MULTILINESTRING(())"), false);
    test_valid(from_wkt<G>("MULTILINESTRING((),(),())"), false);
    test_valid(from_wkt<G>("MULTILINESTRING((),(0 1,1 0))"), false);

    // multilinestring with invalid linestrings
    test_valid(from_wkt<G>("MULTILINESTRING((0 0),(0 1,1 0))"), false);
    test_valid(from_wkt<G>("MULTILINESTRING((0 0,0 0),(0 1,1 0))"), false);
    test_valid(from_wkt<G>("MULTILINESTRING((0 0),(1 0))"), false);
    test_valid(from_wkt<G>("MULTILINESTRING((0 0,0 0),(1 0,1 0))"), false);
    test_valid(from_wkt<G>("MULTILINESTRING((0 0),(0 0))"), false);
    test_valid(from_wkt<G>("MULTILINESTRING((0 0,1 0,0 0),(5 0))"), false);
    test_valid(from_wkt<G>("MULTILINESTRING((0 0,1 0,0 0),(5 0,1 0,4 1))"),
               false);
    test_valid(from_wkt<G>("MULTILINESTRING((0 0,1 0,0 0),(1 0,2 0))"),
               false);

    // valid multilinestrings
    test_valid(from_wkt<G>("MULTILINESTRING((0 0,1 0,2 0),(5 0,1 0,4 1))"),
               true);
    test_valid(from_wkt<G>("MULTILINESTRING((0 0,1 0,2 0),(1 0,2 0))"),
               true);
    test_valid(from_wkt<G>("MULTILINESTRING((0 0,1 1),(0 1,1 0))"), true);
    test_valid(from_wkt<G>("MULTILINESTRING((0 0,1 1,2 2),(0 1,1 0,2 2))"), true);
}

#if 0
BOOST_AUTO_TEST_CASE( test_is_valid_rest )
{
    typedef open_polygon_type op;
    typedef closed_polygon_type cp;
    typedef multi_polygon_type mpl;

    test_valid(from_wkt<op>("POLYGON(())"), true);
    test_valid(from_wkt<op>("POLYGON((),())"), true);
    test_valid(from_wkt<op>("POLYGON((0 0,1 0,1 1),())"), true);
    test_valid(from_wkt<op>("POLYGON((0 0,1 0,1 0,1 1),())"), true);
    test_valid(from_wkt<op>("POLYGON((0 0,1 0))"), false);
    test_valid(from_wkt<op>("POLYGON((0 0,2 0,0.5 0,0.5 1))"), false);
    test_valid(from_wkt<op>("POLYGON((0 0,2 0,0.5 0,0.5 0))"), false);
    test_valid(from_wkt<op>("POLYGON((0 0,1 0,1 1,1 0.5))"), false);
    test_valid(from_wkt<op>("POLYGON((0 0,1 0,1 1))"), true);
    test_valid(from_wkt<op>("POLYGON((0 0,1 0,2 1,2 2,1 3))"), true);
    test_valid(from_wkt<op>("POLYGON((0 0,2 0,4 1,1 0))"), false);
    test_valid(from_wkt<op>("POLYGON((0 0,1 0,3 1,-1 2,3 3,3 4,0 4))"), false);
    test_valid(from_wkt<op>("POLYGON((0 0,1 0,3 1,0 2,3 3,3 4,0 4))"), false);
    test_valid(from_wkt<op>("POLYGON((0 0,10 0,10 10,0 10),\
                             (1 1,1 2,2 2,2 1))"), false);

    test_valid(from_wkt<cp>("POLYGON(())"), true);
    test_valid(from_wkt<cp>("POLYGON((),())"), true);
    test_valid(from_wkt<cp>("POLYGON((0 0,1 0,1 1,0 0),())"), true);
    test_valid(from_wkt<cp>("POLYGON((0 0,1 0,1 0,1 1,0 0),())"), true);
    test_valid(from_wkt<cp>("POLYGON((0 0,1 0,0 0))"), false);
    test_valid(from_wkt<cp>("POLYGON((0 0,2 0,0.5 0,0.5 1,0 0))"), false);
    test_valid(from_wkt<cp>("POLYGON((0 0,2 0,0.5 0,0.5 0,0 0))"), false);
    test_valid(from_wkt<cp>("POLYGON((0 0,1 0,1 1,1 0.5,0 0))"), false);
    test_valid(from_wkt<cp>("POLYGON((0 0,1 0,1 1,0 0))"), true);
    test_valid(from_wkt<cp>("POLYGON((0 0,1 0,2 1,2 2,1 3,0 0))"), true);
    test_valid(from_wkt<cp>("POLYGON((0 0,2 0,4 1,1 0,0 0))"), false);
    test_valid(from_wkt<cp>("POLYGON((0 0,1 0,3 1,-1 2,3 3,3 4,0 4,0 0))"),
                false);
    test_valid(from_wkt<cp>("POLYGON((0 0,1 0,3 1,0 2,3 3,3 4,0 4,0 0))"),
                false);
    test_valid(from_wkt<cp>("POLYGON((0 0,10 0,10 10,0 10,0 0),\
                              (1 1,1 2,2 2,2 1,1 1))"), false);


    test_valid(from_wkt<mpl>("MULTIPOLYGON()"), true);
    test_valid(from_wkt<mpl>("MULTIPOLYGON( ((),()) )"), true);
    test_valid(from_wkt<mpl>("MULTIPOLYGON( (()),(()) )"), true);
    test_valid(from_wkt<mpl>("MULTIPOLYGON( ((),()),(()) )"), true);
    test_valid(from_wkt<mpl>("MULTIPOLYGON( ((0 0),()),(()) )"), true);
    test_valid(from_wkt<mpl>("MULTIPOLYGON( ((0 0),()),((1 1)) )"), true);
#ifdef GEOMETRY_TEST_INCLUDE_FAILING_TESTS
    //    test_valid(from_wkt<mpl>("MULTIPOLYGON( ((0 0),()),((0 0)) )"), false);
#endif

    test_valid(from_wkt<mpl>("MULTIPOLYGON(((0 0,1 0,2 1,2 2,1 3)),\
                              ((10 0,11 0,11 1)))"), true);

    test_valid(from_wkt<mpl>("MULTIPOLYGON(((0 0,1 0,1 0,2 1,2 2,1 3)),\
                              ((10 0,11 0,11 1,11 1)))"), true);

    test_valid(from_wkt<mpl>("MULTIPOLYGON(((0 0,1 0,3 1,0 2,3 3,3 4,0 4)),\
                              ((10 0,11 0,11 1)))"), false);
}
#endif
