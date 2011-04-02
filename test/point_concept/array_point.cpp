// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2011 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2011 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <test_common/test_point.hpp>

#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include "function_requiring_a_point.hpp"

int main()
{
    float p1[3] = { 0, 0, 0 };
    const float p2[3] = { 0, 0, 0 };
    test::function_requiring_a_point(p1, p2);
    return 0;
}
