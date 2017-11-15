// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2017.
// Modifications copyright (c) 2017, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_ALGORITHMS_DISSOLVE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_ALGORITHMS_DISSOLVE_HPP


#include <map>
#include <vector>

#include <boost/range.hpp>

#include <boost/geometry/algorithms/detail/overlay/get_turns.hpp>
#include <boost/geometry/algorithms/detail/overlay/self_turn_points.hpp>
#include <boost/geometry/algorithms/detail/overlay/overlay.hpp>

#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/enrichment_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/traversal_info.hpp>

#include <boost/geometry/algorithms/detail/point_on_border.hpp>

#include <boost/geometry/algorithms/detail/overlay/enrich_intersection_points.hpp>
#include <boost/geometry/algorithms/detail/overlay/traverse.hpp>

#include <boost/geometry/algorithms/detail/overlay/add_rings.hpp>
#include <boost/geometry/algorithms/detail/overlay/assign_parents.hpp>
#include <boost/geometry/algorithms/detail/overlay/ring_properties.hpp>
#include <boost/geometry/algorithms/detail/overlay/select_rings.hpp>

#include <boost/geometry/algorithms/convert.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/strategies/intersection.hpp>


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


template<typename Geometry>
class backtrack_for_dissolve
{
public :
    typedef detail::overlay::backtrack_state state_type;

    template
        <
            typename Operation,
            typename Rings,
            typename Turns,
            typename IntersectionStrategy,
            typename RobustPolicy,
            typename Visitor
        >
    static inline void apply(std::size_t size_at_start,
                Rings& rings,
                typename boost::range_value<Rings>::type& ring,
                Turns& turns,
                typename boost::range_value<Turns>::type const& /*turn*/,
                Operation& operation,
                detail::overlay::traverse_error_type,
                Geometry const& ,
                Geometry const& ,
                IntersectionStrategy const& ,
                RobustPolicy const& ,
                state_type& state,
                Visitor const& /*visitor*/
                )
    {
        state.m_good = false;

        // Make bad output clean
        rings.resize(size_at_start);
        ring.clear();

        // Reject this as a starting point
        operation.visited.set_rejected();

        // And clear all visit info
        clear_visit_info(turns);
    }
};

template <typename Geometry, typename GeometryOut>
struct dissolve_ring_or_polygon
{
    template <typename Turns>
    static inline void clear(Turns& turns)
    {
        typedef typename boost::range_value<Turns>::type turn_type;

        for (typename boost::range_iterator<Turns>::type
            it = boost::begin(turns);
            it != boost::end(turns);
            ++it)
        {
            turn_type& turn = *it;
            turn.discarded = false;
            turn.cluster_id = -1;
            turn.has_colocated_both = false;
            turn.switch_source = false;
            turn.touch_only = false;
        }

        clear_visit_info(turns);
    }


