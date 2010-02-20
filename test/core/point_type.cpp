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
    BOOST_CHECK_EQUAL(typeid(typename boost::geometry::point_type<G>::type).name(),
        typeid(Expected).name());
}

template <typename P>
void test_all()
{
    test_geometry<P, P>();
    test_geometry<const P, P>();
    test_geometry<boost::geometry::linestring<P> , P>();
    test_geometry<boost::geometry::linear_ring<P> , P>();
    test_geometry<boost::geometry::polygon<P> , P>();
    test_geometry<boost::geometry::box<P> , P>();
    test_geometry<boost::geometry::segment<P> , P>();
    test_geometry<boost::geometry::segment<const P> , P>();

    test_geometry<std::vector<P>, P>();
    test_geometry<std::deque<P>, P>();

    test_geometry<boost::array<P, 5>, P>();
}

int test_main(int, char* [])
{
    test_geometry<int[2], int[2]>();
    test_geometry<float[2], float[2]>();
    test_geometry<double[2], double[2]>();

    test_geometry<int[3], int[3]>();
    test_geometry<float[3], float[3]>();
    test_geometry<double[3], double[3]>();

    test_geometry<boost::tuple<double, double>,
                boost::tuple<double, double> >();
    test_geometry<boost::tuple<double, double, double>,
                boost::tuple<double, double, double> >();

    test_all<boost::geometry::point<int, 2, boost::geometry::cs::cartesian> >();
    test_all<boost::geometry::point<float, 2, boost::geometry::cs::cartesian> >();
    test_all<boost::geometry::point<double, 2, boost::geometry::cs::cartesian> >();

    return 0;
}
