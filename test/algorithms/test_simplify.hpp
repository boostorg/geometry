// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_SIMPLIFY_HPP
#define BOOST_GEOMETRY_TEST_SIMPLIFY_HPP

// Test-functionality, shared between single and multi tests

#include <iomanip>
#include <sstream>
#include <geometry_test_common.hpp>
#include <boost/geometry/algorithms/simplify.hpp>
#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/strategies/strategies.hpp>
#include <boost/geometry/io/wkt/wkt.hpp>
#include <boost/variant/variant.hpp>

template <typename Tag, typename Geometry>
struct test_inserter
{
    static void apply(Geometry& , std::string const& , double )
    {}
};

template <typename Geometry>
struct test_inserter<bg::linestring_tag, Geometry>
{
    template <typename DistanceMeasure>
    static void apply(Geometry& geometry,
            std::string const& expected,
            DistanceMeasure const& distance)
    {
        Geometry simplified;
        bg::detail::simplify::simplify_insert(geometry,
            std::back_inserter(simplified), distance);

        std::ostringstream out;
        // TODO: instead of comparing the full string (with more or less decimal digits),
        // we should call something more robust to check the test for example geometry::equals
        out << std::setprecision(12) << bg::wkt(simplified);
        BOOST_CHECK_EQUAL(out.str(), expected);
    }
};


template <typename Geometry, typename DistanceMeasure>
void check_geometry(Geometry const& geometry,
                    std::string const& expected,
                    DistanceMeasure const&  distance)
{
    Geometry simplified;
    bg::simplify(geometry, simplified, distance);

    std::ostringstream out;
    out << std::setprecision(12) << bg::wkt(simplified);
    BOOST_CHECK_EQUAL(out.str(), expected);
}

template <typename Geometry, typename Strategy, typename DistanceMeasure>
void check_geometry(Geometry const& geometry,
                    std::string const& expected,
                    DistanceMeasure const& distance,
                    Strategy const& strategy)
{
    Geometry simplified;
    bg::simplify(geometry, simplified, distance, strategy);

    std::ostringstream out;
    out << std::setprecision(12) << bg::wkt(simplified);
    BOOST_CHECK_EQUAL(out.str(), expected);
}

template <typename Geometry, typename Strategy, typename DistanceMeasure>
void test_geometry(std::string const& wkt,
        std::string const& expected,
        DistanceMeasure const& distance,
        Strategy const& strategy)
{
    typedef typename bg::point_type<Geometry>::type point_type;

    Geometry geometry;
    bg::read_wkt(wkt, geometry);
    boost::variant<Geometry> v(geometry);

    check_geometry(geometry, expected, distance);
    check_geometry(v, expected, distance);


    //BOOST_CONCEPT_ASSERT( (bg::concept::SimplifyStrategy<Strategy, point_type>) );

    check_geometry(geometry, expected, distance, strategy);
    check_geometry(v, expected, distance, strategy);

    // Check inserter (if applicable)
    test_inserter
        <
            typename bg::tag<Geometry>::type,
            Geometry
        >::apply(geometry, expected, distance);
}

template <typename Geometry, typename DistanceMeasure>
void test_geometry(std::string const& wkt,
        std::string const& expected,
        DistanceMeasure distance)
{
    // Define default strategy for testing
    typedef bg::strategy::simplify::douglas_peucker
        <
            typename bg::point_type<Geometry>::type,
            bg::strategy::distance::projected_point<double>
        > dp;

    test_geometry<Geometry>(wkt, expected, distance, dp());
}


#endif
