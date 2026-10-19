// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2017-2022.
// Modifications copyright (c) 2019-2022, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_ALGORITHMS_DISSOLVE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_ALGORITHMS_DISSOLVE_HPP


#include <map>
#include <deque>

#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/size.hpp>
#include <boost/range/value_type.hpp>

#include <boost/geometry/algorithms/detail/dummy_geometries.hpp>

#include <boost/geometry/algorithms/detail/overlay/graph/assign_side_counts.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_turns.hpp>
#include <boost/geometry/algorithms/detail/overlay/self_turn_points.hpp>
#include <boost/geometry/algorithms/detail/overlay/overlay.hpp>

#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/enrichment_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/traversal_info.hpp>

#include <boost/geometry/algorithms/detail/overlay/enrich_intersection_points.hpp>
#include <boost/geometry/algorithms/detail/overlay/traverse.hpp>

#include <boost/geometry/algorithms/detail/overlay/add_rings.hpp>
#include <boost/geometry/algorithms/detail/overlay/assign_parents.hpp>
#include <boost/geometry/algorithms/detail/overlay/ring_properties.hpp>
#include <boost/geometry/algorithms/detail/overlay/select_rings.hpp>

#include <boost/geometry/algorithms/convert.hpp>
#include <boost/geometry/algorithms/correct.hpp>
#include <boost/geometry/algorithms/sym_difference.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/geometries/multi_polygon.hpp>

#include <boost/geometry/policies/robustness/segment_ratio.hpp>

#include <boost/geometry/extensions/algorithms/detail/overlay/dissolver.hpp>

namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace dissolve
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


template <typename Ring, typename GeometryOut, bool Reverse>
struct dissolve_ring
{
    // TODO: revise / reconsider this. 
    // - it gives currently the best output
    // - but for some cases, it should NOT be done
    // - it should be adapted to the new graph algorithm
    template <typename Turns>
    static void adapt_turns(Turns& turns)
    {
        for (auto& turn : turns)
        {
            for (auto& op : turn.operations)
            {
                if (op.operation == detail::overlay::operation_intersection)
                {
                    // Make all ii->uu, iu->uu, etc, basically handle most
                    // as if it is union
                    op.operation = detail::overlay::operation_union;
                    op.preference_index++;
                }
            }
        }
    }

    template
    <
        typename OutputIterator,
        typename Strategy, typename Visitor
    >
    static inline void apply_one(Ring const& input_ring,
                OutputIterator out,
                Strategy const& strategy,
                Visitor& visitor)
    {
        using point_type = typename point_type<Ring>::type;

        // Get the self-intersection points, including turns
        using turn_info = detail::overlay::traversal_turn_info
            <
                point_type,
                typename segment_ratio_type<point_type>::type
            >;

        std::deque<turn_info> turns;
        detail::dissolve::no_interrupt_policy policy;
        detail::self_get_turn_points::self_turns
            <
                Reverse,
                detail::overlay::assign_null_policy
            >(input_ring, strategy, turns, policy, 0, false);

        adapt_turns(turns);

        visitor.visit_turns(1, turns);

        if (boost::size(turns) == 0)
        {
            // No self-turns, then add original ring,
            // possibly reversing the order
            GeometryOut g;
            geometry::convert(input_ring, g);
            geometry::correct(g);
            *out++ = g;
        }

        std::map<signed_size_type, detail::overlay::cluster_info> clusters;

        // Enrich/traverse the polygons
        // Handle colocations, gathering clusters and (below) their properties.
        detail::overlay::handle_colocations(turns, clusters);

        enrich_intersection_points<Reverse, Reverse, overlay_dissolve>(turns,
                    clusters, input_ring, input_ring, strategy);

        detail::overlay::colocate_clusters(clusters, turns);

        detail::overlay::assign_side_counts
            <
                Reverse, Reverse, overlay_dissolve
            >(input_ring, input_ring, turns, clusters, strategy, visitor);

        detail::overlay::get_properties_ahead<Reverse, Reverse>(turns, clusters, input_ring, input_ring, strategy);

        std::deque<Ring> rings;
        std::map<ring_identifier, overlay::ring_turn_info> turn_info_per_ring;

        detail::overlay::traverse<Reverse, Reverse, Ring, Ring, overlay_dissolve>::apply
                (
                    input_ring, input_ring,
                    strategy,
                    turns, rings,
                    turn_info_per_ring,
                    clusters,
                    visitor
                );

        visitor.visit_turns(3, turns);
        visitor.visit_generated_rings(rings);

        detail::overlay::get_ring_turn_info<overlay_dissolve>(turn_info_per_ring, turns, clusters);

        using area_strategy_type = decltype(strategy.area(detail::dummy_ring()));
        using area_result_type = typename area_strategy_type::template result_type<point_type>::type;
        using properties = detail::overlay::ring_properties<point_type, area_result_type>;

        std::map<ring_identifier, properties> selected;

        detail::overlay::select_rings<overlay_dissolve>(input_ring, turn_info_per_ring, selected, strategy);

        {
            ring_identifier id(2, 0, -1);
            for (auto it = boost::begin(rings); it != boost::end(rings); ++it)
            {
                selected[id] = properties(*it, strategy);
                id.multi_index++;
            }
        }

        detail::overlay::assign_parents<overlay_dissolve>(input_ring,
            rings, selected, strategy);
        detail::overlay::add_rings<GeometryOut>(selected, input_ring, rings, out, strategy);
    }

