// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <algorithms/test_centroid.hpp>

#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>


template <typename P>
void test_2d()
{
    test_centroid<bg::model::linestring<P> >("LINESTRING(1 1, 2 2, 3 3)", 2.0, 2.0);
    test_centroid<bg::model::linestring<P> >("LINESTRING(0 0,0 4, 4 4)", 1.0, 3.0);
    test_centroid<bg::model::linestring<P> >("LINESTRING(0 0,3 3,0 6,3 9,0 12)", 1.5, 6.0);

    test_centroid<bg::model::linear_ring<P> >(
        "POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2"
        ",3.7 1.6,3.4 2,4.1 3,5.3 2.6,5.4 1.2,4.9 0.8,2.9 0.7,2 1.3))",
        4.06923363095238, 1.65055803571429);
    test_centroid<bg::model::polygon<P> >(
        "POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2"
        ",3.7 1.6,3.4 2,4.1 3,5.3 2.6,5.4 1.2,4.9 0.8,2.9 0.7,2 1.3))",
        4.06923363095238, 1.65055803571429);

    // with holes
    test_centroid<bg::model::polygon<P> >(
        "POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2"
        ",3.7 1.6,3.4 2,4.1 3,5.3 2.6,5.4 1.2,4.9 0.8,2.9 0.7,2 1.3)"
        ",(4 2,4.2 1.4,4.8 1.9,4.4 2.2,4 2))"
        ,
        4.0466264962959677, 1.6348996057331333);


    // ccw
    test_centroid<bg::model::linear_ring<P, false> >(
        "POLYGON((2 1.3,2.9 0.7,4.9 0.8,5.4 1.2,5.3 2.6,4.1 3,3.4 2"
            ",3.7 1.6,3.4 1.2,2.8 1.8,2.4 1.7,2 1.3))",
        4.06923363095238, 1.65055803571429);

    // open / closed
    test_centroid<bg::model::linear_ring<P, true, true> >(
            "POLYGON((1 1,2 2,3 1,2 0,1 1))", 2.0, 1.0);
    test_centroid<bg::model::linear_ring<P, true, false> >(
            "POLYGON((1 1,2 2,3 1,2 0))", 2.0, 1.0);

    test_centroid<bg::model::box<P> >("POLYGON((1 2,3 4))", 2, 3);
    test_centroid<P>("POINT(3 3)", 3, 3);
}


template <typename P>
void test_3d()
{
    test_centroid<bg::model::box<P> >("POLYGON((1 2 3,5 6 7))", 3, 4, 5);
    test_centroid<P>("POINT(1 2 3)", 1, 2, 3);
}


int test_main(int, char* [])
{
    test_2d<bg::model::d2::point_xy<double> >();
    test_2d<boost::tuple<float, float> >();
    test_2d<bg::model::d2::point_xy<float> >();

    test_3d<boost::tuple<double, double, double> >();

#if defined(HAVE_TTMATH)
    test_2d<bg::model::d2::point_xy<ttmath_big> >();
#endif

    return 0;
}
