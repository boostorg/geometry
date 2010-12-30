// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <geometry_test_common.hpp>


#include <boost/geometry/arithmetic/arithmetic.hpp>

#include <boost/geometry/algorithms/assign.hpp>

#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>
#include <test_common/test_point.hpp>


template <typename P>
void test_addition()
{
    P p1;
    bg::assign(p1, 1, 2, 3);
    bg::add_value(p1, 10);
    BOOST_CHECK(bg::get<0>(p1) == 11);
    BOOST_CHECK(bg::get<1>(p1) == 12);
    BOOST_CHECK(bg::get<2>(p1) == 13);

    P p2;
    bg::assign(p2, 4, 5, 6);
    bg::add_point(p1, p2);
    BOOST_CHECK(bg::get<0>(p1) == 15);
    BOOST_CHECK(bg::get<1>(p1) == 17);
    BOOST_CHECK(bg::get<2>(p1) == 19);
}

template <typename P>
void test_subtraction()
{
    P p1;
    bg::assign(p1, 1, 2, 3);
    bg::subtract_value(p1, 10);
    BOOST_CHECK(bg::get<0>(p1) == -9);
    BOOST_CHECK(bg::get<1>(p1) == -8);
    BOOST_CHECK(bg::get<2>(p1) == -7);

    P p2;
    bg::assign(p2, 4, 6, 8);
    bg::subtract_point(p1, p2);
    BOOST_CHECK(bg::get<0>(p1) == -13);
    BOOST_CHECK(bg::get<1>(p1) == -14);
    BOOST_CHECK(bg::get<2>(p1) == -15);
}

template <typename P>
void test_multiplication()
{
    P p1;
    bg::assign(p1, 1, 2, 3);
    bg::multiply_value(p1, 5);
    BOOST_CHECK(bg::get<0>(p1) == 5);
    BOOST_CHECK(bg::get<1>(p1) == 10);
    BOOST_CHECK(bg::get<2>(p1) == 15);

    P p2;
    bg::assign(p2, 4, 5, 6);
    bg::multiply_point(p1, p2);
    BOOST_CHECK(bg::get<0>(p1) == 20);
    BOOST_CHECK(bg::get<1>(p1) == 50);
    BOOST_CHECK(bg::get<2>(p1) == 90);
}

template <typename P>
void test_division()
{
    P p1;
    bg::assign(p1, 50, 100, 150);
    bg::divide_value(p1, 5);
    BOOST_CHECK(bg::get<0>(p1) == 10);
    BOOST_CHECK(bg::get<1>(p1) == 20);
    BOOST_CHECK(bg::get<2>(p1) == 30);

    P p2;
    bg::assign(p2, 2, 4, 6);
    bg::divide_point(p1, p2);
    BOOST_CHECK(bg::get<0>(p1) == 5);
    BOOST_CHECK(bg::get<1>(p1) == 5);
    BOOST_CHECK(bg::get<2>(p1) == 5);
}


template <typename P>
void test_all()
{
    test_addition<P>();
    test_subtraction<P>();
    test_multiplication<P>();
    test_division<P>();
}


int test_main(int, char* [])
{
    test_all<int[3]>();
    test_all<float[3]>();
    test_all<double[3]>();
    test_all<test::test_point>();
    test_all<bg::model::point<int, 3, bg::cs::cartesian> >();
    test_all<bg::model::point<float, 3, bg::cs::cartesian> >();
    test_all<bg::model::point<double, 3, bg::cs::cartesian> >();

    return 0;
}
