// Boost.Geometry (aka GGL, Generic Geometry Library) Point concept unit tests
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GEOMETRY_TEST_POINT_CONCEPT_FUNCTION_ASSERTING_A_POINT_HPP
#define GEOMETRY_TEST_POINT_CONCEPT_FUNCTION_ASSERTING_A_POINT_HPP

#include <boost/concept/requires.hpp>

#include <boost/geometry/geometries/concepts/point_concept.hpp>

namespace test
{
    template <typename P, typename CP>
    void function_asserting_a_point(P& p1, const CP& p2)
    {
        BOOST_CONCEPT_ASSERT((boost::geometry::concept::Point<P>));
        BOOST_CONCEPT_ASSERT((boost::geometry::concept::ConstPoint<P>));

        get<0>(p1) = get<0>(p2);
    }
}

#endif // GEOMETRY_TEST_POINT_CONCEPT_FUNCTION_ASSERTING_A_POINT_HPP
