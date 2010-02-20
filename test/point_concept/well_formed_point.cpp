// Boost.Geometry (aka GGL, Generic Geometry Library) Point concept test file
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/tuple/tuple.hpp>

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/geometries/register/point.hpp>

#include "function_requiring_a_point.hpp"

struct point: public boost::tuple<float, float>
{
};

BOOST_GEOMETRY_REGISTER_POINT_2D(point, float, cs::cartesian, get<0>(), get<1>())

int main()
{
    point p1;
    const point p2;
    test::function_requiring_a_point(p1, p2);
    return 0;
}
