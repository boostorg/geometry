// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2017 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2017-2021.
// Modifications copyright (c) 2017-2021 Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SELF_TURN_POINTS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SELF_TURN_POINTS_HPP


#include <cstddef>


#include <boost/geometry/algorithms/detail/disjoint/box_box.hpp>
#include <boost/geometry/algorithms/detail/partition_lambda.hpp>
#include <boost/geometry/algorithms/detail/overlay/do_reverse.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_turns.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/point_order.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/strategies/detail.hpp>
#include <boost/geometry/strategies/relate/services.hpp>

#include <boost/geometry/util/condition.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace self_get_turn_points
{

struct no_interrupt_policy
{
    static bool const enabled = false;
    static bool const has_intersections = false;


    template <typename Range>
    static inline bool apply(Range const&)
    {
        return false;
    }
};

template <bool Reverse, typename TurnPolicy>
struct get_turns
{
    template <typename Geometry, typename Strategy, typename RobustPolicy, typename Turns, typename InterruptPolicy>
    static inline bool apply(
            Geometry const& geometry,
            Strategy const& strategy,
            RobustPolicy const& robust_policy,
            Turns& turns,
            InterruptPolicy& interrupt_policy,
            int source_index, bool skip_adjacent)
    {
        typedef model::box
            <
                typename geometry::robust_point_type
                <
                    typename geometry::point_type<Geometry>::type,
                    RobustPolicy
                >::type
            > box_type;

        // sectionalize in two dimensions to detect
        // all potential spikes correctly
        typedef geometry::sections<box_type, 2> sections_type;

        typedef std::integer_sequence<std::size_t, 0, 1> dimensions;

        sections_type sections;
        geometry::sectionalize<Reverse, dimensions>(geometry, robust_policy,
                                                    sections, strategy);

        auto visit_sections = [&](auto const& section1, auto const& section2)
        {
            if (detail::disjoint::disjoint_box_box(section1.bounding_box,
                                                   section2.bounding_box,
                                                   strategy)
                && ! section1.duplicate
                && ! section2.duplicate)
            {
                return true;
            }

            using section_type = std::decay_t<decltype(section1)>;

            // This will return false if interrupted
            return detail::get_turns::get_turns_in_sections
                    <
                        Geometry, Geometry,
                        Reverse, Reverse,
                        section_type, section_type,
                        TurnPolicy
                    >::apply(source_index, geometry, section1,
                             source_index, geometry, section2,
                             false, skip_adjacent,
                             strategy,
                             robust_policy,
                             turns, interrupt_policy);
        };

        auto expand_box = [&strategy](auto& box, auto const& section)
        {
            geometry::expand(box, section.bounding_box, strategy);
        };
        auto overlaps_box = [&strategy](auto const& box, auto const& section)
        {
            return ! detail::disjoint::disjoint_box_box(box, section.bounding_box, strategy);
        };

        partition_lambda
            <
                box_type
            >(sections, expand_box, overlaps_box, visit_sections);

        return ! interrupt_policy.has_intersections;
    }
};


}} // namespace detail::self_get_turn_points
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    bool Reverse,
    typename GeometryTag,
    typename Geometry,
    typename TurnPolicy
>
struct self_get_turn_points
{
};


template
<
    bool Reverse,
    typename Ring,
    typename TurnPolicy
>
struct self_get_turn_points
    <
        Reverse, ring_tag, Ring,
        TurnPolicy
    >
    : detail::self_get_turn_points::get_turns<Reverse, TurnPolicy>
{};


template
<
    bool Reverse,
    typename Box,
    typename TurnPolicy
>
struct self_get_turn_points
    <
        Reverse, box_tag, Box,
        TurnPolicy
    >
{
    template <typename Strategy, typename RobustPolicy, typename Turns, typename InterruptPolicy>
    static inline bool apply(
            Box const& ,
            Strategy const& ,
            RobustPolicy const& ,
            Turns& ,
            InterruptPolicy& ,
            int /*source_index*/,
            bool /*skip_adjacent*/)
    {
        return true;
    }
};


template
<
    bool Reverse,
    typename Polygon,
    typename TurnPolicy
>
struct self_get_turn_points
    <
        Reverse, polygon_tag, Polygon,
        TurnPolicy
    >
    : detail::self_get_turn_points::get_turns<Reverse, TurnPolicy>
{};


template
<
    bool Reverse,
    typename MultiPolygon,
    typename TurnPolicy
