// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_FOLLOW_LINEAR_LINEAR_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_FOLLOW_LINEAR_LINEAR_HPP

#include <cstddef>
#include <iterator>

#include <boost/assert.hpp>
#include <boost/range.hpp>

#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/multi/core/tags.hpp>

#include <boost/geometry/algorithms/detail/overlay/copy_segments.hpp>
#include <boost/geometry/algorithms/detail/overlay/follow.hpp>
#include <boost/geometry/algorithms/detail/overlay/overlay_type.hpp>
#include <boost/geometry/algorithms/detail/overlay/segment_identifier.hpp>
#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>

#include <boost/geometry/algorithms/detail/turns/debug_turn.hpp>

#include <boost/geometry/algorithms/convert.hpp>
#include <boost/geometry/algorithms/not_implemented.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{

namespace following { namespace linear
{




// follower for linear/linear geometries set operations

template <typename Turn, typename Operation>
static inline bool is_entering(Turn const& turn,
                               Operation const& operation)
{
    if ( turn.method != method_touch && turn.method != method_touch_interior )
    {
        return false;
    }
    return operation.operation == operation_intersection;
}



template <typename Turn, typename Operation>
static inline bool is_staying_inside(Turn const& turn,
                                     Operation const& operation, 
                                     bool entered)
{
    if ( !entered )
    {
        return false;
    }

    if ( turn.method != method_equal && turn.method != method_collinear )
    {
        return false;
    }
    return operation.operation == operation_continue;
}



template <typename Turn, typename Operation>
static inline bool is_leaving(Turn const& turn,
                              Operation const& operation,
                              bool entered)
{
    if ( !entered )
    {
        return false;
    }

    if ( turn.method != method_touch
         && turn.method != method_touch_interior
         && turn.method != method_equal
         && turn.method != method_collinear )
    {
        return false;
    }

    if ( operation.operation == operation_blocked )
    {
        return true;
    }

    if ( operation.operation != operation_union )
    {
        return false;
    }

    return operation.is_collinear;
}



template <typename Turn, typename Operation>
static inline bool is_isolated_point(Turn const& turn,
                                     Operation const& operation,
                                     bool entered)
{
    if ( entered )
    {
        return false;
    }

    if ( turn.method == method_collinear )
    {
        BOOST_ASSERT( operation.operation == operation_continue );
        return true;
    }

    if ( turn.method == method_crosses )
    {
        return true;
    }

    if ( turn.method != method_touch && turn.method != method_touch_interior )
    {
        return false;
    }

    if ( operation.operation == operation_blocked )
    {
        return true;
    }

    if ( operation.operation != operation_union )
    {
        return false;
    }

    return !operation.is_collinear;
}









template
<
    typename LinestringOut,
    typename Linestring,
    typename Linear,
    overlay_type OverlayType,
    bool FollowIsolatedPoints,
    bool FollowContinueTurns
>
class follow_linestring_linear_linestring
{
protected:
    typedef following::action_selector<OverlayType> action;

    template
    <
        typename TurnIterator,
        typename TurnOperationIterator,
        typename SegmentIdentifier,
        typename OutputIterator
    >
    static inline OutputIterator
    process_turn(TurnIterator it,
                 TurnOperationIterator op_it,
                 bool& first, bool& entered,
                 std::size_t& enter_count,
                 Linestring const& linestring,
                 LinestringOut& current_piece,
                 SegmentIdentifier& current_segment_id,
                 OutputIterator oit)
    {
        if ( is_entering(*it, *op_it) )
        {
#ifdef GEOMETRY_TEST_DEBUG
            detail::turns::debug_turn(*it, *op_it, "-> Entering");
#endif

            entered = true;
            if ( enter_count == 0 )
            {
                action::enter(current_piece, linestring,
                              current_segment_id,
                              op_it->seg_id.segment_index,
                              it->point, *op_it, oit);
            }
            ++enter_count;
        }
        else if ( is_leaving(*it, *op_it, entered) )
        {
#ifdef GEOMETRY_TEST_DEBUG
            detail::turns::debug_turn(*it, *op_it, "-> Leaving");
#endif

            --enter_count;
            if ( enter_count == 0 )
            {
                entered = false;
                action::leave(current_piece, linestring,
                              current_segment_id,
                              op_it->seg_id.segment_index,
                              it->point, *op_it, oit);
            }
        }
        else if ( FollowIsolatedPoints
                  && is_isolated_point(*it, *op_it, entered) )
        {
#ifdef GEOMETRY_TEST_DEBUG
            detail::turns::debug_turn(*it, *op_it, "-> Isolated point");
#endif

            action::isolated_point(current_piece, linestring,
                                   current_segment_id,
                                   op_it->seg_id.segment_index,
                                   it->point, *op_it, oit);
        }
        else if ( FollowContinueTurns
                  && is_staying_inside(*it, *op_it, entered) )
        {
#ifdef GEOMETRY_TEST_DEBUG
            detail::turns::debug_turn(*it, *op_it, "-> Staying inside");
#endif

            entered = true;
        }
        first = false;
        return oit;
    }

