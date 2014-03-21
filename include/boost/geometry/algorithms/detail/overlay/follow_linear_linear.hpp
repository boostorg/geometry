// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_FOLLOW_LINEAR_LINEAR_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_FOLLOW_LINEAR_LINEAR_HPP

#include <boost/geometry/algorithms/detail/overlay/follow.hpp>
#include <boost/geometry/algorithms/detail/turns/debug_turn.hpp>

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
                                     bool entered,
                                     bool is_point1,
                                     bool is_point2)
{
    if ( entered )
    {
        return false;
    }

    if ( turn.method == method_collinear && (is_point1 || is_point2) )
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




template <bool Enable>
struct is_point
{
    template <typename Linestring>
    static inline bool apply(Linestring const& linestring)
    {
        return false;
    }
};


template<>
struct is_point<true>
{
    template <typename Linestring>
    static inline bool apply(Linestring const& linestring)
    {
        BOOST_ASSERT( boost::size(linestring) >= 2 );

        return boost::size(linestring) == 2
            && geometry::equals(*boost::begin(linestring),
                                *(++boost::begin(linestring))
                                );
    }
};







template
<
    typename LineStringOut,
    typename LineString1,
    typename LineString2,
    overlay_type OverlayType,
    bool FollowIsolatedPoints
>
class follow_linestring_linestring_linestring
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
                 LineString1 const& linestring1,
                 LineString2 const& linestring2,
                 bool is_point1,
                 bool is_point2,
                 LineStringOut& current_piece,
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
                action::enter(current_piece, linestring1,
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
                action::leave(current_piece, linestring1,
                              current_segment_id,
                              op_it->seg_id.segment_index,
                              it->point, *op_it, oit);
            }
        }
#ifndef BOOST_GEOMETRY_INTERSECTION_DO_NOT_INCLUDE_ISOLATED_POINTS
        else if ( FollowIsolatedPoints
                  && is_isolated_point(*it, *op_it, entered, is_point1, is_point2) )
        {
#ifdef GEOMETRY_TEST_DEBUG
            detail::turns::debug_turn(*it, *op_it, "-> Isolated point");
#endif

            action::isolated_point(current_piece, linestring1,
                                   current_segment_id,
                                   op_it->seg_id.segment_index,
                                   it->point, *op_it, oit);
        }
