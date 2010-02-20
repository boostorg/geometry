// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <deque>
#include <vector>

#include <boost/concept/requires.hpp>
#include <ggl_test_common.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/algorithms/make.hpp>
#include <boost/geometry/algorithms/clear.hpp>
#include <boost/geometry/algorithms/append.hpp>
#include <boost/geometry/algorithms/num_points.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/adapted/std_as_linestring.hpp>
#include <boost/geometry/geometries/adapted/boost_array_as_linestring.hpp>

#include <test_common/test_point.hpp>

template <typename G>
void test_geometry()
{
    G geometry;
    typedef typename boost::geometry::point_type<G>::type P;

    boost::geometry::append(geometry, boost::geometry::make_zero<P>());
    BOOST_CHECK_EQUAL(boost::geometry::num_points(geometry), 1);

    boost::geometry::clear(geometry);
    BOOST_CHECK_EQUAL(boost::geometry::num_points(geometry), 0);
    //P p = boost::range::front(geometry);
}

template <typename P>
void test_all()
{
    test_geometry<boost::geometry::linestring<P> >();
    test_geometry<boost::geometry::linear_ring<P> >();
    test_geometry<boost::geometry::polygon<P> >();

    test_geometry<std::vector<P> >();
    test_geometry<std::deque<P> >();
    //test_geometry<std::list<P> >();
}

int test_main(int, char* [])
{
    test_all<test::test_point>();
    test_all<boost::geometry::point<int, 2, boost::geometry::cs::cartesian> >();
    test_all<boost::geometry::point<float, 2, boost::geometry::cs::cartesian> >();
    test_all<boost::geometry::point<double, 2, boost::geometry::cs::cartesian> >();

    return 0;
}
