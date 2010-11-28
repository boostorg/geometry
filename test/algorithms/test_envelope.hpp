// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_ENVELOPE_HPP
#define BOOST_GEOMETRY_TEST_ENVELOPE_HPP


#include <geometry_test_common.hpp>

#include <boost/geometry/algorithms/envelope.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>


template<std::size_t DimensionCount>
struct check_result
{};

template <>
struct check_result<2>
{
    template <typename Box, typename T>
    static void apply(Box const& b, const T& x1, const T& y1, const T& z1,
                const T& x2, const T& y2, const T& z2)
    {
        BOOST_CHECK_CLOSE(double(bg::get<bg::min_corner, 0>(b)), double(x1), 0.001);
        BOOST_CHECK_CLOSE(double(bg::get<bg::min_corner, 1>(b)), double(y1), 0.001);

        BOOST_CHECK_CLOSE(double(bg::get<bg::max_corner, 0>(b)), double(x2), 0.001);
        BOOST_CHECK_CLOSE(double(bg::get<bg::max_corner, 1>(b)), double(y2), 0.001);
    }
};

template <>
struct check_result<3>
{
    template <typename Box, typename T>
    static void apply(Box const& b, const T& x1, const T& y1, const T& z1,
                const T& x2, const T& y2, const T& z2)
    {
        BOOST_CHECK_CLOSE(double(bg::get<bg::min_corner, 0>(b)), double(x1), 0.001);
        BOOST_CHECK_CLOSE(double(bg::get<bg::min_corner, 1>(b)), double(y1), 0.001);
        BOOST_CHECK_CLOSE(double(bg::get<bg::min_corner, 2>(b)), double(z1), 0.001);

        BOOST_CHECK_CLOSE(double(bg::get<bg::max_corner, 0>(b)), double(x2), 0.001);
        BOOST_CHECK_CLOSE(double(bg::get<bg::max_corner, 1>(b)), double(y2), 0.001);
        BOOST_CHECK_CLOSE(double(bg::get<bg::max_corner, 2>(b)), double(z2), 0.001);
    }
};


template <typename Geometry, typename T>
void test_envelope(std::string const& wkt,
                   const T& x1, const T& x2,
                   const T& y1, const T& y2,
                   const T& z1 = 0, const T& z2 = 0)
{
    Geometry geometry;
    bg::read_wkt(wkt, geometry);
    bg::model::box<typename bg::point_type<Geometry>::type > b;
    bg::envelope(geometry, b);

    check_result<bg::dimension<Geometry>::type::value>::apply(b, x1, y1, z1, x2, y2, z2);
}

template <typename Geometry, typename T>
void test_envelope_strategy(std::string const& wkt,
                   const T& x1, const T& x2,
                   const T& y1, const T& y2,
                   const T& z1 = 0, const T& z2 = 0)
{
    Geometry geometry;
    bg::read_wkt(wkt, geometry);
    bg::model::box<typename bg::point_type<Geometry>::type > b;
    bg::envelope(geometry, b);

    check_result<bg::dimension<Geometry>::type::value>::apply(b, x1, y1, z1, x2, y2, z2);
}



#endif
