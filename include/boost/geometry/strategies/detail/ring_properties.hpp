// Boost.Geometry
//
// Copyright (c) 2026 Tinko Bartels.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_DETAIL_RING_PROPERTIES_HPP
#define BOOST_GEOMETRY_STRATEGIES_DETAIL_RING_PROPERTIES_HPP

#include <type_traits>
#include <utility>
#include <vector>
#include <boost/geometry/algorithms/detail/ring_identifier.hpp>
#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/area_result.hpp>
#include <boost/geometry/algorithms/detail/expand_by_epsilon.hpp>
#include <boost/geometry/util/math.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{

namespace ring_properties_detail
{

template <typename...>
using void_t = void;

} // namespace ring_properties_detail

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
struct ring_measure_type<Geometry, Strategy, ring_properties_detail::void_t
    <typename Strategy::template ring_measure_type<Geometry>>>
{
    using type = typename Strategy::template ring_measure_type<Geometry>;
};

// These are signed-measure policies, not tests for geometric degeneracy: a
// self-intersecting ring may have zero signed measure without being collinear.
template <typename Strategy>
class area_ring_topology
{
public:
    explicit area_ring_topology(Strategy const& strategy)
        : m_strategy(strategy)
    {}

    template <typename RingOrBox>
    auto signed_measure(RingOrBox const& ring_or_box) const
        -> typename geometry::area_result<RingOrBox, Strategy>::type
    {
        return geometry::area(ring_or_box, m_strategy);
    }

    template <typename Measure>
    Measure absolute_measure(Measure const& value) const
    {
        return geometry::math::abs(value);
    }

    template <typename Measure1, typename Measure2>
    bool measure_less(Measure1 const& a, Measure2 const& b) const
    {
        return a < b;
    }

    // The raw sign and the tolerance-based tests answer different questions.
    // In particular, tiny positive measures still count in the union shortcut.
    template <typename Measure>
    int measure_sign(Measure const& value) const
    {
        return value > 0 ? 1 : value < 0 ? -1 : 0;
    }

    template <typename Measure>
    bool measure_is_positive(Measure const& value) const
    {
        return geometry::math::larger(value, 0);
    }

    template <typename Measure>
    bool measure_is_negative(Measure const& value) const
    {
        return geometry::math::smaller(value, 0);
    }

    template <typename Measure>
    bool measure_is_zero(Measure const& value) const
    {
        return geometry::math::equals(value, 0);
    }

    template <typename Box>
    void expand_envelope(Box& box) const
    {
        geometry::detail::expand_by_epsilon(box);
    }

private:
    Strategy const& m_strategy;
};

namespace ring_properties_detail
{

template <typename Strategy>
inline auto get_ring_topology(Strategy const& strategy, int)
    -> decltype(strategy.ring_topology())
{
    return strategy.ring_topology();
}

template <typename Strategy>
inline area_ring_topology<Strategy> get_ring_topology(Strategy const& strategy, long)
{
    return area_ring_topology<Strategy>(strategy);
}

} // namespace ring_properties_detail

template <typename Strategy>
inline decltype(auto) get_ring_topology(Strategy const& strategy)
{
    return ring_properties_detail::get_ring_topology(strategy, 0);
}

template <typename Point>
struct exact_ring_properties
{
    using point_type = Point;
    using topology_only = std::true_type;

    bool valid = false;
    // Selection's range_in_geometry may skip the actual first range vertex.
    Point point;
    // Parenting uses a surviving boundary sample, excluding canceled spikes.
    Point boundary_point;
    bool clockwise = false;
    int direction = 0;
    bool reversed = false;
    bool discarded = false;
    ring_identifier parent;
    std::vector<ring_identifier> children;

    exact_ring_properties() = default;

    template <typename RingOrBox, typename Strategy>
    exact_ring_properties(RingOrBox const& geometry, Strategy const& strategy)
    {
        auto const topology = get_ring_topology(strategy);
        auto const result = topology.analyze(geometry, boundary_point);
        valid = result.valid && topology.representative(geometry, point);
        clockwise = result.clockwise;
        direction = result.direction;
    }

    int effective_direction() const { return reversed ? -direction : direction; }
};

namespace ring_properties_detail
{
template <typename Properties, typename = void>
struct is_topological : std::false_type {};

template <typename Properties>
struct is_topological<Properties, void_t<typename Properties::topology_only>> : std::true_type {};
}

}}}} // namespace boost::geometry::strategies::detail

#endif // BOOST_GEOMETRY_STRATEGIES_DETAIL_RING_PROPERTIES_HPP
