// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_ALGORITHMS_IS_SIMPLE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_IS_SIMPLE_HPP

#include <algorithm>

#include <boost/range.hpp>

#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/multi/core/tags.hpp>
#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/policies/compare.hpp>

#include <boost/geometry/algorithms/is_valid.hpp>

#include <boost/geometry/algorithms/dispatch/is_simple.hpp>

#include <boost/geometry/algorithms/detail/is_simple/pointlike.hpp>
#include <boost/geometry/algorithms/detail/is_simple/linear.hpp>



// check includes below
#include <boost/geometry/algorithms/unique.hpp>
#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/intersects.hpp>
#include <boost/geometry/algorithms/disjoint.hpp>
#include <boost/geometry/algorithms/detail/point_is_spike_or_equal.hpp>
#include <boost/geometry/views/closeable_view.hpp>
#include <deque>
#include <algorithm>




namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace is_simple
{


template
<
    typename Geometry,
    bool AllowEmpty = true,
    typename Tag = typename tag<Geometry>::type
>
struct is_below_minimal_size
{
    static inline bool apply(Geometry const&)
    {
        return false;
    }
};


template <typename Linestring>
struct is_below_minimal_size<Linestring, true, linestring_tag>
{
    static inline bool apply(Linestring const& linestring)
    {
        return boost::size(linestring) == 1;
    }
};





template <typename Polygon>
struct is_simple_polygon
{
    static inline bool apply(Polygon const& polygon)
    {
        typedef typename ring_type<Polygon>::type Ring;

        BOOST_AUTO_TPL(it, boost::begin(geometry::interior_rings(polygon)));
        for (; it != boost::end(geometry::interior_rings(polygon)); ++it)
        {
            if ( !is_simple_range<Ring>::apply(*it) )
            {
                return false;
            }
        }

        return is_simple_range<Ring>::apply(geometry::exterior_ring(polygon));
    }
};





template <typename MultiGeometry, bool CheckIntersections>
struct is_simple_multigeometry
{
    static inline bool apply(MultiGeometry const& multigeometry)
    {
        typedef typename boost::range_value<MultiGeometry>::type Geometry;

        BOOST_AUTO_TPL(it, boost::begin(multigeometry));
        for (; it != boost::end(multigeometry); ++it)
        {
            if ( !dispatch::is_simple<Geometry>::apply(*it) )
            {
                return false;
            }
        }


        if ( CheckIntersections )
        {
            BOOST_AUTO_TPL(it1, boost::begin(multigeometry));
            it = it1;
            for (; it != boost::end(multigeometry); ++it)
            {
                for (; it1 != boost::end(multigeometry); ++it1)
                {
                    if ( geometry::intersects(*it, *it1) )
                    {
                        return false;
                    }
                }
            }
        }
        return true;
    }
};



}} // namespace detail::is_simple
#endif // DOXYGEN_NO_DETAIL




#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{



// A segment is always simple.
// A segment is a curve.
// A curve is simple if it does not pass through the same point twice,
// with the possible exception of its two endpoints
//
// Reference: OGC 06-103r4 (§6.1.6.1)
template <typename Segment>
struct is_simple<Segment, segment_tag>
    : dispatch::is_valid<Segment, segment_tag>
{};
#if 0
    static inline bool apply(Segment const& segment)
    {
        return dispatch::is_valid<Segment>::apply(segment);
    }
};
#endif

// A box is always simple
// A box is a Polygon, and it satisfies the conditions for Polygon validity.
//
// Reference (for polygon validity): OGC 06-103r4 (§6.1.11.1)
template <typename Box>
struct is_simple<Box, box_tag>
    : dispatch::is_valid<Box, box_tag>
{};
#if 0
    static inline bool apply(Box const& box)
    {
        return dispatch::is_valid<Box>::apply(box);
    }
};
#endif


// A Ring is a Polygon.
// A Polygon is always a simple geometric object provided that it is valid.
//
// Reference (for polygon validity): OGC 06-103r4 (§6.1.11.1)
template <typename Ring>
struct is_simple<Ring, ring_tag>
    : detail::is_simple::is_simple_range<Ring>
{};


// A Polygon is always a simple geometric object provided that it is valid.
//
// Reference (for validity of Polygons): OGC 06-103r4 (§6.1.11.1)
template <typename Polygon>
struct is_simple<Polygon, polygon_tag>
    : dispatch::is_valid<Polygon, polygon_tag>
{};



// Not clear what the definition is.
// Right now we check that each element is simple (in fact valid), and
// that the MultiPolygon is also valid.
//
// Reference (for validity of MultiPolygons): OGC 06-103r4 (§6.1.14)
template <typename MultiPolygon>
struct is_simple<MultiPolygon, multi_polygon_tag>
    : dispatch::is_valid<MultiPolygon, multi_polygon_tag>
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH



template <typename Geometry>
inline bool is_simple(Geometry const& g)
{
    return dispatch::is_simple<Geometry>::apply(g);
}



}} // namespace boost::geometry



#endif // BOOST_GEOMETRY_ALGORITHMS_IS_SIMPLE_HPP