    template
    <
        typename SegmentIdentifier,
        typename OutputIterator
    >
    static inline OutputIterator
    process_end(bool entered,
                Linestring const& linestring,
                SegmentIdentifier const& current_segment_id,
                LinestringOut& current_piece,
                OutputIterator oit)
    {
        if ( action::is_entered(entered) )
        {
            geometry::copy_segments<false>(linestring,
                                           current_segment_id,
                                           boost::size(linestring) - 1,
                                           current_piece);
        }

        // Output the last one, if applicable
        if (::boost::size(current_piece) > 1)
        {
            *oit++ = current_piece;
        }

        return oit;
    }

public:
    template <typename TurnIterator, typename OutputIterator>
    static inline OutputIterator
    apply(Linestring const& linestring, Linear const& linear,
          TurnIterator start, TurnIterator beyond,
          OutputIterator oit)
    {
        typedef typename boost::range_iterator
            <
                typename std::iterator_traits
                    <
                        TurnIterator
                    >::value_type::container_type const
            >::type turn_operation_iterator;

        // Iterate through all intersection points (they are
        // ordered along the each line)

        LinestringOut current_piece;
        geometry::segment_identifier current_segment_id(0, -1, -1, -1);

        bool entered = false;
        bool first = true;
        std::size_t enter_count = 0;

        for (TurnIterator it = start; it != beyond; ++it)
        {
            turn_operation_iterator op_it = boost::begin(it->operations);

            oit = process_turn(it, op_it,
                               first, entered, enter_count, 
                               linestring,
                               current_piece, current_segment_id,
                               oit);
        }

        BOOST_ASSERT( enter_count == 0 );

        return process_end(entered, linestring,
                           current_segment_id, current_piece,
                           oit);
    }
};




template
<
    typename LinestringOut,
    typename MultiLinestring,
    typename Linear,
    overlay_type OverlayType,
    bool FollowIsolatedPoints,
    bool FollowContinueTurns
>
class follow_multilinestring_linear_linestring
    : follow_linestring_linear_linestring
        <
            LinestringOut,
            typename boost::range_value<MultiLinestring>::type,
            Linear,
            OverlayType,
            FollowIsolatedPoints,
            FollowContinueTurns
        >
{
protected:
    typedef typename boost::range_value<MultiLinestring>::type Linestring;

    typedef follow_linestring_linear_linestring
        <
            LinestringOut, Linestring, Linear,
            OverlayType, FollowIsolatedPoints, FollowContinueTurns
        > Base;

    typedef following::action_selector<OverlayType> action;

    typedef typename boost::range_iterator
        <
            MultiLinestring const
        >::type linestring_iterator;


    template <typename OutputIt, overlay_type OT>
    struct copy_linestrings_in_range
    {
        static inline OutputIt
        apply(linestring_iterator begin, linestring_iterator beyond,
              OutputIt oit)
        {
            return oit;
        }
    };

    template <typename OutputIt>
    struct copy_linestrings_in_range<OutputIt, overlay_difference>
    {
        static inline OutputIt
        apply(linestring_iterator begin, linestring_iterator beyond,
              OutputIt oit)
        {
            for (linestring_iterator ls_it = begin; ls_it != beyond; ++ls_it)
            {
                LinestringOut line_out;
                geometry::convert(*ls_it, line_out);
                *oit++ = line_out;
            }
            return oit;
        }
    };

public:
    template <typename TurnIterator, typename OutputIterator>
    static inline OutputIterator
    apply(MultiLinestring const& multilinestring, Linear const& linear,
          TurnIterator start, TurnIterator beyond,
          OutputIterator oit)
    {
        typedef typename boost::range_iterator
            <
                typename std::iterator_traits
                    <
                        TurnIterator
                    >::value_type::container_type const
            >::type turn_operation_iterator;

        typedef copy_linestrings_in_range
            <
                OutputIterator, OverlayType
            > copy_linestrings;

        linestring_iterator ls_begin = boost::begin(multilinestring);
        linestring_iterator ls_end = boost::end(multilinestring);

        // Iterate through all intersection points (they are
        // ordered along the each line)

        int current_multi_id = -1;

        turn_operation_iterator op_it = boost::begin(start->operations);
        current_multi_id = op_it->seg_id.multi_index;

        oit = copy_linestrings::apply(ls_begin,
                                      ls_begin + current_multi_id,
                                      oit);

        TurnIterator turns_begin = start, turns_end;
        do {
            // find last turn with this multi-index
            turns_end = turns_begin;
            do
            {
                ++turns_end;
                op_it = boost::begin(turns_end->operations);
            }
            while ( turns_end != beyond
                    && op_it->seg_id.multi_index == current_multi_id );

            oit = Base::apply(*(boost::begin(multilinestring)
                                + current_multi_id),
                              linear, turns_begin, turns_end, oit);

            int new_multi_id(0);
            linestring_iterator ls_beyond_last = ls_end;
            if ( turns_end != beyond )
            {
                op_it = boost::begin(turns_end->operations);
                new_multi_id = op_it->seg_id.multi_index;
                ls_beyond_last = ls_begin + new_multi_id;
            }
            oit = copy_linestrings::apply(ls_begin + current_multi_id + 1,
                                          ls_beyond_last,
                                          oit);

            current_multi_id = new_multi_id;
            turns_begin = turns_end;
        }
        while ( turns_end != beyond );

        return oit;
    }
};






template
<
    typename LinestringOut,
    typename Geometry1,
    typename Geometry2,
    overlay_type OverlayType,
    bool FollowIsolatedPoints,
    bool FollowContinueTurns,
    typename TagOut = typename tag<LinestringOut>::type,
    typename TagIn1 = typename tag<Geometry1>::type
>
struct follow
    : not_implemented<LinestringOut, Geometry1>
{};



template
<
    typename LinestringOut,
    typename Linestring,
    typename Linear,
    overlay_type OverlayType,
    bool FollowIsolatedPoints,
    bool FollowContinueTurns
>
struct follow
    <
        LinestringOut, Linestring, Linear,
        OverlayType, FollowIsolatedPoints, FollowContinueTurns,
        linestring_tag, linestring_tag
    > : follow_linestring_linear_linestring
        <
            LinestringOut, Linestring, Linear,
            OverlayType, FollowIsolatedPoints, FollowContinueTurns
        >
{};


template
<
    typename LinestringOut,
    typename MultiLinestring,
    typename Linear,
    overlay_type OverlayType,
    bool FollowIsolatedPoints,
    bool FollowContinueTurns
>
struct follow
    <
        LinestringOut, MultiLinestring, Linear,
        OverlayType, FollowIsolatedPoints, FollowContinueTurns,
        linestring_tag, multi_linestring_tag
    > : follow_multilinestring_linear_linestring
        <
            LinestringOut, MultiLinestring, Linear,
            OverlayType, FollowIsolatedPoints, FollowContinueTurns
        >
{};



}} // namespace following::linear

}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_FOLLOW_LINEAR_LINEAR_HPP
