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
    BOOST_CHECK_EQUAL(typeid(typename bg::tag<G>::type).name(),
        typeid(Expected).name());
}

template <typename P, size_t D>
void test_all()
{
    test_geometry<P, bg::point_tag>();
    test_geometry<P const, bg::point_tag>();
    test_geometry<bg::model::linestring<P> , bg::linestring_tag>();
    test_geometry<bg::model::ring<P> , bg::ring_tag>();
    test_geometry<bg::model::polygon<P> , bg::polygon_tag>();
    test_geometry<bg::model::box<P> , bg::box_tag>();
    test_geometry<bg::model::segment<P> , bg::segment_tag>();
    test_geometry<bg::model::referring_segment<P const> , bg::segment_tag>();

    test_geometry<std::vector<P>, bg::linestring_tag>();
    test_geometry<std::deque<P>, bg::linestring_tag>();

    test_geometry<boost::array<P, 5>, bg::linestring_tag>();
}

int test_main(int, char* [])
{
    test_geometry<int[2], bg::point_tag>();
    test_geometry<float[2], bg::point_tag>();
    test_geometry<double[2], bg::point_tag>();

    test_geometry<int[3], bg::point_tag>();
    test_geometry<float[3], bg::point_tag>();
    test_geometry<double[3], bg::point_tag>();

    test_geometry<boost::tuple<double, double>, bg::point_tag>();
    test_geometry<boost::tuple<double, double, double>, bg::point_tag>();

    test_all<bg::model::point<int, 2, bg::cs::cartesian>, 2 >();
    test_all<bg::model::point<float, 2, bg::cs::cartesian>, 2 >();
    test_all<bg::model::point<double, 2, bg::cs::cartesian>, 2 >();

    return 0;
}
