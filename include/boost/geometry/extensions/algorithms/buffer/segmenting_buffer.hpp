// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_SEGMENTING_BUFFER_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_SEGMENTING_BUFFER_HPP


#include <cstddef>
#include <vector>

#include <boost/foreach.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include <boost/range.hpp>

#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/ring_type.hpp>

#include <boost/geometry/extensions/strategies/buffer_side.hpp>
#include <boost/geometry/extensions/strategies/buffer_join_round.hpp>
#include <boost/geometry/util/math.hpp>

#include <boost/geometry/algorithms/detail/disjoint.hpp>


#include <boost/geometry/extensions/algorithms/dissolve.hpp>


// TEMPORARY do not use yet.


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace buffer
{


template <typename Point, typename PointOut, typename DistanceType>
inline bool calculate_parallels(Point const& point1, Point const& point2,
            PointOut& p1, PointOut& p2, DistanceType const& distance)
{
    typedef typename coordinate_type<Point>::type coordinate_type;

    // Simulate a vector d (dx,dy)
    coordinate_type dx = get<0>(point2) - get<0>(point1);
    coordinate_type dy = get<1>(point2) - get<1>(point1);

    // For normalization [0,1] (=dot product d.d, sqrt)
    // TODO: promote to non-integer
    coordinate_type length = sqrt(dx * dx + dy * dy);

    // Because coordinates are not equal, should been checked before, length should not be zero
    if(geometry::math::equals(length, 0))
    {
        return false;
    }

    // Generate the normalized perpendicular p, to the left (ccw)
    coordinate_type px = -dy / length;
    coordinate_type py = dx / length;

    set<0>(p1, get<0>(point1) + px * distance);
    set<1>(p1, get<1>(point1) + py * distance);

    set<0>(p2, get<0>(point2) + px * distance);
    set<1>(p2, get<1>(point2) + py * distance);
    return true;
}



template
<
    typename GeometryOut, typename Range,
    typename DistanceStrategy,
    typename JoinStrategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    , typename Mapper
#endif
>
struct per_segment_buffer
{
    typedef typename point_type<Range>::type input_point_type;
    typedef typename point_type<GeometryOut>::type point_type;
    typedef typename coordinate_type<point_type>::type coordinate_type;
    typedef coordinate_type distance_type; // TODO promote to FP
    typedef segment<point_type const> segment_type;
    typedef typename geometry::ring_type<GeometryOut>::type ring_type;
    typedef typename strategy_side<typename cs_tag<point_type>::type>::type side;

    typedef typename boost::range_iterator<Range const>::type iterator_type;


    static inline void calculate_tail(input_point_type const& tail, input_point_type const& head,
            point_type& tail_left, point_type& tail_right,
            distance_type& distance_left,
            distance_type& distance_right,
            distance_type& buffered_length)
    {
        coordinate_type ext_x = get<0>(head) - get<0>(tail);
        coordinate_type ext_y = get<1>(head) - get<1>(tail);
        distance_type segment_length = sqrt(ext_x * ext_x + ext_y * ext_y);

        if (buffered_length < geometry::math::abs(distance_left))
        {
            distance_left = buffered_length * distance_left < 0 ? -1.0 : 1.0;
        }
        if (buffered_length < geometry::math::abs(distance_right))
        {
            distance_right = buffered_length * distance_right < 0 ? -1.0 : 1.0;
        }

        distance_type prop_left = geometry::math::abs(distance_left) / segment_length;
        distance_type prop_right = geometry::math::abs(distance_right) / segment_length;

        set<0>(tail_left, get<0>(tail) - ext_x * prop_left);
        set<1>(tail_left, get<1>(tail) - ext_y * prop_left);
        set<0>(tail_right, get<0>(tail) - ext_x * prop_right);
        set<1>(tail_right, get<1>(tail) - ext_y * prop_right);

        buffered_length += segment_length;
    }


    static inline void calculate_head(input_point_type const& tail, input_point_type const& head,
            point_type& head_left, point_type& head_right,
            distance_type& distance_left,
            distance_type& distance_right,
            distance_type const& rest_length)
    {
        coordinate_type ext_x = get<0>(head) - get<0>(tail);
        coordinate_type ext_y = get<1>(head) - get<1>(tail);
        distance_type segment_length = sqrt(ext_x * ext_x + ext_y * ext_y);

        if (rest_length < distance_left)
        {
            distance_left = rest_length;
        }
        if (rest_length < distance_right)
        {
            distance_right = rest_length;
        }

        distance_type prop_left = distance_left / segment_length;
        distance_type prop_right = distance_right / segment_length;

        set<0>(head_left, get<0>(head) + ext_x * prop_left);
        set<1>(head_left, get<1>(head) + ext_y * prop_left);
        set<0>(head_right, get<0>(head) + ext_x * prop_right);
        set<1>(head_right, get<1>(head) + ext_y * prop_right);
    }

    static inline void apply(Range const& range,
        std::vector<GeometryOut>& buffered,
        DistanceStrategy const& distance_strategy,
        JoinStrategy const& join_strategy
    #ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
        , Mapper& mapper
    #endif
        )
    {
        // Buffer all segments separately, and put them in a temporary vector
        std::vector<GeometryOut> buffered_pieces;

        bool first = true;

        strategy::buffer::join_round2<point_type> new_join;
        //strategy::buffer::join_none<point_type> new_join;

        distance_type range_length = geometry::length(range);
        distance_type buffered_length = coordinate_type();

        iterator_type it = boost::begin(range);
        //iterator_type it_c = boost::end(range);
        for (iterator_type previous = it++;
            it != boost::end(range);
            ++it)
        {
            bool const last = it + 1 == boost::end(range);

            distance_type distance_left = distance_strategy.apply(
                            *previous, *it, buffer_side_left);
            distance_type distance_right = distance_strategy.apply(
                            *previous, *it, buffer_side_right);

            point_type left_ab_p1, left_ab_p2, right_ab_p1, right_ab_p2;

            if (detail::disjoint::disjoint_point_point(*previous, *it)
                && calculate_parallels(*previous, *it, left_ab_p1, left_ab_p2,
                        distance_left)
                && calculate_parallels(*previous, *it, right_ab_p1, right_ab_p2,
                        -distance_right)
                )
            {
                {
                    point_type tail_left, tail_right, head_left, head_right;

                    // For flat buffers, generate corners only if outside range of buffer_distance
                    distance_type distance_tail_left = distance_left;
                    distance_type distance_tail_right = distance_right;
                    distance_type distance_head_left = distance_left;
                    distance_type distance_head_right = distance_right;

                    calculate_tail(*previous, *it,
                            tail_left, tail_right,
                            distance_tail_left, distance_tail_right,
                            buffered_length);

                    distance_type const rest_length = range_length - buffered_length;

                    calculate_head(*previous, *it,
                            head_left, head_right,
                            distance_head_left, distance_head_right,
                            rest_length);

                    // Add non-axis aligned rectangle
                    buffered_pieces.resize(buffered_pieces.size() + 1);
                    ring_type& ring = exterior_ring(buffered_pieces.back());
                    ring.push_back(left_ab_p1);
                    ring.push_back(left_ab_p2);

                    if (! last)
                    {
                        new_join.apply(*it, left_ab_p2, left_ab_p2, head_left,
                                    distance_left, distance_head_left,
                                    std::back_inserter(ring));

                        ring.push_back(head_left);
                        ring.push_back(head_right);

                        new_join.apply(*it, right_ab_p2, head_right, right_ab_p2,
                                    distance_right, distance_head_right,
                                    std::back_inserter(ring));
                    }

                    ring.push_back(right_ab_p2);
                    ring.push_back(right_ab_p1);

                    if (! first)
                    {
                        new_join.apply(*previous, right_ab_p1, right_ab_p1, tail_right,
                                    distance_right, distance_tail_right,
                                    std::back_inserter(ring));

                        ring.push_back(tail_right);
                        ring.push_back(tail_left);

                        new_join.apply(*previous, left_ab_p1, tail_left, left_ab_p1,
                                    distance_left, distance_tail_left,
                                    std::back_inserter(ring));
                    }

                    ring.push_back(left_ab_p1);
                }

                previous = it;

                first = false;
            }
        }

        // TEMP, uncomment to see what was actually generated
        //buffered.swap(buffered_pieces);
        //return;
        // END TEMP



        BOOST_FOREACH(GeometryOut const& p, buffered_pieces)
        {
            if (buffered.empty())
            {
                buffered.push_back(p);
            }
            else if (boost::size(buffered) == 1)
            {
                std::vector<GeometryOut> unioned;
                geometry::union_inserter<GeometryOut>(buffered.front(), p, std::back_inserter(unioned));
                buffered.swap(unioned);
            }
            else
            {
                std::cerr << " D " << buffered.size();
                /*std::vector<GeometryOut> dissolved;
                dissolved.push_back(p);
                geometry::dissolver(buffered, dissolved);
                dissolved.swap(buffered);*/
            }
        }

        /***
        std::vector<GeometryOut> dissolved;
        BOOST_FOREACH(GeometryOut const& p, buffered)
        {
            geometry::dissolve(p, dissolved);
        }
        dissolved.swap(buffered);
        ***/

        // Output
    }
};



template
<
    typename GeometryOut, typename Geometry,
    typename DistanceStrategy,
    typename JoinStrategy
#ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
    , typename Mapper
#endif
>
inline void segmenting_buffer(Geometry const& geometry,
        std::vector<GeometryOut>& buffered,
        DistanceStrategy const& distance_strategy,
        JoinStrategy const& join_strategy
    #ifdef BOOST_GEOMETRY_DEBUG_WITH_MAPPER
        , Mapper& mapper
    #endif
        )
{
    per_segment_buffer
        <
            GeometryOut, Geometry,
            DistanceStrategy, JoinStrategy
        >::apply(geometry, buffered, distance_strategy, join_strategy);
}



}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL





}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_SEGMENTING_BUFFER_HPP
