// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2020 Digvijay Janartha, Hamirpur, India.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <geometry_test_common.hpp>

#include <boost/core/ignore_unused.hpp>
#include <boost/geometry/geometries/concepts/point_concept.hpp>
#include <boost/geometry/algorithms/make.hpp>
#include <boost/geometry/geometries/point.hpp>

#include <test_common/test_point.hpp>


template <typename T>
bg::model::point<T, 3, bg::cs::cartesian> create_point()
{
    T t1 = 1;
    T t2 = 2;
    T t3 = 3;
    return bg::model::point<T, 3, bg::cs::cartesian>(t1, t2, t3);
}

template <typename P, typename T>
void check_point(P& to_check, T x, T y, T z)
{
    BOOST_CHECK_EQUAL(bg::get<0>(to_check), x);
    BOOST_CHECK_EQUAL(bg::get<1>(to_check), y);
    BOOST_CHECK_EQUAL(bg::get<2>(to_check), z);
}

template <typename T>
void test_construction()
{
    bg::model::point<T, 3, bg::cs::cartesian> p(create_point<T>());
    check_point(p, 1, 2, 3);
}

template <typename T>
void test_assignment()
{
    bg::model::point<T, 3, bg::cs::cartesian> p(create_point<T>());
    bg::set<0>(p, 4);
    bg::set<1>(p, 5);
    bg::set<2>(p, 6);
    check_point(p, 4, 5, 6);
}

template <typename T>
void test_all()
{
    test_construction<T>();
    test_assignment<T>();

    typedef bg::model::point<T, 3, bg::cs::cartesian> P;

    // Compilation tests, all things should compile.
    BOOST_CONCEPT_ASSERT( (bg::concepts::ConstPoint<P>) );
    BOOST_CONCEPT_ASSERT( (bg::concepts::Point<P>) );

    typedef typename bg::coordinate_type<P>::type T1;
    boost::ignore_unused<T1>();
}

int test_main(int, char* [])
{
    test_all<int>();
    test_all<float>();
    test_all<double>();

    return 0;
}
