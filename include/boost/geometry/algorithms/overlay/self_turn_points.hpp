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
    typename IntersectionPoints,
    typename IntersectionStrategy,
    typename AssignPolicy
>
struct get_turns
{
    static inline bool apply(
            Geometry const& geometry,
            bool return_if_found,
            IntersectionPoints& intersection_points)
    {
        typedef typename geometry::sections
            <
                geometry::box <typename geometry::point_type<Geometry>::type>,
                1
            > sections_type;

        sections_type sec;
        geometry::sectionalize(geometry, sec);

        bool trivial = true;
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
                    geometry::detail::get_turns::get_turns_in_sections
                    <
                        Geometry, Geometry,
                        typename boost::range_value<sections_type>::type,
                        typename boost::range_value<sections_type>::type,
                        IntersectionPoints, IntersectionStrategy,
                        AssignPolicy
                    >::apply(
#ifdef BG_SELF_NEGATIVE
                            -2, geometry, *it1,
                            -1, geometry, *it2,
#else
                            0, geometry, *it1,
                            0, geometry, *it2,
#endif
                            return_if_found,
                            intersection_points, trivial);
                }
            }
        }
        return trivial;
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
    typename IntersectionPoints,
    typename IntersectionStrategy,
    typename AssignPolicy
>
struct self_get_turn_points
{
};


template
<
    typename Ring,
    typename IntersectionPoints,
    typename IntersectionStrategy,
    typename AssignPolicy
>
struct self_get_turn_points
    <
        ring_tag, false, Ring,
        IntersectionPoints,
        IntersectionStrategy,
        AssignPolicy
    >
    : detail::self_get_turn_points::get_turns
        <
            Ring,
            IntersectionPoints,
            IntersectionStrategy,
            AssignPolicy
        >
{};


template
<
    typename Polygon,
    typename IntersectionPoints,
    typename IntersectionStrategy,
    typename AssignPolicy
>
struct self_get_turn_points
    <
        polygon_tag, false, Polygon,
        IntersectionPoints, IntersectionStrategy,
        AssignPolicy
    >
    : detail::self_get_turn_points::get_turns
        <
            Polygon,
            IntersectionPoints,
            IntersectionStrategy,
            AssignPolicy
        >
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Calculate self intersections of a geometry
    \ingroup overlay
    \tparam Geometry geometry type
    \tparam IntersectionPoints type of intersection container
                (e.g. vector of "intersection_point"'s)
    \param geometry geometry
    \param intersection_points container which will contain intersection points
 */
template <typename AssignPolicy, typename Geometry, typename IntersectionPoints>
inline void get_turns(Geometry const& geometry,
            IntersectionPoints& intersection_points)
{
    concept::check<Geometry>();

    typedef typename strategy_intersection
        <
            typename cs_tag<Geometry>::type,
            Geometry,
            Geometry,
            typename boost::range_value<IntersectionPoints>::type
        >::segment_intersection_strategy_type strategy_type;

    typedef typename boost::remove_const<Geometry>::type ncg_type;

    dispatch::self_get_turn_points
            <
                typename tag<ncg_type>::type,
                is_multi<ncg_type>::type::value,
                ncg_type,
                IntersectionPoints, strategy_type,
                AssignPolicy
            >::apply(geometry, false, intersection_points);
}



}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_OVERLAY_SELF_TURN_POINTS_HPP
