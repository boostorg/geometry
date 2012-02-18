// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_BUFFER_INSERTER_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_BUFFER_INSERTER_HPP

#include <cstddef>

#include <boost/numeric/conversion/cast.hpp>

#include <boost/range.hpp>
#include <boost/typeof/typeof.hpp>

#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>

#include <boost/geometry/extensions/strategies/buffer_side.hpp>
#include <boost/geometry/extensions/algorithms/buffer/buffered_piece_collection.hpp>
#include <boost/geometry/extensions/algorithms/buffer/line_line_intersection.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace buffer
{


template
<
    typename RingOutput,
    typename Tag
>
struct buffer_range
{
    typedef typename point_type<RingOutput>::type output_point_type;
    typedef typename coordinate_type<RingOutput>::type coordinate_type;
    typedef model::referring_segment<output_point_type const> segment_type;

    template
    <
        typename Collection,
        typename Iterator,
        typename DistanceStrategy,
        typename JoinStrategy
    >
    static inline void iterate(Collection& collection,
                Iterator begin, Iterator end,
                buffer_side_selector side,
                DistanceStrategy const& distance,
                JoinStrategy const& join_strategy
            )
    {
        output_point_type previous_p1, previous_p2;
        output_point_type first_p1, first_p2;

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

                std::vector<output_point_type> range_out;
                if (! first)
                {
                    output_point_type p;
                    segment_type s1(p1, p2);
                    segment_type s2(previous_p1, previous_p2);

                    if (line_line_intersection<output_point_type, segment_type>::apply(s1, s2, p))
                    {
                        join_strategy.apply(p, *prev, previous_p2, p1,
                                    distance.apply(*prev, *it, side),
                                    range_out);
                    }
                }
                else
                {
                    first = false;
                    first_p1 = p1;
                    first_p2 = p2;
                }
                if (! range_out.empty())
                {
                    collection.add_piece(*prev, range_out);
                    range_out.clear();
                }
                collection.add_piece(*prev, *it, p1, p2);

                previous_p1 = p1;
                previous_p2 = p2;
                prev = it;
            }
        }

        // Might be replaced by specialization
        if(boost::is_same<Tag, ring_tag>::value)
        {
            // Generate closing corner
            output_point_type p;
            segment_type s1(previous_p1, previous_p2);
            segment_type s2(first_p1, first_p2);
            line_line_intersection<output_point_type, segment_type>::apply(s1, s2, p);

            std::vector<output_point_type> range_out;
            join_strategy.apply(p, *begin, previous_p2, first_p1,
                distance.apply(*(end - 1), *begin, side),
                range_out);
            if (! range_out.empty())
            {
                collection.add_piece(*begin, range_out);
            }

            // Buffer is closed automatically by last closing corner (NOT FOR OPEN POLYGONS - TODO)
        }
    }
};

}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template
<
    typename Tag,
    typename RingInput,
    typename RingOutput
>
struct buffer_inserter
{};


template
<
    typename RingInput,
    typename RingOutput
>
struct buffer_inserter<ring_tag, RingInput, RingOutput>
    : public detail::buffer::buffer_range
        <
            RingOutput, 
            ring_tag
        >
{

    template
    <
        typename Collection, typename DistanceStrategy, typename JoinStrategy
    >
    static inline void apply(RingInput const& ring,
            Collection& collection,
            DistanceStrategy const& distance,
            JoinStrategy const& join_strategy)
    {
        iterate(collection, boost::begin(ring), boost::end(ring),
                buffer_side_left,
                distance, join_strategy);
    }
};


template
<
    typename Linestring,
    typename Polygon
>
struct buffer_inserter<linestring_tag, Linestring, Polygon>
    : public detail::buffer::buffer_range
        <
            typename ring_type<Polygon>::type, 
            linestring_tag
        >
{

    template<typename Collection, typename DistanceStrategy, typename JoinStrategy>
    static inline void apply(Linestring const& linestring, Collection& collection,
            DistanceStrategy const& distance,
            JoinStrategy const& join_strategy)
    {
        collection.add_input();
        iterate(collection, boost::begin(linestring), boost::end(linestring),
                buffer_side_left,
                distance, join_strategy);

        iterate(collection, boost::rbegin(linestring), boost::rend(linestring),
                buffer_side_right,
                distance, join_strategy);

    }
};


template
<
    typename PolygonInput,
    typename PolygonOutput
>
struct buffer_inserter<polygon_tag, PolygonInput, PolygonOutput>
{

    template <typename Collection, typename DistanceStrategy, typename JoinStrategy>
    static inline void apply(PolygonInput const& polygon, Collection& collection,
            DistanceStrategy const& distance,
            JoinStrategy const& join_strategy)
    {

        typedef typename ring_type<PolygonInput>::type input_ring_type;
        typedef typename ring_type<PolygonOutput>::type output_ring_type;

        typedef buffer_inserter<ring_tag, input_ring_type, output_ring_type> policy;

        {
            collection.add_input();

            policy::apply(exterior_ring(polygon), collection,
                    distance, join_strategy);
        }

        typename interior_return_type<PolygonInput const>::type rings
                    = interior_rings(polygon);
        for (BOOST_AUTO_TPL(it, boost::begin(rings)); it != boost::end(rings); ++it)
        {
            output_ring_type ring;

            collection.add_input();

            policy::apply(*it, collection, distance, join_strategy);
        }

    }
};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


template
<
    typename GeometryOutput,
    typename GeometryInput,
    typename OutputIterator,
    typename DistanceStrategy,
    typename JoinStrategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    , typename Mapper
#endif
>
inline void buffer_inserter(GeometryInput const& geometry_input, OutputIterator out,
        DistanceStrategy const& distance_strategy, JoinStrategy const& join_strategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
            , Mapper& mapper
#endif
    )
{
    detail::buffer::buffered_piece_collection<typename geometry::ring_type<GeometryOutput>::type> collection;

    dispatch::buffer_inserter
        <
            typename tag<GeometryInput>::type, 
            GeometryInput, 
            GeometryOutput
        >::apply(geometry_input, collection, distance_strategy, join_strategy);

    collection.get_turns(geometry_input);

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    collection.map_offsetted(mapper);
#endif

    collection.discard_points();
    collection.enrich();
    collection.traverse();

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    //collection.map<geometry::polygon_tag>(mapper);
    collection.map_turns(mapper);
    collection.map_traverse(mapper);
#endif

    collection.assign<GeometryOutput>(out);
}


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_BUFFER_INSERTER_HPP
