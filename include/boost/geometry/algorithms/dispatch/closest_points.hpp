// Boost.Geometry

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DISPATCH_CLOSEST_POINTS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DISPATCH_CLOSEST_POINTS_HPP


#include <boost/geometry/algorithms/not_implemented.hpp>

#include <boost/geometry/core/reverse_dispatch.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tag_cast.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/strategies/detail.hpp>
#include <boost/geometry/strategies/closest_points.hpp>
#include <boost/geometry/strategies/closest_points/services.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template
<
    typename Geometry1, typename Geometry2, typename Strategies,
    bool IsUmbrella = strategies::detail::is_umbrella_strategy<Strategies>::value,
    bool Reverse = reverse_dispatch<Geometry1, Geometry2>::value
>
struct closest_points_strategy_type
{
    typedef decltype(std::declval<Strategies>().closest_points(std::declval<Geometry1>(), std::declval<Geometry2>())) type;
};

template <typename Geometry1, typename Geometry2, typename Strategies>
struct closest_points_strategy_type<Geometry1, Geometry2, Strategies, true, true>
    : closest_points_strategy_type<Geometry2, Geometry1, Strategies, true, false>
{};


template
<
    typename Geometry1, typename Geometry2, 
    typename Strategy = typename strategies::closest_points::services::default_strategy
        <
            Geometry1, Geometry2
        >::type,
    typename Tag1 = typename tag_cast
        <
            typename tag<Geometry1>::type,
            segment_tag,
            box_tag,
            linear_tag,
            areal_tag
        >::type,
    typename Tag2 = typename tag_cast
        <
            typename tag<Geometry2>::type,
            segment_tag,
            box_tag,
            linear_tag,
            areal_tag
        >::type,
    typename StrategyTag = typename strategy::closest_points::services::tag
        <
            typename closest_points_strategy_type<Geometry1, Geometry2, Strategy>::type
        >::type,
    bool Reverse = reverse_dispatch<Geometry1, Geometry2>::type::value
>
struct closest_points : not_implemented<Tag1, Tag2>
{};



} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DISPATCH_CLOSEST_POINTS_HPP