>
struct self_get_turn_points
    <
        Reverse, multi_polygon_tag, MultiPolygon,
        TurnPolicy
    >
    : detail::self_get_turn_points::get_turns<Reverse, TurnPolicy>
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


namespace resolve_strategy
{

template
<
    bool Reverse,
    typename AssignPolicy,
    typename Strategies,
    bool IsUmbrella = strategies::detail::is_umbrella_strategy<Strategies>::value
>
struct self_get_turn_points
{
    template
    <
        typename Geometry,
        typename RobustPolicy,
        typename Turns,
        typename InterruptPolicy
    >
    static inline void apply(Geometry const& geometry,
                             Strategies const& strategies,
                             RobustPolicy const& robust_policy,
                             Turns& turns,
                             InterruptPolicy& interrupt_policy,
                             int source_index,
                             bool skip_adjacent)
    {
        using turn_policy = detail::overlay::get_turn_info<AssignPolicy>;

        dispatch::self_get_turn_points
                <
                    Reverse,
                    typename tag<Geometry>::type,
                    Geometry,
                    turn_policy
                >::apply(geometry, strategies, robust_policy, turns, interrupt_policy,
                         source_index, skip_adjacent);
    }
};

template <bool Reverse, typename AssignPolicy, typename Strategy>
struct self_get_turn_points<Reverse, AssignPolicy, Strategy, false>
{
    template
    <
        typename Geometry,
        typename RobustPolicy,
        typename Turns,
        typename InterruptPolicy
    >
    static inline void apply(Geometry const& geometry,
                             Strategy const& strategy,
                             RobustPolicy const& robust_policy,
                             Turns& turns,
                             InterruptPolicy& interrupt_policy,
                             int source_index,
                             bool skip_adjacent)
    {
        using strategies::relate::services::strategy_converter;

        self_get_turn_points
            <
                Reverse,
                AssignPolicy,
                decltype(strategy_converter<Strategy>::get(strategy))
            >::apply(geometry,
                     strategy_converter<Strategy>::get(strategy),
                     robust_policy,
                     turns,
                     interrupt_policy,
                     source_index,
                     skip_adjacent);
    }
};


} // namespace resolve_strategy


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace self_get_turn_points
{

// Version where Reverse can be specified manually. TODO:
// can most probably be merged with self_get_turn_points::get_turn
template
<
    bool Reverse,
    typename AssignPolicy,
    typename Geometry,
    typename Strategy,
    typename RobustPolicy,
    typename Turns,
    typename InterruptPolicy
>
inline void self_turns(Geometry const& geometry,
                       Strategy const& strategy,
                       RobustPolicy const& robust_policy,
                       Turns& turns,
                       InterruptPolicy& interrupt_policy,
                       int source_index = 0,
                       bool skip_adjacent = false)
{
    concepts::check<Geometry const>();

    resolve_strategy::self_get_turn_points
            <
                Reverse, AssignPolicy, Strategy
            >::apply(geometry, strategy, robust_policy, turns, interrupt_policy,
                     source_index, skip_adjacent);
}

}} // namespace detail::self_get_turn_points
#endif // DOXYGEN_NO_DETAIL

/*!
    \brief Calculate self intersections of a geometry
    \ingroup overlay
    \tparam Geometry geometry type
    \tparam Turns type of intersection container
                (e.g. vector of "intersection/turn point"'s)
    \param geometry geometry
    \param strategy strategy to be used
    \param robust_policy policy to handle robustness issues
    \param turns container which will contain intersection points
    \param interrupt_policy policy determining if process is stopped
        when intersection is found
    \param source_index source index for generated turns
    \param skip_adjacent indicates if adjacent turns should be skipped
 */
template
<
    typename AssignPolicy,
    typename Geometry,
    typename Strategy,
    typename RobustPolicy,
    typename Turns,
    typename InterruptPolicy
>
inline void self_turns(Geometry const& geometry,
                       Strategy const& strategy,
                       RobustPolicy const& robust_policy,
                       Turns& turns,
                       InterruptPolicy& interrupt_policy,
                       int source_index = 0,
                       bool skip_adjacent = false)
{
    concepts::check<Geometry const>();

    static bool const reverse =  detail::overlay::do_reverse
        <
            geometry::point_order<Geometry>::value
        >::value;

    resolve_strategy::self_get_turn_points
            <
                reverse, AssignPolicy, Strategy
            >::apply(geometry, strategy, robust_policy, turns, interrupt_policy,
                     source_index, skip_adjacent);
}



}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SELF_TURN_POINTS_HPP
