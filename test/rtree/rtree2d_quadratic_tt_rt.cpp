// Boost.Geometry Index
// Unit Test

// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <rtree/test_rtree.hpp>

#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>

int test_main(int, char* [])
{

#ifdef HAVE_TTMATH
    typedef bg::model::point<ttmath_big, 2, bg::cs::cartesian> P2ttmc;

    test_rtree<P2ttmc>(bgi::runtime::quadratic(4, 2));
#endif

    return 0;
}
