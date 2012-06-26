// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_TEST_INTERSECTION_CONTENT_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_TEST_INTERSECTION_CONTENT_HPP

#include <geometry_index_test_common.hpp>

#include <boost/geometry/extensions/index/algorithms/intersection_content.hpp>
#include <boost/geometry/extensions/index/algorithms/overlap.hpp>

template <typename Geometry>
void test_intersection_content(Geometry const& geometry1, Geometry const& geometry2,
            typename bgi::default_content_result<Geometry>::type expected_value)
{
    typename bgi::default_content_result<Geometry>::type value = bgi::intersection_content(geometry1, geometry2);

#ifdef GEOMETRY_TEST_DEBUG
    std::ostringstream out;
    out << typeid(typename bg::coordinate_type<Geometry>::type).name()
        << " "
        << typeid(typename bgi::default_content_result<Geometry>::type).name()
        << " "
        << "intersection_content : " << value
        << std::endl;
    std::cout << out.str();
#endif

    BOOST_CHECK_CLOSE(value, expected_value, 0.0001);

    // temp
    BOOST_CHECK_CLOSE(bgi::overlap(geometry1, geometry2), expected_value, 0.0001);
}

template <typename Geometry>
void test_geometry(std::string const& wkt1, std::string const& wkt2,
            typename bgi::default_content_result<Geometry>::type expected_value)
{
    Geometry geometry1, geometry2;
    bg::read_wkt(wkt1, geometry1);
    bg::read_wkt(wkt2, geometry2);
    test_intersection_content(geometry1, geometry2, expected_value);
}

#endif
