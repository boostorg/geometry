// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_CENTROID_HPP
#define BOOST_GEOMETRY_TEST_CENTROID_HPP

// Test-functionality, shared between single and multi tests

#include <geometry_test_common.hpp>

#include <boost/geometry/strategies/strategies.hpp>
#include <boost/geometry/algorithms/centroid.hpp>
#include <boost/geometry/algorithms/distance.hpp>

#include <boost/geometry/domains/gis/io/wkt/read_wkt.hpp>


template<std::size_t D>
struct check_result
{
};

template <>
struct check_result<2>
{
    template <typename Point, typename T>
    static void apply(Point const& p, T const& x, T const& y, T const&)
    {
        BOOST_CHECK_CLOSE(bg::get<0>(p), x, 0.001);
        BOOST_CHECK_CLOSE(bg::get<1>(p), y, 0.001);
    }
};


template <>
struct check_result<3>
{
    template <typename Point, typename T>
    static void apply(Point const& p, T const& x, T const& y, T const& z)
    {
        BOOST_CHECK_CLOSE(bg::get<0>(p), x, 0.001);
        BOOST_CHECK_CLOSE(bg::get<1>(p), y, 0.001);
        BOOST_CHECK_CLOSE(bg::get<2>(p), z, 0.001);
    }
};



template <typename CalculationType, typename Geometry, typename Point>
void test_with_other_calculation_type(Geometry const& geometry, Point& c1)
{
    typedef typename bg::point_type<Geometry>::type point_type;
    // Calculate it with user defined strategy
    point_type c2;
    bg::centroid(geometry, c2,
        bg::strategy::centroid::bashein_detmer<point_type, point_type, CalculationType>());

    std::cout << typeid(CalculationType).name() << ": " << std::setprecision(20)
        << bg::get<0>(c2) << " " << bg::get<1>(c2)
        << " -> difference: " << bg::distance(c1, c2)
        << std::endl;
}

template <typename Geometry, typename T>
void test_centroid(std::string const& wkt, T const& x, T const& y, T const& z = T())
{
    Geometry geometry;
    bg::read_wkt(wkt, geometry);
    typedef typename bg::point_type<Geometry>::type point_type;
    point_type c1;
    bg::centroid(geometry, c1);
    check_result<bg::dimension<Geometry>::type::value>::apply(c1, x, y, z);

#ifdef REPORT_RESULTS
    std::cout << "normal: " << std::setprecision(20) << bg::get<0>(c1) << " " << bg::get<1>(c1) << std::endl;

    //test_with_other_calculation_type<long long>(geometry, c1);
    test_with_other_calculation_type<float>(geometry, c1);
    test_with_other_calculation_type<long double>(geometry, c1);
#if defined(HAVE_TTMATH)
    test_with_other_calculation_type<ttmath_big>(geometry, c1);
#endif

#endif
}


#endif
