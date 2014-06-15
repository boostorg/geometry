// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE test_is_valid
#endif

#include <iostream>
#include <string>

#include <boost/test/included/unit_test.hpp>

#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/ring.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/multi_point.hpp>
#include <boost/geometry/geometries/multi_linestring.hpp>
#include <boost/geometry/geometries/multi_polygon.hpp>

#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/io/wkt/read.hpp>

#ifdef GEOMETRY_TEST_DEBUG
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/io/wkt/write.hpp>
#include <boost/geometry/io/dsv/write.hpp>
#endif

#include <boost/geometry/algorithms/is_valid.hpp>

namespace bg = ::boost::geometry;

typedef bg::model::point<double, 2, bg::cs::cartesian> point_type;
typedef bg::model::segment<point_type>                 segment_type;
typedef bg::model::box<point_type>                     box_type;
typedef bg::model::linestring<point_type>              linestring_type;
typedef bg::model::multi_linestring<linestring_type>   multi_linestring_type;
// cw, ccw, open and closed rings
typedef bg::model::ring<point_type,false,false>        open_ccw_ring_type;
typedef bg::model::ring<point_type,false,true>         closed_ccw_ring_type;
typedef bg::model::ring<point_type,true,false>         open_cw_ring_type;
typedef bg::model::ring<point_type,true,true>          closed_cw_ring_type;
// cw, ccw, open and closed polygons
typedef bg::model::polygon<point_type,false,false>     open_ccw_polygon_type;
typedef bg::model::polygon<point_type,false,true>      closed_ccw_polygon_type;
typedef bg::model::polygon<point_type,true,false>      open_cw_polygon_type;
typedef bg::model::polygon<point_type,true,true>       closed_cw_polygon_type;

// multi-geometries
typedef bg::model::multi_point<point_type>             multi_point_type;


template <typename Geometry>
Geometry from_wkt(std::string const& wkt)
{
    Geometry g;
    bg::read_wkt(wkt, g);
    return g;
}


//----------------------------------------------------------------------------


#ifdef GEOMETRY_TEST_DEBUG
template <typename Geometry, typename Tag = typename bg::tag<Geometry>::type>
struct pretty_print_geometry
{
    static inline std::ostream&
    apply(std::ostream& os, Geometry const& geometry)
    {
        os << bg::wkt(geometry);
        return os;
    }
};

template <typename Box>
struct pretty_print_geometry<Box, bg::box_tag>
{
    static inline std::ostream&
    apply(std::ostream& os, Box const& box)
    {
        return os << "BOX" << bg::dsv(box);
    }
};

template <typename Segment>
struct pretty_print_geometry<Segment, bg::segment_tag>
{
    static inline std::ostream&
    apply(std::ostream& os, Segment const& segment)
    {
        return os << "SEGMENT" << bg::dsv(segment);
    }
};

template <typename Ring>
struct pretty_print_geometry<Ring, bg::ring_tag>
{
    static inline std::ostream&
    apply(std::ostream& os, Ring const& ring)
    {
        return os << "RING" << bg::dsv(ring);
    }
};
#endif


//----------------------------------------------------------------------------


template <typename Geometry>
void test_valid(Geometry const& g, bool expected_result)
{
#ifdef GEOMETRY_TEST_DEBUG
    std::cout << "=======" << std::endl;
#endif

    bool valid = bg::is_valid(g);
    BOOST_CHECK( valid == expected_result );

#ifdef GEOMETRY_TEST_DEBUG
    std::cout << "Geometry: ";
    pretty_print_geometry<Geometry>::apply(std::cout, g);
    std::cout << std::endl;
    std::cout << std::boolalpha;
    std::cout << "is valid:? " << valid << std::endl;
    std::cout << "expected result: " << expected_result << std::endl;
    std::cout << "=======" << std::endl;
    std::cout << std::endl << std::endl;
    std::cout << std::noboolalpha;
#endif
}


