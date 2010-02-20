// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_WITHIN_HPP
#define BOOST_GEOMETRY_TEST_WITHIN_HPP


#include <geometry_test_common.hpp>

#include <boost/geometry/core/ring_type.hpp>
#include <boost/geometry/algorithms/within.hpp>
#include <boost/geometry/strategies/strategies.hpp>
#include <boost/geometry/geometries/linear_ring.hpp>
#include <boost/geometry/geometries/polygon.hpp>

#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>


template <typename Geometry1, typename Geometry2>
void test_geometry(std::string const& wkt1,
        std::string const& wkt2, bool expected)
{
    Geometry1 geometry1;
    Geometry2 geometry2;

    boost::geometry::read_wkt(wkt1, geometry1);
    boost::geometry::read_wkt(wkt2, geometry2);

    bool detected = boost::geometry::within(geometry1, geometry2);

    BOOST_CHECK_MESSAGE(detected == expected,
        "within: " << wkt1
        << " in " << wkt2
        << " -> Expected: " << expected
        << " detected: " << detected);
}


template <typename Point, bool CW>
void test_ordered_ring(std::string const& wkt_point,
        std::string const& wkt_geometry, bool expected, bool on_border)
{
    typedef boost::geometry::linear_ring<Point, std::vector, CW> ring_type;
    ring_type ring;
    Point point;

    boost::geometry::read_wkt(wkt_geometry, ring);
    if (! CW)
    {
        std::reverse(boost::begin(ring), boost::end(ring));
    }
    boost::geometry::read_wkt(wkt_point, point);

    bool detected = boost::geometry::within(point, ring);

    BOOST_CHECK_MESSAGE(detected == expected,
        "within: " << wkt_point
        << " in " << wkt_geometry
        << " -> Expected: " << expected
        << " detected: " << detected);

    // other strategy (note that this one cannot detect OnBorder
    // (without modifications)

    boost::geometry::strategy::within::franklin<Point> franklin;
    detected = boost::geometry::within(point, ring, franklin);
    if (! on_border)
    {
        BOOST_CHECK_MESSAGE(detected == expected,
            "within: " << wkt_point
            << " in " << wkt_geometry
            << " -> Expected: " << expected
            << " detected: " << detected);
    }


    boost::geometry::strategy::within::crossings_multiply<Point> cm;
    detected = boost::geometry::within(point, ring, cm);
    if (! on_border)
    {
        BOOST_CHECK_MESSAGE(detected == expected,
            "within: " << wkt_point
            << " in " << wkt_geometry
            << " -> Expected: " << expected
            << " detected: " << detected);
    }
}

template <typename Point>
void test_ring(std::string const& wkt_point,
        std::string const& wkt_geometry,
        bool expected, bool on_border)
{
    test_ordered_ring<Point, true>(wkt_point, wkt_geometry, expected, on_border);
    test_ordered_ring<Point, false>(wkt_point, wkt_geometry, expected, on_border);
    test_geometry<Point, boost::geometry::polygon<Point> >(wkt_point, wkt_geometry, expected);
}

#endif
