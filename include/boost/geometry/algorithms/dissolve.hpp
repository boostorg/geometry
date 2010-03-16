// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DISSOLVE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DISSOLVE_HPP


#include <boost/geometry/algorithms/overlay/get_turns.hpp>
#include <boost/geometry/algorithms/overlay/self_turn_points.hpp>

#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/enrichment_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/traversal_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/calculate_distance_policy.hpp>

#include <boost/geometry/algorithms/detail/point_on_border.hpp>

#include <boost/geometry/algorithms/overlay/enrich_intersection_points.hpp>
#include <boost/geometry/algorithms/overlay/traverse.hpp>
#include <boost/geometry/algorithms/overlay/assemble.hpp>

#include <boost/geometry/algorithms/convert.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace dissolve
{


template <typename Geometry, typename GeometryOut>
struct dissolve_ring_or_polygon
{
    template <typename OutputIterator>
    static inline OutputIterator apply(Geometry const& geometry,
                OutputIterator out)
    {
        // Get the self-intersection points, including turns
        typedef detail::overlay::traversal_turn_info
            <
                typename point_type<Geometry>::type
            > turn_info;

        std::vector<turn_info> turns;
        detail::get_turns::no_interrupt_policy policy;
        geometry::get_turns
            <
                detail::overlay::calculate_distance_policy
            >(geometry, turns, policy);

        // The dissolve process is not necessary if there are no turns at all

        if (boost::size(turns) > 0)
        {
            typedef typename ring_type<Geometry>::type ring_type;
            typedef std::vector<ring_type> out_vector;
            out_vector rings;

            // Enrich the turns
            typedef typename strategy_side
            <
                typename cs_tag<Geometry>::type
            >::type side_strategy_type;

            enrich_intersection_points(turns, geometry, geometry,
                        side_strategy_type());


            // Traverse the polygons twice in two different directions
            traverse(geometry, geometry, detail::overlay::operation_union,
                            turns, rings);

            clear_visit_info(turns);

            traverse(geometry, geometry, detail::overlay::operation_intersection,
                            turns, rings);

            return detail::overlay::assemble<GeometryOut>(rings, turns,
                            geometry, geometry, 1, true, out);
        }
        else
        {
            GeometryOut g;
            geometry::convert(geometry, g);
            *out++ = g;
            return out;
        }
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
    \param geometry first geometry
    \param output container which will contain dissolved geometry
 */
template
<
    typename GeometryOut,
    typename Geometry,
    typename OutputIterator
>
inline OutputIterator dissolve_inserter(Geometry const& geometry, OutputIterator out)
{
    concept::check<Geometry const>();
    concept::check<GeometryOut>();

    return dispatch::dissolve
    <
        typename tag<Geometry>::type,
        typename tag<GeometryOut>::type,
        Geometry,
        GeometryOut
    >::apply(geometry, out);
}


template
<
    typename Geometry,
    typename GeometryOut
>
inline void dissolve(Geometry const& geometry, GeometryOut& out)
{
    concept::check<Geometry const>();
    concept::check<GeometryOut>();

    std::vector<GeometryOut> v;
    dispatch::dissolve
    <
        typename tag<Geometry>::type,
        typename tag<GeometryOut>::type,
        Geometry,
        GeometryOut
    >::apply(geometry, std::back_inserter(v));
    if (boost::size(v) > 0)
    {
        out = v.front();
    }
    else
    {
        out = geometry;
    }
}



}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DISSOLVE_HPP