//----------------------------------------------------------------------------


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

    test_valid(from_wkt<G>("SEGMENT(0 0,0 0)"), false);
    test_valid(from_wkt<G>("SEGMENT(0 0,1 0)"), true);
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
    test_valid(from_wkt<G>("BOX(0 0,-1 0)"), false);
    test_valid(from_wkt<G>("BOX(0 0,0 -1)"), false);
    test_valid(from_wkt<G>("BOX(0 0,-1 -1)"), false);

    // boxes of zero area; they are not 2-dimensional, so invalid
    test_valid(from_wkt<G>("BOX(0, 0, 0, 0)"), false);
    test_valid(from_wkt<G>("BOX(0 0,1 0)"), false);
    test_valid(from_wkt<G>("BOX(0 0,0 1)"), false);

    test_valid(from_wkt<G>("BOX(0 0,1 1)"), true);
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
    static const bool accept_spikes = true;
    test_valid(from_wkt<G>("LINESTRING(0 0,1 2,0 0)"), accept_spikes);
    test_valid(from_wkt<G>("LINESTRING(0 0,1 2,1 2,0 0)"), accept_spikes);
    test_valid(from_wkt<G>("LINESTRING(0 0,0 0,1 2,1 2,0 0)"), accept_spikes);
    test_valid(from_wkt<G>("LINESTRING(0 0,0 0,0 0,1 2,1 2,0 0,0 0)"),
               accept_spikes);
    test_valid(from_wkt<G>("LINESTRING(0 0,10 0,5 0)"), accept_spikes);    
    test_valid(from_wkt<G>("LINESTRING(0 0,10 0,10 10,5 0,0 0)"),
               accept_spikes);
    test_valid(from_wkt<G>("LINESTRING(0 0,10 0,10 10,5 0,4 0,6 0)"),
               accept_spikes);
    test_valid(from_wkt<G>("LINESTRING(0 0,1 0,1 1,5 5,4 4)"),
               accept_spikes);
    test_valid(from_wkt<G>("LINESTRING(0 0,1 0,1 1,5 5,4 4,6 6)"),
               accept_spikes);
    test_valid(from_wkt<G>("LINESTRING(0 0,1 0,1 1,5 5,4 4,4 0)"),
               accept_spikes);
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

    // multilinstring that has linestrings with spikes
    static const bool accept_spikes = true;
    test_valid(from_wkt<G>("MULTILINESTRING((0 0,1 0,0 0),(5 0,1 0,4 1))"),
               accept_spikes);
    test_valid(from_wkt<G>("MULTILINESTRING((0 0,1 0,0 0),(1 0,2 0))"),
               accept_spikes);
    

    // valid multilinestrings
    test_valid(from_wkt<G>("MULTILINESTRING((0 0,1 0,2 0),(5 0,1 0,4 1))"),
               true);
    test_valid(from_wkt<G>("MULTILINESTRING((0 0,1 0,2 0),(1 0,2 0))"),
               true);
    test_valid(from_wkt<G>("MULTILINESTRING((0 0,1 1),(0 1,1 0))"), true);
    test_valid(from_wkt<G>("MULTILINESTRING((0 0,1 1,2 2),(0 1,1 0,2 2))"), true);
}

