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


#include <boost/geometry/algorithms/detail/point_on_border.hpp>
#include <boost/geometry/algorithms/detail/within/implementation.hpp>
#include <boost/geometry/strategies/detail/ring_properties.hpp>


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
        this->area = strategies::detail::get_ring_topology(strategy).signed_measure(ring_or_box);
        valid = geometry::point_on_border(this->point, ring_or_box);
    }

    inline area_type get_area() const
    {
        // Reversal changes the orientation of the stored signed measure only;
        // it neither recomputes coordinates nor decides whether a ring is degenerate.
        return reversed ? -area : area;
    }
};

template <typename Point, typename WorkingPoint>
struct ring_properties<Point, strategies::detail::topological_ring_tag<WorkingPoint>>
    : strategies::detail::exact_ring_properties<WorkingPoint>
{
    using base = strategies::detail::exact_ring_properties<WorkingPoint>;
    using base::base;
};

}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_RING_PROPERTIES_HPP
