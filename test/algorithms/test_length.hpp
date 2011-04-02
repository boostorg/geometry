// Boost.Geometry (aka GGL, Generic Geometry Library) 
// Unit Test

// Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_LENGTH_HPP
#define BOOST_GEOMETRY_TEST_LENGTH_HPP

#include <geometry_test_common.hpp>

#include <boost/geometry/algorithms/length.hpp>
#include <boost/geometry/domains/gis/io/wkt/read_wkt.hpp>
#include <boost/geometry/strategies/strategies.hpp>


template <typename Geometry>
void test_length(Geometry const& geometry, long double expected_length)
{
    typename bg::default_length_result<Geometry>::type length = bg::length(geometry);

#ifdef GEOMETRY_TEST_DEBUG
    std::ostringstream out;
    out << typeid(typename bg::coordinate_type<Geometry>::type).name()
        << std::endl
        << typeid(typename bg::default_length_result<Geometry>::type).name()
        << std::endl
        << "length : " << bg::length(geometry)
        << std::endl;
    std::cout << out.str();
#endif

    BOOST_CHECK_CLOSE(length, expected_length, 0.0001);
}


template <typename Geometry>
void test_geometry(std::string const& wkt, double expected_length)
{
    Geometry geometry;
    bg::read_wkt(wkt, geometry);
    test_length(geometry, expected_length);
}


#endif
