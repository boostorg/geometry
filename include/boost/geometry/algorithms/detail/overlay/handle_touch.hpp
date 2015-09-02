// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2015 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_HANDLE_TOUCH_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_HANDLE_TOUCH_HPP

#include <cstddef>

#include <map>
#include <vector>

#include <boost/range.hpp>

#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>
#include <boost/geometry/geometries/concepts/check.hpp>
#include <boost/geometry/algorithms/detail/ring_identifier.hpp>
#include <boost/geometry/algorithms/detail/overlay/segment_identifier.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{


template <typename Turns>
class handle_touch_uu
{
private :
    typedef typename boost::range_value<Turns>::type turn_type;
    typedef typename boost::range_iterator<Turns>::type turn_iterator;
    typedef typename boost::range_iterator<Turns const>::type turn_const_iterator;

    typedef typename boost::range_iterator
        <
            typename turn_type::container_type const
        >::type operation_const_iterator;

    typedef std::map<ring_identifier, std::vector<int> > map_type;

public :
    static inline void apply(detail::overlay::operation_type operation,
                Turns& turns)
    {
        if (! has_uu(turns))
        {
            // Performance - if there is no u/u at all, nothing to be done
            return;
        }

        map_type turns_per_ring;
        create_ring_map(turns, turns_per_ring);

        handle(turns, turns_per_ring);
    }

private :

    // Generic utility to be moved somewhere else
    static inline
    ring_identifier ring_id_from_seg_id(const segment_identifier& seg_id)
    {
        return ring_identifier(seg_id.source_index,
                               seg_id.multi_index,
                               seg_id.ring_index);
    }

    static inline
    ring_identifier ring_id_from_op(const turn_type& turn,
                                    int operation_index)
    {
        return ring_id_from_seg_id(turn.operations[operation_index].seg_id);
    }

    static inline bool has_uu(const Turns& turns)
    {
        for (turn_const_iterator it = boost::begin(turns);
            it != boost::end(turns);
            ++it)
        {
            const turn_type& turn = *it;
            if (turn.both(operation_union))
            {
                return true;
            }
        }
        return false;
    }

    // Create a map of turns per ring (in both sources), excluding u/u turns
    // and other discarded turns
    static inline void create_ring_map(const Turns& turns, map_type& map)
    {
        int index = 0;
        for (turn_const_iterator it = boost::begin(turns);
            it != boost::end(turns);
            ++it, ++index)
        {
            const turn_type& turn = *it;
            if (! turn.both(operation_union) && ! turn.discarded)
            {
                map[ring_id_from_op(turn, 0)].push_back(index);
                map[ring_id_from_op(turn, 1)].push_back(index);
            }
        }
    }


    static inline void handle(Turns& turns, const map_type& map)
    {
        // Iterate through all u/u points
        int index = 0;
        for (turn_iterator it = boost::begin(turns);
            it != boost::end(turns);
            ++it, ++index)
        {
            turn_type& turn = *it;
            if (turn.both(operation_union))
            {
                bool traverse = turn_should_be_traversed(turns, turn, map);
#ifdef BOOST_GEOMETRY_DEBUG_HANDLE_TOUCH
                std::cout << " " << index << " "
                             << std::boolalpha << traverse
                          << std::endl;
#endif

                if (traverse)
                {
                    // Remove the discarded flag
                    turn.discarded = false;

                    // TODO: insert into sequence
                }
            }
        }
    }

    static inline bool turn_should_be_traversed(const Turns& turns,
                                                const turn_type& uu_turn,
                                                const map_type& map)
    {
        // Suppose this is a u/u turn between P and Q
        // Examine all other turns on P and check if Q can be reached

        ring_identifier const ring_id_p = ring_id_from_op(uu_turn, 0);
        ring_identifier const ring_id_q = ring_id_from_op(uu_turn, 1);

        // Use one of the operations and check if you can reach the other

        map_type::const_iterator mit = map.find(ring_id_p);
        if (mit == map.end())
        {
            // No other turns found
            return false;
        }

#ifdef BOOST_GEOMETRY_DEBUG_HANDLE_TOUCH
        std::cout << " Check p: " << ring_id_p << " q: " << ring_id_q << std::endl;
#endif

        for (std::vector<int>::const_iterator vit = mit->second.begin();
             vit != mit->second.end();
             ++vit)
        {
            int const turn_index = *vit;
            const turn_type& current_turn = turns[turn_index];
#ifdef BOOST_GEOMETRY_DEBUG_HANDLE_TOUCH
            std::cout << "-> Examine " << turn_index << std::endl;
#endif
            if (can_reach(turns, current_turn, ring_id_q, turn_index))
            {
                return true;
            }
        }

        return false;
    }

    static inline bool can_reach(const Turns& turns,
                                 const turn_type& turn,
                                 const ring_identifier& target_ring_id,
                                 signed_size_type original_turn_index,
                                 std::size_t iteration = 0)
    {
        signed_size_type const turns_size =
                static_cast<signed_size_type>(boost::size(turns));

        // For any union operation in both operations, check if the specified
        // ring can be reached again
        for (operation_const_iterator it = boost::begin(turn.operations);
            it != boost::end(turn.operations);
            ++it)
        {
            if (it->operation == operation_union)
            {
                signed_size_type index = it->enriched.travels_to_ip_index;
                if (index == original_turn_index)
                {
                    // Traveled through, not found
                    return false;
                }
                if (index >= 0 && index < turns_size)
                {
#ifdef BOOST_GEOMETRY_DEBUG_HANDLE_TOUCH
                    std::cout << "   Now to " << index << std::endl;
#endif
                    const turn_type& new_turn = turns[index];


                    ring_identifier const ring_id1 = ring_id_from_op(new_turn, 0);
                    ring_identifier const ring_id2 = ring_id_from_op(new_turn, 1);
                    if (ring_id1 == target_ring_id
                            || ring_id2 == target_ring_id)
                    {
#ifdef BOOST_GEOMETRY_DEBUG_HANDLE_TOUCH
                        std::cout << " Found!" << std::endl;
#endif
                        return true;
                    }

                    if (iteration >= boost::size(turns))
                    {
                        // Defensive check to avoid infinite recursion
                        return false;
                    }

                    // Recursively check this turn
                    if (can_reach(turns, new_turn, target_ring_id,
                                         original_turn_index, iteration + 1))
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }
};

template <typename Turns>
inline void handle_touch(detail::overlay::operation_type operation,
                         Turns& turns)
{
    handle_touch_uu<Turns>::apply(operation, turns);
}

}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_HANDLE_TOUCH_HPP
