// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/geometry/geometries/geometries.hpp>

#include <algorithms/test_perimeter.hpp>


template <typename P>
void test_all()
{
    // 3-4-5 triangle
    //test_geometry<std::pair<P, P> >("LINESTRING(0 0,3 4)", 5);

    test_geometry<bg::model::ring<P> >(
            "POLYGON((0 0,0 1,1 1,1 0,0 0))", 4);
    test_geometry<bg::model::polygon<P> >(
            "POLYGON((0 0,0 1,1 0,0 0))", 1.0 + 1.0 + sqrt(2.0));
    test_geometry<bg::model::polygon<P> >(
            "POLYGON((0 0,0 4,4 4,4 0,0 0),(1 1,2 1,2 2,1 2,1 1))", 20);
}

template <typename P>
void test_open()
{
    typedef bg::model::polygon<P, true, false> open_polygon;
    test_geometry<open_polygon>("POLYGON((0 0,0 1,1 1,1 0))", 4);
}


int test_main(int, char* [])
{
    //test_all<bg::model::d2::point_xy<int> >();
    test_all<bg::model::d2::point_xy<float> >();
    test_all<bg::model::d2::point_xy<double> >();

    test_open<bg::model::d2::point_xy<double> >();

#if defined(HAVE_TTMATH)
    test_all<bg::model::d2::point_xy<ttmath_big> >();
#endif

    return 0;
}
