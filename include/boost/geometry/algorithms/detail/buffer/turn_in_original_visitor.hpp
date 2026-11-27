// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2016-2023.
// Modifications copyright (c) 2016-2023 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_TURN_IN_ORIGINAL_VISITOR
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_TURN_IN_ORIGINAL_VISITOR

#include <type_traits>

#include <boost/core/ignore_unused.hpp>
#include <boost/range/size.hpp>

#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/algorithms/detail/buffer/buffer_policies.hpp>
#include <boost/geometry/algorithms/detail/disjoint/interface.hpp>
#include <boost/geometry/algorithms/expand.hpp>
#include <boost/geometry/strategies/agnostic/point_in_poly_winding.hpp>
#include <boost/geometry/strategies/buffer.hpp>
#include <boost/geometry/util/math.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace buffer
{

// A winding strategy can opt into section pruning. Without an axis there
// is no guarantee that skipping any section preserves its winding count.
template <typename Winding, typename = void>
struct winding_scan_axis : std::integral_constant<int, -1> {};

template <typename Winding>
struct winding_scan_axis<Winding, std::void_t<decltype(Winding::buffer_scan_axis)>>
    : std::integral_constant<int, Winding::buffer_scan_axis> {};


template <typename Strategy>
struct original_get_box
{
    explicit original_get_box(Strategy const& strategy)
        : m_strategy(strategy)
    {}

    template <typename Box, typename Original>
    inline void apply(Box& total, Original const& original) const
    {
        geometry::expand(total, original.m_box, m_strategy);
    }

    Strategy const& m_strategy;
};

template <typename Strategy>
struct original_overlaps_box
{
    explicit original_overlaps_box(Strategy const& strategy)
        : m_strategy(strategy)
    {}

    template <typename Box, typename Original>
    inline bool apply(Box const& box, Original const& original) const
    {
        return ! detail::disjoint::disjoint_box_box(box, original.m_box,
                                                    m_strategy);
    }

    Strategy const& m_strategy;
};

struct include_turn_policy
{
    template <typename Turn>
    static inline bool apply(Turn const& turn)
    {
        return turn.is_traversable;
    }
};

template <typename Strategy>
struct turn_in_original_overlaps_box
{
    explicit turn_in_original_overlaps_box(Strategy const& strategy)
        : m_strategy(strategy)
    {}

    template <typename Box, typename Turn>
    inline bool apply(Box const& box, Turn const& turn) const
    {
        if (! turn.is_traversable || turn.within_original)
        {
            // Skip all points already processed
            return false;
        }

        if constexpr (strategies::relate::services::exact_predicates<Strategy>::value)
            return turn_covered_by(turn, box, m_strategy);
        else
            return ! geometry::detail::disjoint::disjoint_point_box(turn.point, box, m_strategy);
    }

    Strategy const& m_strategy;
};

//! Check if specified is in range of specified iterators
//! Return value of strategy (true if we can bail out)
template
<
    typename Strategy,
    typename State,
    typename Point,
    typename Iterator
>
inline bool point_in_range(Strategy& strategy, State& state,
        Point const& point, Iterator begin, Iterator end)
{
    boost::ignore_unused(strategy);

    Iterator it = begin;
    for (Iterator previous = it++; it != end; ++previous, ++it)
    {
        if (! strategy.apply(point, *previous, *it, state))
        {
            // We're probably on the boundary
            return false;
        }
    }
    return true;
}

template
<
    std::size_t Axis,
    typename Strategy,
    typename State,
    typename Point,
    typename CoordinateType,
    typename Iterator
>
inline bool point_in_section(Strategy& strategy, State& state,
        Point const& point, CoordinateType const& value,
        Iterator begin, Iterator end,
        int direction)
{
    if (direction == 0)
    {
        // Not a monotonic section, or no change along the scan axis.
        return point_in_range(strategy, state, point, begin, end);
    }

    // We're in a monotonic section along the winding axis.
    Iterator it = begin;

    for (Iterator previous = it++; it != end; ++previous, ++it)
    {
        // Depending on sections.direction we can quit for this section
        CoordinateType const previous_value = geometry::get<Axis>(*previous);

        // The stock winding strategy uses math::equals (epsilon),
        // counting half a crossing per segment touching the scan line at a vertex.
        // A strict comparison could skip the other half, so only bail out
        // when the previous vertex is beyond the scan line by more than epsilon.
        if (((direction == 1 && value < previous_value)
             || (direction == -1 && value > previous_value))
            && !math::equals(value, previous_value))
        {
            return true;
        }

        if (! strategy.apply(point, *previous, *it, state))
        {
            // We're probably on the boundary
            return false;
        }
    }
    return true;
}

template <typename Winding, typename Point, typename Box, typename Enclosure>
inline auto section_may_contribute(Winding const& winding, Point const& point,
                                   Box const& box, Enclosure const& enclosure, int)
    -> decltype(winding.section_may_contribute(point, box, enclosure))
{
    return winding.section_may_contribute(point, box, enclosure);
}

template <typename Winding, typename Point, typename Box, typename Enclosure>
inline auto section_may_contribute(Winding const& winding, Point const& point,
                                   Box const& box, Enclosure const&, long)
    -> decltype(winding.section_may_contribute(point, box))
{
    return winding.section_may_contribute(point, box);
}

template <typename Winding, typename State, typename Point, typename Section, typename Ring,
          typename Enclosure>
inline auto visit_original_section(Winding const& winding, State& state, Point const& point,
                                   Section const& section, Ring const& ring,
                                   Enclosure const& enclosure, int)
    -> decltype(section_may_contribute(winding, point, section.bounding_box, enclosure, 0))
{
    return !section_may_contribute(winding, point, section.bounding_box, enclosure, 0)
        || point_in_range(winding, state, point, ring.begin() + section.begin_index,
                          ring.begin() + section.end_index + 1);
}

template <typename Winding, typename State, typename Point, typename Section, typename Ring,
          typename Enclosure>
inline bool visit_original_section(Winding const& winding, State& state, Point const& point,
                                   Section const& section, Ring const& ring, Enclosure const&, long)
{
    // Preserve the stock winding strategies' tolerance at section endpoints.
    constexpr auto axis = winding_scan_axis<Winding>::value;
    auto const value = geometry::get<axis>(point);
    return !(value >= geometry::get<min_corner, axis>(section.bounding_box)
          && value <= geometry::get<max_corner, axis>(section.bounding_box))
        || point_in_section<axis>(winding, state, point, value, ring.begin() + section.begin_index,
                                  ring.begin() + section.end_index + 1, section.directions[0]);
}

template <typename Point, typename Original, typename PointInGeometryStrategy,
          typename Enclosure = no_turn_enclosure>
inline int point_in_original(Point const& point, Original const& original,
                             PointInGeometryStrategy const& strategy, Enclosure const& enclosure = {})
{
    typename PointInGeometryStrategy::state_type state;

    if constexpr (winding_scan_axis<PointInGeometryStrategy>::value >= 0)
    {
        // On short rings, walking the sections costs more than a full scan.
        if (!original.m_sections.empty()
            && boost::size(original.m_ring) - boost::size(original.m_sections) >= 16)
        {
            for (auto const& section : original.m_sections)
            {
                if (!section.duplicate && section.begin_index < section.end_index
                    && !visit_original_section(strategy, state, point, section,
                                               original.m_ring, enclosure, 0))
                {
                    break;
                }
            }
            return strategy.result(state);
        }
    }

    point_in_range(strategy, state, point, original.m_ring.begin(), original.m_ring.end());
    return strategy.result(state);
}


template <typename Turns, typename Strategy>
class turn_in_original_visitor
{
public:
    turn_in_original_visitor(Turns& turns, Strategy const& strategy)
        : m_mutable_turns(turns)
        , m_strategy(strategy)
    {}

    template <typename Turn, typename Original>
    inline bool apply(Turn const& turn, Original const& original)
    {
        if (boost::empty(original.m_ring))
        {
            // Skip empty rings
            return true;
        }

        if (! turn.is_traversable || turn.within_original)
        {
            // Skip all points already processed
            return true;
        }

        bool const outside = [&]
        {
            if constexpr (strategies::relate::services::exact_predicates<Strategy>::value)
                return !turn_covered_by(turn, original.m_box, m_strategy);
            else
                return geometry::disjoint(turn.point, original.m_box, m_strategy);
        }();
        if (outside)
        {
            // Skip all disjoint
            return true;
        }

        int const code = point_in_original(turn.point, original,
                                           m_strategy.relate(turn.point, original.m_ring),
                                           turn.enclosure);

        if (code == -1)
        {
            return true;
        }

        Turn& mutable_turn = m_mutable_turns[turn.turn_index];

        if (code == 0)
        {
            // On border of original: always discard
            mutable_turn.is_traversable = false;
        }

        // Point is inside an original ring
        if (original.m_is_interior)
        {
            mutable_turn.count_in_original--;
        }
        else if (original.m_has_interiors)
        {
            mutable_turn.count_in_original++;
        }
        else
        {
            // It is an exterior ring and there are no interior rings.
            // Then we are completely ready with this turn
            mutable_turn.within_original = true;
            mutable_turn.count_in_original = 1;
        }

        return true;
    }

private :
    Turns& m_mutable_turns;
    Strategy const& m_strategy;
};


}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_TURN_IN_ORIGINAL_VISITOR
