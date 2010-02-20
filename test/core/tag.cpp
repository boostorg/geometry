// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <geometry_test_common.hpp>

#include <boost/geometry/core/tag.hpp>

#include <boost/geometry/geometries/geometries.hpp>

#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>

#include <boost/geometry/geometries/adapted/boost_array_as_linestring.hpp>
#include <boost/geometry/geometries/adapted/std_as_linestring.hpp>

#include <vector>
#include <deque>


template <typename G, typename Expected>
void test_geometry()
{
    BOOST_CHECK_EQUAL(typeid(typename boost::geometry::tag<G>::type).name(),
        typeid(Expected).name());
}

template <typename P, size_t D>
void test_all()
{
    test_geometry<P, boost::geometry::point_tag>();
    test_geometry<const P, boost::geometry::point_tag>();
    test_geometry<boost::geometry::linestring<P> , boost::geometry::linestring_tag>();
    test_geometry<boost::geometry::linear_ring<P> , boost::geometry::ring_tag>();
    test_geometry<boost::geometry::polygon<P> , boost::geometry::polygon_tag>();
    test_geometry<boost::geometry::box<P> , boost::geometry::box_tag>();
    test_geometry<boost::geometry::segment<P> , boost::geometry::segment_tag>();
    test_geometry<boost::geometry::segment<const P> , boost::geometry::segment_tag>();

    test_geometry<std::vector<P>, boost::geometry::linestring_tag>();
    test_geometry<std::deque<P>, boost::geometry::linestring_tag>();

    test_geometry<boost::array<P, 5>, boost::geometry::linestring_tag>();
}

int test_main(int, char* [])
{
    test_geometry<int[2], boost::geometry::point_tag>();
    test_geometry<float[2], boost::geometry::point_tag>();
    test_geometry<double[2], boost::geometry::point_tag>();

    test_geometry<int[3], boost::geometry::point_tag>();
    test_geometry<float[3], boost::geometry::point_tag>();
    test_geometry<double[3], boost::geometry::point_tag>();

    test_geometry<boost::tuple<double, double>, boost::geometry::point_tag>();
    test_geometry<boost::tuple<double, double, double>, boost::geometry::point_tag>();

    test_all<boost::geometry::point<int, 2, boost::geometry::cs::cartesian>, 2 >();
    test_all<boost::geometry::point<float, 2, boost::geometry::cs::cartesian>, 2 >();
    test_all<boost::geometry::point<double, 2, boost::geometry::cs::cartesian>, 2 >();

    return 0;
}
