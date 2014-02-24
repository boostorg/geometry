// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_FOLLOW_LINEAR_LINEAR_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_FOLLOW_LINEAR_LINEAR_HPP

#include <boost/geometry/algorithms/detail/overlay/follow.hpp>

namespace boost { namespace geometry
{

namespace detail { namespace overlay
{

namespace following { namespace linear
{




// follower for linear/linear geometries set operations

template <typename Turn, typename Operation>
static inline bool is_entering(Turn const& turn, Operation const& op)
{
    if ( turn.method != method_touch && turn.method != method_touch_interior )
    {
        return false;
    }
    return op.operation == operation_intersection;
}



template <typename Turn, typename Operation>
static inline bool is_staying_inside(Turn const& turn, Operation const& op, 
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
    return op.operation == operation_continue;
}



template <typename Turn, typename Operation>
static inline bool is_leaving(Turn const& turn, Operation const& op, 
                              Operation const& reverse_op,
                              bool entered)
{
    if ( !entered )
    {
        return false;
    }

    if ( turn.method != method_touch &&
         turn.method != method_touch_interior &&
         turn.method != method_equal &&
         turn.method != method_collinear )
    {
        return false;
    }

    if ( op.operation == operation_blocked )
    {
        return true;
    }

    if ( op.operation != operation_union )
    {
        return false;
    }

    if ( turn.operations[1].operation == operation_intersection )
    {
        return true;
    }

    BOOST_ASSERT( turn.operations[1].operation == operation_union ||
                  turn.operations[1].operation == operation_blocked );

    return reverse_op.operation == operation_intersection;
}


template <typename Turn, typename Operation>
static inline bool is_isolated_point(Turn const& turn, Operation const& op, 
                                     Operation const& reverse_op,
                                     bool entered)
{
    if ( entered )
    {
        return false;
    }

    if ( turn.method == method_crosses )
    {
        return true;
    }

    if ( turn.method != method_touch && turn.method != method_touch_interior )
    {
        return false;
    }

    if ( op.operation == operation_blocked )
    {
        return true;
    }

    if ( op.operation != operation_union )
    {
        return false;
    }

    if ( turn.operations[1].operation == operation_intersection )
    {
        return false;
    }

    BOOST_ASSERT( turn.operations[1].operation == operation_union ||
                  turn.operations[1].operation == operation_blocked );

    return reverse_op.operation == operation_union
        || reverse_op.operation == operation_blocked;
}


template
<
    typename LineStringOut,
    typename LineString1,
    typename LineString2,
    overlay_type OverlayType
>
class follow_linestring_linestring_linestring
{
protected:
    typedef typename point_type<LineStringOut>::type PointOut;
    typedef traversal_turn_info<PointOut> turn_info;

    typedef typename boost::range_iterator
        <
            typename turn_info::container_type
        >::type turn_operation_iterator_type;

    typedef following::action_selector<OverlayType> action;

