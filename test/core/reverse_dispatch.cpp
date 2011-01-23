// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <geometry_test_common.hpp>

#include <boost/geometry/core/reverse_dispatch.hpp>

#include <boost/geometry/geometries/geometries.hpp>



template <typename Geometry1, typename Geometry2, bool Expected>
void test_reversed()
{
    BOOST_CHECK_EQUAL((bg::reverse_dispatch<Geometry1, Geometry2>::type::value),
                Expected);
}


template <typename P>
void test_all()
{

    test_reversed<P, P, false>();
    test_reversed<P, bg::model::linestring<P>, false>();
    test_reversed<bg::model::linestring<P>, P, true>();
    test_reversed<bg::model::ring<P>, P, true>();
    test_reversed<bg::model::linestring<P>, bg::model::ring<P>, false>();
    test_reversed<bg::model::ring<P>, bg::model::linestring<P>, true>();
}

template <typename P1, typename P2>
void test_mixed()
{
    test_reversed<P1, P2, false>();
}


int test_main(int, char* [])
{
    test_all<bg::model::point<int, 2, bg::cs::cartesian> >();
    test_mixed
        <
            bg::model::point<int, 2, bg::cs::cartesian>,
            bg::model::point<int, 2, bg::cs::spherical<bg::degree> >
        >();
    test_mixed
        <
            bg::model::point<int, 2, bg::cs::spherical<bg::degree> >,
            bg::model::point<int, 2, bg::cs::spherical<bg::radian> >
        >();
    return 0;
}
