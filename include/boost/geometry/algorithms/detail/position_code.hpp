// Boost.Geometry

// Copyright (c) 2025 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_POSITION_CODE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_POSITION_CODE_HPP

#include <boost/geometry/strategies/detail/direction_code.hpp>
#include <boost/geometry/strategies/side.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

// Position coding of the point with respect to a segment.
// This is a combination of side and direction_code.
// It is counter clockwise from the segment.
// (because polygons are on the right side of a segment, and this way
// we can walk through the ranks ascending.
// 
//        3
//        |
//   4    *    2    *: p2
//        |
//        1
//        ^         ^: p1
template <typename PointA, typename PointB, typename Point, typename SideStrategy>
int get_position_code(PointA const& p1, PointB const& p2, Point const& point,
                      SideStrategy const& side_strategy)
{
    auto const side = side_strategy.apply(p1, p2, point);
    if (side == strategy::side::left) return 4;
    if (side == strategy::side::right) return 2;

    int const direction = strategies::detail::direction_code(p1, p2, point, side_strategy);
    return direction == -1 ? 1 : direction == 1 ? 3 : 0;
}

} // namespace detail
#endif //DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_POSITION_CODE_HPP
