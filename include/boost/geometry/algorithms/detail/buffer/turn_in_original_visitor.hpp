// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_TURN_IN_ORIGINAL_VISITOR
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_TURN_IN_ORIGINAL_VISITOR

#include <boost/geometry/algorithms/expand.hpp>
#include <boost/geometry/algorithms/detail/disjoint/point_box.hpp>
#include <boost/geometry/strategies/agnostic/point_in_poly_winding.hpp>
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

template <typename Point, typename Original>
inline int point_in_original(Point const& point, Original const& original)
{
    typedef strategy::within::winding<Point> strategy_type;

    typedef typename Original::original_robust_ring_type ring_type;
    typedef typename Original::sections_type sections_type;
    typedef typename boost::range_iterator<ring_type const>::type iterator_type;
    typedef typename boost::range_iterator<sections_type const>::type siterator_type;

    typename geometry::coordinate_type<Point>::type const point_y
            = geometry::get<1>(point);

    typename strategy_type::state_type state;
    strategy_type strategy;

    // Walk through all monotonic sections
    for (siterator_type sit = boost::begin(original.m_sections);
        sit != boost::end(original.m_sections);
        ++sit)
    {
        typename boost::range_value<sections_type const>::type const& section = *sit;
        if (! section.duplicate
            && section.begin_index < section.end_index
            && point_y >= geometry::get<min_corner, 1>(section.bounding_box)
            && point_y <= geometry::get<max_corner, 1>(section.bounding_box))
        {
            // Walk through this section
            iterator_type it
                = boost::begin(original.m_ring) + section.begin_index;
            iterator_type const end
                = boost::begin(original.m_ring) + section.end_index + 1;
            for (iterator_type previous = it++; it != end; ++previous, ++it)
            {
                if (! strategy.apply(point, *previous, *it, state))
                {
                    break;
                }

                // Depending on sections.direction we can quit the inner loop
                typename geometry::coordinate_type<Point>::type const ring_y
                        = geometry::get<1>(*it);

                if (section.directions[0] == 1 && point_y < ring_y)
                {
                    break;
                }
                else if (section.directions[0] == -1 && point_y > ring_y)
                {
                    break;
                }
            }
        }
    }

    return strategy.result(state);
}


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

        int const code = point_in_original(turn.robust_point, original);

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
