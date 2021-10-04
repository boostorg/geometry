// Boost.Geometry

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_STRATEGIES_CLOSEST_POINTS_SERVICES_HPP
#define BOOST_GEOMETRY_STRATEGIES_CLOSEST_POINTS_SERVICES_HPP


#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/static_assert.hpp>

#include <boost/geometry/strategies/detail.hpp>
#include <boost/geometry/strategies/tags.hpp>


namespace boost { namespace geometry
{

namespace strategies { namespace closest_points
{

namespace services
{

template
<
    typename Geometry1,
    typename Geometry2,
    typename CSTag1 = typename geometry::cs_tag<Geometry1>::type,
    typename CSTag2 = typename geometry::cs_tag<Geometry2>::type
>
struct default_strategy
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE(
        "Not implemented for these Geometries' coordinate systems.",
        Geometry1, Geometry2, CSTag1, CSTag2);
};

template <typename Strategy>
struct strategy_converter
{
    static auto get(Strategy const&)
    {
        return strategies::detail::not_implemented();
    }
};

template
<
    typename Geometry1,
    typename Geometry2,
    typename Strategy,
    typename CSTag1 = typename geometry::cs_tag<Geometry1>::type,
    typename CSTag2 = typename geometry::cs_tag<Geometry2>::type
>
struct custom_strategy_converter
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE(
        "Not implemented for this Strategy.",
        Strategy);
};

template <typename Strategy> struct tag
{
    BOOST_GEOMETRY_STATIC_ASSERT_FALSE(
        "Not implemented for this Strategy.",
        Strategy);
};


} // namespace services

}} // namespace strategies::closest_points

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_CLOSEST_POINTS_SERVICES_HPP
