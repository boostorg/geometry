// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_OVERLAY_COPY_SEGMENT_POINT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_OVERLAY_COPY_SEGMENT_POINT_HPP


#include <boost/geometry/core/ring_type.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/iterators/ever_circling_iterator.hpp>

#include <boost/geometry/iterators/range_type.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace copy_segments {


template <typename Range, typename SegmentIdentifier, typename PointOut>
struct copy_segment_point_range
{
    static inline bool apply(Range const& range,
            SegmentIdentifier const& seg_id, bool second,
            PointOut& point)
    {
        int index = seg_id.segment_index;
        if (second)
        {
            index++;
            if (index >= boost::size(range))
            {
                index = 0;
            }
        }

        // Exception?
        if (index >= boost::size(range))
        {
            return false;
        }

        geometry::copy_coordinates(range[index], point);
        return true;
    }
};


template <typename Polygon, typename SegmentIdentifier, typename PointOut>
struct copy_segment_point_polygon
{
    static inline bool apply(Polygon const& polygon,
            SegmentIdentifier const& seg_id, bool second,
            PointOut& point)
    {
        // Call ring-version with the right ring
        return copy_segment_point_range
            <
                typename geometry::ring_type<Polygon>::type,
                SegmentIdentifier,
                PointOut
            >::apply
                (
                    seg_id.ring_index < 0
                    ? geometry::exterior_ring(polygon)
                    : geometry::interior_rings(polygon)[seg_id.ring_index],
                    seg_id, second,
                    point
                );
    }
};


template <typename Box, typename SegmentIdentifier, typename PointOut>
struct copy_segment_point_box
{
    static inline bool apply(Box const& box,
            SegmentIdentifier const& seg_id, bool second,
            PointOut& point)
    {
        int index = seg_id.segment_index;
        if (second)
        {
            index++;
        }

        PointOut ll, lr, ul, ur;
        assign_box_corners(box, ll, lr, ul, ur);
        switch(index)
        {
            case 1 : point = ul; break;
            case 2 : point = ur; break;
            case 3 : point = lr; break;
            default : // 0,4 or 'overflow'
                point = ll; break;
        }
        return true;
    }
};




}} // namespace detail::copy_segments
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch {


template
<
    typename Tag,
    typename GeometryIn,
    typename SegmentIdentifier,
    typename PointOut
>
struct copy_segment_point
{};


template <typename LineString, typename SegmentIdentifier, typename PointOut>
struct copy_segment_point<linestring_tag, LineString, SegmentIdentifier, PointOut>
    : detail::copy_segments::copy_segment_point_range
        <
            LineString, SegmentIdentifier, PointOut
        >
{};


template <typename Ring, typename SegmentIdentifier, typename PointOut>
struct copy_segment_point<ring_tag, Ring, SegmentIdentifier, PointOut>
    : detail::copy_segments::copy_segment_point_range
        <
            Ring, SegmentIdentifier, PointOut
        >
{};

template <typename Polygon, typename SegmentIdentifier, typename PointOut>
struct copy_segment_point<polygon_tag, Polygon, SegmentIdentifier, PointOut>
    : detail::copy_segments::copy_segment_point_polygon
        <
            Polygon, SegmentIdentifier, PointOut
        >
{};


template <typename Box, typename SegmentIdentifier, typename PointOut>
struct copy_segment_point<box_tag, Box, SegmentIdentifier, PointOut>
    : detail::copy_segments::copy_segment_point_box
        <
            Box, SegmentIdentifier, PointOut
        >
{};



} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH





/*!
    \brief Helper function, copies a point from a segment
    \ingroup overlay
 */
template<typename Geometry, typename SegmentIdentifier, typename PointOut>
inline bool copy_segment_point(Geometry const& geometry,
            SegmentIdentifier const& seg_id, bool second,
            PointOut& point_out)
{
    concept::check<const Geometry>();

    return dispatch::copy_segment_point
        <
            typename tag<Geometry>::type,
            Geometry,
            SegmentIdentifier,
            PointOut
        >::apply(geometry, seg_id, second, point_out);
}


/*!
    \brief Helper function, to avoid the same construct several times,
        copies a point, based on a source-index and two geometries
    \ingroup overlay
 */
template
<
    typename Geometry1,
    typename Geometry2,
    typename SegmentIdentifier,
    typename PointOut
>
inline bool copy_segment_point(Geometry1 const& geometry1, Geometry2 const& geometry2,
            SegmentIdentifier const& seg_id, bool second,
            PointOut& point_out)
{
    concept::check<const Geometry1>();
    concept::check<const Geometry2>();

    if (seg_id.source_index == 0)
    {
        return dispatch::copy_segment_point
            <
                typename tag<Geometry1>::type,
                Geometry1,
                SegmentIdentifier,
                PointOut
            >::apply(geometry1, seg_id, second, point_out);
    }
    else if (seg_id.source_index == 1)
    {
        return dispatch::copy_segment_point
            <
                typename tag<Geometry2>::type,
                Geometry2,
                SegmentIdentifier,
                PointOut
            >::apply(geometry2, seg_id, second, point_out);
    }
    // Exception?
    return false;
}


/*!
    \brief Helper function, to avoid the same construct several times,
        copies a point, based on a source-index and two geometries
    \ingroup overlay
 */
template
<
    typename Geometry1,
    typename Geometry2,
    typename SegmentIdentifier,
    typename PointOut
>
inline bool copy_segment_points(Geometry1 const& geometry1, Geometry2 const& geometry2,
            SegmentIdentifier const& seg_id,
            PointOut& point1, PointOut& point2)
{
    concept::check<const Geometry1>();
    concept::check<const Geometry2>();

    return copy_segment_point(geometry1, geometry2, seg_id, false, point1)
        && copy_segment_point(geometry1, geometry2, seg_id, true,  point2);
}




}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_OVERLAY_COPY_SEGMENT_POINT_HPP
