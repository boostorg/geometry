// Boost.Geometry

// Copyright (c) 2025 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_DETAIL_POSITION_CODE_HPP
#define BOOST_GEOMETRY_STRATEGIES_DETAIL_POSITION_CODE_HPP

#include <boost/geometry/strategies/detail/direction_code.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{

namespace position_code_detail
{

template <typename PointA, typename PointB, typename Point, typename Strategy>
inline auto apply(PointA const& p1, PointB const& p2, Point const& point,
                  Strategy const& strategy, int)
    -> decltype(strategy.position_code(p1, p2, point))
{
    return strategy.position_code(p1, p2, point);
}

template <typename PointA, typename PointB, typename Point, typename Strategy>
inline int apply(PointA const& p1, PointB const& p2, Point const& point,
                 Strategy const& strategy, long)
{
    auto const side = get_side_strategy(strategy).apply(p1, p2, point);
    if (side == 1)
    {
        return 4;
    }
    else if (side == -1)
    {
        return 2;
    }

    auto const dir_code = strategies::detail::direction_code(p1, p2, point, strategy);
    if (dir_code == -1)
    {
        return 1;
    }
    else if (dir_code == 1)
    {
        return 3;
    }

    return 0;
}

} // namespace position_code_detail

// Counterclockwise ranks around the segment: backward 1, right 2,
// forward 3, left 4; 0 denotes an equal endpoint or degenerate segment.
template <typename PointA, typename PointB, typename Point, typename Strategy>
inline int position_code(PointA const& p1, PointB const& p2, Point const& point,
                         Strategy const& strategy)
{
    return position_code_detail::apply(p1, p2, point, strategy, 0);
}

}}}} // namespace boost::geometry::strategies::detail

#endif // BOOST_GEOMETRY_STRATEGIES_DETAIL_POSITION_CODE_HPP
