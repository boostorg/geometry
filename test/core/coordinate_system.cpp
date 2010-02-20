// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <ggl_test_common.hpp>


#include <boost/geometry/core/coordinate_system.hpp>


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
    BOOST_CHECK_EQUAL(typeid(typename boost::geometry::coordinate_system<G>::type).name(),
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
    // Because of the included headerfiles, there are always cartesian
    test_geometry<int[2], boost::geometry::cs::cartesian>();
    test_geometry<float[2], boost::geometry::cs::cartesian>();
    test_geometry<double[2], boost::geometry::cs::cartesian>();

    test_geometry<int[3], boost::geometry::cs::cartesian>();
    test_geometry<float[3], boost::geometry::cs::cartesian>();
    test_geometry<double[3], boost::geometry::cs::cartesian>();

    // Because of the included headerfiles, there are always cartesian
    test_geometry<boost::tuple<double, double>, boost::geometry::cs::cartesian>();
    test_geometry<boost::tuple<double, double, double>, boost::geometry::cs::cartesian>();


    // Test cartesian
    test_all<boost::geometry::point<int, 2, boost::geometry::cs::cartesian>, boost::geometry::cs::cartesian>();
    test_all<boost::geometry::point<float, 2, boost::geometry::cs::cartesian>, boost::geometry::cs::cartesian>();
    test_all<boost::geometry::point<double, 2, boost::geometry::cs::cartesian>, boost::geometry::cs::cartesian>();

    // Test spherical
    test_all<boost::geometry::point<int, 2, boost::geometry::cs::spherical<boost::geometry::degree> >,
            boost::geometry::cs::spherical<boost::geometry::degree> >();
    test_all<boost::geometry::point<float, 2, boost::geometry::cs::spherical<boost::geometry::degree> >,
            boost::geometry::cs::spherical<boost::geometry::degree> >();
    test_all<boost::geometry::point<double, 2, boost::geometry::cs::spherical<boost::geometry::degree> >,
            boost::geometry::cs::spherical<boost::geometry::degree> >();

    test_all<boost::geometry::point<int, 2, boost::geometry::cs::spherical<boost::geometry::radian> >,
                boost::geometry::cs::spherical<boost::geometry::radian> >();
    test_all<boost::geometry::point<float, 2, boost::geometry::cs::spherical<boost::geometry::radian> >,
                boost::geometry::cs::spherical<boost::geometry::radian> >();
    test_all<boost::geometry::point<double, 2, boost::geometry::cs::spherical<boost::geometry::radian> >,
                boost::geometry::cs::spherical<boost::geometry::radian> >();

    // Test other
    test_all<boost::geometry::point<double, 2, boost::geometry::cs::polar<boost::geometry::degree> >,
            boost::geometry::cs::polar<boost::geometry::degree> >();

    test_all<boost::geometry::point<double, 2, boost::geometry::cs::geographic<boost::geometry::degree> >,
            boost::geometry::cs::geographic<boost::geometry::degree> >();

    return 0;
}
