// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_FILTERED_SEGMENT_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_FILTERED_SEGMENT_HPP

#include <boost/geometry/algorithms/detail/convert_point_to_point.hpp>
#include <boost/geometry/strategies/cartesian/detail/segment_events.hpp>
#include <boost/geometry/strategies/cartesian/exact/segment_position.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{
struct copy_segment_endpoint
{
    template <typename Source, typename Destination>
    static void apply(Source const& source, Destination& destination)
    { geometry::detail::conversion::convert_point_to_point(source, destination); }
};

template <typename Policy, typename Coordinate>
using segment_endpoint_policy = cartesian_segment_events::endpoint_policy
    <Policy, segment_position<Coordinate>, copy_segment_endpoint>;

template <typename Selected, typename Coordinate, typename A, typename B>
typename Selected::return_type collinear_segment_event(A const& a, B const& b)
{
    return cartesian_segment_events::collinear<Selected, segment_position<Coordinate>, segment_position_tag>(
        a, b, [](auto const& x, auto const& y) { return static_equal_points<Coordinate>(x, y); });
}

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian
#endif
