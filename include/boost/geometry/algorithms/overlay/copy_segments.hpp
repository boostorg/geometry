// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_OVERLAY_COPY_SEGMENTS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_OVERLAY_COPY_SEGMENTS_HPP

#include <boost/assert.hpp>

#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/core/ring_type.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>

#include <boost/geometry/geometries/concepts/check.hpp>

#include <boost/geometry/algorithms/overlay/intersection_point.hpp>

#include <boost/geometry/iterators/ever_circling_iterator.hpp>

#include <boost/geometry/iterators/range_type.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace copy_segments {


template <typename Ring, typename SegmentIdentifier, typename RangeOut>
struct copy_segments_ring
{
    static inline void apply(Ring const& ring,
            SegmentIdentifier const& seg_id, int to_index,
            RangeOut& current_output)
    {
        typedef typename boost::range_const_iterator<Ring>::type iterator;

        typedef geometry::ever_circling_iterator<iterator> ec_iterator;

        // The problem: sometimes we want to from "3" to "2" -> end = "3" -> end == begin
        // This is not convenient with iterators.

        // So we use the ever-circling iterator and determine when to step out

        int from_index = seg_id.segment_index + 1;

        // Sanity check
        BOOST_ASSERT(from_index < boost::size(ring));

        ec_iterator it(boost::begin(ring), boost::end(ring),
                    boost::begin(ring) + from_index);

        // [2..4] -> 4 - 2 + 1 = 3 -> {2,3,4} -> OK
        // [4..2],size=6 -> 6 - 4 + 2 + 1 = 5 -> {4,5,0,1,2} -> OK
        // [1..1], travel the whole ring round
        int count = from_index <= to_index
            ? to_index - from_index + 1
            : boost::size(ring) - from_index + to_index + 1;

        for (int i = 0; i < count; ++i, ++it)
        {
#ifdef BOOST_GEOMETRY_DEBUG_INTERSECTION
            std::cout << "  add: ("
                << geometry::get<0>(*it) << ", " << geometry::get<1>(*it) << ")"
                << std::endl;
#endif
            typename geometry::point_type<RangeOut>::type p;
            geometry::copy_coordinates(*it, p);
            //current_output.push_back(p);
            *(std::back_inserter(current_output)++) = p;
        }
    }
};


template <typename Polygon, typename SegmentIdentifier, typename RangeOut>
struct copy_segments_polygon
{
    static inline void apply(Polygon const& polygon,
            SegmentIdentifier const& seg_id, int to_index,
            RangeOut& current_output)
    {
        // Call ring-version with the right ring
        copy_segments_ring
            <
                typename geometry::ring_type<Polygon>::type,
                SegmentIdentifier,
                RangeOut
            >::apply
                (
                    seg_id.ring_index < 0
                    ? geometry::exterior_ring(polygon)
                    : geometry::interior_rings(polygon)[seg_id.ring_index],
                    seg_id, to_index,
                    current_output
                );
    }
};


template <typename Box, typename SegmentIdentifier, typename RangeOut>
struct copy_segments_box
{
    static inline void apply(Box const& box,
            SegmentIdentifier const& seg_id, int to_index,
            RangeOut& current_output)
    {
        // Convert again...
        // TODO: avoid that...

        typedef typename point_type<Box>::type point_type;

        point_type ll, lr, ul, ur;
        assign_box_corners(box, ll, lr, ul, ur);

        std::vector<point_type> points;
        points.push_back(ll);
        points.push_back(ul);
        points.push_back(ur);
        points.push_back(lr);
        points.push_back(ll);

        copy_segments_ring
            <
                std::vector<point_type>,
                SegmentIdentifier,
                RangeOut
            >
            ::apply(points, seg_id, to_index, current_output);
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
    typename RangeOut
>
struct copy_segments
{
};


template <typename Ring, typename SegmentIdentifier, typename RangeOut>
struct copy_segments<ring_tag, Ring, SegmentIdentifier, RangeOut>
    : detail::copy_segments::copy_segments_ring
        <
            Ring, SegmentIdentifier, RangeOut
        >
{};

template <typename Polygon, typename SegmentIdentifier, typename RangeOut>
struct copy_segments<polygon_tag, Polygon, SegmentIdentifier, RangeOut>
    : detail::copy_segments::copy_segments_polygon
        <
            Polygon, SegmentIdentifier, RangeOut
        >
{};


template <typename Box, typename SegmentIdentifier, typename RangeOut>
struct copy_segments<box_tag, Box, SegmentIdentifier, RangeOut>
    : detail::copy_segments::copy_segments_box
        <
            Box, SegmentIdentifier, RangeOut
        >
{};



} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH





/*!
    \brief Traverses through intersection points / geometries
    \ingroup overlay
 */
template<typename Geometry, typename SegmentIdentifier, typename RangeOut>
inline void copy_segments(Geometry const& geometry,
            SegmentIdentifier const& seg_id, int to_index,
            RangeOut& range_out)
{
    concept::check<const Geometry>();

    dispatch::copy_segments
        <
            typename tag<Geometry>::type,
            Geometry,
            SegmentIdentifier,
            RangeOut
        >::apply(geometry, seg_id, to_index, range_out);
}


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_OVERLAY_COPY_SEGMENTS_HPP