BOOST_AUTO_TEST_CASE( test_is_valid_ring )
{
    typedef open_ccw_ring_type OG;
    typedef closed_ccw_ring_type CG;

    // open rings
    // ----------
#ifdef GEOMETRY_TEST_DEBUG
    std::cout << std::endl << std::endl;
    std::cout << "************************************" << std::endl;
    std::cout << " is_valid: RING (open) " << std::endl;
    std::cout << "************************************" << std::endl;
#endif

    // not enough points
    test_valid(from_wkt<OG>("POLYGON(())"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0))"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,1 0))"), false);

    // duplicate points
    test_valid(from_wkt<OG>("POLYGON((0 0,0 0,0 0))"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,1 0,1 0))"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,1 0,0 0))"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,1 0,1 1,0 0))"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,1 0,1 0,1 1))"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,1 0,1 0,1 1,0 0))"), false);

    // with spikes
    test_valid(from_wkt<OG>("POLYGON((0 0,2 0,2 2,0 2,1 2))"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,2 0,1 0,2 2))"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,1 0,2 0,1 0,4 0,4 4))"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,2 0,2 2,1 0))"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,2 0,1 0))"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,5 0,5 5,4 4,5 5,0 5))"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,5 0,5 5,4 4,3 3,5 5,0 5))"), false);

    // with self-crossings
    test_valid(from_wkt<OG>("POLYGON((0 0,5 0,5 5,3 -1,0 5))"), false);

    // with self-intersections
    test_valid(from_wkt<OG>("POLYGON((0 0,5 0,5 5,3 5,3 0,2 0,2 5,0 5))"),
                            false);

    test_valid(from_wkt<OG>("POLYGON((0 0,5 0,5 5,3 5,3 0,2 5,0 5))"),
                            false);

    test_valid(from_wkt<OG>("POLYGON((0 0,5 0,5 1,1 1,1 2,2 2,3 1,\
                             4 2,5 2,5 5,0 5))"),
                            false);
    // next two suggested by Adam Wulkiewicz
    test_valid(from_wkt<OG>("POLYGON((0 0,5 0,5 5,0 5,4 4,2 2,0 5))"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,5 0,5 5,1 4,4 4,4 1,0 5))"),
               false);
    test_valid(from_wkt<OG>("POLYGON((0 0,5 0,5 5,4 4,1 4,1 1,4 1,4 4,0 5))"),
               false);
    test_valid(from_wkt<OG>("POLYGON((0 0,5 0,5 5,4 4,4 1,1 1,1 4,4 4,0 5))"),
               false);

    // valid rings
    test_valid(from_wkt<OG>("POLYGON((0 0,1 0,1 1))"), true);
    test_valid(from_wkt<OG>("POLYGON((0 0,1 0,1 1,0 1))"), true);

    typedef open_cw_ring_type OG1;
    test_valid(from_wkt<OG1>("POLYGON((0 0,1 1,1 0))"), true);
    test_valid(from_wkt<OG1>("POLYGON((0 0,0 1,1 1,1 0))"), true);

    // closed rings
    // ------------
#ifdef GEOMETRY_TEST_DEBUG
    std::cout << std::endl << std::endl;
    std::cout << "************************************" << std::endl;
    std::cout << " is_valid: RING (closed) " << std::endl;
    std::cout << "************************************" << std::endl;
