// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_APPEND_HPP
#define BOOST_GEOMETRY_ALGORITHMS_APPEND_HPP

#include <boost/range.hpp>


#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/container_access.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/util/copy.hpp>

namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace append
{

template <typename Geometry, typename Point>
struct append_point
{
    static inline void apply(Geometry& geometry, Point const& point,
                int = 0, int = 0)
    {
        typename geometry::point_type<Geometry>::type copy;
        copy_coordinates(point, copy);
        write::push_back(geometry, copy);
    }
};


template <typename Geometry, typename Range>
struct append_range
{
    typedef typename boost::range_value<Range>::type point_type;

    static inline void apply(Geometry& geometry, Range const& range,
                int = 0, int = 0)
    {
        for (typename boost::range_iterator<Range const>::type
            it = boost::begin(range);
             it != boost::end(range);
             ++it)
        {
            append_point<Geometry, point_type>::apply(geometry, *it);
        }
    }
};


template <typename Polygon, typename Point>
struct point_to_polygon
{
    typedef typename ring_type<Polygon>::type range_type;

    static inline void apply(Polygon& polygon, Point const& point,
                int ring_index, int = 0)
    {
        if (ring_index == -1)
        {
            append_point<range_type, Point>::apply(
                        exterior_ring(polygon), point);
        }
        else if (ring_index < int(num_interior_rings(polygon)))
        {
            append_point<range_type, Point>::apply(
                        interior_rings(polygon)[ring_index], point);
        }
    }
};


template <typename Polygon, typename Range>
struct range_to_polygon
{
    typedef typename ring_type<Polygon>::type ring_type;

    static inline void apply(Polygon& polygon, Range const& range,
                int ring_index, int )
    {
        if (ring_index == -1)
        {
            append_range<ring_type, Range>::apply(
                        exterior_ring(polygon), range);
        }
        else if (ring_index < int(num_interior_rings(polygon)))
        {
            append_range<ring_type, Range>::apply(
                        interior_rings(polygon)[ring_index], range);
        }
    }
};


}} // namespace detail::append
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

// (RoP = range or point = use std library)

// Default case (where RoP will be range/array/etc)
template <typename Tag, typename TagRoP, typename G, typename RoP>
struct append : detail::append::append_range<G, RoP> {};

// Append a point to any geometry
template <typename Tag, typename G, typename P>
struct append<Tag, point_tag, G, P>
    : detail::append::append_point<G, P> {};

// Never possible to append anything to a point/box/n-sphere
template <typename TagRoP, typename Point, typename RoP>
struct append<point_tag, TagRoP, Point, RoP> {};

template <typename TagRoP, typename Box, typename RoP>
struct append<box_tag, TagRoP, Box, RoP> {};


template <typename Polygon, typename TagRange, typename Range>
struct append<polygon_tag, TagRange, Polygon, Range>
        : detail::append::range_to_polygon<Polygon, Range> {};

template <typename Polygon, typename Point>
struct append<polygon_tag, point_tag, Polygon, Point>
        : detail::append::point_to_polygon<Polygon, Point> {};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Appends one or more points to a linestring, ring, polygon, multi
    \ingroup append
    \param geometry a geometry
    \param range_or_point the point or range to add
    \param ring_index the index of the ring in case of a polygon:
        exterior ring (-1, the default) or  interior ring index
    \param multi_index reserved for multi polygons
 */
template <typename Geometry, typename RoP>
inline void append(Geometry& geometry, RoP const& range_or_point,
            int ring_index = -1, int multi_index = 0)
{
    concept::check<Geometry>();

    dispatch::append
        <
            typename tag<Geometry>::type,
            typename tag<RoP>::type,
            Geometry,
            RoP
        >::apply(geometry, range_or_point, ring_index, multi_index);
}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_APPEND_HPP
