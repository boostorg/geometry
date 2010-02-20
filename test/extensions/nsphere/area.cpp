// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <geometry_test_common.hpp>

#include <boost/geometry/extensions/nsphere/algorithms/area.hpp>
#include <boost/geometry/extensions/nsphere/geometries/nsphere.hpp>


template <typename P, typename T>
void test_area_circle()
{
    boost::geometry::nsphere<P, T> c;

    boost::geometry::set<0>(c.center(), 0);
    boost::geometry::set<1>(c.center(), 0);
    c.radius(2);

    double d = boost::geometry::area(c);
    BOOST_CHECK_CLOSE(d, 4 * 3.1415926535897932384626433832795, 0.001);
}



int test_main(int, char* [])
{
    test_area_circle<boost::geometry::point<double, 2, boost::geometry::cs::cartesian>, double>();
    return 0;
}
