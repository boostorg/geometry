// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
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

#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>


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
        BOOST_CHECK_CLOSE(double(boost::geometry::get<0>(p)), double(x), 0.001);
        BOOST_CHECK_CLOSE(double(boost::geometry::get<1>(p)), double(y), 0.001);
    }
};


template <>
struct check_result<3>
{
    template <typename Point, typename T>
    static void apply(Point const& p, T const& x, T const& y, T const& z)
    {
        BOOST_CHECK_CLOSE(double(boost::geometry::get<0>(p)), double(x), 0.001);
        BOOST_CHECK_CLOSE(double(boost::geometry::get<1>(p)), double(y), 0.001);
        BOOST_CHECK_CLOSE(double(boost::geometry::get<2>(p)), double(z), 0.001);
    }
};



template <typename CalculationType, typename Geometry, typename Point>
void test_with_other_calculation_type(Geometry const& geometry, Point& c1)
{
    typedef typename boost::geometry::point_type<Geometry>::type point_type;
    // Calculate it with user defined strategy
    point_type c2;
    boost::geometry::centroid(geometry, c2,
        boost::geometry::strategy::centroid_::bashein_detmer<point_type, point_type, CalculationType>());

    std::cout << typeid(CalculationType).name() << ": " << std::setprecision(20)
        << boost::geometry::get<0>(c2) << " " << boost::geometry::get<1>(c2)
        << " -> difference: " << boost::geometry::distance(c1, c2)
        << std::endl;
}

template <typename Geometry, typename T>
void test_centroid(std::string const& wkt, T const& x, T const& y, T const& z = T())
{
    Geometry geometry;
    boost::geometry::read_wkt(wkt, geometry);
    typedef typename boost::geometry::point_type<Geometry>::type point_type;
    point_type c1;
    boost::geometry::centroid(geometry, c1);
    check_result<boost::geometry::dimension<Geometry>::type::value>::apply(c1, x, y, z);

#ifdef REPORT_RESULTS
    std::cout << "normal: " << std::setprecision(20) << boost::geometry::get<0>(c1) << " " << boost::geometry::get<1>(c1) << std::endl;

    //test_with_other_calculation_type<long long>(geometry, c1);
    test_with_other_calculation_type<float>(geometry, c1);
    test_with_other_calculation_type<long double>(geometry, c1);
#if defined(HAVE_GMP)
    test_with_other_calculation_type<boost::numeric_adaptor::gmp_value_type>(geometry, c1);
#endif
#if defined(HAVE_CLN)
    test_with_other_calculation_type<boost::numeric_adaptor::cln_value_type>(geometry, c1);
#endif

#endif
}


#endif
