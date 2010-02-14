// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_INTERSECTS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_INTERSECTS_HPP


/*!
\defgroup intersects intersects: detect if a geometry self-intersects or if two geometries intersect
\par Source descriptions:
- OGC description: Returns 1 (TRUE) if  this geometric object spatially
    intersects anotherGeometry.
- OGC: a.Intersects(b) <=> ! a.Disjoint(b)
\note There are two overloaded versions:
- with one geometry, detecting self-intersections
- with two geometries, deferring to disjoint, returning !disjoint

\par Geometries:
- \b ring
- \b polygon
- for two geometries: same is disjoint

\note if one geometry is completely within another geometry, it "intersects"

*/


#include <boost/geometry/geometries/concepts/check.hpp>


#include <boost/geometry/algorithms/overlay/self_turn_points.hpp>

#include <boost/geometry/algorithms/disjoint.hpp>



namespace boost { namespace geometry
{

/*!
    \brief Determine if there is at least one intersection
        (crossing or self-tangency)
    \note This function can be called for one geometry (self-intersection) and
        also for two geometries (intersection)
    \ingroup intersects
    \tparam Geometry geometry type
    \param geometry geometry
    \return true if there are intersections, else false
 */
template <typename Geometry>
inline bool intersects(Geometry const& geometry)
{
    concept::check<Geometry const>();


    typedef detail::overlay::turn_info
        <
            typename geometry::point_type<Geometry>::type
        > turn_info;
    std::deque<turn_info> turns;

    typedef typename strategy_intersection
        <
            typename cs_tag<Geometry>::type,
            Geometry,
            Geometry,
            typename geometry::point_type<Geometry>::type
        >::segment_intersection_strategy_type segment_intersection_strategy_type;

    detail::disjoint::disjoint_interrupt_policy policy;
    detail::self_get_turn_points::get_turns
            <
                Geometry,
                std::deque<turn_info>,
                segment_intersection_strategy_type,
                detail::overlay::assign_null_policy,
                detail::disjoint::disjoint_interrupt_policy
            >::apply(geometry, turns, policy);
    return policy.has_intersections;
}


/*!
    \brief Determine if there is at least one intersection
    \ingroup intersects
    \tparam Geometry1 first geometry type
    \tparam Geometry2 second geometry type
    \param geometry1 first geometry
    \param geometry2 second geometry
    \return true if there are intersection(s), else false
 */
template <typename Geometry1, typename Geometry2>
inline bool intersects(Geometry1 const& geometry1, Geometry2 const& geometry2)
{
    concept::check<const Geometry1>();
    concept::check<const Geometry2>();

    return ! geometry::disjoint(geometry1, geometry2);
}



}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_INTERSECTS_HPP
