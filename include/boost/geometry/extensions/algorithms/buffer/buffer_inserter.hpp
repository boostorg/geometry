// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2012 Barend Gehrels, Amsterdam, the Netherlands.

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

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
#  include <boost/geometry/extensions/algorithms/buffer/buffered_piece_collection_with_mapper.hpp>
#endif


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
            typename Point,
            typename DistanceStrategy
        >
    static inline void generate_side(Point const& ip1, Point const& ip2,
                buffer_side_selector side,
                DistanceStrategy const& distance,
                output_point_type& p1, output_point_type& p2)
    {
        // Generate a block along (left or right of) the segment

        // Simulate a vector d (dx,dy)
        coordinate_type dx = get<0>(ip2) - get<0>(ip1);
        coordinate_type dy = get<1>(ip2) - get<1>(ip1);

        // For normalization [0,1] (=dot product d.d, sqrt)
        // TODO promoted_type
        coordinate_type const length = sqrt(dx * dx + dy * dy);

        // Because coordinates are not equal, length should not be zero
        BOOST_ASSERT((! geometry::math::equals(length, 0)));

        // Generate the normalized perpendicular p, to the left (ccw)
        coordinate_type const px = -dy / length;
        coordinate_type const py = dx / length;

        coordinate_type const d = distance.apply(ip1, ip2, side);

        set<0>(p1, get<0>(ip1) + px * d);
        set<1>(p1, get<1>(ip1) + py * d);
        set<0>(p2, get<0>(ip2) + px * d);
        set<1>(p2, get<1>(ip2) + py * d);
    }

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
                JoinStrategy const& join_strategy, bool close = false)
    {
        output_point_type previous_p1, previous_p2;
        output_point_type first_p1, first_p2;

        bool first = true;

        Iterator it = begin;

        // We want to memorize the last vector too.
        typedef BOOST_TYPEOF(*it) point_type;
        point_type last_ip1, last_ip2;


        for (Iterator prev = it++; it != end; ++it)
        {
            if (! detail::equals::equals_point_point(*prev, *it))
            {
                output_point_type p1, p2;
                last_ip1 = *prev;
                last_ip2 = *it;
                generate_side(*prev, *it, side, distance, p1, p2);

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
                    collection.add_piece(buffered_join, *prev, range_out);
                    range_out.clear();
                }
                collection.add_piece(buffered_segment, *prev, *it, p1, p2);

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
            if (line_line_intersection<output_point_type, segment_type>::apply(s1, s2, p))
            {
                std::vector<output_point_type> range_out;
                join_strategy.apply(p, *begin, previous_p2, first_p1,
                    distance.apply(*(end - 1), *begin, side),
                    range_out);
                if (! range_out.empty())
                {
                    collection.add_piece(buffered_join, *begin, range_out);
                }
            }

            // Buffer is closed automatically by last closing corner (NOT FOR OPEN POLYGONS - TODO)
        }
        else if (boost::is_same<Tag, linestring_tag>::value)
        {
            // Assume flat-end-strategy for now
            // TODO fix this (approach) for one-side buffer (1.5 - -1.0)
            output_point_type rp1, rp2;
            generate_side(last_ip2, last_ip1, 
                    side == buffer_side_left 
                    ? buffer_side_right 
                    : buffer_side_left, 
                distance, rp2, rp1);

            // For flat end:
            std::vector<output_point_type> range_out;
            range_out.push_back(previous_p2);
            if (close)
            {
                range_out.push_back(rp2);
            }
            collection.add_piece(buffered_flat_end, range_out);
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
    typedef detail::buffer::buffer_range
        <
            RingOutput, 
            ring_tag
        > base;

    template
    <
        typename Collection, typename DistanceStrategy, typename JoinStrategy
    >
    static inline void apply(RingInput const& ring,
            Collection& collection,
            DistanceStrategy const& distance,
            JoinStrategy const& join_strategy)
    {
		if (boost::size(ring) > 3)
		{
			base::iterate(collection, boost::begin(ring), boost::end(ring),
					buffer_side_left,
					distance, join_strategy);
		}
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
    typedef detail::buffer::buffer_range
        <
            typename ring_type<Polygon>::type, 
            linestring_tag
        > base;

    template<typename Collection, typename DistanceStrategy, typename JoinStrategy>
    static inline void apply(Linestring const& linestring, Collection& collection,
            DistanceStrategy const& distance,
            JoinStrategy const& join_strategy)
    {
		if (boost::size(linestring) > 1)
		{
			collection.start_new_ring();
			base::iterate(collection, boost::begin(linestring), boost::end(linestring),
					buffer_side_left,
					distance, join_strategy);
                
			base::iterate(collection, boost::rbegin(linestring), boost::rend(linestring),
					buffer_side_right,
					distance, join_strategy, true);
		}

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
            collection.start_new_ring();
            policy::apply(exterior_ring(polygon), collection,
                    distance, join_strategy);
        }

        typename interior_return_type<PolygonInput const>::type rings
                    = interior_rings(polygon);
        for (BOOST_AUTO_TPL(it, boost::begin(rings)); it != boost::end(rings); ++it)
        {
            collection.start_new_ring();
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
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    detail::buffer::buffered_piece_collection_with_mapper
#else
    detail::buffer::buffered_piece_collection
#endif
        <
            typename geometry::ring_type<GeometryOutput>::type
        > collection;

    dispatch::buffer_inserter
        <
            typename tag<GeometryInput>::type, 
            GeometryInput, 
            GeometryOutput
        >::apply(geometry_input, collection, distance_strategy, join_strategy);

    collection.get_turns(geometry_input, distance_strategy.factor());

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    //collection.map_offsetted(mapper);
	//collection.map_offsetted_points(mapper);
	collection.map_turns(mapper);
    //collection.map_opposite_locations(mapper);
#endif

    collection.discard_rings();
    collection.discard_turns();
    collection.enrich();
    collection.traverse();

#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    //collection.map_turns(mapper);
    collection.map_pieces<geometry::polygon_tag>(mapper); //, false, true);
    //collection.map_traverse(mapper);
#endif

    collection.template assign<GeometryOutput>(out);
}


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_BUFFER_INSERTER_HPP
