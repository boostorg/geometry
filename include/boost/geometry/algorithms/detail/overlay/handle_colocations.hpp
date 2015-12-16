// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2015 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_HANDLE_COLOCATIONS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_HANDLE_COLOCATIONS_HPP

#include <cstddef>
#include <algorithm>
#include <map>
#include <vector>

#include <boost/range.hpp>
#include <boost/geometry/algorithms/detail/overlay/overlay_type.hpp>
#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>
#include <boost/geometry/algorithms/detail/ring_identifier.hpp>
#include <boost/geometry/algorithms/detail/overlay/segment_identifier.hpp>

#if defined(BOOST_GEOMETRY_DEBUG_HANDLE_COLOCATIONS)
#  include <iostream>
#  include <boost/geometry/algorithms/detail/overlay/debug_turn_info.hpp>
#  include <boost/geometry/io/wkt/wkt.hpp>
#  define BOOST_GEOMETRY_DEBUG_IDENTIFIER
#endif

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{

struct turn_operation_index
{
    turn_operation_index(signed_size_type ti = -1,
                         signed_size_type oi = -1)
        : turn_index(ti)
        , op_index(oi)
    {}

    signed_size_type turn_index;
    signed_size_type op_index; // basically only 0,1
};


template <typename TurnPoints>
struct less_by_fraction_and_type
{
    inline less_by_fraction_and_type(TurnPoints const& turn_points)
        : m_turns(turn_points)
    {
    }

    inline bool operator()(turn_operation_index const& left,
                           turn_operation_index const& right) const
    {
        typedef typename boost::range_value<TurnPoints>::type turn_type;
        typedef typename turn_type::turn_operation_type turn_operation_type;

        turn_type const& left_turn = m_turns[left.turn_index];
        turn_type const& right_turn = m_turns[right.turn_index];
        turn_operation_type const& left_op
                = left_turn.operations[left.op_index];

        turn_operation_type const& right_op
                = right_turn.operations[right.op_index];

        if (! (left_op.fraction == right_op.fraction))
        {
            return left_op.fraction < right_op.fraction;
        }

        // Order xx first - used to discard any following colocated turn
        bool const left_both_xx = left_turn.both(operation_blocked);
        bool const right_both_xx = right_turn.both(operation_blocked);
        if (left_both_xx && ! right_both_xx)
        {
            return true;
        }
        if (! left_both_xx && right_both_xx)
        {
            return false;
        }

        turn_operation_type const& left_other_op
                = left_turn.operations[1 - left.op_index];

        turn_operation_type const& right_other_op
                = right_turn.operations[1 - right.op_index];

        // Fraction is the same, now sort on ring id, first outer ring,
        // then interior rings
        return left_other_op.seg_id < right_other_op.seg_id;
    }

private:
    TurnPoints const& m_turns;
};


template
<
    typename TurnPoints,
    typename OperationVector
>
inline void handle_colocation_cluster(TurnPoints& turn_points,
        OperationVector const& vec)
{
    typedef typename boost::range_value<TurnPoints>::type turn_type;
    typedef typename turn_type::turn_operation_type turn_operation_type;

    std::vector<turn_operation_index>::const_iterator vit = vec.begin();

    turn_operation_index cluster_toi = *vit;

    for (++vit; vit != vec.end(); ++vit)
    {
        turn_type& cluster_turn = turn_points[cluster_toi.turn_index];
        turn_operation_type const& cluster_op
                = cluster_turn.operations[cluster_toi.op_index];

        turn_operation_index const& toi = *vit;
        turn_type& turn = turn_points[toi.turn_index];
        turn_operation_type const& op = turn.operations[toi.op_index];

        if (cluster_op.fraction == op.fraction)
        {
            // In case of colocated xx turns, all other turns may NOT be
            // followed at all. xx cannot be discarded (otherwise colocated
            // turns are followed).
            if (cluster_turn.both(operation_blocked))
            {
                turn.discarded = true;
                turn.colocated = true;
            }
        }
        else
        {
            // Not on same fraction on this segment
            // assign for next potential cluster
            cluster_toi = toi;
        }
    }
}


// Checks colocated turns and flags combinations of uu/other, possibly a
// combination of a ring touching another geometry's interior ring which is
// tangential to the exterior ring

// This function can be extended to replace handle_tangencies: at each
// colocation incoming and outgoing vectors should be inspected

template <typename TurnPoints>
inline void handle_colocations(TurnPoints& turn_points)
{
    typedef std::map
        <
            segment_identifier,
            std::vector<turn_operation_index>
        > map_type;

    // Create and fill map on segment-identifier Map is sorted on seg_id,
    // meaning it is sorted on ring_identifier too. This means that exterior
    // rings are handled first. If there is a colocation on the exterior ring,
    // that information can be used for the interior ring too
    map_type map;

    int index = 0;
    for (typename boost::range_iterator<TurnPoints>::type
            it = boost::begin(turn_points);
         it != boost::end(turn_points);
         ++it, ++index)
    {
        map[it->operations[0].seg_id].push_back(turn_operation_index(index, 0));
        map[it->operations[1].seg_id].push_back(turn_operation_index(index, 1));
    }

    // Check if there are multiple turns on one or more segments,
    // if not then nothing is to be done
    bool colocations = 0;
    for (typename map_type::const_iterator it = map.begin();
         it != map.end();
         ++it)
    {
        if (it->second.size() > 1u)
        {
            colocations = true;
            break;
        }
    }

    if (! colocations)
    {
        return;
    }

    // Sort all vectors, per same segment
    less_by_fraction_and_type<TurnPoints> less(turn_points);
    for (typename map_type::iterator it = map.begin();
         it != map.end(); ++it)
    {
        std::sort(it->second.begin(), it->second.end(), less);
    }

    for (typename map_type::const_iterator it = map.begin();
         it != map.end(); ++it)
    {
        if (it->second.size() > 1)
        {

            handle_colocation_cluster(turn_points, it->second);
        }
    }

#if defined(BOOST_GEOMETRY_DEBUG_HANDLE_COLOCATIONS)
    std::cout << "*** Colocations " << map.size() << std::endl;
    for (typename map_type::const_iterator it = map.begin();
         it != map.end(); ++it)
    {
        std::cout << it->first << std::endl;
        for (std::vector<turn_operation_index>::const_iterator vit
             = it->second.begin(); vit != it->second.end(); ++vit)
        {
            turn_operation_index const& toi = *vit;
            std::cout << geometry::wkt(turn_points[toi.turn_index].point)
                << std::boolalpha
                << " discarded=" << turn_points[toi.turn_index].discarded
                << " colocated=" << turn_points[toi.turn_index].colocated
                << " " << operation_char(turn_points[toi.turn_index].operations[0].operation)
                << " "  << turn_points[toi.turn_index].operations[0].seg_id
                << " "  << turn_points[toi.turn_index].operations[0].fraction
                << " // " << operation_char(turn_points[toi.turn_index].operations[1].operation)
                << " "  << turn_points[toi.turn_index].operations[1].seg_id
                << " "  << turn_points[toi.turn_index].operations[1].fraction
                << std::endl;
        }
    }
#endif // DEBUG

}


}} // namespace detail::overlay
#endif //DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_HANDLE_COLOCATIONS_HPP
