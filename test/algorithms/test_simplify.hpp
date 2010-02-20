// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_SIMPLIFY_HPP
#define BOOST_GEOMETRY_TEST_SIMPLIFY_HPP

// Test-functionality, shared between single and multi tests

#include <ggl_test_common.hpp>
#include <boost/geometry/algorithms/simplify.hpp>
#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/extensions/gis/io/wkt/wkt.hpp>

template <typename Tag, typename Geometry>
struct test_inserter
{
    static void apply(Geometry& , std::string const& , double )
    {}
};

template <typename Geometry>
struct test_inserter<boost::geometry::linestring_tag, Geometry>
{
    static void apply(Geometry& geometry, std::string const& expected, double distance)
    {
        Geometry simplified;
        boost::geometry::simplify_inserter(geometry,
            std::back_inserter(simplified), distance);

        std::ostringstream out;
        out << boost::geometry::wkt(simplified);
        BOOST_CHECK_EQUAL(out.str(), expected);
    }
};


template <typename Geometry>
void test_geometry(std::string const& wkt, std::string const& expected, double distance)
{
    Geometry geometry, simplified;

    // Generate polygon using only integer coordinates and obvious results
    // Polygon is a hexagon, having one extra point (2,1) on a line which should be filtered out.
    boost::geometry::read_wkt(wkt, geometry);
    boost::geometry::simplify(geometry, simplified, distance);

    {
        std::ostringstream out;
        out << boost::geometry::wkt(simplified);

        BOOST_CHECK_MESSAGE(out.str() == expected,
            "simplify: " << boost::geometry::wkt(geometry)
            << " expected " << expected
            << " got " << boost::geometry::wkt(simplified));
    }

    // Check using user-specified strategy
    typedef typename boost::geometry::point_type<Geometry>::type point_type;
    typedef typename boost::geometry::cs_tag<point_type>::type tag;
    typedef boost::geometry::strategy::distance::projected_point
        <
            point_type,
            point_type
        > strategy;
    typedef boost::geometry::strategy::simplify::douglas_peucker
        <
            point_type,
            strategy
        > simplify_strategy_type;

    BOOST_CONCEPT_ASSERT( (boost::geometry::concept::SimplifyStrategy<simplify_strategy_type>) );
    boost::geometry::simplify(geometry, simplified, distance, simplify_strategy_type());

    {
        std::ostringstream out;
        out << boost::geometry::wkt(simplified);
        BOOST_CHECK_EQUAL(out.str(), expected);
    }

    // Check inserter (if applicable)
    test_inserter
        <
            typename boost::geometry::tag<Geometry>::type,
            Geometry
        >::apply(geometry, expected, distance);
}


#endif
