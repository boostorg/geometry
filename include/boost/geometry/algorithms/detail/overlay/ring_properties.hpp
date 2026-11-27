// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2017-2022.
// Modifications copyright (c) 2017-2022 Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_RING_PROPERTIES_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_RING_PROPERTIES_HPP


#include <type_traits>
#include <vector>

#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/detail/ring_identifier.hpp>
#include <boost/geometry/algorithms/detail/point_on_border.hpp>
#include <boost/geometry/strategies/detail/ring_topology.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{

template <typename Point, typename AreaType>
struct ring_properties
{
    using point_type = Point;
    using area_type = AreaType;

    bool valid;

    // Filled by "select_rings"
    Point point;
    area_type area;

    // Filled by "update_ring_selection"
    bool reversed;

    // Filled/used by "assign_rings"
    bool discarded;
    ring_identifier parent;
    area_type parent_area;
    std::vector<ring_identifier> children;

    inline ring_properties()
        : valid(false)
        , area(area_type())
        , reversed(false)
        , discarded(false)
        , parent_area(-1)
    {}

    template <typename RingOrBox, typename Strategy>
    inline ring_properties(RingOrBox const& ring_or_box, Strategy const& strategy)
        : reversed(false)
        , discarded(false)
        , parent_area(-1)
    {
        this->area = geometry::area(ring_or_box, strategy);
        valid = geometry::point_on_border(this->point, ring_or_box);
    }

    inline area_type get_area() const
    {
        // Reversal changes the orientation of the stored signed measure only;
        // it neither recomputes coordinates nor decides whether a ring is degenerate.
        return reversed ? -area : area;
    }
};

template <typename OutputPoint, typename Point>
struct ring_properties<OutputPoint, strategies::detail::topological_ring_tag<Point>>
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

    ring_properties() = default;

    template <typename RingOrBox, typename Strategy>
    ring_properties(RingOrBox const& geometry, Strategy const& strategy)
    {
        auto const topology = strategy.ring_topology();
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
struct is_topological<Properties, std::void_t<typename Properties::topology_only>> : std::true_type {};
} // namespace ring_properties_detail

template <bool Reverse, typename Rings, typename RingMap, typename Strategy>
void insert_traversed_rings(Rings const& rings, RingMap& selected, Strategy const& strategy)
{
    ring_identifier id(2, 0, -1);
    for (auto const& ring : rings)
    {
        auto& properties = selected.try_emplace(id, ring, strategy).first->second;
        properties.reversed = Reverse;
        ++id.multi_index;
    }
}

}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_RING_PROPERTIES_HPP
