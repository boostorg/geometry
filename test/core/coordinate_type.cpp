// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <ggl_test_common.hpp>


#include <boost/geometry/core/coordinate_type.hpp>


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
    BOOST_CHECK_EQUAL(typeid(typename boost::geometry::coordinate_type<G>::type).name(),
        typeid(Expected).name());
}

template <typename P, typename Expected>
void test_all()
{
    test_geometry<P, Expected>();
    test_geometry<const P, Expected>();
    test_geometry<boost::geometry::linestring<P> , Expected>();
    test_geometry<boost::geometry::linear_ring<P> , Expected>();
    test_geometry<boost::geometry::polygon<P> , Expected>();
    test_geometry<boost::geometry::box<P> , Expected>();
    test_geometry<boost::geometry::segment<P> , Expected>();
    test_geometry<boost::geometry::segment<const P> , Expected>();

    test_geometry<std::vector<P>, Expected>();
    test_geometry<std::deque<P>, Expected>();

    test_geometry<boost::array<P, 5>, Expected>();
}

int test_main(int, char* [])
{
    test_geometry<int[2], int>();
    test_geometry<float[2], float>();
    test_geometry<double[2], double>();

    test_geometry<int[3], int>();
    test_geometry<float[3], float>();
    test_geometry<double[3], double>();

    test_geometry<boost::tuple<float, float>, float>();
    test_geometry<boost::tuple<double, double>, double>();
    test_geometry<boost::tuple<long double, long double>, long double>();
    test_geometry<boost::tuple<double, double, double>, double>();

    test_all<boost::geometry::point<int, 2, boost::geometry::cs::cartesian>, int>();
    test_all<boost::geometry::point<float, 2, boost::geometry::cs::cartesian>, float>();
    test_all<boost::geometry::point<double, 2, boost::geometry::cs::cartesian>, double>();
    test_all<boost::geometry::point<long double, 2, boost::geometry::cs::cartesian>, long double>();

    return 0;
}
