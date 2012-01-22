// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_POLYGON_BUFFER_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_POLYGON_BUFFER_HPP

#include <cstddef>

#include <boost/numeric/conversion/cast.hpp>

#include <boost/range.hpp>
#include <boost/typeof/typeof.hpp>

#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/interior_rings.hpp>

#include <boost/geometry/extensions/strategies/buffer_side.hpp>
#include <boost/geometry/extensions/algorithms/buffer/line_line_intersection.hpp>




namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace buffer
{


template
<
    typename RingInput,
    typename RingOutput,
    typename DistanceStrategy,
    typename JoinStrategy
>
struct ring_buffer
{
    typedef typename point_type<RingOutput>::type output_point_type;
    typedef typename coordinate_type<output_point_type>::type coordinate_type;
    typedef model::referring_segment<output_point_type const> segment_type;

    template
    <
        typename Iterator
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
        , typename Mapper
#endif
    >
    static inline void iterate(RingOutput& buffered,
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
                bool skip = false;

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

                {
    #ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
                    RingOutput block;
                    block.push_back(*prev);
                    block.push_back(*it);
                    block.push_back(p2);
                    block.push_back(p1);
                    block.push_back(*prev);

                    mapper.map(block, "opacity:0.4;fill:rgb(255,128,0);stroke:rgb(0,0,0);stroke-width:1");
    #endif
                }

                if (! first)
                {
                    output_point_type p;
                    segment_type s1(p1, p2);
                    segment_type s2(previous_p1, previous_p2);
                    if (line_line_intersection<output_point_type, segment_type>::apply(s1, s2, p))
                    {
                        join_strategy.apply(p, *prev, previous_p2, p1,
                                    distance.apply(*prev, *it, side),
                                    buffered);
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
        {
            output_point_type p;
            segment_type s1(previous_p1, previous_p2);
            segment_type s2(first_p1, first_p2);
            line_line_intersection<output_point_type, segment_type>::apply(s1, s2, p);

            join_strategy.apply(p, *begin, previous_p2, first_p1,
                distance.apply(*(end - 1), *begin, side),
                buffered);

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            mapper.map(p, "fill:rgb(0,0,0);", 3);
            std::ostringstream out;
            out << index++;
            mapper.text(p, out.str(), "fill:rgb(0,0,0);font-family='Arial';", 5, 5);
#endif
        }
    }

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    template <typename Mapper>
#endif
    static inline void apply(RingInput const& ring,
                RingOutput& buffered,

                DistanceStrategy const& distance,

            JoinStrategy const& join_strategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            , Mapper& mapper
#endif
            )
    {
        iterate(buffered, boost::begin(ring), boost::end(ring),
                buffer_side_left,
                distance, join_strategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            , mapper
#endif
            );


        // Close the generated buffer
        {
            output_point_type p = *boost::begin(buffered);
            buffered.push_back(p);
        }
    }
};



template
<
    typename PolygonInput,
    typename PolygonOutput,
    typename DistanceStrategy,
    typename JoinStrategy
>
struct polygon_buffer
{
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    template <typename Mapper>
#endif
    static inline void apply(PolygonInput const& polygon, PolygonOutput& buffered,
            DistanceStrategy const& distance,
            JoinStrategy const& join_strategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            , Mapper& mapper
#endif
            )
    {
        geometry::clear(buffered);

        typedef typename ring_type<PolygonInput>::type input_ring_type;
        typedef typename ring_type<PolygonOutput>::type output_ring_type;

        typedef ring_buffer<input_ring_type, output_ring_type, DistanceStrategy, JoinStrategy> policy;
        policy::apply(exterior_ring(polygon), exterior_ring(buffered),
                distance, join_strategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
                , mapper
#endif
                );

        typename interior_return_type<PolygonInput const>::type rings
                    = interior_rings(polygon);
        for (BOOST_AUTO_TPL(it, boost::begin(rings)); it != boost::end(rings); ++it)
        {
            output_ring_type ring;
            policy::apply(*it, ring, distance, join_strategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
                , mapper
#endif
                );
            interior_rings(buffered).push_back(ring);
        }
    }
};


}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_POLYGON_BUFFER_HPP
