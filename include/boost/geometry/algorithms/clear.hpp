// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_CLEAR_HPP
#define BOOST_GEOMETRY_ALGORITHMS_CLEAR_HPP

#include <boost/type_traits/remove_const.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace clear {

template <typename Geometry>
struct use_std_clear
{
    static inline void apply(Geometry& geometry)
    {
        geometry.clear();
    }
};

template <typename Geometry>
struct use_traits_clear
{
    static inline void apply(Geometry& geometry)
    {
        traits::clear<Geometry>::apply(geometry);
    }
};

template <typename Polygon>
struct polygon_clear
{
    static inline void apply(Polygon& polygon)
    {
        interior_rings(polygon).clear();
        exterior_ring(polygon).clear();
    }
};

template <typename Geometry>
struct no_action
{
    static inline void apply(Geometry& geometry)
    {
    }
};

}} // namespace detail::clear
#endif // DOXYGEN_NO_DETAIL

#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename Tag, bool UseStd, typename Geometry>
struct clear
{};

// True (default for all geometry types, unless otherwise implemented in traits)
// uses std::clear
template <typename Tag, typename Geometry>
struct clear<Tag, true, Geometry>
    : detail::clear::use_std_clear<Geometry>
{};

// If any geometry specializes use_std<Geometry> to false, specialize to use the traits clear.
template <typename Tag, typename Geometry>
struct clear<Tag, false, Geometry>
    : detail::clear::use_traits_clear<Geometry>
{};

// Point/box/segment do not have clear. So specialize to do nothing.
template <typename Geometry>
struct clear<point_tag, true, Geometry>
    : detail::clear::no_action<Geometry>
{};

template <typename Geometry>
struct clear<box_tag, true, Geometry>
    : detail::clear::no_action<Geometry>
{};

template <typename Geometry>
struct clear<segment_tag, true, Geometry>
    : detail::clear::no_action<Geometry>
{};




// Polygon can (indirectly) use std for clear
template <typename Polygon>
struct clear<polygon_tag, true, Polygon>
    : detail::clear::polygon_clear<Polygon>
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Clears a linestring, linear ring or polygon (exterior+interiors) or multi*
    \details Generic function to clear a geometry
    \ingroup access
    \note points and boxes cannot be cleared, instead they can be set to zero by "assign_zero"
*/
template <typename Geometry>
inline void clear(Geometry& geometry)
{
    concept::check<Geometry>();

    dispatch::clear
        <
            typename tag<Geometry>::type,
            traits::use_std
                <
                    typename boost::remove_const<Geometry>::type
                >::value,
            Geometry
        >::apply(geometry);
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_CLEAR_HPP