    template
    <
        typename TurnIt,
        typename TurnOpIt,
        typename SegmentIdentifier,
        typename OutputIterator
    >
    static inline OutputIterator
    process_turn(TurnIt it, TurnIt it_r,
                 TurnOpIt iit, TurnOpIt iit_r,
                 bool& first, bool& entered,
                 std::size_t& enter_count,
                 LineString1 const& ls1, LineString2 const&,
                 LineStringOut& current_piece,
                 SegmentIdentifier& current_segment_id,
                 OutputIterator oit)
    {
        if ( is_entering(*it, *iit) )
        {
#ifdef GEOMETRY_TEST_DEBUG
            detail::overlay::debug_traverse(*it, *iit, "-> Entering");
#endif

            entered = true;
            if ( enter_count == 0 )
            {
                action::enter(current_piece, ls1, current_segment_id,
                              iit->seg_id.segment_index,
                              it->point, *iit, oit);
            }
            ++enter_count;
        }
        else if ( is_staying_inside(*it, *iit, entered) )
        {
#ifdef GEOMETRY_TEST_DEBUG
            detail::overlay::debug_traverse(*it, *iit, "-> Staying inside");
#endif

            entered = true;
        }
        else if ( is_leaving(*it, *iit, *iit_r, entered) )
        {
#ifdef GEOMETRY_TEST_DEBUG
            detail::overlay::debug_traverse(*it, *iit, "-> Leaving");
#endif

            --enter_count;
            if ( enter_count == 0 )
            {
                entered = false;
                action::leave(current_piece, ls1, current_segment_id,
                              iit->seg_id.segment_index,
                              it->point, *iit, oit);
            }
        }
#ifndef BOOST_GEOMETRY_INTERSECTION_DO_NOT_INCLUDE_ISOLATED_POINTS
        else if ( OverlayType == overlay_intersection &&
                  is_isolated_point(*it, *iit, *iit_r, entered) )
        {
#ifdef GEOMETRY_TEST_DEBUG
            detail::overlay::debug_traverse(*it, *iit, "-> Isolated point");
#endif

            action::isolated_point(current_piece, ls1, current_segment_id,
                                   iit->seg_id.segment_index,
                                   it->point, *iit, oit);
        }
#endif
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
                LineString1 const& linestring1,
                SegmentIdentifier const& current_segment_id,
                LineStringOut& current_piece,
                OutputIterator oit)
    {
        if ( action::is_entered(entered) )
        {
            geometry::copy_segments<false>(linestring1, current_segment_id,
                                           boost::size(linestring1) - 1,
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
    template <typename Turns, typename OutputIterator>
    static inline OutputIterator apply(LineString1 const& linestring1,
                                       LineString2 const& linestring2,
                                       Turns& turns,
                                       Turns& reverse_turns,
                                       OutputIterator oit)
    {
        BOOST_ASSERT( boost::size(turns) == boost::size(reverse_turns) );

        typedef typename Turns::iterator TurnIt;

        // Iterate through all intersection points (they are
        // ordered along the each line)

        LineStringOut current_piece;
        geometry::segment_identifier current_segment_id(0, -1, -1, -1);

        bool entered = false;
        bool first = true;
        std::size_t enter_count = 0;

        TurnIt it = boost::begin(turns);
        TurnIt it_r = boost::begin(reverse_turns);
        for (; it != boost::end(turns); ++it, ++it_r)
        {
            turn_operation_iterator_type iit = boost::begin(it->operations);
            turn_operation_iterator_type iit_r = boost::begin(it_r->operations);
            ++iit_r;

            oit = process_turn(it, it_r, iit, iit_r,
                               first, entered, enter_count, 
                               linestring1, linestring2,
                               current_piece, current_segment_id,
                               oit);
        }

        BOOST_ASSERT( enter_count == 0 );

        return process_end(entered, linestring1,
                           current_segment_id, current_piece,
                           oit);
    }
};



template
<
    typename LineStringOut,
    typename LineString,
    typename MultiLineString,
    overlay_type OverlayType
>
class follow_linestring_multilinestring_linestring
    : follow_linestring_linestring_linestring
        <
            LineStringOut,
            LineString,
            typename boost::range_value<MultiLineString>::type,
            OverlayType
        >
{
protected:
    typedef typename boost::range_value<MultiLineString>::type LineString2;

    typedef follow_linestring_linestring_linestring
        <
            LineStringOut, LineString, LineString2, OverlayType
        > Base;

    typedef typename point_type<LineStringOut>::type PointOut;
    typedef traversal_turn_info<PointOut> turn_info;

    typedef typename boost::range_iterator
        <
            typename turn_info::container_type
        >::type turn_operation_iterator_type;

    typedef following::action_selector<OverlayType> action;

public:
    template <typename Turns, typename OutputIterator>
    static inline OutputIterator apply(LineString const& linestring,
                                       MultiLineString const& multilinestring,
                                       Turns& turns,
                                       Turns& reverse_turns,
                                       OutputIterator oit)
    {
        BOOST_ASSERT( boost::size(turns) == boost::size(reverse_turns) );

        typedef typename Turns::iterator TurnIt;

        // Iterate through all intersection points (they are
        // ordered along the each line)

        LineStringOut current_piece;
        geometry::segment_identifier current_segment_id(0, -1, -1, -1);

        bool entered = false;
        bool first = true;
        std::size_t enter_count = 0;

        TurnIt it = boost::begin(turns);
        TurnIt it_r = boost::begin(reverse_turns);
        for (; it != boost::end(turns); ++it, ++it_r)
        {
            turn_operation_iterator_type iit = boost::begin(it->operations);
            turn_operation_iterator_type iit_r = boost::begin(it_r->operations);
            ++iit_r;

            LineString2 const* ls2 =
                &*(boost::begin(multilinestring) + iit->other_id.multi_index);

            oit = Base::process_turn(it, it_r, iit, iit_r,
                                     first, entered, enter_count,
                                     linestring, *ls2, 
                                     current_piece, current_segment_id,
                                     oit);
        }

        BOOST_ASSERT( enter_count == 0 );

        return Base::process_end(entered, linestring,
                                 current_segment_id, current_piece,
                                 oit);
    }
};





template
<
    typename LineStringOut,
    typename MultiLineString,
    typename LineString,
    overlay_type OverlayType
>
class follow_multilinestring_linestring_linestring
    : follow_linestring_linestring_linestring
        <
            LineStringOut,
            typename boost::range_value<MultiLineString>::type,
            LineString,
            OverlayType
        >
{
protected:
    typedef typename boost::range_value<MultiLineString>::type LineString1;

    typedef follow_linestring_linestring_linestring
        <
            LineStringOut, LineString1, LineString, OverlayType
        > Base;

    typedef typename point_type<LineStringOut>::type PointOut;
    typedef traversal_turn_info<PointOut> turn_info;

    typedef typename boost::range_iterator
        <
            typename turn_info::container_type
        >::type turn_operation_iterator_type;

    typedef following::action_selector<OverlayType> action;

public:
    template <typename Turns, typename OutputIterator>
    static inline OutputIterator apply(MultiLineString const& multilinestring,
                                       LineString const& linestring,
                                       Turns& turns,
                                       Turns& reverse_turns,
                                       OutputIterator oit)
    {
        BOOST_ASSERT( boost::size(turns) == boost::size(reverse_turns) );

        typedef typename Turns::iterator TurnIt;

        // Iterate through all intersection points (they are
        // ordered along the each line)

        LineStringOut current_piece;
        geometry::segment_identifier current_segment_id(0, -1, -1, -1);
        int current_multi_id = -1;

        bool entered = false;
        bool first = true;
        bool first_turn = true;
        std::size_t enter_count = 0;

        // dummy initialization
        LineString1 const* ls1 = &*boost::begin(multilinestring);

        TurnIt it = boost::begin(turns);
        TurnIt it_r = boost::begin(reverse_turns);
        for (; it != boost::end(turns); ++it, ++it_r)
        {
            turn_operation_iterator_type iit = boost::begin(it->operations);
            turn_operation_iterator_type iit_r = boost::begin(it_r->operations);
            ++iit_r;

            if ( iit->seg_id.multi_index != current_multi_id )
            {
                if ( first_turn )
                {
                    first_turn = false;
                }
                else
                {
                    oit = Base::process_end(entered, *ls1,
                                            current_segment_id, current_piece,
                                            oit);

                    // reset values
                    first = true;
                    entered = false;
                    enter_count = 0;
                    current_segment_id
                        = geometry::segment_identifier(0, -1, -1, -1);
                    geometry::clear(current_piece);
                }
                current_multi_id = iit->seg_id.multi_index;
                ls1 = &*(boost::begin(multilinestring) + current_multi_id);
            }

            oit = Base::process_turn(it, it_r, iit, iit_r,
                                     first, entered, enter_count, 
                                     *ls1, linestring,
                                     current_piece, current_segment_id,
                                     oit);
        }

        BOOST_ASSERT( enter_count == 0 );

        return Base::process_end(entered, *ls1,
                                 current_segment_id, current_piece,
                                 oit);
    }
};



template
<
    typename LineStringOut,
    typename MultiLineString1,
    typename MultiLineString2,
    overlay_type OverlayType
>
class follow_multilinestring_multilinestring_linestring
    : follow_linestring_linestring_linestring
        <
            LineStringOut,
            typename boost::range_value<MultiLineString1>::type,
            typename boost::range_value<MultiLineString2>::type,
            OverlayType
        >
{
protected:
    typedef typename boost::range_value<MultiLineString1>::type LineString1;
    typedef typename boost::range_value<MultiLineString2>::type LineString2;

    typedef follow_linestring_linestring_linestring
        <
            LineStringOut, LineString1, LineString2, OverlayType
        > Base;

    typedef typename point_type<LineStringOut>::type PointOut;
    typedef traversal_turn_info<PointOut> turn_info;

    typedef typename boost::range_iterator
        <
            typename turn_info::container_type
        >::type turn_operation_iterator_type;

    typedef following::action_selector<OverlayType> action;

public:
    template <typename Turns, typename OutputIterator>
    static inline OutputIterator apply(MultiLineString1 const& multilinestring1,
                                       MultiLineString2 const& multilinestring2,
                                       Turns& turns,
                                       Turns& reverse_turns,
                                       OutputIterator oit)
    {
        BOOST_ASSERT( boost::size(turns) == boost::size(reverse_turns) );

        typedef typename Turns::iterator TurnIt;

        // Iterate through all intersection points (they are
        // ordered along the each line)

        LineStringOut current_piece;
        geometry::segment_identifier current_segment_id(0, -1, -1, -1);
        int current_multi_id = -1;

        bool entered = false;
        bool first = true;
        bool first_turn = true;
        std::size_t enter_count = 0;

        // dummy initialization
        LineString1 const* ls1 = &*boost::begin(multilinestring1);

        TurnIt it = boost::begin(turns);
        TurnIt it_r = boost::begin(reverse_turns);
        for (; it != boost::end(turns); ++it, ++it_r)
        {
            turn_operation_iterator_type iit = boost::begin(it->operations);
            turn_operation_iterator_type iit_r = boost::begin(it_r->operations);
            ++iit_r;

            if ( iit->seg_id.multi_index != current_multi_id )
            {
                if ( first_turn )
                {
                    first_turn = false;
                }
                else
                {
                    oit = Base::process_end(entered, *ls1,
                                            current_segment_id, current_piece,
                                            oit);

                    // reset values
                    first = true;
                    entered = false;
                    enter_count = 0;
                    current_segment_id
                        = geometry::segment_identifier(0, -1, -1, -1);
                    geometry::clear(current_piece);
                }
                current_multi_id = iit->seg_id.multi_index;
                ls1 = &*(boost::begin(multilinestring1) + current_multi_id);
            }

            LineString2 const* ls2 =
                &*(boost::begin(multilinestring2) + iit->other_id.multi_index);

            oit = Base::process_turn(it, it_r, iit, iit_r,
                                     first, entered, enter_count, 
                                     *ls1, *ls2,
                                     current_piece, current_segment_id,
                                     oit);
        }

        BOOST_ASSERT( enter_count == 0 );

        return Base::process_end(entered, *ls1,
                                 current_segment_id, current_piece,
                                 oit);
    }
};



template
<
    typename GeometryOut,
    typename Geometry1,
    typename Geometry2,
    overlay_type OverlayType,
    typename TagOut = typename tag<GeometryOut>::type,
    typename Tag1 = typename tag<Geometry1>::type,
    typename Tag2 = typename tag<Geometry2>::type
>
struct follow_dispatch
    : not_implemented<GeometryOut, Geometry1, Geometry2>
{};


template
<
    typename LineStringOut,
    typename Linestring1,
    typename Linestring2,
    overlay_type OverlayType
>
struct follow_dispatch
    <
        LineStringOut, Linestring1, Linestring2, OverlayType,
        linestring_tag, linestring_tag, linestring_tag
    > : follow_linestring_linestring_linestring
        <
            LineStringOut, Linestring1, Linestring2, OverlayType
        >
{};


template
<
    typename LineStringOut,
    typename Linestring,
    typename MultiLinestring,
    overlay_type OverlayType
>
struct follow_dispatch
    <
        LineStringOut, Linestring, MultiLinestring, OverlayType,
        linestring_tag, linestring_tag, multi_linestring_tag
    > : follow_linestring_multilinestring_linestring
        <
            LineStringOut, Linestring, MultiLinestring, OverlayType
        >
{};



template
<
    typename LineStringOut,
    typename MultiLinestring,
    typename Linestring,
    overlay_type OverlayType
>
struct follow_dispatch
    <
        LineStringOut, MultiLinestring, Linestring, OverlayType,
        linestring_tag, multi_linestring_tag, linestring_tag
    > : follow_multilinestring_linestring_linestring
        <
            LineStringOut, MultiLinestring, Linestring, OverlayType
        >
{};



template
<
    typename LineStringOut,
    typename MultiLinestring1,
    typename MultiLinestring2,
    overlay_type OverlayType
>
struct follow_dispatch
    <
        LineStringOut, MultiLinestring1, MultiLinestring2, OverlayType,
        linestring_tag, multi_linestring_tag, multi_linestring_tag
    > : follow_multilinestring_multilinestring_linestring
        <
            LineStringOut, MultiLinestring1, MultiLinestring2, OverlayType
        >
{};



template
<
    typename LineStringOut,
    typename Geometry1,
    typename Geometry2,
    overlay_type OverlayType
>
struct follow
    : follow_dispatch<LineStringOut, Geometry1, Geometry2, OverlayType>
{};


}} // namespace following::linear

}} // namespace detail::overlay

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_FOLLOW_LINEAR_LINEAR_HPP
