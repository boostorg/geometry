// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Licensed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_GEOMETRY_STRATEGIES_DETAIL_CLIPPING_HPP
#define BOOST_GEOMETRY_STRATEGIES_DETAIL_CLIPPING_HPP

#include <type_traits>

#include <boost/geometry/strategies/cartesian/clip.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{
template <typename Point, typename Strategy, typename Box>
inline auto get_clipping(Strategy const& strategy, Box const& box)
{
    auto const hook = [&](auto const& s) -> decltype(s.template clipping<Point>(box))
    { return s.template clipping<Point>(box); };
    if constexpr (std::is_invocable_v<decltype(hook), Strategy const&>)
        return hook(strategy);
    else
        return strategy::intersection::liang_barsky<Box, Point>{};
}

}}}} // namespace boost::geometry::strategies::detail

#endif
