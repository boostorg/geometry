// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_COMBINE_HPP
#define BOOST_GEOMETRY_TEST_COMBINE_HPP


#include <geometry_test_common.hpp>

#include <boost/geometry/algorithms/combine.hpp>
#include <boost/geometry/strategies/strategies.hpp>
#include <boost/geometry/domains/gis/io/wkt/read_wkt.hpp>
#include <boost/geometry/algorithms/assign.hpp>
#include <boost/geometry/util/write_dsv.hpp>


template <typename Geometry, typename Box>
void test_combine(Box& box,
                  std::string const& wkt,
                  std::string const& expected)
{
    Geometry geometry;
    bg::read_wkt(wkt, geometry);

    bg::combine(box, geometry);

    std::ostringstream out;
    out << bg::dsv(box, ",", "(", ")", ",", "", "");

    BOOST_CHECK_EQUAL(out.str(), expected);
}

template <typename Geometry, typename Box>
void test_combine_other_strategy(Box& box,
                  std::string const& wkt,
                  std::string const& expected)
{
    Geometry geometry;
    bg::read_wkt(wkt, geometry);


    bg::combine(box, geometry);

    std::ostringstream out;
    out << bg::dsv(box, ",", "(", ")", ",", "", "");

    BOOST_CHECK_EQUAL(out.str(), expected);
}


#endif
