// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_OVERLAY_SELF_TURN_POINTS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_OVERLAY_SELF_TURN_POINTS_HPP

#include <cstddef>

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>
#include <boost/type_traits/remove_const.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/is_multi.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/algorithms/detail/disjoint.hpp>
#include <boost/geometry/algorithms/overlay/get_turns.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace self_get_turn_points {

template
<
    typename Geometry,
    typename Turns,
    typename IntersectionStrategy,
    typename AssignPolicy,
    typename InterruptPolicy
>
struct get_turns
{
    static inline bool apply(
            Geometry const& geometry,
            Turns& turns,
            InterruptPolicy& interrupt_policy)
    {
        typedef typename geometry::sections
            <
                geometry::box <typename geometry::point_type<Geometry>::type>,
                1
            > sections_type;

        sections_type sec;
        geometry::sectionalize(geometry, sec);

        for (typename boost::range_const_iterator<sections_type>::type
                    it1 = sec.begin();
            it1 != sec.end();
            ++it1)
        {
            for (typename boost::range_const_iterator<sections_type>::type
                        it2 = sec.begin();
                it2 != sec.end();
                ++it2)
            {
                if (! geometry::detail::disjoint::disjoint_box_box(
                                it1->bounding_box, it2->bounding_box)
                    && ! it1->duplicate
                    && ! it2->duplicate
                    )
                {
                    if (! geometry::detail::get_turns::get_turns_in_sections
                        <
                            Geometry, Geometry,
                            typename boost::range_value<sections_type>::type,
                            typename boost::range_value<sections_type>::type,
                            Turns, IntersectionStrategy,
                            AssignPolicy, InterruptPolicy
                        >::apply(
                                0, geometry, *it1,
                                0, geometry, *it2,
                                turns, interrupt_policy))
                    {
                        return false;
                    }
                }
            }
        }
        return true;
    }
};


}} // namespace detail::self_get_turn_points
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename GeometryTag,
    bool IsMulti,
    typename Geometry,
    typename Turns,
    typename IntersectionStrategy,
    typename AssignPolicy,
    typename InterruptPolicy
>
struct self_get_turn_points
{
};


template
<
    typename Ring,
    typename Turns,
    typename IntersectionStrategy,
    typename AssignPolicy,
    typename InterruptPolicy
>
struct self_get_turn_points
    <
        ring_tag, false, Ring,
        Turns,
        IntersectionStrategy,
        AssignPolicy, InterruptPolicy
    >
    : detail::self_get_turn_points::get_turns
        <
            Ring,
            Turns,
            IntersectionStrategy,
            AssignPolicy, InterruptPolicy
        >
{};


template
<
    typename Polygon,
    typename Turns,
    typename IntersectionStrategy,
    typename AssignPolicy,
    typename InterruptPolicy
>
struct self_get_turn_points
    <
        polygon_tag, false, Polygon,
        Turns, IntersectionStrategy,
        AssignPolicy, InterruptPolicy
    >
    : detail::self_get_turn_points::get_turns
        <
            Polygon,
            Turns,
            IntersectionStrategy,
            AssignPolicy, InterruptPolicy
        >
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Calculate self intersections of a geometry
    \ingroup overlay
    \tparam Geometry geometry type
    \tparam Turns type of intersection container
                (e.g. vector of "intersection/turn point"'s)
    \param geometry geometry
    \param turns container which will contain intersection points
 */
template
<
    typename AssignPolicy,
    typename Geometry,
    typename Turns,
    typename InterruptPolicy
>
inline void get_turns(Geometry const& geometry,
            Turns& turns, InterruptPolicy& interrupt_policy)
{
    concept::check<Geometry const>();

    typedef typename strategy_intersection
        <
            typename cs_tag<Geometry>::type,
            Geometry,
            Geometry,
            typename boost::range_value<Turns>::type
        >::segment_intersection_strategy_type strategy_type;

    typedef typename boost::remove_const<Geometry>::type ncg_type;

    dispatch::self_get_turn_points
            <
                typename tag<ncg_type>::type,
                is_multi<ncg_type>::type::value,
                ncg_type,
                Turns, strategy_type,
                AssignPolicy, InterruptPolicy
            >::apply(geometry, turns, interrupt_policy);
}



}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_OVERLAY_SELF_TURN_POINTS_HPP
