// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <geometry_test_common.hpp>

#include <boost/geometry/algorithms/assign.hpp>

#include <boost/geometry/strategies/spherical/distance_haversine.hpp>
#include <boost/geometry/strategies/spherical/distance_cross_track.hpp>

#include <boost/geometry/strategies/concepts/distance_concept.hpp>

#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/segment.hpp>


template <typename Point>
void test_distance(double lon1, double lat1,
                   double lon2, double lat2,
                   double lon3, double lat3,
                   double radius, double expected, double tolerance)
{
    typedef boost::geometry::strategy::distance::cross_track
        <
            Point,
            Point
        > strategy_type;


    BOOST_CONCEPT_ASSERT
        (
            (boost::geometry::concept::PointSegmentDistanceStrategy<strategy_type>)
        );


    strategy_type strategy(radius);

    Point p1, p2, p3;
    boost::geometry::assign(p1, lon1, lat1);
    boost::geometry::assign(p2, lon2, lat2);
    boost::geometry::assign(p3, lon3, lat3);
    typename strategy_type::return_type d = strategy.apply(p1, p2, p3);

    BOOST_CHECK_CLOSE((double) d, expected, tolerance);
}

template <typename Point>
void test_all()
{
    double const average_earth_radius = 6372795.0;

    // distance (Paris <-> Amsterdam/Barcelona), ~87 km
    // should be is equal
    // to distance (Paris <-> Barcelona/Amsterdam)
    double const p_to_ab = 86.8 * 1000.0;
    test_distance<Point>(2, 48, 4, 52, 2, 41, average_earth_radius, p_to_ab, 1.0);
    test_distance<Point>(2, 48, 2, 41, 4, 52, average_earth_radius, p_to_ab, 1.0);
}

int test_main(int, char* [])
{
    test_all<boost::geometry::point<double, 2, boost::geometry::cs::spherical<boost::geometry::degree> > >();

    return 0;
}
