// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2012-2014 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_BUFFER_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_BUFFER_HPP

#include <boost/geometry/strategies/agnostic/buffer_side.hpp>
#include <boost/geometry/strategies/agnostic/buffer_join_miter.hpp>
#include <boost/geometry/strategies/agnostic/buffer_join_round.hpp>
#include <boost/geometry/strategies/agnostic/buffer_join_round_by_divide.hpp>
#include <boost/geometry/strategies/agnostic/buffer_distance_symmetric.hpp>
#include <boost/geometry/strategies/agnostic/buffer_distance_asymmetric.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace buffer
{

/*

   A Buffer-join strategy gets 4 input points.
   On the two consecutive segments s1 and s2 (joining at vertex v):

   The lines from parallel at s1, s2 (at buffer-distance) end/start
   in two points perpendicular to the segments: p1 and p2.
   These parallel lines interesct in point ip

             (s2)
              |
              |
              ^
              |
        (p2)  |(v)
        *     +----<--- (s1)

        x(ip) *(p1)


    So, in clockwise order:
        v : vertex point
        p1: perpendicular on left side of segment1<1> (perp1)
        ip: intersection point
        p2: perpendicular on left side of segment2<0> (perp2)
*/




}} // namespace strategy::buffer


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_BUFFER_HPP
