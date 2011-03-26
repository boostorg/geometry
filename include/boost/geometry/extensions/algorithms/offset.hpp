// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_ALGORITHMS_OFFSET_HPP
#define BOOST_GEOMETRY_EXTENSIONS_ALGORITHMS_OFFSET_HPP


#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>

#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/extensions/algorithms/buffer/line_line_intersection.hpp>
#include <boost/geometry/algorithms/detail/disjoint.hpp>
#include <boost/geometry/geometries/concepts/check.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace offset
{


template
<
    typename Range,
    typename RangeOut,
    typename JoinStrategy,
    typename Distance
>
struct offset_range
{
    typedef typename coordinate_type<RangeOut>::type coordinate_type;
    typedef typename point_type<RangeOut>::type output_point_type;
    typedef model::referring_segment<output_point_type const> segment_type;
    typedef typename boost::range_iterator<Range const>::type iterator_type;

    static inline void apply(Range const& range,
                RangeOut& out,
                JoinStrategy const& join,
                Distance const& distance)
    {
        output_point_type previous_p1, previous_p2;
        output_point_type first_p1, first_p2;

        bool first = true;

        iterator_type it = boost::begin(range);
        for (iterator_type prev = it++; it != boost::end(range); ++it)
        {
            if (! detail::equals::equals_point_point(*prev, *it))
            {
                bool skip = false;

                // Simulate a vector d (dx,dy)
                coordinate_type dx = get<0>(*it) - get<0>(*prev);
                coordinate_type dy = get<1>(*it) - get<1>(*prev);

                // For normalization [0,1] (=dot product d.d, sqrt)
                coordinate_type length = sqrt(dx * dx + dy * dy);

                // Because coordinates are not equal, length should not be zero
                BOOST_ASSERT((! geometry::math::equals(length, 0)));

                // Generate the normalized perpendicular p, to the left (ccw)
                coordinate_type px = -dy / length;
                coordinate_type py = dx / length;

                output_point_type p1, p2;

                set<0>(p2, get<0>(*it) + px * distance);
                set<1>(p2, get<1>(*it) + py * distance);

                set<0>(p1, get<0>(*prev) + px * distance);
                set<1>(p1, get<1>(*prev) + py * distance);

                if (! first)
                {
                    output_point_type p;
                    segment_type s1(p1, p2);
                    segment_type s2(previous_p1, previous_p2);
                    if (detail::buffer::line_line_intersection<output_point_type, segment_type>::apply(s1, s2, p))
                    {
                        join.apply(p, *prev, previous_p2, p1, distance, out);
                    }
                    else
                    {
                        skip = false;
                    }
                }
                else
                {
                    first = false;
                    first_p1 = p1;
                    first_p2 = p2;

                    out.push_back(p1);
                }

                if (! skip)
                {
                    previous_p1 = p1;
                    previous_p2 = p2;
                    prev = it;
                }
            }
        }

        // Last one
        out.push_back(previous_p2);

    }
};

}} // namespace detail::offset
#endif



#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename GeometryTag,
    typename GeometryOutTag,
    typename Geometry,
    typename GeometryOut,
    typename JoinStrategy,
    typename Distance
>
struct offset
{};


template
<
    typename Geometry,
    typename GeometryOut,
    typename JoinStrategy,
    typename Distance
>
struct offset
    <
        linestring_tag,
        linestring_tag,
        Geometry,
        GeometryOut,
        JoinStrategy,
        Distance
    >
    : detail::offset::offset_range
        <
            Geometry,
            GeometryOut,
            JoinStrategy,
            Distance
        >
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


template
<
    typename Geometry,
    typename GeometryOut,
    typename JoinStrategy,
    typename Distance
>
inline void offset(Geometry const& geometry, GeometryOut& out,
            JoinStrategy const& join,
            Distance const& distance)
{
    concept::check<Geometry const>();
    concept::check<GeometryOut>();

    dispatch::offset
    <
        typename tag<Geometry>::type,
        typename tag<GeometryOut>::type,
        Geometry,
        GeometryOut,
        JoinStrategy,
        Distance
    >::apply(geometry, out, join, distance);
}


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_EXTENSIONS_ALGORITHMS_OFFSET_HPP
