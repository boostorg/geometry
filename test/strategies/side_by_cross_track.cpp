// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <geometry_test_common.hpp>

#include <boost/geometry/algorithms/assign.hpp>

#include <boost/geometry/strategies/spherical/side_by_cross_track.hpp>

#include <boost/geometry/core/cs.hpp>

#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/segment.hpp>


template <typename Point>
void test_side(double lon1, double lat1,
                   double lon2, double lat2,
                   double lon3, double lat3,
                   int expected)
{
    typedef bg::strategy::side::side_by_cross_track<double> strategy;

    Point p1, p2, p3;
    bg::assign_values(p1, lon1, lat1);
    bg::assign_values(p2, lon2, lat2);
    bg::assign_values(p3, lon3, lat3);
    int s = strategy::apply(p1, p2, p3);

}

template <typename Point>
void test_all()
{
    test_side<Point>(2.0, 48.0, 4.0, 52.0, 2.0, 41.0, 1);
    test_side<Point>(2.0, 48.0, 2.0, 41.0, 4.0, 52.0, -1);
}

int test_main(int, char* [])
{
    test_all<bg::model::point<double, 2, bg::cs::spherical<bg::degree> > >();

    double a = 0;
    double b = sin(a);

    return 0;
}
