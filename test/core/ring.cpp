// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <ggl_test_common.hpp>


// To be tested:
#include <boost/geometry/core/ring_type.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>

// For geometries:
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/polygon.hpp>


#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>




template <typename P>
void test_ring(std::string const& wkt,
    int expected_main_count,
    int expected_interior_ring_count,
    int expected_first_interior_count)
{
    typedef boost::geometry::polygon<P> the_polygon;
    typedef typename boost::geometry::ring_type<the_polygon>::type the_ring;
    typedef typename boost::geometry::interior_type<the_polygon>::type the_interior;

    the_polygon poly;
    read_wkt(wkt, poly);

    the_ring ext = boost::geometry::exterior_ring(poly);
    the_interior rings = boost::geometry::interior_rings(poly);

    BOOST_CHECK_EQUAL(boost::geometry::num_interior_rings(poly), expected_interior_ring_count);
    BOOST_CHECK_EQUAL(boost::size(rings), expected_interior_ring_count);
    BOOST_CHECK_EQUAL(boost::size(ext), expected_main_count);
    if (boost::size(rings) > 0)
    {
        BOOST_CHECK_EQUAL(boost::size(rings.front()), expected_first_interior_count);
    }
}


template <typename P>
void test_all()
{
    test_ring<P>("POLYGON((0 0,0 3,3 3,3 0,0 0),(1 1,1 2,2 2,2 1,1 1))", 5, 1, 5);
    test_ring<P>("POLYGON((0 0,0 3,3 3,3 0,0 0),(1 1,2 2,2 1,1 1),(1 1,1 2,2 2,1 1))", 5, 2, 4);
    test_ring<P>("POLYGON((0 0,0 3,3 3,3 0,0 0))", 5, 0, 0);
}


int test_main(int, char* [])
{
    test_all<boost::geometry::point<double, 2, boost::geometry::cs::cartesian> >();
    return 0;
}
