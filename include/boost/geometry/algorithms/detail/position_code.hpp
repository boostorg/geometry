// Boost.Geometry

// Copyright (c) 2025 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_POSITION_CODE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_POSITION_CODE_HPP

#include <boost/geometry/algorithms/detail/direction_code.hpp>
#include <boost/geometry/strategies/detail/position_code.hpp>

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
template <typename PointA, typename PointB, typename Point, typename Strategy>
int get_position_code(PointA const& p1, PointB const& p2, Point const& point,
                      Strategy const& strategy)
{
    return strategies::detail::position_code(p1, p2, point, strategy);
}

} // namespace detail
#endif //DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_POSITION_CODE_HPP
