// Boost.Geometry

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_CLOSEST_POINTS_HPP
#define BOOST_GEOMETRY_STRATEGIES_CLOSEST_POINTS_HPP


#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/static_assert.hpp>
#include <boost/geometry/strategies/tags.hpp>


namespace boost { namespace geometry
{


namespace strategy { namespace closest_points { namespace services
{


template <typename Strategy> struct tag
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE(
        "Not implemented for this Strategy.",
        Strategy);
};

template <typename Strategy, typename P1, typename P2>
struct return_type
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE(
        "Not implemented for this Strategy.",
        Strategy, P1, P2);
};


template <typename Strategy> struct comparable_type
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE(
        "Not implemented for this Strategy.",
        Strategy);
};

template <typename Strategy> struct get_comparable
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE(
        "Not implemented for this Strategy.",
        Strategy);
};

template <typename Strategy, typename P1, typename P2>
struct result_from_distance
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE(
        "Not implemented for this Strategy.",
        Strategy, P1, P2);
};




// Default strategy


/*!
    \brief Traits class binding a default strategy for distance
        to one (or possibly two) coordinate system(s)
    \ingroup distance
    \tparam GeometryTag1 tag (point/segment/box) for which this strategy is the default
    \tparam GeometryTag2 tag (point/segment/box) for which this strategy is the default
    \tparam Point1 first point-type
    \tparam Point2 second point-type
    \tparam CsTag1 tag of coordinate system of first point type
    \tparam CsTag2 tag of coordinate system of second point type
*/
template
<
    typename GeometryTag1,
    typename GeometryTag2,
    typename Point1,
    typename Point2 = Point1,
    typename CsTag1 = typename cs_tag<Point1>::type,
    typename CsTag2 = typename cs_tag<Point2>::type,
    typename UnderlyingStrategy = void
>
struct default_strategy
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE(
        "Not implemented for this Point type combination.",
        Point1, Point2, CsTag1, CsTag2);
};


}}} // namespace strategy::closest_points::services


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_CLOSEST_POINTS_HPP
