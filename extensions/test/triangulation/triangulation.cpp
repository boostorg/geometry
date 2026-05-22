// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2019 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <geometry_test_common.hpp>

#include <vector>

#include <boost/geometry.hpp>
#include <boost/geometry/extensions/triangulation/triangulation.hpp>

namespace bg = boost::geometry;

template <typename P>
void test_all()
{
    typedef bg::model::triangulation<P, false> triangulation;
    std::vector<P> in;
    in.push_back(P(-1, -2));
    in.push_back(P(1, -2));
    in.push_back(P(-2, 0));
    in.push_back(P(2, 0));
    in.push_back(P(1, 2));
    in.push_back(P(-1, 2));
    in.push_back(P(0, 0));
    in.push_back(P(0, 1));
    triangulation t(8);
    bg::delaunay_triangulation(in, t);
    BOOST_CHECK( t.valid() );
}


int test_main(int, char* [])
{
    test_all<bg::model::point<double, 2, bg::cs::cartesian> >();
    return 0;
}
