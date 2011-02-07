// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_CLEAR_HPP
#define BOOST_GEOMETRY_ALGORITHMS_CLEAR_HPP

#include <boost/mpl/assert.hpp>
#include <boost/type_traits/remove_const.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/container_access.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace clear
{

template <typename Geometry>
struct collection_clear
{
    static inline void apply(Geometry& geometry)
    {
        write::clear(geometry);
    }
};

template <typename Polygon>
struct polygon_clear
{
    static inline void apply(Polygon& polygon)
    {
        write::clear(interior_rings(polygon));
        write::clear(exterior_ring(polygon));
    }
};

template <typename Geometry>
struct no_action
{
    static inline void apply(Geometry& )
    {
    }
};

}} // namespace detail::clear
#endif // DOXYGEN_NO_DETAIL

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename Tag, typename Geometry>
struct clear
{
    BOOST_MPL_ASSERT_MSG
        (
            false, NOT_OR_NOT_YET_IMPLEMENTED_FOR_THIS_GEOMETRY_TYPE
            , (types<Geometry>)
        );
};

// Point/box/segment do not have clear. So specialize to do nothing.
template <typename Geometry>
struct clear<point_tag, Geometry>
    : detail::clear::no_action<Geometry>
{};

template <typename Geometry>
struct clear<box_tag, Geometry>
    : detail::clear::no_action<Geometry>
{};

template <typename Geometry>
struct clear<segment_tag, Geometry>
    : detail::clear::no_action<Geometry>
{};

template <typename Geometry>
struct clear<linestring_tag, Geometry>
    : detail::clear::collection_clear<Geometry>
{};

template <typename Geometry>
struct clear<ring_tag, Geometry>
    : detail::clear::collection_clear<Geometry>
{};


// Polygon can (indirectly) use std for clear
template <typename Polygon>
struct clear<polygon_tag, Polygon>
    : detail::clear::polygon_clear<Polygon>
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Clears a linestring, linear ring or polygon (exterior+interiors) or multi*
    \details Generic function to clear a geometry
    \ingroup clear
    \note points and boxes cannot be cleared, instead they can be set to zero by "assign_zero"
*/
template <typename Geometry>
inline void clear(Geometry& geometry)
{
    concept::check<Geometry>();

    dispatch::clear
        <
            typename tag<Geometry>::type,
            Geometry
        >::apply(geometry);
}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_CLEAR_HPP