#endif

    // not enough points
    test_valid(from_wkt<CG>("POLYGON(())"), false);
    test_valid(from_wkt<CG>("POLYGON((0 0))"), false);
    test_valid(from_wkt<CG>("POLYGON((0 0,1 0))"), false);
    test_valid(from_wkt<CG>("POLYGON((0 0,1 0,2 0))"), false);

    // boundary not closed
    test_valid(from_wkt<CG>("POLYGON((0 0,1 0,1 1,1 2))"), false);

    // duplicate points
    test_valid(from_wkt<CG>("POLYGON((0 0,1 0,1 0,0 0))"), false);
    test_valid(from_wkt<CG>("POLYGON((0 0,1 0,0 0,0 0))"), false);
    test_valid(from_wkt<CG>("POLYGON((0 0,1 0,1 1,0 0,0 0))"), false);
    test_valid(from_wkt<CG>("POLYGON((0 0,1 0,1 0,1 1,0 0))"), false);
    test_valid(from_wkt<CG>("POLYGON((0 0,1 0,1 0,1 1,0 0,0 0))"), false);

    // with spikes
    test_valid(from_wkt<CG>("POLYGON((0 0,2 0,2 2,0 2,1 2,0 0))"), false);
    test_valid(from_wkt<CG>("POLYGON((0 0,2 0,1 0,2 2,0 0))"), false);
    test_valid(from_wkt<CG>("POLYGON((0 0,1 0,2 0,1 0,4 0,4 4,0 0))"), false);
    test_valid(from_wkt<CG>("POLYGON((0 0,2 0,2 2,1 0,0 0))"), false);
    test_valid(from_wkt<CG>("POLYGON((0 0,2 0,1 0,0 0))"), false);
    test_valid(from_wkt<CG>("POLYGON((0 0,5 0,5 5,4 4,5 5,0 5,0 0))"),
               false);
    test_valid(from_wkt<CG>("POLYGON((0 0,5 0,5 5,4 4,3 3,5 5,0 5,0 0))"),
               false);

    // with self-crossings
    test_valid(from_wkt<CG>("POLYGON((0 0,5 0,5 5,3 -1,0 5,0 0))"), false);

    // with self-intersections
    test_valid(from_wkt<CG>("POLYGON((0 0,5 0,5 5,3 5,3 0,2 0,2 5,0 5,0 0))"),
                            false);

    test_valid(from_wkt<CG>("POLYGON((0 0,5 0,5 5,3 5,3 0,2 5,0 5,0 0))"),
                            false);

    test_valid(from_wkt<CG>("POLYGON((0 0,5 0,5 1,1 1,1 2,2 2,3 1,\
                             4 2,5 2,5 5,0 5,0 0))"),
                            false);

    // next two suggested by Adam Wulkiewicz
    test_valid(from_wkt<CG>("POLYGON((0 0,5 0,5 5,0 5,4 4,2 2,0 5,0 0))"),
               false);
    test_valid(from_wkt<CG>("POLYGON((0 0,5 0,5 5,1 4,4 4,4 1,0 5,0 0))"),
               false);
    test_valid(from_wkt<CG>("POLYGON((0 0,5 0,5 5,4 4,4 1,1 1,1 4,4 4,0 5,0 0))"),
               false);
    test_valid(from_wkt<CG>("POLYGON((0 0,5 0,5 5,4 4,1 4,1 1,4 1,4 4,0 5,0 0))"),
               false);

    // valid rings
    test_valid(from_wkt<CG>("POLYGON((0 0,1 0,1 1,0 0))"), true);
    test_valid(from_wkt<CG>("POLYGON((0 0,1 0,1 1,0 1,0 0))"), true);
}

