// Boost.Geometry

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_CLOSEST_POINTS_UTILITIES_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_CLOSEST_POINTS_UTILITIES_HPP

#include <boost/geometry/strategies/distance.hpp>

namespace boost { namespace geometry
{

namespace detail { namespace closest_points
{

struct set_segment_from_points
{
    template <typename Point, typename Segment>
    static inline void apply(Point const& p1, Point const& p2, Segment& segment)
    {
        set<0,0>(segment, get<0>(p1));
        set<0,1>(segment, get<1>(p1));
        set<1,0>(segment, get<0>(p2));
        set<1,1>(segment, get<1>(p2));
    }
};


struct swap_segment_points
{
    template <typename Segment>
    static inline void apply(Segment& segment)
    {
        auto temp = get<0,0>(segment);
        set<0,0>(segment, get<1,0>(segment));
        set<1,0>(segment, temp);
        temp = get<0,1>(segment);
        set<0,1>(segment, get<1,1>(segment));
        set<1,1>(segment, temp);
    }
};
        

template <typename Geometry1, typename Geometry2, typename Strategies>
using strategy_t = decltype(
    std::declval<Strategies>().comparable_distance(std::declval<Geometry1>(), std::declval<Geometry2>()));


template <typename Geometry1, typename Geometry2, typename Strategies>
using creturn_t = typename strategy::distance::services::return_type
    <
        strategy_t<Geometry1, Geometry2, Strategies>,
        typename point_type<Geometry1>::type,
        typename point_type<Geometry2>::type
    >::type;

}} // namespace detail::closest_points

}} // namespace boost::geometry

#endif //BOOST_GEOMETRY_ALGORITHMS_DETAIL_CLOSEST_POINTS_UTILITIES_HPP