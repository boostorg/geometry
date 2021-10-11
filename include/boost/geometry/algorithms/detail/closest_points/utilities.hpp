// Boost.Geometry

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_CLOSEST_POINTS_UTILITIES_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_CLOSEST_POINTS_UTILITIES_HPP

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
        
}} // namespace detail::closest_points

}} // namespace boost::geometry

#endif //BOOST_GEOMETRY_ALGORITHMS_DETAIL_CLOSEST_POINTS_UTILITIES_HPP