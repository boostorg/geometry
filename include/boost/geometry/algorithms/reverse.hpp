// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_REVERSE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_REVERSE_HPP

#include <algorithm>

#include <boost/range.hpp>
#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/geometries/concepts/check.hpp>


/*!
\defgroup reverse reverse: reverse a geometry
    This is functionally equivalent to the std::reverse algorithm.
    For a linestring or a linear ring, it is exactly the same as calling the std::reverse algorithm.
    For a polygon or a multi-geometry, all its rings or elements are reversed.

    No check on order is applied. So a clockwise polygon (having positive area)
    will be made counterclockwise (having negative area).

    The first and last points are reversed as well, even if they are closed and the same.
*/


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace reverse
{


template <typename Range>
struct range_reverse
{
    static inline void apply(Range& range)
    {
        std::reverse(boost::begin(range), boost::end(range));
    }
};


template <typename Polygon>
struct polygon_reverse
{
    static inline void apply(Polygon& polygon)
    {
        typedef typename geometry::ring_type<Polygon>::type ring_type;

        typedef range_reverse<ring_type> per_range;
        per_range::apply(exterior_ring(polygon));

        for (typename boost::range_iterator
                <
                    typename interior_type<Polygon>::type
                >::type it = boost::begin(interior_rings(polygon));
             it != boost::end(interior_rings(polygon));
             ++it)
        {
            per_range::apply(*it);
        }
    }
};



}} // namespace detail::reverse
#endif // DOXYGEN_NO_DETAIL



#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template
<
    typename Tag,
    typename Geometry
>
struct reverse
{
    static inline void apply(Geometry&)
    {}
};


template <typename Ring>
struct reverse<ring_tag, Ring>
    : detail::reverse::range_reverse<Ring>
{};


template <typename LineString>
struct reverse<linestring_tag, LineString>
    : detail::reverse::range_reverse<LineString>
{};


template <typename Polygon>
struct reverse<polygon_tag, Polygon>
    : detail::reverse::polygon_reverse<Polygon>
{};


} // namespace dispatch
#endif


/*!
    \ingroup reverse
    \tparam Geometry geometry type
    \param geometry the geometry to make reverse
*/
template <typename Geometry>
inline void reverse(Geometry& geometry)
{
    concept::check<Geometry>();

    dispatch::reverse
        <
            typename tag<Geometry>::type,
            Geometry
        >::apply(geometry);
}

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_REVERSE_HPP
