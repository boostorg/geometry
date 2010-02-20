// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <ggl_test_common.hpp>

#include <boost/geometry/algorithms/make.hpp>


#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>
#include <test_common/test_point.hpp>

using namespace boost::geometry;

template <typename P>
box<P> create_box()
{
    P p1;
    P p2;
    assign(p1, 1, 2, 5);
    assign(p2, 3, 4, 6);
    return box<P>(p1, p2);
}

template <typename B, typename T>
void check_box(B& to_check,
               T min_x, T min_y, T min_z,
               T max_x, T max_y, T max_z)
{
    BOOST_CHECK_EQUAL(get<0>(to_check.min_corner()), min_x);
    BOOST_CHECK_EQUAL(get<1>(to_check.min_corner()), min_y);
    BOOST_CHECK_EQUAL(get<2>(to_check.min_corner()), min_z);
    BOOST_CHECK_EQUAL(get<0>(to_check.max_corner()), max_x);
    BOOST_CHECK_EQUAL(get<1>(to_check.max_corner()), max_y);
    BOOST_CHECK_EQUAL(get<2>(to_check.max_corner()), max_z);
}

template <typename P>
void test_construction()
{
    typedef typename coordinate_type<P>::type T;

    box<P> b1 = make_zero<box<P> >();
    check_box(b1, T(),T(),T(),T(),T(),T());

    box<P> b2(create_box<P>());
    check_box(b2, 1,2,5,3,4,6);

    box<P> b3 = make_inverse<box<P> >();
    check_box(b3, boost::numeric::bounds<T>::highest(),
                  boost::numeric::bounds<T>::highest(),
                  boost::numeric::bounds<T>::highest(),
                  boost::numeric::bounds<T>::lowest(),
                  boost::numeric::bounds<T>::lowest(),
                  boost::numeric::bounds<T>::lowest());
}

template <typename P>
void test_assignment()
{
    box<P> b(create_box<P>());
    set<0>(b.min_corner(), 10);
    set<1>(b.min_corner(), 20);
    set<2>(b.min_corner(), 30);
    set<0>(b.max_corner(), 40);
    set<1>(b.max_corner(), 50);
    set<2>(b.max_corner(), 60);
    check_box(b, 10,20,30,40,50,60);
}

template <typename P>
void test_all()
{
    test_construction<P>();
    test_assignment<P>();
}

int test_main(int, char* [])
{
    test_all<int[3]>();
    test_all<float[3]>();
    test_all<double[3]>();
    test_all<test::test_point>();
    test_all<point<int, 3, cs::cartesian> >();
    test_all<point<float, 3, cs::cartesian> >();
    test_all<point<double, 3, cs::cartesian> >();

    return 0;
}
