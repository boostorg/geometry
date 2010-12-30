// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_AREA_HPP
#define BOOST_GEOMETRY_TEST_AREA_HPP


#include <geometry_test_common.hpp>

#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>


template <typename Geometry>
void test_area(Geometry const& geometry, long double expected_area)
{
    long double area = bg::area(geometry);

#ifdef GEOMETRY_TEST_DEBUG
    std::ostringstream out;
    out << typeid(typename bg::coordinate_type<Geometry>::type).name()
        << " "
        << typeid(typename bg::area_result<Geometry>::type).name()
        << " "
        << "area : " << bg::area(geometry)
        << std::endl;
    std::cout << out.str();
#endif

    BOOST_CHECK_CLOSE(area, expected_area, 0.0001);

    // Test with explicitly defined strategies
    bg::strategy::area::by_triangles
        <
            typename bg::point_type<Geometry>::type
        > strategy1;

    area = bg::area(geometry, strategy1);

    bg::strategy::area::by_triangles
        <
            typename bg::point_type<Geometry>::type,
            long double
        > strategy2;

    area = bg::area(geometry, strategy2);

}

template <typename Geometry>
void test_geometry(std::string const& wkt, double expected_area)
{
    Geometry geometry;
    bg::read_wkt(wkt, geometry);
    test_area(geometry, expected_area);
}


#endif
