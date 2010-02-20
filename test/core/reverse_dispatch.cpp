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
    BOOST_CHECK_EQUAL((boost::geometry::reverse_dispatch<Geometry1, Geometry2>::type::value),
                Expected);
}


template <typename P>
void test_all()
{

    test_reversed<P, P, false>();
    test_reversed<P, boost::geometry::linestring<P>, false>();
    test_reversed<boost::geometry::linestring<P>, P, true>();
    test_reversed<boost::geometry::linear_ring<P>, P, true>();
    test_reversed<boost::geometry::linestring<P>, boost::geometry::linear_ring<P>, false>();
    test_reversed<boost::geometry::linear_ring<P>, boost::geometry::linestring<P>, true>();
}

template <typename P1, typename P2>
void test_mixed()
{
    test_reversed<P1, P2, false>();
}


int test_main(int, char* [])
{
    test_all<boost::geometry::point<int, 2, boost::geometry::cs::cartesian> >();
    test_mixed
        <
            boost::geometry::point<int, 2, boost::geometry::cs::cartesian>,
            boost::geometry::point<int, 2, boost::geometry::cs::spherical<boost::geometry::degree> >
        >();
    test_mixed
        <
            boost::geometry::point<int, 2, boost::geometry::cs::spherical<boost::geometry::degree> >,
            boost::geometry::point<int, 2, boost::geometry::cs::spherical<boost::geometry::radian> >
        >();
    return 0;
}
