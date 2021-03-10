// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2021 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_DISCARD_DUPLICATE_TURNS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_DISCARD_DUPLICATE_TURNS_HPP

#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_ring.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{


template <typename Geometry0, typename Geometry1>
inline bool is_consecutive(segment_identifier const& first,
                           segment_identifier const& second,
                           Geometry0 const& geometry0,
                           Geometry1 const& geometry1)
{
    if (first.source_index == second.source_index
        && first.ring_index == second.ring_index
        && first.piece_index == second.piece_index
        && first.multi_index == second.multi_index)
    {
        // If the segment distance is 1, there are no segments in between
        // and the segments are consecutive.
        signed_size_type const sd = first.source_index == 0
                        ? segment_distance(geometry0, first, second)
                        : segment_distance(geometry1, first, second);
        return sd == 1;
    }
    return false;
}


// Returns true if two turns correspond to each other in the sense that one has
// "method_start" and the other has "method_touch" or "method_interior" at that
// same point (but with next segment)
template <typename Turn, typename Geometry0, typename Geometry1>
inline bool corresponding_turn(Turn const& turn, Turn const& start_turn,
                               Geometry0 const& geometry0,
                               Geometry1 const& geometry1)
{
    for (std::size_t i = 0; i < 2; i++)
    {
        for (std::size_t j = 0; j < 2; j++)
        {
            if (turn.operations[i].seg_id == start_turn.operations[j].seg_id
                && (turn.method == method_touch
                    || turn.method == method_touch_interior))
            {
                // Verify if the other operation is consecutive
                return is_consecutive(turn.operations[1 - i].seg_id,
                                      start_turn.operations[1 - j].seg_id,
                                      geometry0, geometry1);
            }
        }
    }
    return false;
}


template <typename Turns, typename Geometry0, typename Geometry1>
inline void discard_duplicate_start_turns(Turns& turns,
                                          Geometry0 const& geometry0,
                                          Geometry1 const& geometry1)
{
    // Start turns are generated, in case the previous turn is missed
    // But often it is not missed, and then it should still be deleted
    // This is how it can be
    // (in float, collinear, points far apart due to floating point precision)
    //   [m, i s:0, v:6 1/1 (1) // u s:1, v:5  pnt (2.54044, 3.12623)]
    //   [s, i s:0, v:7 0/1 (0) // u s:1, v:5  pnt (2.70711, 3.29289)]

    // 1 Build map (segment->turnids) of start turns
    std::map<segment_identifier, std::vector<std::size_t>> start_turns_per_segment;
    std::size_t index = 0;
    for (auto const& turn : turns)
    {
        if (turn.method == method_start)
        {
            for (const auto& op : turn.operations)
            {
                start_turns_per_segment[op.seg_id].push_back(index);
            }
        }
        index++;
    }

    // 2: Verify all other turns if they are present in the map, and if so,
    //    if they have the other turns as corresponding
    for (auto const& turn : turns)
    {
        if (turn.method != method_start)
        {
            for (const auto& op : turn.operations)
            {
                auto it = start_turns_per_segment.find(op.seg_id);
                if (it != start_turns_per_segment.end())
                {
                    for (std::size_t const& i : it->second)
                    {
                        if (corresponding_turn(turn, turns[i],
                                               geometry0, geometry1))
                        {
                            turns[i].discarded = true;
                        }
                    }
                }
            }
        }
        index++;
    }
}


}} // namespace detail::overlay
#endif //DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_DISCARD_DUPLICATE_TURNS_HPP
