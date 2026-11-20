// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Licensed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_GEOMETRY_STRATEGIES_DETAIL_CLIPPING_HPP
#define BOOST_GEOMETRY_STRATEGIES_DETAIL_CLIPPING_HPP

#include <boost/geometry/strategies/cartesian/clip.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{
namespace clipping_detail
{

template <typename Point, typename Strategy, typename Box>
inline auto get(Strategy const& strategy, Box const& box, int)
    -> decltype(strategy.template clipping<Point>(box))
{
    return strategy.template clipping<Point>(box);
}

template <typename Point, typename Strategy, typename Box>
inline strategy::intersection::liang_barsky<Box, Point>
get(Strategy const&, Box const&, long) { return {}; }

} // namespace clipping_detail

template <typename Point, typename Strategy, typename Box>
inline auto get_clipping(Strategy const& strategy, Box const& box)
{
    return clipping_detail::get<Point>(strategy, box, 0);
}

}}}} // namespace boost::geometry::strategies::detail

#endif
