// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_LINESTRING_BUFFER_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_LINESTRING_BUFFER_HPP

#include <cstddef>

#include <boost/numeric/conversion/cast.hpp>

#include <boost/range.hpp>

#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/strategies/buffer_side.hpp>

#include <boost/geometry/extensions/algorithms/buffer/line_line_intersection.hpp>
#include <boost/geometry/extensions/algorithms/buffer/intersecting_inserter.hpp>




namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace buffer
{


template
<
    typename Linestring,
    typename Polygon,
    typename DistanceStrategy,
    typename JoinStrategy
>
struct linestring_buffer
{
    typedef typename coordinate_type<Polygon>::type coordinate_type;
    typedef typename point_type<Polygon>::type output_point_type;
    typedef model::referring_segment<output_point_type const> segment_type;

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    typedef typename ring_type<Polygon>::type ring_type;
#endif

    template
    <
        typename Inserter,
        typename Iterator
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
        , typename Mapper
#endif
    >
    static inline void iterate(Inserter& inserter,
                Iterator begin, Iterator end,
                buffer_side_selector side,
                DistanceStrategy const& distance,
                JoinStrategy const& join
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

                coordinate_type d = distance.apply(*prev, *it, side);

                set<0>(p2, get<0>(*it) + px * d);
                set<1>(p2, get<1>(*it) + py * d);

                set<0>(p1, get<0>(*prev) + px * d);
                set<1>(p1, get<1>(*prev) + py * d);

                {
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
                    ring_type block;
                    block.push_back(*prev);
                    block.push_back(*it);
                    block.push_back(p2);
                    block.push_back(p1);
                    block.push_back(*prev);

                    mapper.map(block, "opacity:0.4;fill:rgb(0,255,0);stroke:rgb(0,0,0);stroke-width:1");
#endif
                }

                if (! first)
                {
                    output_point_type p;
                    segment_type s1(p1, p2);
                    segment_type s2(previous_p1, previous_p2);
                    if (line_line_intersection<output_point_type, segment_type>::apply(s1, s2, p))
                    {
                        join.apply(p, *prev, previous_p2, p1,
                                    distance.apply(*prev, *it, side),
                                    inserter.get_ring());
                        {
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
                            mapper.map(p, "fill:rgb(0,0,0);", 3);

                            std::ostringstream out;
                            out << index++;
                            mapper.text(p, out.str(), "fill:rgb(0,0,0);font-family='Arial';", 5, 5);
#endif
                        }
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

                    inserter.insert(p1);
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
        inserter.insert(previous_p2);
    }


    template
    <
        typename Inserter
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
        , typename Mapper
#endif
    >
    static inline void apply(Linestring const& linestring, Inserter& inserter,
            DistanceStrategy const& distance,
            JoinStrategy const& join
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            , Mapper& mapper
#endif
            )
    {
        typedef typename boost::range_iterator
            <
                Linestring const
            >::type iterator_type;

        inserter.start_ring();

        iterate(inserter, boost::begin(linestring), boost::end(linestring),
            buffer_side_left,
            distance, join
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            , mapper
#endif
            );

        iterate(inserter, boost::rbegin(linestring), boost::rend(linestring),
            buffer_side_right, distance, join
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            , mapper
#endif
            );

        inserter.close_and_insert_ring();
    }
};


}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_LINESTRING_BUFFER_HPP
