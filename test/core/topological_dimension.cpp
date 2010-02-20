// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <ggl_test_common.hpp>

#include <boost/geometry/core/topological_dimension.hpp>

#include <boost/geometry/geometries/geometries.hpp>

#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>

#include <boost/geometry/geometries/adapted/boost_array_as_linestring.hpp>
#include <boost/geometry/geometries/adapted/std_as_linestring.hpp>

#include <vector>
#include <deque>


template <typename G, std::size_t Expected>
void test_geometry()
{
    BOOST_CHECK_EQUAL(boost::geometry::topological_dimension<G>::type::value, Expected);
}

template <typename P>
void test_all()
{
    test_geometry<P, 0>();
    test_geometry<const P, 0>();
    test_geometry<boost::geometry::linestring<P> , 1>();
    test_geometry<boost::geometry::linear_ring<P> , 2>(); // being discussed
    test_geometry<boost::geometry::polygon<P> , 2>();
    test_geometry<boost::geometry::box<P> , 2>();
    test_geometry<boost::geometry::segment<P> , 1>();
    test_geometry<boost::geometry::segment<const P> , 1>();

    test_geometry<std::vector<P>, 1>();
    test_geometry<std::deque<P>, 1>();

    test_geometry<boost::array<P, 5>, 1>();
}

int test_main(int, char* [])
{
    test_geometry<int[2], 0>();
    test_geometry<float[2], 0>();
    test_geometry<double[2], 0>();

    test_geometry<int[3], 0>();
    test_geometry<float[3], 0>();
    test_geometry<double[3], 0>();

    test_geometry<boost::tuple<double, double>, 0>();
    test_geometry<boost::tuple<double, double, double>, 0>();

    test_all<boost::geometry::point<int, 2, boost::geometry::cs::cartesian> >();
    test_all<boost::geometry::point<float, 2, boost::geometry::cs::cartesian> >();
    test_all<boost::geometry::point<double, 2, boost::geometry::cs::cartesian> >();

    return 0;
}