    template
    <
        typename OutputIterator,
        typename Strategy, typename Visitor
    >
    static inline OutputIterator apply(Ring const& geometry,
                OutputIterator out,
                Strategy const& strategy,
                Visitor& visitor)
    {
        using multi_polygon = model::multi_polygon<GeometryOut>;
        multi_polygon step1;
        apply_one(geometry, std::back_inserter(step1), strategy, visitor);

        // Step 2: remove mutual overlap
        {
            multi_polygon step2; // TODO avoid this, output to "out", if possible
            detail::dissolver::dissolver_generic
                <
                    detail::dissolver::plusmin_policy
                >::apply(step1, step2, strategy);
            for (auto it = step2.begin(); it != step2.end(); ++it)
            {
                *out++ = *it;
            }
        }
        return out;
    }
};

template <typename Polygon, typename GeometryOut, bool Reverse>
struct dissolve_polygon
{
    typedef typename ring_type<Polygon>::type ring_type;

    template
    <
        typename OutputCollection,
        typename Strategy, typename Visitor
    >
    static inline void apply_ring(ring_type const& ring,
                OutputCollection& out,
                Strategy const& strategy,
                Visitor& visitor)
    {
        bool const orientation_ok = geometry::area(ring) >= 0;
        if (orientation_ok)
        {
            dissolve_ring<ring_type, GeometryOut, Reverse>
                    ::apply(ring, 
                            std::back_inserter(out), strategy, visitor);
        }
        else
        {
            // Apply the whole dissolve implementation reversed
            dissolve_ring<ring_type, GeometryOut, ! Reverse>
                    ::apply(ring, 
                            std::back_inserter(out), strategy, visitor);
        }
    }

    template
    <
        typename Rings,
        typename OutputCollection,
        typename Strategy, typename Visitor
    >
    static inline void apply_rings(Rings const& rings,
                OutputCollection& out,
                Strategy const& strategy,
                Visitor& visitor)
    {
        for (auto it = boost::begin(rings); it != boost::end(rings); ++it)
        {
            apply_ring(*it, out, strategy, visitor);
        }
    }

    template
    <
        typename OutputIterator,
        typename Strategy, typename Visitor
    >
    static inline OutputIterator apply(Polygon const& polygon,
                OutputIterator out,
                Strategy const& strategy,
                Visitor& visitor)
    {
        using multi_polygon = model::multi_polygon<GeometryOut>;

        // Handle exterior ring
        multi_polygon exterior_out;
        apply_ring(exterior_ring(polygon), 
                   exterior_out, strategy, visitor);

        // Dissolve all the (negative) interior rings into
        // a (positive) mulpolygon. Do this per interior ring and combine them.
        multi_polygon interior_out_per_ring;
        apply_rings(interior_rings(polygon), 
                   interior_out_per_ring, strategy, visitor);

        // Remove mutual overlap in the interior ring output
        multi_polygon interior_out;
        detail::dissolver::dissolver_generic
            <
                detail::dissolver::plusmin_policy
            >::apply(interior_out_per_ring, interior_out, strategy);

        // Subtract the interior rings from the output. Where interior rings
        // are partly or completely outside the polygon, sym_difference will
        // turn them into exterior rings. This is probably what most users will
        // expect - alternatively, difference could be used to have them pure
        // as interior rings only
        return detail::sym_difference::sym_difference_insert<GeometryOut>(
                    exterior_out, interior_out, out);
    }
};


}} // namespace detail::dissolve
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename Geometry,
    typename GeometryOut,
    bool Reverse,
    typename GeometryTag = typename tag<Geometry>::type,
    typename GeometryOutTag = typename tag<GeometryOut>::type
>
struct dissolve
    : not_implemented<GeometryTag, GeometryOutTag>
{};


template<typename Ring, typename RingOut, bool Reverse>
struct dissolve<Ring, RingOut, Reverse, ring_tag, ring_tag>
    : detail::dissolve::dissolve_ring<Ring, RingOut, Reverse>
{};


