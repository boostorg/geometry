// Boost.Geometry
//
// Copyright (c) 2026 Tinko Bartels.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_DETAIL_RING_TOPOLOGY_HPP
#define BOOST_GEOMETRY_STRATEGIES_DETAIL_RING_TOPOLOGY_HPP

#include <type_traits>
#include <boost/geometry/algorithms/area_result.hpp>
#include <boost/geometry/strategies/detail/topology.hpp>
#include <boost/geometry/util/math.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{

template <typename Point>
struct topological_ring_tag { using point_type = Point; };

struct ring_topology_result
{
    bool valid = false;
    bool clockwise = false;
    int direction = 0;
};

// An umbrella may select the ring metadata's measure independently of its
// ordinary area strategy, e.g. an exact rational for overlay intermediates.
template <typename Geometry, typename Strategy, typename = void>
struct ring_measure_type : geometry::area_result<Geometry, Strategy>
{};

template <typename Geometry, typename Strategy>
struct ring_measure_type<Geometry, Strategy,
                         std::void_t<typename Strategy::template ring_measure_type<Geometry>>>
{
    using type = typename Strategy::template ring_measure_type<Geometry>;
};

enum class ring_direction { negative, zero, positive, unordered };

// An area is a signed measure, not a degeneracy predicate. Some parenting
// shortcuts intentionally use the strict sign even with a tolerant strategy.
template <bool WithTolerance = true, typename Strategy, typename Measure>
inline ring_direction classify_ring_measure(Strategy const&, Measure const& value)
{
    if constexpr (WithTolerance && uses_overlay_tolerance<Strategy>)
    {
        if (geometry::math::equals(value, 0)) return ring_direction::zero;
    }
    return value > 0 ? ring_direction::positive
         : value < 0 ? ring_direction::negative
         : value == 0 ? ring_direction::zero : ring_direction::unordered;
}

}}}} // namespace boost::geometry::strategies::detail

#endif // BOOST_GEOMETRY_STRATEGIES_DETAIL_RING_TOPOLOGY_HPP