#endif
        else if ( is_staying_inside(*it, *op_it, entered) )
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
                LineString1 const& linestring1,
                SegmentIdentifier const& current_segment_id,
                LineStringOut& current_piece,
                OutputIterator oit)
    {
        if ( action::is_entered(entered) )
        {
            geometry::copy_segments<false>(linestring1,
                                           current_segment_id,
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
                                       Turns const& turns,
                                       OutputIterator oit)
    {
        typedef typename boost::range_iterator<Turns const>::type TurnIterator;
        typedef typename boost::range_value<Turns>::type TurnInfo;
        typedef typename boost::range_iterator
            <
                typename TurnInfo::container_type const
            >::type TurnOperationIterator;

        // Iterate through all intersection points (they are
        // ordered along the each line)

        LineStringOut current_piece;
        geometry::segment_identifier current_segment_id(0, -1, -1, -1);

        bool entered = false;
        bool first = true;
        std::size_t enter_count = 0;

        bool is_point1 = is_point<FollowIsolatedPoints>::apply(linestring1);
        bool is_point2 = is_point<FollowIsolatedPoints>::apply(linestring2);

        for (TurnIterator it = boost::begin(turns); it != boost::end(turns); ++it)
        {
            TurnOperationIterator op_it = boost::begin(it->operations);

            oit = process_turn(it, op_it,
                               first, entered, enter_count, 
                               linestring1, linestring2,
                               is_point1, is_point2,
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
    overlay_type OverlayType,
    bool FollowIsolatedPoints
>
class follow_linestring_multilinestring_linestring
    : follow_linestring_linestring_linestring
        <
            LineStringOut,
            LineString,
            typename boost::range_value<MultiLineString>::type,
            OverlayType,
            FollowIsolatedPoints
        >
{
protected:
    typedef typename boost::range_value<MultiLineString>::type LineString2;

    typedef follow_linestring_linestring_linestring
        <
            LineStringOut, LineString, LineString2,
            OverlayType, FollowIsolatedPoints
        > Base;

    typedef following::action_selector<OverlayType> action;

public:
    template <typename Turns, typename OutputIterator>
    static inline OutputIterator apply(LineString const& linestring,
                                       MultiLineString const& multilinestring,
                                       Turns const& turns,
                                       OutputIterator oit)
    {
        typedef typename boost::range_iterator<Turns const>::type TurnIterator;
        typedef typename boost::range_value<Turns>::type TurnInfo;
        typedef typename boost::range_iterator
            <
                typename TurnInfo::container_type const
            >::type TurnOperationIterator;

        // Iterate through all intersection points (they are
        // ordered along the each line)

        LineStringOut current_piece;
        geometry::segment_identifier current_segment_id(0, -1, -1, -1);

        bool entered = false;
        bool first = true;
        std::size_t enter_count = 0;

        bool is_point1 = is_point<FollowIsolatedPoints>::apply(linestring);
        
        for (TurnIterator it = boost::begin(turns); it != boost::end(turns); ++it)
        {
            TurnOperationIterator op_it = boost::begin(it->operations);

            LineString2 const* linestring2 =
                &*(boost::begin(multilinestring) + op_it->other_id.multi_index);

            oit = Base::process_turn(it, op_it,
                                     first, entered, enter_count,
                                     linestring, *linestring2, 
                                     is_point1,
                                     is_point<FollowIsolatedPoints>::apply(*linestring2),
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
    overlay_type OverlayType,
    bool FollowIsolatedPoints
>
class follow_multilinestring_linestring_linestring
    : follow_linestring_linestring_linestring
        <
            LineStringOut,
            typename boost::range_value<MultiLineString>::type,
            LineString,
            OverlayType,
            FollowIsolatedPoints
        >
{
protected:
    typedef typename boost::range_value<MultiLineString>::type LineString1;

    typedef follow_linestring_linestring_linestring
        <
            LineStringOut, LineString1, LineString,
            OverlayType, FollowIsolatedPoints
        > Base;

    typedef following::action_selector<OverlayType> action;

public:
    template <typename Turns, typename OutputIterator>
    static inline OutputIterator apply(MultiLineString const& multilinestring,
                                       LineString const& linestring,
                                       Turns const& turns,
                                       OutputIterator oit)
    {
        typedef typename boost::range_iterator<Turns const>::type TurnIterator;
        typedef typename boost::range_value<Turns>::type TurnInfo;
        typedef typename boost::range_iterator
            <
                typename TurnInfo::container_type const
            >::type TurnOperationIterator;

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
        LineString1 const* linestring1 = &*boost::begin(multilinestring);

        bool is_point1 = is_point<FollowIsolatedPoints>::apply(*linestring1);
        bool is_point2 = is_point<FollowIsolatedPoints>::apply(linestring);

        TurnIterator it = boost::begin(turns);
        for (TurnIterator it = boost::begin(turns); it != boost::end(turns); ++it)
        {
            TurnOperationIterator op_it = boost::begin(it->operations);

            if ( op_it->seg_id.multi_index != current_multi_id )
            {
                if ( first_turn )
                {
                    first_turn = false;
                }
                else
                {
                    oit = Base::process_end(entered, *linestring1,
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
                current_multi_id = op_it->seg_id.multi_index;
                linestring1 =
                    &*(boost::begin(multilinestring) + current_multi_id);
                is_point1 = is_point<FollowIsolatedPoints>::apply(*linestring1);
            }

            oit = Base::process_turn(it, op_it,
                                     first, entered, enter_count, 
                                     *linestring1, linestring,
                                     is_point1, is_point2,
                                     current_piece, current_segment_id,
                                     oit);
        }

        BOOST_ASSERT( enter_count == 0 );

        return Base::process_end(entered, *linestring1,
                                 current_segment_id, current_piece,
                                 oit);
    }
};



template
<
    typename LineStringOut,
    typename MultiLineString1,
    typename MultiLineString2,
    overlay_type OverlayType,
    bool FollowIsolatedPoints
>
class follow_multilinestring_multilinestring_linestring
    : follow_linestring_linestring_linestring
        <
            LineStringOut,
            typename boost::range_value<MultiLineString1>::type,
            typename boost::range_value<MultiLineString2>::type,
            OverlayType,
            FollowIsolatedPoints
        >
{
protected:
    typedef typename boost::range_value<MultiLineString1>::type LineString1;
    typedef typename boost::range_value<MultiLineString2>::type LineString2;

    typedef follow_linestring_linestring_linestring
        <
            LineStringOut, LineString1, LineString2,
            OverlayType, FollowIsolatedPoints
        > Base;

    typedef following::action_selector<OverlayType> action;

public:
    template <typename Turns, typename OutputIterator>
    static inline OutputIterator apply(MultiLineString1 const& multilinestring1,
                                       MultiLineString2 const& multilinestring2,
                                       Turns const& turns,
                                       OutputIterator oit)
    {
        typedef typename boost::range_iterator<Turns const>::type TurnIterator;
        typedef typename boost::range_value<Turns>::type TurnInfo;
        typedef typename boost::range_iterator
            <
                typename TurnInfo::container_type const
            >::type TurnOperationIterator;

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
        LineString1 const* linestring1 = &*boost::begin(multilinestring1);

        bool is_point1 = is_point<FollowIsolatedPoints>::apply(*linestring1);
        
        for (TurnIterator it = boost::begin(turns); it != boost::end(turns); ++it)
        {
            TurnOperationIterator op_it = boost::begin(it->operations);

            if ( op_it->seg_id.multi_index != current_multi_id )
            {
                if ( first_turn )
                {
                    first_turn = false;
                }
                else
                {
                    oit = Base::process_end(entered, *linestring1,
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
                current_multi_id = op_it->seg_id.multi_index;
                linestring1 =
                    &*(boost::begin(multilinestring1) + current_multi_id);
                is_point1 = is_point<FollowIsolatedPoints>::apply(*linestring1);
            }

            LineString2 const* linestring2 =
                &*(boost::begin(multilinestring2) + op_it->other_id.multi_index);
            oit = Base::process_turn(it, op_it,
                                     first, entered, enter_count, 
                                     *linestring1, *linestring2,
                                     is_point1,
                                     is_point<FollowIsolatedPoints>::apply(*linestring2),
                                     current_piece, current_segment_id,
                                     oit);
        }

        BOOST_ASSERT( enter_count == 0 );

        return Base::process_end(entered, *linestring1,
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
    bool FollowIsolatedPoints,
    typename TagOut = typename tag<GeometryOut>::type,
    typename TagIn1 = typename tag<Geometry1>::type,
    typename TagIn2 = typename tag<Geometry2>::type
>
struct follow_dispatch
    : not_implemented<GeometryOut, Geometry1, Geometry2>
{};


template
<
    typename LineStringOut,
    typename Linestring1,
    typename Linestring2,
    overlay_type OverlayType,
    bool FollowIsolatedPoints
>
struct follow_dispatch
    <
        LineStringOut, Linestring1, Linestring2,
        OverlayType, FollowIsolatedPoints,
        linestring_tag, linestring_tag, linestring_tag
    > : follow_linestring_linestring_linestring
        <
            LineStringOut, Linestring1, Linestring2,
            OverlayType, FollowIsolatedPoints
        >
{};


template
<
    typename LineStringOut,
    typename Linestring,
    typename MultiLinestring,
    overlay_type OverlayType,
    bool FollowIsolatedPoints
>
struct follow_dispatch
    <
        LineStringOut, Linestring, MultiLinestring,
        OverlayType, FollowIsolatedPoints,
        linestring_tag, linestring_tag, multi_linestring_tag
    > : follow_linestring_multilinestring_linestring
        <
            LineStringOut, Linestring, MultiLinestring,
            OverlayType, FollowIsolatedPoints
        >
{};



template
<
    typename LineStringOut,
    typename MultiLinestring,
    typename Linestring,
    overlay_type OverlayType,
    bool FollowIsolatedPoints
>
struct follow_dispatch
    <
        LineStringOut, MultiLinestring, Linestring,
        OverlayType, FollowIsolatedPoints,
        linestring_tag, multi_linestring_tag, linestring_tag
    > : follow_multilinestring_linestring_linestring
        <
            LineStringOut, MultiLinestring, Linestring,
            OverlayType, FollowIsolatedPoints
        >
{};



template
<
    typename LineStringOut,
    typename MultiLinestring1,
    typename MultiLinestring2,
    overlay_type OverlayType,
    bool FollowIsolatedPoints
>
struct follow_dispatch
    <
        LineStringOut, MultiLinestring1, MultiLinestring2,
        OverlayType, FollowIsolatedPoints,
        linestring_tag, multi_linestring_tag, multi_linestring_tag
    > : follow_multilinestring_multilinestring_linestring
        <
            LineStringOut, MultiLinestring1, MultiLinestring2,
            OverlayType, FollowIsolatedPoints
        >
{};



template
<
    typename LineStringOut,
    typename Geometry1,
    typename Geometry2,
    overlay_type OverlayType,
    bool FollowIsolatedPoints
>
struct follow
    : follow_dispatch
        <
            LineStringOut, Geometry1, Geometry2,
            OverlayType, FollowIsolatedPoints
        >
{};


}} // namespace following::linear

}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_FOLLOW_LINEAR_LINEAR_HPP
