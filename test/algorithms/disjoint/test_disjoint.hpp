// Boost.Geometry

// Copyright (c) 2016 Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/geometry/algorithms/disjoint.hpp>
#include <boost/geometry/algorithms/make.hpp>

#include <boost/geometry/io/wkt/read.hpp>

template <typename Geometry1, typename Geometry2>
void test_disjoint(std::string const& wkt1,
                   std::string const& wkt2,
                   bool const expected_result)
{
    Geometry1 geometry1;
    bg::read_wkt(wkt1, geometry1);

    Geometry2 geometry2;
    bg::read_wkt(wkt2, geometry2);

    BOOST_CHECK_EQUAL(bg::disjoint(geometry1, geometry2), expected_result);
    //reverse
    BOOST_CHECK_EQUAL(bg::disjoint(geometry2, geometry1), expected_result);
}

template <typename Geometry1, typename Geometry2, typename Strategy>
void test_disjoint_strategy(std::string const& wkt1,
                            std::string const& wkt2,
                            bool const expected_result,
                            Strategy strategy)
{
    Geometry1 geometry1;
    bg::read_wkt(wkt1, geometry1);

    Geometry2 geometry2;
    bg::read_wkt(wkt2, geometry2);

    BOOST_CHECK_EQUAL(bg::disjoint(geometry1, geometry2, strategy),
                      expected_result);
    //reverse
    BOOST_CHECK_EQUAL(bg::disjoint(geometry2, geometry1, strategy),
                      expected_result);
}
