// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels, Geodan B.V. Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <ggl_test_common.hpp>

#include <boost/geometry/extensions/nsphere/nsphere.hpp>

#include <boost/geometry/geometries/geometries.hpp>

#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>


template <typename Geometry>
void test_circle(std::string const& wkt_geometry, bool expected)
{
    boost::geometry::circle circle;
    boost::geometry::assign(circle, 1.0, 1.0, 3.0);

    Geometry geometry;
    boost::geometry::read_wkt(wkt_geometry, geometry);

    bool detected = boost::geometry::within(geometry, circle);

    BOOST_CHECK_MESSAGE(detected == expected,
        "within: " << wkt_geometry
        << " in circle (1,1) with radius 3"
        << " -> Expected: " << expected
        << " detected: " << detected);
}




template <typename P>
void test_circles()
{
    // Circles do not yet run on CLN/GMP
    test_circle<P>("POINT(2 1)", true);
    test_circle<P>("POINT(12 1)", false);

    test_circle<boost::geometry::linestring<P> >("LINESTRING(1 1,2 1,2 2)", true);
    test_circle<boost::geometry::linestring<P> >("LINESTRING(1 1,2 1,2 2,10 10)", false);
}


int test_main( int , char* [] )
{
    test_circles<boost::geometry::point_xy<double> >();

    return 0;
}
