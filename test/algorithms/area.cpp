// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2011 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2011 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/math/constants/constants.hpp>

#include <algorithms/test_area.hpp>

#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/ring.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include <test_geometries/all_custom_ring.hpp>
#include <test_geometries/all_custom_polygon.hpp>
//#define GEOMETRY_TEST_DEBUG

template <typename Polygon>
void test_polygon()
{
    // Rotated square, length=sqrt(2) -> area=2
    test_geometry<Polygon>("POLYGON((1 1,2 2,3 1,2 0,1 1))", 2.0);
    test_geometry<Polygon>("POLYGON((1 1,2 2,3 1,2 0,1 1))", 2.0);
    test_geometry<Polygon>("POLYGON((0 0,0 7,4 2,2 0,0 0))", 16.0);
    test_geometry<Polygon>("POLYGON((1 1,2 1,2 2,1 2,1 1))", -1.0);
    test_geometry<Polygon>("POLYGON((0 0,0 7,4 2,2 0,0 0), (1 1,2 1,2 2,1 2,1 1))", 15.0);
}


template <typename P>
void test_all()
{
    test_geometry<bg::model::box<P> >("POLYGON((0 0,2 2))", 4.0);
    test_geometry<bg::model::box<P> >("POLYGON((2 2,0 0))", 4.0);

    test_polygon<bg::model::polygon<P> >();
    test_polygon<all_custom_polygon<P> >();

    // clockwise rings (second is wrongly ordered)
    test_geometry<bg::model::ring<P> >("POLYGON((0 0,0 7,4 2,2 0,0 0))", 16.0);
    test_geometry<bg::model::ring<P> >("POLYGON((0 0,2 0,4 2,0 7,0 0))", -16.0);

    test_geometry<all_custom_ring<P> >("POLYGON((0 0,0 7,4 2,2 0,0 0))", 16.0);

    // ccw
    test_geometry<bg::model::polygon<P, false> >
            ("POLYGON((0 0,0 7,4 2,2 0,0 0), (1 1,2 1,2 2,1 2,1 1))", -15.0);
}

template <typename Point>
void test_spherical()
{
    bg::model::polygon<Point> geometry;

    // unit-sphere has area of 4-PI. Polygon covering 1/8 of it:
    double expected = 4.0 * boost::math::constants::pi<double>() / 8.0;
    bg::read_wkt("POLYGON((0 0,0 90,90 0,0 0))", geometry);

    double area = bg::area(geometry);
    BOOST_CHECK_CLOSE(area, expected, 0.0001);

    // With strategy, radius 2 -> 4 pi r^2
    bg::strategy::area::huiller
        <
            typename bg::point_type<Point>::type
        > strategy(2.0);

    area = bg::area(geometry, strategy);
    BOOST_CHECK_CLOSE(area, 2.0 * 2.0 * expected, 0.0001);
}

template <typename P>
void test_ccw()
{
    typedef bg::model::polygon<P, false> ccw_polygon;
    // counterclockwise rings (second is wrongly ordered)
    test_geometry<ccw_polygon>("POLYGON((1 1,2 2,3 1,2 0,1 1))", -2.0);
    test_geometry<ccw_polygon>("POLYGON((1 1,2 0,3 1,2 2,1 1))", +2.0);
    test_geometry<ccw_polygon>("POLYGON((0 0,0 7,4 2,2 0,0 0))", -16.0);
    test_geometry<ccw_polygon>("POLYGON((0 0,2 0,4 2,0 7,0 0))", +16.0);
}

template <typename P>
void test_open()
{
    typedef bg::model::polygon<P, true, false> open_polygon;
    test_geometry<open_polygon>("POLYGON((1 1,2 2,3 1,2 0))", 2.0);
    // Note the triangular testcase used in CCW is not sensible for open/close
}

template <typename P>
void test_open_ccw()
{
    typedef bg::model::polygon<P, false, false> open_polygon;
    test_geometry<open_polygon>("POLYGON((1 1,2 0,3 1,2 2))", 2.0);
    // Note the triangular testcase used in CCW is not sensible for open/close
}



int test_main(int, char* [])
{
    test_all<bg::model::point<int, 2, bg::cs::cartesian> >();
    test_all<bg::model::point<float, 2, bg::cs::cartesian> >();
    test_all<bg::model::point<double, 2, bg::cs::cartesian> >();

    test_spherical<bg::model::point<double, 2, bg::cs::spherical<bg::degree> > >();

    test_ccw<bg::model::point<double, 2, bg::cs::cartesian> >();
    test_open<bg::model::point<double, 2, bg::cs::cartesian> >();
    test_open_ccw<bg::model::point<double, 2, bg::cs::cartesian> >();

#ifdef HAVE_TTMATH
    test_all<bg::model::d2::point_xy<ttmath_big> >();
#endif

    return 0;
}
