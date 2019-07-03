// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2018-2019 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <geometry_test_common.hpp>

#include <boost/geometry/arithmetic/general_form.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/io/wkt/wkt.hpp>

namespace
{
    // Boost.Test does not support BOOST_CHECK_CLOSE for integral types
    template <typename T>
    bool is_small(T const& value)
    {
        static long double const epsilon = 1.0e-5;
        return std::fabs(value) < epsilon;
    }
}

template <typename T, typename C>
void verify_point_on_line(bg::arithmetic::general_form<T> const& f,
                          C const& x, C const& y)
{
    BOOST_CHECK_MESSAGE(is_small(f.a * x + f.b * y + f.c),
                        "Point is not located on the line");
}

template <typename T>
void test_construct_line()
{
    typedef bg::arithmetic::general_form<T> gf;

    // Horizontal through origin
    gf f = bg::arithmetic::construct_line<T>(0, 0, 10, 0);
    verify_point_on_line(f, 0, 0);
    verify_point_on_line(f, 10, 0);

    // Horizontal line above origin
    f = bg::arithmetic::construct_line<T>(0, 5, 10, 5);
    verify_point_on_line(f, 0, 5);
    verify_point_on_line(f, 10, 5);

    // Vertical through origin
    f = bg::arithmetic::construct_line<T>(0, 0, 0, 10);
    verify_point_on_line(f, 0, 0);
    verify_point_on_line(f, 0, 10);

    // Vertical line left from origin
    f = bg::arithmetic::construct_line<T>(5, 0, 5, 10);
    verify_point_on_line(f, 5, 0);
    verify_point_on_line(f, 5, 10);

    // Diagonal through origin
    f = bg::arithmetic::construct_line<T>(0, 0, 8, 10);
    verify_point_on_line(f, 0, 0);
    verify_point_on_line(f, 8, 10);

    // Diagonal not through origin
    f = bg::arithmetic::construct_line<T>(5, 2, -8, 10);
    verify_point_on_line(f, 5, 2);
    verify_point_on_line(f, -8, 10);
}

template <typename T>
void test_side_value()
{
    typedef bg::arithmetic::general_form<T> gf;

    // Horizontal line going right
    gf f = bg::arithmetic::construct_line<T>(0, 0, 10, 0);

    // Point above (= on left side)
    T d = bg::arithmetic::side_value(f, 5, 5);
    BOOST_CHECK_MESSAGE(d > 0, "point not on left side");

    // Point below (= on right side)
    d = bg::arithmetic::side_value(f, 5, -5);
    BOOST_CHECK_MESSAGE(d < 0, "point not on right side");

    // Diagonal not through origin, from right (down) to left (up)
    f = bg::arithmetic::construct_line<T>(5, 2, -7, 10);
    d = bg::arithmetic::side_value(f, 5, 2);
    BOOST_CHECK_MESSAGE(is_small(d), "point not on line");
    d = bg::arithmetic::side_value(f, -7, 10);
    BOOST_CHECK_MESSAGE(is_small(d), "point not on line");

    // vector is (-12, 8), move (-3,2) on the line from (5,2)
    d = bg::arithmetic::side_value(f, 2, 4);
    BOOST_CHECK_MESSAGE(is_small(d), "point not on line");

    // Go perpendicular (2,3) from (2,4) up, so right of the line (negative)
    d = bg::arithmetic::side_value(f, 4, 7);
    BOOST_CHECK_MESSAGE(d < 0, "point not on right side");

    // Go perpendicular (2,3) from (2,4) down, so left of the line (positive)
    d = bg::arithmetic::side_value(f, 0, 1);
    BOOST_CHECK_MESSAGE(d > 0, "point not on left side");
}


template <typename T>
void test_get_intersection()
{
    typedef bg::arithmetic::general_form<T> gf;

    // Diagonal lines (first is same as in distance measure,
    // second is perpendicular and used there for distance measures)
    gf p = bg::arithmetic::construct_line<T>(5, 2, -7, 10);
    gf q = bg::arithmetic::construct_line<T>(4, 7, 0, 1);

    typedef bg::model::point<T, 2, bg::cs::cartesian> point_type;
    point_type ip;
    BOOST_CHECK(bg::arithmetic::get_intersection(ip, p, q));

    BOOST_CHECK_MESSAGE(is_small(bg::get<0>(ip) - 2), "x-coordinate wrong");
    BOOST_CHECK_MESSAGE(is_small(bg::get<1>(ip) - 4), "y-coordinate wrong");

    verify_point_on_line(p, bg::get<0>(ip), bg::get<1>(ip));
    verify_point_on_line(q, bg::get<0>(ip), bg::get<1>(ip));
}

