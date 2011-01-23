// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <geometry_test_common.hpp>


#include <boost/geometry/core/coordinate_dimension.hpp>


#include <boost/geometry/geometries/geometries.hpp>

#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>

#include <boost/geometry/geometries/adapted/boost_array_as_linestring.hpp>
#include <boost/geometry/geometries/adapted/std_as_linestring.hpp>

#include <vector>
#include <deque>


template <typename G, int Expected>
void test_geometry()
{
    BOOST_CHECK_EQUAL(bg::dimension<G>::type::value, Expected);
}

template <typename P, size_t D>
void test_all()
{
    test_geometry<P, D>();
    test_geometry<P const, D>();
    test_geometry<bg::model::linestring<P> , D>();
    test_geometry<bg::model::ring<P> , D>();
    test_geometry<bg::model::polygon<P> , D>();
    test_geometry<bg::model::box<P> , D>();
    test_geometry<bg::model::segment<P> , D>();
    test_geometry<bg::model::referring_segment<P const> , D>();

    test_geometry<std::vector<P>, D>();
    test_geometry<std::deque<P>, D>();

    test_geometry<boost::array<P, 5>, D>();
}

int test_main(int, char* [])
{
    test_geometry<int[2], 2>();
    test_geometry<float[2], 2>();
    test_geometry<double[2], 2>();

    test_geometry<int[3], 3>();
    test_geometry<float[3], 3>();
    test_geometry<double[3], 3>();

    test_geometry<boost::tuple<double, double>, 2>();
    test_geometry<boost::tuple<double, double, double>, 3>();

    test_all<bg::model::point<int, 2, bg::cs::cartesian>, 2 >();
    test_all<bg::model::point<float, 2, bg::cs::cartesian>, 2 >();
    test_all<bg::model::point<double, 2, bg::cs::cartesian>, 2 >();

    return 0;
}
