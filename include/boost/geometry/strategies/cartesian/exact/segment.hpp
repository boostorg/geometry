// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Licensed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_SEGMENT_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_SEGMENT_HPP

#include <boost/geometry/strategies/cartesian/exact/filtered_segment.hpp>
#include <boost/geometry/strategies/cartesian/exact/native_predicates.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

template <typename Coordinate>
struct segment_intersection
{
    using cs_tag = cartesian_tag;
    struct direction_only {};

    template <typename Range1, typename Range2, typename Policy>
    static typename Policy::return_type apply(Range1 const& p, Range2 const& q, Policy const&)
    {
        using selected = segment_endpoint_policy<Policy, Coordinate>;
        return cartesian_segment_events::classify<false, selected>(
            p.at(0), p.at(1), q.at(0), q.at(1),
            [](auto const& a, auto const& b, auto const& point) { return static_orientation<Coordinate>(a, b, point); },
            [](auto const& a, auto const& b) { return static_equal_points<Coordinate>(a, b); },
            [](auto const& a, auto const& b) { return collinear_segment_event<selected, Coordinate>(a, b); },
            [](int) { return direction_only{}; });
    }
};

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian
#endif