template<typename Polygon, typename PolygonOut, bool Reverse>
struct dissolve<Polygon, PolygonOut, Reverse, polygon_tag, polygon_tag>
    : detail::dissolve::dissolve_polygon<Polygon, PolygonOut, Reverse>
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


namespace resolve_strategy
{

template
<
    typename GeometryOut,
    typename Strategy,
    bool IsUmbrella = strategies::detail::is_umbrella_strategy<Strategy>::value
>
struct dissolve
{
    template
    <
        typename Geometry, typename OutputIterator,
        typename Visitor
    >
    static inline OutputIterator apply(Geometry const& geometry,
                                       OutputIterator out,
                                       Strategy const& strategy,
                                       Visitor& visitor)
    {
        return dispatch::dissolve
            <
                Geometry,
                GeometryOut,
                detail::overlay::do_reverse
                    <
                        geometry::point_order<Geometry>::value
                    >::value
            >::apply(geometry, out, strategy, visitor);
    }
};

template <typename GeometryOut, typename Strategy>
struct dissolve<GeometryOut, Strategy, false>
{
    template
    <
        typename Geometry, typename OutputIterator,
        typename Visitor
    >
    static inline OutputIterator apply(Geometry const& geometry,
                                       OutputIterator out,
                                       Strategy const& strategy,
                                       Visitor& visitor)
    {
        using strategies::relate::services::strategy_converter;

        return dispatch::dissolve
            <
                Geometry,
                GeometryOut,
                detail::overlay::do_reverse
                    <
                        geometry::point_order<Geometry>::value
                    >::value
            >::apply(geometry, out,
                     strategy_converter<Strategy>::get(strategy),
                     visitor);
    }
};

} // namespace resolve_strategy


/*!
    \brief Removes self intersections from a geometry
    \ingroup overlay
    \tparam Geometry geometry type
    \tparam OutputIterator type of intersection container
        (e.g. vector of "intersection/turn point"'s)
    \tparam Strategy type of a strategy
    \param geometry first geometry
    \param out output iterator getting dissolved geometry
    \param strategy a strategy
    \note Currently dissolve with a (multi)linestring does NOT remove internal
        overlap, it only tries to connect multiple line end-points.
        TODO: we should change this behaviour and add a separate "connect"
        algorithm, and let dissolve work like polygon.
 */
template
<
    typename GeometryOut,
    typename Geometry,
    typename OutputIterator,
    typename Strategy
>
inline OutputIterator dissolve_inserter(Geometry const& geometry,
                                        OutputIterator out,
                                        Strategy const& strategy)
{
    concepts::check<Geometry const>();
    concepts::check<GeometryOut>();

    detail::overlay::overlay_null_visitor visitor;

    return resolve_strategy::dissolve
        <
            GeometryOut, Strategy
        >::apply(geometry, out, strategy, visitor);
}

/*!
    \brief Removes self intersections from a geometry
    \ingroup overlay
    \tparam Geometry geometry type
    \tparam OutputIterator type of intersection container
        (e.g. vector of "intersection/turn point"'s)
    \param geometry first geometry
    \param out output iterator getting dissolved geometry
    \note Currently dissolve with a (multi)linestring does NOT remove internal
        overlap, it only tries to connect multiple line end-points.
        TODO: we should change this behaviour and add a separate "connect"
        algorithm, and let dissolve work like polygon.
 */
template
<
    typename GeometryOut,
    typename Geometry,
    typename OutputIterator
>
inline OutputIterator dissolve_inserter(Geometry const& geometry,
                                        OutputIterator out)
{
    typedef typename strategies::relate::services::default_strategy
        <
            Geometry, Geometry
        >::type strategy_type;

    return dissolve_inserter<GeometryOut>(geometry, out, strategy_type());
}


template
<
    typename Geometry,
    typename Collection,
    typename Strategy
>
inline void dissolve(Geometry const& geometry, Collection& output_collection,
                     Strategy const& strategy)
{
    concepts::check<Geometry const>();

    typedef typename boost::range_value<Collection>::type geometry_out;

    concepts::check<geometry_out>();

    detail::overlay::overlay_null_visitor visitor;

    resolve_strategy::dissolve
        <
            geometry_out, Strategy
        >::apply(geometry, 
                 std::back_inserter(output_collection),
                 strategy, visitor);
}

template
<
    typename Geometry,
    typename Collection
>
inline void dissolve(Geometry const& geometry, Collection& output_collection)
{
    typedef typename strategies::relate::services::default_strategy
        <
            Geometry, Geometry
        >::type strategy_type;

    dissolve(geometry, output_collection, strategy_type());
}



}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_ALGORITHMS_DISSOLVE_HPP