    template
    <
        typename RescalePolicy, typename OutputIterator,
        typename Strategy, typename Visitor
    >
    static inline OutputIterator apply(Geometry const& geometry,
                RescalePolicy const& rescale_policy,
                OutputIterator out,
                Strategy const& strategy,
                Visitor& visitor)
    {
        typedef typename point_type<Geometry>::type point_type;

        // Get the self-intersection points, including turns
        typedef detail::overlay::traversal_turn_info
            <
                point_type,
                typename segment_ratio_type<point_type, RescalePolicy>::type
            > turn_info;

        std::vector<turn_info> turns;
        detail::dissolve::no_interrupt_policy policy;
        geometry::self_turns
            <
                detail::overlay::assign_null_policy
            >(geometry, strategy, rescale_policy, turns, policy, 0, false);

        visitor.visit_turns(1, turns);

        if (boost::size(turns) == 0)
        {
            // No self-turns, then add original geometry
            GeometryOut g;
            geometry::convert(geometry, g);
            *out++ = g;
            return out;
        }

        typedef typename ring_type<Geometry>::type ring_type;
        typedef std::vector<ring_type> out_vector;
        out_vector rings;

        typedef std::map
            <
                signed_size_type,
                detail::overlay::cluster_info
            > cluster_type;

        cluster_type clusters;

        // Enrich/traverse the polygons twice: first for union...
        typename Strategy::side_strategy_type const
            side_strategy = strategy.get_side_strategy();

        enrich_intersection_points<false, false, overlay_dissolve_union>(turns,
                    clusters, geometry, geometry, rescale_policy,
                    side_strategy);

        visitor.visit_turns(2, turns);

        visitor.visit_clusters(clusters, turns);

        std::map<ring_identifier, overlay::ring_turn_info> turn_info_per_ring;

        detail::overlay::traverse
            <
                false, false,
                Geometry, Geometry,
                overlay_dissolve_union,
                backtrack_for_dissolve<Geometry>
            >::apply(geometry, geometry,
                     strategy, rescale_policy,
                     turns, rings, turn_info_per_ring, clusters, visitor);

        visitor.visit_turns(3, turns);

        // ... and then for intersection
        clear(turns);
        enrich_intersection_points<false, false, overlay_dissolve_intersection>(turns,
                    clusters, geometry, geometry, rescale_policy,
                    side_strategy);

        visitor.visit_turns(4, turns);

        detail::overlay::traverse
            <
                false, false,
                Geometry, Geometry,
                overlay_dissolve_intersection,
                backtrack_for_dissolve<Geometry>
            >::apply(geometry, geometry,
                     strategy, rescale_policy,
                     turns, rings, turn_info_per_ring, clusters, visitor);

        visitor.visit_turns(5, turns);

        detail::overlay::get_ring_turn_info<overlay_dissolve_union>(turn_info_per_ring, turns, clusters);

        typedef typename geometry::point_type<Geometry>::type point_type;
        typedef typename Strategy::template area_strategy
            <
                point_type
            >::type area_strategy_type;
        typedef typename area_strategy_type::return_type area_result_type;
        typedef detail::overlay::ring_properties<point_type, area_result_type> properties;

        std::map<ring_identifier, properties> selected;

        detail::overlay::select_rings<overlay_dissolve_union>(geometry, turn_info_per_ring, selected, strategy);

        // Add intersected rings
        area_strategy_type const area_strategy = strategy.template get_area_strategy<point_type>();
        {
            ring_identifier id(2, 0, -1);
            for (typename boost::range_iterator<std::vector<ring_type> const>::type
                    it = boost::begin(rings);
                    it != boost::end(rings);
                    ++it)
            {
                selected[id] = properties(*it, area_strategy);
                id.multi_index++;
            }
        }

        // Assign parents, checking orientation and discarding negative
        // children with negative parents
        detail::overlay::assign_parents(geometry, rings, selected,
                                        strategy, true, true);
        return detail::overlay::add_rings<GeometryOut>(selected, geometry, rings, out, area_strategy);
    }
};



}} // namespace detail::dissolve
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename GeometryTag,
    typename GeometryOutTag,
    typename Geometry,
    typename GeometryOut
>
struct dissolve
    : not_implemented<GeometryTag, GeometryOutTag>
{};


template<typename Polygon, typename PolygonOut>
struct dissolve<polygon_tag, polygon_tag, Polygon, PolygonOut>
    : detail::dissolve::dissolve_ring_or_polygon<Polygon, PolygonOut>
{};


template<typename Ring, typename RingOut>
struct dissolve<ring_tag, ring_tag, Ring, RingOut>
    : detail::dissolve::dissolve_ring_or_polygon<Ring, RingOut>
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH



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

    typedef typename geometry::rescale_policy_type
    <
        typename geometry::point_type<Geometry>::type
    >::type rescale_policy_type;

    rescale_policy_type robust_policy
        = geometry::get_rescale_policy<rescale_policy_type>(geometry);

    detail::overlay::overlay_null_visitor visitor;

    return dispatch::dissolve
    <
        typename tag<Geometry>::type,
        typename tag<GeometryOut>::type,
        Geometry,
        GeometryOut
    >::apply(geometry, robust_policy, out, strategy, visitor);
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
inline OutputIterator dissolve_inserter(Geometry const& geometry, OutputIterator out)
{
    typedef typename strategy::intersection::services::default_strategy
        <
            typename cs_tag<Geometry>::type
        >::type strategy_type;

    return dissolve_inserter<GeometryOut>(geometry, out, strategy_type());
}


template
<
    typename Geometry,
    typename Collection,
    typename Strategy
>
inline void dissolve(Geometry const& geometry, Collection& output_collection, Strategy const& strategy)
{
    concepts::check<Geometry const>();

    typedef typename boost::range_value<Collection>::type geometry_out;

    concepts::check<geometry_out>();

    detail::overlay::overlay_null_visitor visitor;

    dispatch::dissolve
    <
        typename tag<Geometry>::type,
        typename tag<geometry_out>::type,
        Geometry,
        geometry_out
    >::apply(geometry, detail::no_rescale_policy(),
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
    typedef typename strategy::intersection::services::default_strategy
        <
            typename cs_tag<Geometry>::type
        >::type strategy_type;

    dissolve(geometry, output_collection, strategy_type());
}



}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_ALGORITHMS_DISSOLVE_HPP
