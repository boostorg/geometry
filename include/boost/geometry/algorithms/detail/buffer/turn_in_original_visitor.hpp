// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_TURN_IN_ORIGINAL_VISITOR
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_TURN_IN_ORIGINAL_VISITOR

#include <boost/geometry/algorithms/expand.hpp>
#include <boost/geometry/algorithms/detail/disjoint/point_box.hpp>
#include <boost/geometry/algorithms/detail/within/point_in_geometry.hpp>
#include <boost/geometry/strategies/buffer.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace buffer
{

struct original_get_box
{
    template <typename Box, typename Original>
    static inline void apply(Box& total, Original const& original)
    {
        geometry::expand(total, original.m_box);
    }
};

struct original_ovelaps_box
{
    template <typename Box, typename Original>
    static inline bool apply(Box const& box, Original const& original)
    {
        return ! detail::disjoint::disjoint_box_box(box, original.m_box);
    }
};

struct include_turn_policy
{
    template <typename Turn>
    static inline bool apply(Turn const& turn)
    {
        return turn.location == location_ok;
    }
};

struct turn_in_original_ovelaps_box
{
    template <typename Box, typename Turn>
    static inline bool apply(Box const& box, Turn const& turn)
    {
        if (turn.location != location_ok || turn.within_original)
        {
            // Skip all points already processed
            return false;
        }

        return ! dispatch::disjoint
            <
                typename Turn::robust_point_type,
                Box
            >::apply(turn.robust_point, box);
    }
};


template <typename Turns>
class turn_in_original_visitor
{
public:
    turn_in_original_visitor(Turns& turns)
        : m_mutable_turns(turns)
    {}

    template <typename Turn, typename Original>
    inline void apply(Turn const& turn, Original const& original, bool first = true)
    {
        boost::ignore_unused_variable_warning(first);

        if (turn.location != location_ok || turn.within_original)
        {
            // Skip all points already processed
            return;
        }

        if (geometry::disjoint(turn.robust_point, original.m_box))
        {
            // Skip all disjoint
            return;
        }

        int const code = detail::within::point_in_geometry(turn.robust_point, original.m_ring);

        if (code == -1)
        {
            return;
        }

        Turn& mutable_turn = m_mutable_turns[turn.turn_index];

        if (code == 0)
        {
            // On border of original: always discard
            mutable_turn.location = location_discard;
        }

        // Point is inside an original ring
        if (original.m_is_interior)
        {
            mutable_turn.count_in_original--;
        }
        else if (original.m_has_interiors)
        {
            mutable_turn.count_in_original++;
        }
        else
        {
            // It is an exterior ring and there are no interior rings.
            // Then we are ready with this point
            mutable_turn.within_original = true;
            mutable_turn.count_in_original = 1;
        }
    }

private :
    Turns& m_mutable_turns;
};


}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_TURN_IN_ORIGINAL_VISITOR