template <typename T>
void test_same_direction()
{
    bg::arithmetic::general_form<T> p, q;

    // Exactly opposite, diagonal
    p = bg::arithmetic::construct_line<T>(2, 1, 12, 11);
    q = bg::arithmetic::construct_line<T>(12, 11, 2, 1);
    BOOST_CHECK(! bg::arithmetic::similar_direction(p, q));

    // Exactly opposite, horizontal
    p = bg::arithmetic::construct_line<T>(0, 0, 10, 0);
    q = bg::arithmetic::construct_line<T>(10, 0, 0, 0);
    BOOST_CHECK(! bg::arithmetic::similar_direction(p, q));

    // Exactly opposite, vertical
    p = bg::arithmetic::construct_line<T>(0, 0, 0, 10);
    q = bg::arithmetic::construct_line<T>(0, 10, 0, 0);
    BOOST_CHECK(! bg::arithmetic::similar_direction(p, q));

    // Exactly equal, diagonal
    p = bg::arithmetic::construct_line<T>(0, 0, 10, 10);
    q = bg::arithmetic::construct_line<T>(0, 0, 10, 10);
    BOOST_CHECK(bg::arithmetic::similar_direction(p, q));

    // Exactly equal, horizontal
    p = bg::arithmetic::construct_line<T>(0, 0, 10, 0);
    q = bg::arithmetic::construct_line<T>(0, 0, 10, 0);
    BOOST_CHECK(bg::arithmetic::similar_direction(p, q));

    // Exactly equal, vertical
    p = bg::arithmetic::construct_line<T>(0, 0, 0, 10);
    q = bg::arithmetic::construct_line<T>(0, 0, 0, 10);
    BOOST_CHECK(bg::arithmetic::similar_direction(p, q));

    // Coming together, diagonal
    p = bg::arithmetic::construct_line<T>(0, 0, 10, 10);
    q = bg::arithmetic::construct_line<T>(20, 20, 10, 10);
    BOOST_CHECK(! bg::arithmetic::similar_direction(p, q));

    // Leaving from common point, diagonal
    p = bg::arithmetic::construct_line<T>(10, 10, 0, 0);
    q = bg::arithmetic::construct_line<T>(0, 0, 10, 10);
    BOOST_CHECK(! bg::arithmetic::similar_direction(p, q));

    // Continuing each other, diagonal
    p = bg::arithmetic::construct_line<T>(0, 0, 10, 10);
    q = bg::arithmetic::construct_line<T>(10, 10, 20, 20);
    BOOST_CHECK(bg::arithmetic::similar_direction(p, q));

    // (Nearly) perpendicular
    p = bg::arithmetic::construct_line<T>(0, 0, 10, 10);
    q = bg::arithmetic::construct_line<T>(0, 0, -10, 10);
    BOOST_CHECK(! bg::arithmetic::similar_direction(p, q));

    // 45 deg
    p = bg::arithmetic::construct_line<T>(0, 0, 10, 10);
    q = bg::arithmetic::construct_line<T>(0, 0, 0, 10);
    BOOST_CHECK(bg::arithmetic::similar_direction(p, q));

    // a bit more than 45 deg
    p = bg::arithmetic::construct_line<T>(0, 0, 10, 10);
    q = bg::arithmetic::construct_line<T>(0, 0, -1, 10);
    BOOST_CHECK(! bg::arithmetic::similar_direction(p, q));

    // 135 deg
    p = bg::arithmetic::construct_line<T>(0, 0, 10, 10);
    q = bg::arithmetic::construct_line<T>(0, 0, -10, 0);
    BOOST_CHECK(! bg::arithmetic::similar_direction(p, q));
}

template <typename T>
void test_degenerate()
{
    typedef bg::arithmetic::general_form<T> gf;

    gf f = bg::arithmetic::construct_line<T>(0, 0, 10, 0);
    BOOST_CHECK(! bg::arithmetic::is_degenerate(f));

    f = bg::arithmetic::construct_line<T>(0, 0, 0, 10);
    BOOST_CHECK(! bg::arithmetic::is_degenerate(f));

    f = bg::arithmetic::construct_line<T>(0, 0, 10, 10);
    BOOST_CHECK(! bg::arithmetic::is_degenerate(f));

    f = bg::arithmetic::construct_line<T>(0, 0, 0, 0);
    BOOST_CHECK(bg::arithmetic::is_degenerate(f));
}


template <typename T>
void test_all()
{
    test_construct_line<T>();
    test_side_value<T>();
    test_get_intersection<T>();
    test_same_direction<T>();
    test_degenerate<T>();
}

int test_main(int, char* [])
{
    test_all<double>();
    test_all<long double>();
    test_all<float>();
    test_all<int>();
    return 0;
}
