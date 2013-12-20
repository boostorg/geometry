// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013.
// Modifications copyright (c) 2013, Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_TURNS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_TURNS_HPP

#include <boost/geometry/algorithms/detail/overlay/get_turns.hpp>

namespace boost { namespace geometry {

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace relate { namespace turns {

struct operation_order_uibc
{
    template <typename Op> static inline
    int apply(Op const& op)
    {
        switch(op.operation)
        {
        case detail::overlay::operation_opposite : return 0;
        case detail::overlay::operation_none : return 0;
        case detail::overlay::operation_union : return 1;
        case detail::overlay::operation_intersection : return 2;
        case detail::overlay::operation_blocked : return 3;
        case detail::overlay::operation_continue : return 4;
        }
        return -1;
    };
};

// sort turns by G1 - source_index == 0 by:
// seg_id -> distance -> operation
template <typename OperationOrder>
struct less_seg_dist_op
{
    template <typename Op> static inline
    bool use_operation(Op const& left, Op const& right)
    {
        return OperationOrder::apply(left) < OperationOrder::apply(right);
    }

    template <typename Op> static inline
    bool use_other_multi_ring_id(Op const& left, Op const& right)
    {
        //return left.other_id.ring_index < right.other_id.ring_index;
        
        if ( left.other_id.ring_index == -1 )
        {
            if ( right.other_id.ring_index == -1 )
                return use_operation(left, right); // sort by operation
            else
                return true; // right always greater
        }
        else // left.other_id.ring_index != -1
        {
            if ( right.other_id.ring_index == -1 )
                return false; // left always greater

            // here both ring_indexes are greater than -1
            // so first, sort also by multi_index
            return left.other_id.multi_index < right.other_id.multi_index || (
                       left.other_id.multi_index == right.other_id.multi_index && (
                       left.other_id.ring_index < right.other_id.ring_index || (
                           left.other_id.ring_index == right.other_id.ring_index &&
                           use_operation(left, right) )
                    )
                );
        }
    }

    template <typename Op> static inline
    bool use_distance(Op const& left, Op const& right)
    {
        return left.enriched.distance < right.enriched.distance || (
                    geometry::math::equals(left.enriched.distance, right.enriched.distance) &&
                    use_other_multi_ring_id(left, right)
                );
    }

    template <typename Turn>
    inline bool operator()(Turn const& left, Turn const& right) const
    {
        segment_identifier const& sl = left.operations[0].seg_id;
        segment_identifier const& sr = right.operations[0].seg_id;

        return sl < sr || ( sl == sr && use_distance(left.operations[0], right.operations[0]) );
    }
};

template <typename Turn> inline
bool is_valid_method(Turn const& turn)
{
    return turn.method != detail::overlay::method_none
        && turn.method != detail::overlay::method_disjoint
        && turn.method != detail::overlay::method_error;
}

template <typename Turn> inline
bool is_valid_operation(Turn const& turn)
{
    BOOST_ASSERT(!turn.has(detail::overlay::operation_opposite));
    return !turn.both(detail::overlay::operation_none);
}

template <typename Turn> inline
bool is_valid_turn(Turn const& turn)
{
    return is_valid_method(turn) && is_valid_operation(turn);
}

template <bool IsCyclic, typename TurnIt, typename Cond> inline
TurnIt find_next_if(TurnIt first, TurnIt current, TurnIt last, Cond cond)
{
    if ( first == last )
        return last;

    if ( current != last )
    {
        TurnIt it = current;
        ++it;

        for ( ; it != last ; ++it )
            if ( cond(*it) )
                return it;
    }

    if ( IsCyclic )
    {
        for ( TurnIt it = first ; it != current ; ++it )
            if ( cond(*it) )
                return it;
    }

    return last;
}

template <bool IsCyclic, typename TurnIt, typename Cond> inline
TurnIt find_previous_if(TurnIt first, TurnIt current, TurnIt last, Cond cond)
{
    typedef std::reverse_iterator<TurnIt> Rit;
    Rit rlast = Rit(first);
    if ( current == last )
        return last;
    ++current;
    Rit res = find_next_if<IsCyclic>(Rit(last), Rit(current), rlast, cond);
    if ( res == rlast )
        return last;
    else
        return --res.base();
}

template <typename TurnIt, typename Cond> inline
TurnIt find_first_if(TurnIt first, TurnIt last, Cond cond)
{
    return std::find_if(first, last, cond);
}

template <typename TurnIt, typename Cond> inline
TurnIt find_last_if(TurnIt first, TurnIt last, Cond cond)
{
    typedef std::reverse_iterator<TurnIt> Rit;
    Rit rlast = Rit(first);
    Rit res = std::find_if(Rit(last), rlast, cond);
    if ( res == rlast )
        return last;
    else
        return --res.base();
}

}}} // namespace detail::relate::turns
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_TURNS_HPP
