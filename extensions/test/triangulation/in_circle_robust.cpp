// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2019 program.

// This file was modified by Oracle on 2021.
// Modifications copyright (c) 2021, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <geometry_test_common.hpp>

#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/strategy/cartesian/in_circle_robust.hpp>

template <typename P>
void test_all()
{
    typedef bg::strategy::in_circle::in_circle_robust<double, 2> inc2;
    typedef bg::strategy::in_circle::in_circle_robust<double, 1> inc1;
    typedef bg::strategy::in_circle::in_circle_robust<double, 0> inc0;

    P col1(0.0, 0.0), col2(1.0, 0.0), col3(0.0, 1.0);
    P in(0.5,0.5) , on(1.0, 0.0), out(-0.5, -0.5);
    int in2 = inc2::apply(col1, col2, col3, in);
    BOOST_CHECK_GT(in2, 0);
    int in1 = inc1::apply(col1, col2, col3, in);
    BOOST_CHECK_GT(in1, 0);
    int in0 = inc0::apply(col1, col2, col3, in);
    BOOST_CHECK_GT(in0, 0);

    int on2 = inc2::apply(col1, col2, col3, on);
    BOOST_CHECK_EQUAL(on2, 0);
    int on1 = inc1::apply(col1, col2, col3, on);
    BOOST_CHECK_EQUAL(on1, 0);
    int on0 = inc0::apply(col1, col2, col3, on);
    BOOST_CHECK_EQUAL(on0, 0);

    int out2 = inc2::apply(col1, col2, col3, out);
    BOOST_CHECK_GT(0, out2);
    int out1 = inc1::apply(col1, col2, col3, out);
    BOOST_CHECK_GT(0, out1);
    int out0 = inc0::apply(col1, col2, col3, out);
    BOOST_CHECK_GT(0, out0);

    P hard1(0, 0), hard2(1e20, 0), hard3(0, 1e20);
    P inhard(0.5, 0.5);
    int hardr  = inc2::apply(hard1, hard2, hard3, inhard);
    BOOST_CHECK_GT(hardr, 0);
}


int test_main(int, char* [])
{
    test_all<bg::model::d2::point_xy<double> >();
    return 0;
}
