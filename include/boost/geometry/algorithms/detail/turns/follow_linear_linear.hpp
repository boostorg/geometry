// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_DIFFERENCE_FOLLOW_LINEAR_LINEAR_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_DIFFERENCE_FOLLOW_LINEAR_LINEAR_HPP

namespace boost { namespace geometry
{

namespace detail { namespace turns
{

namespace following
{




// follower for linear/linear geometries set operations

template <typename Turn, typename Operation>
static inline bool is_entering(Turn const& turn, Operation const& op)
{
    if ( turn.method != overlay::method_touch &&
         turn.method != overlay::method_touch_interior )
    {
        return false;
    }
    return op.operation == overlay::operation_intersection;
}



template 
<
    typename Turn, 
    typename Operation, 
    typename LineString1, 
    typename LineString2
>
static inline bool is_staying_inside(Turn const& turn, Operation const& op, 
                                     bool entered, bool first, 
                                     LineString1 const& linestring1,
                                     LineString2 const& linestring2)

{
    if ( !entered )
    {
        return false;
    }

    if ( turn.method != overlay::method_equal &&
         turn.method != overlay::method_collinear )
    {
        return false;
    }
    return op.operation == overlay::operation_continue;
}



template
<
    typename Turn,
    typename Operation,
    typename Linestring1,
    typename Linestring2
>
static inline bool is_leaving(Turn const& turn, Operation const& op, 
                              Operation const& reverse_op,
                              bool entered, bool first, 
                              Linestring1 const& linestring1,
                              Linestring2 const& linestring2)
{
    if ( !entered )
    {
        return false;
    }

    if ( turn.method != overlay::method_touch &&
         turn.method != overlay::method_touch_interior &&
         turn.method != overlay::method_equal &&
         turn.method != overlay::method_collinear )
    {
        return false;
    }

    if ( op.operation == overlay::operation_blocked )
    {
        return true;
    }

    if ( op.operation != overlay::operation_union )
    {
        return false;
    }

    if ( turn.operations[1].operation == overlay::operation_intersection )
    {
        return true;
    }

    BOOST_CHECK( turn.operations[1].operation == overlay::operation_union ||
                 turn.operations[1].operation == overlay::operation_blocked );

    return reverse_op.operation == overlay::operation_intersection;
}



template
<
    typename LineStringOut,
    typename MultiLineString1,
    typename MultiLineString2,
    overlay_type OverlayType
>
class follow
{
protected:
    typedef typename point_type<LineStringOut>::type PointOut;
    typedef overlay::traversal_turn_info<PointOut> turn_info;

    typedef typename boost::range_value<MultiLineString1>::type LineString1;
    typedef typename boost::range_value<MultiLineString2>::type LineString2;

    typedef typename boost::range_iterator
        <
            typename turn_info::container_type
        >::type turn_operation_iterator_type;

    typedef detail::overlay::following::action_selector<OverlayType> action;

public:
    template <typename Turns, typename OutputIterator>
    static inline OutputIterator apply(MultiLineString1& multilinestring1,
                                       MultiLineString2& multilinestring2,
                                       Turns& turns,
                                       Turns& reverse_turns,
                                       OutputIterator oit)
    {
        BOOST_CHECK( boost::size(turns) == boost::size(reverse_turns) );

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
        LineString1& ls1 = *boost::begin(multilinestring1);

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
                    current_multi_id = iit->seg_id.multi_index;
                    ls1 = *(boost::begin(multilinestring1) + current_multi_id);
                }
                else
                {
                    if (action::is_entered(entered))
                    {
                        geometry::copy_segments<false>(ls1, current_segment_id,
                                                       boost::size(ls1) - 1,
                                                       current_piece);
                    }

                    // Output the last one, if applicable
                    if (::boost::size(current_piece) > 1)
                    {
                        *oit++ = current_piece;
                    }

                    first = true;
                    entered = false;
                    enter_count = 0;
                    current_segment_id = geometry::segment_identifier(0, -1, -1, -1);
                    geometry::clear(current_piece);
                    current_multi_id = iit->seg_id.multi_index;
                    ls1 = *(boost::begin(multilinestring1) + current_multi_id);
                }
            }

            LineString2 const& ls2 =
                *(boost::begin(multilinestring2) + iit->other_id.multi_index);

            if ( is_entering(*it, *iit) )
            {
                detail::overlay::debug_traverse(*it, *iit, "-> Entering");

                entered = true;
                if ( enter_count == 0 )
                {
                    action::enter(current_piece, ls1, current_segment_id,
                                  iit->seg_id.segment_index,
                                  it->point, *iit, oit);
                }
                ++enter_count;
            }
            else if ( is_staying_inside(*it, *iit, entered, first, ls1, ls2) )
            {
                detail::overlay::debug_traverse(*it, *iit, "-> Staying inside");

                entered = true;
            }
            else if ( is_leaving(*it, *iit, *iit_r, entered, first, ls1, ls2) )
            {
                detail::overlay::debug_traverse(*it, *iit, "-> Leaving");

                --enter_count;
                if ( enter_count == 0 )
                {
                    entered = false;
                    action::leave(current_piece, ls1, current_segment_id,
                                  iit->seg_id.segment_index,
                                  it->point, *iit, oit);
                }
            }
            first = false;
        }

#ifdef PRINT_DEBUG
        std::cout << "*** enter count: " << enter_count << std::endl;
#endif
        BOOST_CHECK( enter_count == 0 );
        if (action::is_entered(entered))
        {
            geometry::copy_segments<false>(ls1, current_segment_id,
                                           boost::size(ls1) - 1,
                                           current_piece);
        }

        // Output the last one, if applicable
        if (::boost::size(current_piece) > 1)
        {
            *oit++ = current_piece;
        }

        return oit;
    }
};




} // namespace following

}} // namespace detail::turns

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_DIFFERENCE_FOLLOW_LINEAR_LINEAR_HPP
