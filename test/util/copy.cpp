// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <geometry_test_common.hpp>

#include <boost/geometry/util/copy.hpp>

#include <boost/geometry/algorithms/assign.hpp>

#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>
#include <test_common/test_point.hpp>


template <typename P>
void test_all()
{
    P p1;
    boost::geometry::assign(p1, 1, 22, 333);
    P p2;
    boost::geometry::copy_coordinates(p1, p2);
    BOOST_CHECK(boost::geometry::get<0>(p2) == 1);
    BOOST_CHECK(boost::geometry::get<1>(p2) == 22);
    BOOST_CHECK(boost::geometry::get<2>(p2) == 333);
}

int test_main(int, char* [])
{
    test_all<int[3]>();
    test_all<float[3]>();
    test_all<double[3]>();
    test_all<test::test_point>();
    test_all<boost::geometry::point<int, 3, boost::geometry::cs::cartesian> >();
    test_all<boost::geometry::point<float, 3, boost::geometry::cs::cartesian> >();
    test_all<boost::geometry::point<double, 3, boost::geometry::cs::cartesian> >();

    return 0;
}
