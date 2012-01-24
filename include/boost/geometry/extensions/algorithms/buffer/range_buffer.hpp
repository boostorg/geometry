// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_RANGE_BUFFER_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_RANGE_BUFFER_HPP

#include <cstddef>

#include <boost/range.hpp>

#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/extensions/strategies/buffer_side.hpp>

#include <boost/geometry/extensions/algorithms/buffer/line_line_intersection.hpp>

#include <boost/geometry/geometries/segment.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace buffer
{

template
<
    typename RingOutput,
    typename DistanceStrategy,
    typename JoinStrategy,
    typename Tag
>
struct range_buffer
{
    typedef typename point_type<RingOutput>::type output_point_type;
    typedef typename coordinate_type<RingOutput>::type coordinate_type;
    typedef model::referring_segment<output_point_type const> segment_type;

    template
    <
        typename BufferAppender,
        typename Iterator
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
        , typename Mapper
#endif
    >
    static inline void iterate(BufferAppender& appender,
                Iterator begin, Iterator end,
                buffer_side_selector side,
                DistanceStrategy const& distance,
                JoinStrategy const& join_strategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
                , Mapper& mapper
#endif
            )
    {
        output_point_type previous_p1, previous_p2;
        output_point_type first_p1, first_p2;

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
        int index = 0;
#endif

        bool first = true;

        Iterator it = begin;
        for (Iterator prev = it++; it != end; ++it)
        {
            if (! detail::equals::equals_point_point(*prev, *it))
            {
                // Generate a block along (left or right of) the segment

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

                coordinate_type d = distance.apply(*prev, *it, side);

                set<0>(p2, get<0>(*it) + px * d);
                set<1>(p2, get<1>(*it) + py * d);

                set<0>(p1, get<0>(*prev) + px * d);
                set<1>(p1, get<1>(*prev) + py * d);

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
                {
                    RingOutput block;
                    block.push_back(*prev);
                    block.push_back(*it);
                    block.push_back(p2);
                    block.push_back(p1);
                    block.push_back(*prev);

                    if(boost::is_same<Tag, ring_tag>::value)
                    {
                        mapper.map(block, "opacity:0.4;fill:rgb(255,128,0);stroke:rgb(0,0,0);stroke-width:1");
                    }
                    else if(boost::is_same<Tag, linestring_tag>::value)
                    {
                        mapper.map(block, "opacity:0.4;fill:rgb(0,255,0);stroke:rgb(0,0,0);stroke-width:1");
                    }
                }
#endif

                if (! first)
                {
                    output_point_type p;
                    segment_type s1(p1, p2);
                    segment_type s2(previous_p1, previous_p2);
                    if (line_line_intersection<output_point_type, segment_type>::apply(s1, s2, p))
                    {
                        join_strategy.apply(p, *prev, previous_p2, p1,
                                    distance.apply(*prev, *it, side),
                                    appender);

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
                        {
                            mapper.map(p, "fill:rgb(0,0,0);", 3);

                            std::ostringstream out;
                            out << index++;
                            mapper.text(p, out.str(), "fill:rgb(0,0,0);font-family='Arial';", 5, 5);
                        }
#endif
                    }
                }
                else
                {
                    first = false;
                    first_p1 = p1;
                    first_p2 = p2;

                    // Might be replaced by specialization
                    if(boost::is_same<Tag, linestring_tag>::value)
                    {
                        appender.append(p1);
                    }
                }

                previous_p1 = p1;
                previous_p2 = p2;
                prev = it;
            }
        }

        // Might be replaced by specialization
        if(boost::is_same<Tag, linestring_tag>::value)
        {
            appender.append(previous_p2);
        }
        else if(boost::is_same<Tag, ring_tag>::value)
        {
            // Generate closing corner
            output_point_type p;
            segment_type s1(previous_p1, previous_p2);
            segment_type s2(first_p1, first_p2);
            line_line_intersection<output_point_type, segment_type>::apply(s1, s2, p);

            join_strategy.apply(p, *begin, previous_p2, first_p1,
                distance.apply(*(end - 1), *begin, side),
                appender);

            // Close the generated buffer (NOT FOR OPEN POLYGONS - TODO)
            appender.append(first_p1);

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            {
                mapper.map(p, "fill:rgb(0,0,0);", 3);
                std::ostringstream out;
                out << index++;
                mapper.text(p, out.str(), "fill:rgb(0,0,0);font-family='Arial';", 5, 5);
            }
#endif
        }
    }
};

}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_RANGE_BUFFER_HPP
