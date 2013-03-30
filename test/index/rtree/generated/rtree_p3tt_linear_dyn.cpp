// Boost.Geometry Index
// Unit Test

// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <rtree/test_rtree.hpp>

#include <boost/geometry/geometries/point.hpp>

int test_main(int, char* [])
{
#ifdef HAVE_TTMATH
    typedef bg::model::point<ttmath_big, 3, bg::cs::cartesian> Point;
    test_rtree_for_point<Point>(bgi::dynamic_linear(8, 3));
#endif
    return 0;
}
