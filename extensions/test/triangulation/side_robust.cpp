// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2019 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <geometry_test_common.hpp>

#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/extensions/triangulation/strategies/cartesian/side_robust.hpp>

template <typename P>
void test_all()
{
    typedef bg::strategy::side::side_robust<double, 3> side3;
    typedef bg::strategy::side::side_robust<double, 2> side2;
    typedef bg::strategy::side::side_robust<double, 1> side1;
    typedef bg::strategy::side::side_robust<double, 0> side0;

    P col1(1.0, 1.0), col2(2.0, 2.0), col3(3.0, 3.0);
    int col3r = side3::apply(col1, col2, col3);
    BOOST_CHECK_EQUAL(0, col3r);
    int col2r = side2::apply(col1, col2, col3);
    BOOST_CHECK_EQUAL(0, col2r);
    int col1r = side1::apply(col1, col2, col3);
    BOOST_CHECK_EQUAL(0, col1r);
    int col0r = side0::apply(col1, col2, col3);
    BOOST_CHECK_EQUAL(0, col0r);

    P easy1(0.0, 0.0), easy2(1.0, 1.0), easy3(0.0, 1.0);
    int easy3r = side3::apply(easy1, easy2, easy3);
    BOOST_CHECK_GT(easy3r, 0);
    int easy2r = side2::apply(easy1, easy2, easy3);
    BOOST_CHECK_GT(easy2r, 0);
    int easy1r = side1::apply(easy1, easy2, easy3);
    BOOST_CHECK_GT(easy1r, 0);
    int easy0r = side0::apply(easy1, easy2, easy3);
    BOOST_CHECK_GT(easy0r, 0);

    P medium1(1.0, 1.0), medium2(1.0e20, 1.0e20), medium3(1.0, 2.0);
    int medium3r = side3::apply(medium1, medium2, medium3);
    BOOST_CHECK_GT(medium3r, 0);
    int medium2r = side2::apply(medium1, medium2, medium3);
    BOOST_CHECK_GT(medium2r, 0);

    P hard1(1.0e-20, 1.0e-20), hard2(1.0e20, 1.0e20), hard3(1.0, 2.0);
    int hard3r = side3::apply(hard1, hard2, hard3);
    BOOST_CHECK_GT(hard3r, 0);
}


int test_main(int, char* [])
{
    test_all<bg::model::d2::point_xy<double> >();
    return 0;
}
