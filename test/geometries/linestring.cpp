// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2020 Digvijay Janartha, Hamirpur, India.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>

#include <geometry_test_common.hpp>

#include <boost/core/ignore_unused.hpp>
#include <boost/geometry/algorithms/make.hpp>
#include <boost/geometry/algorithms/append.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/concepts/linestring_concept.hpp>
#include <boost/geometry/geometries/adapted/c_array.hpp>
#include <boost/geometry/geometries/adapted/boost_tuple.hpp>
#include <boost/geometry/io/dsv/write.hpp>

#include <test_common/test_point.hpp>

BOOST_GEOMETRY_REGISTER_C_ARRAY_CS(cs::cartesian)
BOOST_GEOMETRY_REGISTER_BOOST_TUPLE_CS(cs::cartesian)


template <typename P>
bg::model::linestring<P> create_linestring()
{   
    bg::model::linestring<P> l1;
    P p1;
    bg::assign_values(p1, 1, 2, 3);
    bg::append(l1, p1);
    return l1;
}

template <typename L, typename T>
void check_linestring(L& to_check, T x, T y, T z)
{
    BOOST_CHECK_EQUAL(bg::get<0>(to_check[0]), x);
    BOOST_CHECK_EQUAL(bg::get<1>(to_check[0]), y);
    BOOST_CHECK_EQUAL(bg::get<2>(to_check[0]), z);
}

template <typename P>
void test_construction()
{
    bg::model::linestring<P> l1(create_linestring<P>());
    check_linestring(l1, 1, 2, 3);
}

template <typename P>
void test_compilation()
{   
    typedef bg::model::linestring<P> L;

    BOOST_CONCEPT_ASSERT( (bg::concepts::ConstLinestring<L>) );
    BOOST_CONCEPT_ASSERT( (bg::concepts::Linestring<L>) );

    typedef typename bg::coordinate_type<L>::type T;
    typedef typename bg::point_type<L>::type LP;
    boost::ignore_unused<T, LP>();
}

template <typename P>
void test_all()
{   
    test_construction<P>();
    test_compilation<P>();
}

template <typename P>
void test_custom_linestring(std::initializer_list<P> IL)
{
    bg::model::linestring<P> l1(IL);
    std::ostringstream out;
    out << bg::dsv(l1);
    BOOST_CHECK_EQUAL(out.str(), "((1, 2), (2, 3), (3, 4))");
}

template <typename P>
void test_custom()
{   
    std::initializer_list<P> IL = {P(1, 2), P(2, 3), P(3, 4)};
    test_custom_linestring<P>(IL);
}


int test_main(int, char* [])
{   
    test_all<bg::model::point<int, 3, bg::cs::cartesian> >();
    test_all<bg::model::point<float, 3, bg::cs::cartesian> >();
    test_all<bg::model::point<double, 3, bg::cs::cartesian> >();

    test_custom<bg::model::point<double, 2, bg::cs::cartesian> >();
    test_custom<bg::model::d2::point_xy<double> >();

    return 0;
}
