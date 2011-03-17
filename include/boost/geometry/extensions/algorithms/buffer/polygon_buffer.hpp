// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
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

#include <boost/geometry/strategies/buffer_side.hpp>
#include <boost/geometry/extensions/algorithms/buffer/line_line_intersection.hpp>




namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace buffer
{


template <typename RingInput, typename RingOutput, typename JoinStrategy>
struct ring_buffer
{
    typedef typename point_type<RingOutput>::type output_point_type;
    typedef typename coordinate_type<output_point_type>::type coordinate_type;

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    template <typename Mapper>
#endif
    static inline void apply(RingInput const& ring, RingOutput& buffered,
            coordinate_type distance,
            JoinStrategy const& join_strategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            , Mapper& mapper
#endif
            )
    {
        typedef segment<output_point_type const> segment_type;
        typedef typename boost::range_iterator
            <
                RingInput const
            >::type iterator_type;

        output_point_type previous_p1, previous_p2;
        output_point_type first_p1, first_p2;
        bool first = true;

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
        int index = 0;
#endif

        iterator_type it = boost::begin(ring);
        for (iterator_type prev = it++;
            it != boost::end(ring); ++it)
        {
            if (! detail::equals::equals_point_point(*prev, *it))
            {
                bool skip = false;

                // Generate a block along (int most cases to the left of) the segment

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

                coordinate_type d = distance;

                set<0>(p2, get<0>(*it) + px * d);
                set<1>(p2, get<1>(*it) + py * d);

                set<0>(p1, get<0>(*prev) + px * d);
                set<1>(p1, get<1>(*prev) + py * d);

                {
                    RingOutput block;
                    block.push_back(*prev);
                    block.push_back(*it);
                    block.push_back(p2);
                    block.push_back(p1);
                    block.push_back(*prev);

    #ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
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
                        join_strategy.apply(p, *prev, previous_p2, p1, distance, buffered);
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

            join_strategy.apply(p, *boost::begin(ring), previous_p2, first_p1, distance, buffered);

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            mapper.map(p, "fill:rgb(0,0,0);", 3);
            std::ostringstream out;
            out << index++;
            mapper.text(p, out.str(), "fill:rgb(0,0,0);font-family='Arial';", 5, 5);
#endif
        }

        // Close the generated buffer
        {
            output_point_type p = *boost::begin(buffered);
            buffered.push_back(p);
        }
    }
};



template <typename PolygonInput, typename PolygonOutput, typename JoinStrategy>
struct polygon_buffer
{
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    template <typename Mapper>
#endif
    static inline void apply(PolygonInput const& polygon, PolygonOutput& buffered,
            typename coordinate_type<PolygonOutput>::type distance,
            JoinStrategy const& join_strategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            , Mapper& mapper
#endif
            )
    {
        geometry::clear(buffered);

        typedef typename ring_type<PolygonInput>::type input_ring_type;
        typedef typename ring_type<PolygonOutput>::type output_ring_type;

        typedef ring_buffer<input_ring_type, output_ring_type, JoinStrategy> policy;
        policy::apply(exterior_ring(polygon), exterior_ring(buffered),
                distance, join_strategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
                , mapper
#endif
                );

        typename interior_return_type<Polygon const>::type rings
                    = interior_rings(polygon);
        for (BOOST_AUTO_TPL(it, boost::begin(rings); it != boost::end(rings); ++it)
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
