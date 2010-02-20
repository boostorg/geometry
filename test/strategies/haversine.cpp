// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <ggl_test_common.hpp>

#include <boost/concept/requires.hpp>
#include <boost/concept_check.hpp>

#include <boost/geometry/algorithms/assign.hpp>
#include <boost/geometry/strategies/spherical/distance_haversine.hpp>
#include <boost/geometry/strategies/concepts/distance_concept.hpp>


#include <boost/geometry/geometries/point.hpp>


template <typename Point>
struct test_distance
{
    typedef boost::geometry::strategy::distance::haversine
        <
            Point,
            Point
        > haversine_type;


    BOOST_CONCEPT_ASSERT
        (
            (boost::geometry::concept::PointDistanceStrategy<haversine_type>)
        );


    static void test(double lon1, double lat1, double lon2, double lat2,
                       double radius, double expected, double tolerance)
    {
        haversine_type strategy(radius);

        Point p1, p2;
        boost::geometry::assign(p1, lon1, lat1);
        boost::geometry::assign(p2, lon2, lat2);
        typename haversine_type::return_type d1 = strategy.apply(p1, p2);

        BOOST_CHECK_CLOSE((double) d1, expected, tolerance);
    }
};

template <typename Point>
void test_all()
{
    double const average_earth_radius = 6372795.0;

    // earth to unit-sphere -> divide by earth circumference, then it is from 0-1,
    // then multiply with 2 PI, so effectively just divide by earth radius
    double e2u = 1.0 / average_earth_radius;

    // ~ Amsterdam/Paris
    double const a_p = 467.2704 * 1000.0;
    test_distance<Point>::test(4, 52, 2, 48, average_earth_radius, a_p, 1.0);
    test_distance<Point>::test(2, 48, 4, 52, average_earth_radius, a_p, 1.0);
    test_distance<Point>::test(4, 52, 2, 48, 1.0, a_p * e2u, 0.001);

    // ~ Amsterdam/Barcelona
    double const a_b = 1232.9065 * 1000.0;
    test_distance<Point>::test(4, 52, 2, 41, average_earth_radius, a_b, 1.0);
    test_distance<Point>::test(2, 41, 4, 52, average_earth_radius, a_b, 1.0);
    test_distance<Point>::test(4, 52, 2, 41, 1.0, a_b * e2u, 0.001);
}


int test_main(int, char* [])
{
    test_all<boost::geometry::point<double, 2, boost::geometry::cs::spherical<boost::geometry::degree> > >();

    return 0;
}
