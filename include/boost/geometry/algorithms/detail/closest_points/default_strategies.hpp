// Boost.Geometry

// Copyright (c) 2019, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_CLOSEST_POINTS_DEFAULT_STRATEGIES_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_CLOSEST_POINTS_DEFAULT_STRATEGIES_HPP

#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tag_cast.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/reverse_dispatch.hpp>

#include <boost/geometry/strategies/closest_points.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace closest_points
{



// Helper metafunction for default strategy retrieval
template
<
    typename Geometry1,
    typename Geometry2 = Geometry1,
    typename Tag1 = typename tag_cast
        <
            typename tag<Geometry1>::type, pointlike_tag
        >::type,
    typename Tag2 = typename tag_cast
        <
            typename tag<Geometry2>::type, pointlike_tag
        >::type,
    bool Reverse = geometry::reverse_dispatch<Geometry1, Geometry2>::type::value
>
struct default_strategy
    : strategy::closest_points::services::default_strategy
          <
              point_tag, segment_tag,
              typename point_type<Geometry1>::type,
              typename point_type<Geometry2>::type
          >
{};

template
<
    typename Geometry1,
    typename Geometry2,
    typename Tag1,
    typename Tag2
>
struct default_strategy<Geometry1, Geometry2, Tag1, Tag2, true>
    : default_strategy<Geometry2, Geometry1, Tag2, Tag1, false>
{};


template <typename Pointlike1, typename Pointlike2>
struct default_strategy
    <
        Pointlike1, Pointlike2,
        pointlike_tag, pointlike_tag, false
    > : strategy::closest_points::services::default_strategy
          <
              point_tag, point_tag,
              typename point_type<Pointlike1>::type,
              typename point_type<Pointlike2>::type
          >
{};


template <typename Pointlike, typename Box>
struct default_strategy<Pointlike, Box, pointlike_tag, box_tag, false>
    : strategy::closest_points::services::default_strategy
          <
              point_tag, box_tag,
              typename point_type<Pointlike>::type,
              typename point_type<Box>::type
          >
{};


template <typename Box1, typename Box2>
struct default_strategy<Box1, Box2, box_tag, box_tag, false>
    : strategy::closest_points::services::default_strategy
          <
              box_tag, box_tag,
              typename point_type<Box1>::type,
              typename point_type<Box2>::type
          >
{};

template <typename Linear, typename Box>
struct default_strategy<Linear, Box, segment_tag, box_tag, false>
    : strategy::closest_points::services::default_strategy
          <
              segment_tag, box_tag,
              typename point_type<Linear>::type,
              typename point_type<Box>::type
          >
{};

template <typename Linear, typename Box>
struct default_strategy<Linear, Box, linear_tag, box_tag, false>
    : strategy::closest_points::services::default_strategy
          <
              segment_tag, box_tag,
              typename point_type<Linear>::type,
              typename point_type<Box>::type
          >
{};



// Helper metafunction for default point-segment strategy retrieval
template <typename Geometry1, typename Geometry2, typename Strategy>
struct default_ps_strategy
    : strategy::closest_points::services::default_strategy
          <
              point_tag, segment_tag,
              typename point_type<Geometry1>::type,
              typename point_type<Geometry2>::type,
              typename cs_tag<typename point_type<Geometry1>::type>::type,
              typename cs_tag<typename point_type<Geometry2>::type>::type,
              Strategy
          >
{};



}} // namespace detail::closest_points
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_CLOSEST_POINTS_DEFAULT_STRATEGIES_HPP
