// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_OVERLAY_SELF_INTERSECTION_POINTS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_OVERLAY_SELF_INTERSECTION_POINTS_HPP

#include <cstddef>

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>
#include <boost/type_traits/remove_const.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/is_multi.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/algorithms/detail/disjoint.hpp>
#include <boost/geometry/algorithms/overlay/get_intersection_points.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace self_intersection_points {

template
<
    typename Geometry,
    typename IntersectionPoints,
    typename IntersectionStrategy
>
struct check_ips
{
    static inline bool apply(
            Geometry const& geometry,
            bool return_if_found,
            IntersectionPoints& intersection_points)
    {
        typedef typename geometry::sections
            <
                geometry::box < typename geometry::point_type<Geometry>::type >, 1
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
                    geometry::detail::get_intersection_points::get_ips_in_sections
                    <
                        Geometry, Geometry,
                        typename boost::range_value<sections_type>::type,
                        typename boost::range_value<sections_type>::type,
                        IntersectionPoints, IntersectionStrategy
                    >::apply(
                            0, geometry, *it1,
                            0, geometry, *it2,
                            return_if_found,
                            intersection_points, trivial);
                }
            }
        }
        return trivial;
    }
};


}} // namespace detail::self_intersection_points
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
    typename IntersectionStrategy
>
struct self_intersection_points
{
};


template<typename Ring, typename IntersectionPoints, typename IntersectionStrategy>
struct self_intersection_points
    <
        ring_tag, false, Ring,
        IntersectionPoints,
        IntersectionStrategy
    >
    : detail::self_intersection_points::check_ips
        <
            Ring,
            IntersectionPoints,
            IntersectionStrategy
        >
{};


template<typename Polygon, typename IntersectionPoints, typename IntersectionStrategy>
struct self_intersection_points
    <
        polygon_tag, false, Polygon,
        IntersectionPoints, IntersectionStrategy
    >
    : detail::self_intersection_points::check_ips
        <
            Polygon,
            IntersectionPoints,
            IntersectionStrategy
        >
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Calculate self intersections of a geometry
    \ingroup overlay
    \tparam Geometry geometry type
    \tparam IntersectionPoints type of intersection container (e.g. vector of "intersection_point"'s)
    \param geometry geometry
    \param intersection_points container which will contain intersection points
    \return TRUE if it is trivial, else FALSE
 */
template <typename Geometry, typename IntersectionPoints>
inline bool get_intersection_points(Geometry const& geometry,
            IntersectionPoints& intersection_points)
{
    concept::check<Geometry>();

    typedef typename strategy_intersection
        <
            typename cs_tag<Geometry>::type,
            Geometry,
            Geometry,
            typename boost::range_value<IntersectionPoints>::type
        >::segment_intersection_strategy_type segment_intersection_strategy_type;

    typedef typename boost::remove_const<Geometry>::type ncg_type;

    return dispatch::self_intersection_points
            <
                typename tag<ncg_type>::type,
                is_multi<ncg_type>::type::value,
                ncg_type,
                IntersectionPoints, segment_intersection_strategy_type
            >::apply(geometry, false, intersection_points);
}



}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_OVERLAY_SELF_INTERSECTION_POINTS_HPP