BOOST_AUTO_TEST_CASE( test_is_valid_polygon )
{
    typedef open_ccw_polygon_type OG;
    //    typedef closed_ccw_polygon_type CG;

    // open polygons
    // -------------
#ifdef GEOMETRY_TEST_DEBUG
    std::cout << std::endl << std::endl;
    std::cout << "************************************" << std::endl;
    std::cout << " is_valid: POLYGON (open) " << std::endl;
    std::cout << "************************************" << std::endl;
#endif

    // not enough points in exterior ring
    test_valid(from_wkt<OG>("POLYGON(())"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0))"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,1 0))"), false);

    // not enough points in interior ring
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),())"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),(1 1))"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),(1 1,2 2))"), false);

    // duplicate points in exterior ring
    test_valid(from_wkt<OG>("POLYGON((0 0,0 0,0 0))"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,1 0,1 0))"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,1 0,0 0))"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,1 0,1 1,0 0))"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,1 0,1 0,1 1))"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,1 0,1 0,1 1,0 0))"), false);

    // duplicate points in interior ring
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),(1 1,1 1,1 1))"),
               false);
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),(1 1,2 1,2 1))"),
               false);
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),(1 1,2 1,1 1))"),
               false);
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),(1 1,2 1,2 2,1 1))"),
               false);
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),(1 1,2 1,2 1,2 2))"),
               false);
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),(1 1,2 1,2 1,2 2,1 1))"),
               false);

    // with spikes in exterior ring
    test_valid(from_wkt<OG>("POLYGON((0 0,2 0,2 2,0 2,1 2))"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,2 0,1 0,2 2))"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,1 0,2 0,1 0,4 0,4 4))"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,2 0,2 2,1 0))"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,2 0,1 0))"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,5 0,5 5,4 4,5 5,0 5))"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,5 0,5 5,4 4,3 3,5 5,0 5))"), false);

    // with spikes in interior ring
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),(1 1,3 1,3 3,1 3,2 3))"),
               false);
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),(1 1,3 1,2 1,3 3))"),
               false);
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),(1 1,2 1,3 1,2 1,4 1,4 4))"),
               false);
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),(1 1,3 1,3 3,2 1))"),
               false);
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),(1 1,3 1,2 1))"),
               false);

    // with self-crossings in exterior ring
    test_valid(from_wkt<OG>("POLYGON((0 0,5 0,5 5,3 -1,0 5))"),
               false);
    // example from Norvald Ryeng
    test_valid(from_wkt<OG>("POLYGON((100 1300,140 1300,140 170,100 1700))"),
               false);
    // and with point order reversed
    test_valid(from_wkt<OG>("POLYGON((100 1300,100 1700,140 170,140 1300))"),
               false);

    // with self-crossings in interior ring
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),\
                             (3 3,3 7,4 6,2 6))"),
               false);

    // with self-crossings between rings
    test_valid(from_wkt<OG>("POLYGON((0 0,5 0,5 5,0 5),(1 1,2 1,1 -1))"),
               false);

    // with self-intersections in exterior ring
    test_valid(from_wkt<OG>("POLYGON((0 0,5 0,5 5,3 5,3 0,2 0,2 5,0 5))"),
                            false);

    test_valid(from_wkt<OG>("POLYGON((0 0,5 0,5 5,3 5,3 0,2 5,0 5))"),
                            false);

    test_valid(from_wkt<OG>("POLYGON((0 0,5 0,5 1,1 1,1 2,2 2,3 1,\
                             4 2,5 2,5 5,0 5))"),
                            false);

    // next two suggested by Adam Wulkiewicz
    test_valid(from_wkt<OG>("POLYGON((0 0,5 0,5 5,0 5,4 4,2 2,0 5))"), false);
    test_valid(from_wkt<OG>("POLYGON((0 0,5 0,5 5,1 4,4 4,4 1,0 5))"),
               false);
    test_valid(from_wkt<OG>("POLYGON((0 0,5 0,5 5,4 4,1 4,1 1,4 1,4 4,0 5))"),
               false);
    test_valid(from_wkt<OG>("POLYGON((0 0,5 0,5 5,4 4,4 1,1 1,1 4,4 4,0 5))"),
               false);

    // with self-intersections in interior ring
    test_valid(from_wkt<OG>("POLYGON((-10 -10,10 -10,10 10,-10 10),\
                            (0 0,5 0,5 5,3 5,3 0,2 0,2 5,0 5))"),
                            false);
    test_valid(from_wkt<OG>("POLYGON((-10 -10,10 -10,10 10,-10 10),\
                            (0 0,5 0,5 5,3 5,3 0,2 5,0 5))"),
                            false);

    test_valid(from_wkt<OG>("POLYGON((-10 -10,10 -10,10 10,-10 10),\
                            (0 0,5 0,5 1,1 1,1 2,2 2,3 1,4 2,5 2,5 5,0 5))"),
                            false);

    // with self-intersections between rings
    // hole has common segment with exterior ring
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),\
                            (1 1,1 10,2 10,2 1))"),
               false);
    // hole touches exterior ring at one point
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),\
                            (1 1,1 10,2 1))"),
               true);
    // "hole" is outside the exterior ring, but touches it
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),\
                            (5 10,4 11,6 11))"),
               false);
    // hole touches exterior ring at vertex
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),\
                            (0 0,1 4,4 1))"),
               true);
    // "hole" is completely outside the exterior ring
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),\
                            (20 20,20 21,21 21,21 20))"),
               false);

    // example from Norvald Ryeng
    test_valid(from_wkt<OG>("POLYGON((58 31,56.57 30,62 33),\
                             (35 9,28 14,31 16),\
                             (23 11,29 5,26 4))"),
               false);
    // and with points reversed
    test_valid(from_wkt<OG>("POLYGON((58 31,62 33,56.57 30),\
                             (35 9,31 16,28 14),\
                             (23 11,26 4,29 5))"),
               false);

    // "hole" is completely inside another "hole"
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),\
                            (1 1,1 9,9 9,9 1),(2 2,2 8,8 8,8 2))"),
               false);
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),\
                            (1 1,1 9,9 9,9 1),(2 2,8 2,8 8,2 8))"),
               false);

    // "hole" is inside another "hole" (touching)
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),\
                            (1 1,1 9,9 9,9 1),(2 2,2 8,8 8,9 6,8 2))"),
               false);
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),\
                            (1 1,1 9,9 9,9 1),(2 2,8 2,9 6,8 8,2 8))"),
               false);
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),\
                            (1 1,9 1,9 9,1 9),(2 2,2 8,8 8,9 6,8 2))"),
               false);
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),\
                            (1 1,9 1,9 9,1 9),(2 2,8 2,9 6,8 8,2 8))"),
               false);
    // hole touches exterior ring at two points
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),\
                            (5 0,0 5,5 5))"),
               false);

    // cases with more holes
    // two holes, touching the exterior at the same point
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),\
                            (0 0,1 9,2 9),(0 0,9 2,9 1))"),
               true);
    // two holes, one inside the other
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),\
                            (0 0,1 9,9 1),(0 0,4 5,5 4))"),
               false);
    // 1st hole touches has common segment with 2nd hole
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),\
                            (1 1,1 5,5 5,5 1),(5 4,5 8,8 8,8 4))"),
               false);
    // 1st hole touches 2nd hole at two points
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),\
                            (1 1,1 9,9 9,9 8,2 8,2 1),\
                            (2 5,5 8,5 5))"),
               false);
    // four holes, each two touching at different points
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),\
                            (0 10,2 1,1 1),(0 10,4 1,3 1),\
                            (10 10,9 1,8 1),(10 10,7 1,6 1))"),
               true);
    // five holes, with two pairs touching each at some point, and
    // fifth hole creating a disconnected component for the interior
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),\
                            (0 10,2 1,1 1),(0 10,4 1,3 1),\
                            (10 10,9 1,8 1),(10 10,7 1,6 1),\
                            (4 1,4 4,6 4,6 1))"),
               false);
    // five holes, with two pairs touching each at some point, and
    // fifth hole creating three disconnected components for the interior
    test_valid(from_wkt<OG>("POLYGON((0 0,10 0,10 10,0 10),\
                            (0 10,2 1,1 1),(0 10,4 1,3 1),\
                            (10 10,9 1,8 1),(10 10,7 1,6 1),\
                            (4 1,4 4,6 4,6 1,5 0))"),
               false);
    

    typedef open_cw_polygon_type OG1;

    test_valid(from_wkt<OG1>("POLYGON((0 0,0 10,10 10,10 0),\
                             (1 1,2 1,1 10))"),
               true);
    test_valid(from_wkt<OG1>("POLYGON((0 0,0 10,10 10,10 0),\
                            (0 10,1 1,2 1),(0 10,3 1,4 1),\
                            (10 10,8 1,9 1),(10 10,6 1,7 1))"),
               true);
    test_valid(from_wkt<OG1>("POLYGON((0 0,0 10,10 10,10 0),\
                             (0 10,1 1,2 1),(0 10,3 1,4 1),\
                             (10 10,8 1,9 1),(10 10,6 1,7 1),\
                             (4 1,5 0,6 1,6 4,4 4))"),
               false);


    // valid rings
    //    test_valid(from_wkt<OG>("POLYGON((0 0,1 0,1 1))"), true);
    //    test_valid(from_wkt<OG>("POLYGON((0 0,1 0,1 1,0 1))"), true);
}

BOOST_AUTO_TEST_CASE( test_is_valid_multipolygon )
{
    typedef bg::model::multi_polygon<open_ccw_polygon_type> OG;
    //    typedef closed_ccw_polygon_type CG;

    // open multi-polygons
    // -------------
#ifdef GEOMETRY_TEST_DEBUG
    std::cout << std::endl << std::endl;
    std::cout << "************************************" << std::endl;
    std::cout << " is_valid: MULTIPOLYGON (open) " << std::endl;
    std::cout << "************************************" << std::endl;
#endif

    test_valid(from_wkt<OG>("MULTIPOLYGON((()))"), false);
    test_valid(from_wkt<OG>("MULTIPOLYGON(((0 0)),(()))"), false);
    test_valid(from_wkt<OG>("MULTIPOLYGON(((0 0,1 0)))"), false);
}
