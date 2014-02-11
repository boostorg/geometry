// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle


#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_DIFFERENCE_LINEAR_LINEAR_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_DIFFERENCE_LINEAR_LINEAR_HPP

#include <algorithm>

#include <boost/geometry/algorithms/detail/relate/turns.hpp>

#include <boost/geometry/algorithms/detail/turns/compare_turns.hpp>
#include <boost/geometry/algorithms/detail/turns/print_turns.hpp>
#include <boost/geometry/algorithms/detail/turns/follow_linear_linear.hpp>

#include <boost/geometry/multi/geometries/multi_linestring.hpp>

namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace difference
{


//===========================================================================
//===========================================================================
//===========================================================================



template <typename LinestringOut>
struct multilinestring_multilinestring_linestring
{
    typedef typename point_type<LinestringOut>::type PointOut;
    typedef overlay::traversal_turn_info<PointOut> turn_info;
    typedef std::vector<turn_info> Turns;
    typedef typename Turns::iterator TurnIt;
    typedef detail::get_turns::no_interrupt_policy InterruptPolicy;

    struct AssignPolicy
    {
        static bool const include_no_turn = false;
        static bool const include_degenerate = false;
        static bool const include_opposite = false;

        template
        <
            typename Info,
            typename Point1,
            typename Point2,
            typename IntersectionInfo,
            typename DirInfo
        >
        static inline void apply(Info& info, Point1 const& p1, Point2 const& p2,
                                 IntersectionInfo const& ii, DirInfo const& di)
        {
            overlay::calculate_distance_policy::apply(info, p1, p2, ii, di);
        }
    };

    struct IsContinueTurn
    {
        template <typename Turn>
        bool operator()(Turn const& turn) const
        {
            if ( turn.method != overlay::method_collinear &&
                 turn.method != overlay::method_equal )
            {
                return false;
            }
            overlay::operation_type op[2];
            op[0] = turn.operations[0].operation;
            op[1] = turn.operations[1].operation;

            return
                (op[0] == overlay::operation_continue ||
                 op[0] == overlay::operation_opposite) &&
                (op[1] == overlay::operation_continue ||
                 op[1] == overlay::operation_opposite);
        }
    };


    template <typename Turns>
    static inline void filter_turns(Turns& turns)
    {
#if 1
        typedef typename Turns::iterator TurnIt;

        TurnIt new_end = std::remove_if(turns.begin(), turns.end(),
                                        IsContinueTurn());
        turns.resize( std::distance(turns.begin(), new_end) );
#endif
    }


    template
    <
        typename Linestring1, typename Linestring2,
        typename OutputIterator, typename Strategy
    >
    static inline OutputIterator apply(Linestring1 const& multilinestring1,
                                       Linestring2 const& multilinestring2,
                                       OutputIterator oit,
                                       Strategy const& )
    {
        std::cout << "MultiLinestring-MultiLinestring difference" << std::endl;

        typedef geometry::model::multi_linestring<LinestringOut> MultiLinestringOut;

        MultiLinestringOut mls1, mls2;
        geometry::convert(multilinestring1, mls1);
        geometry::convert(multilinestring2, mls2);

        assert( boost::size(mls1) > 0 );
        assert( boost::size(mls2) > 0 );


        //        canonical<LinestringOut>::apply(ls1);
        //        canonical<LinestringOut>::apply(ls2);

        //        typedef typename point_type<LinestringOut>::type PointOut;

#if 0
        typedef //overlay::assign_null_policy
            overlay::calculate_distance_policy AssignPolicy;
#endif
        //        typedef //overlay::assign_null_policy
        //            detail::union_::assign_union_policy AssignPolicy;

        //        typedef detail::disjoint::disjoint_interrupt_policy InterruptPolicy;

        Turns turns, reverse_turns;

        geometry::detail::relate::turns::get_turns
            <
                MultiLinestringOut,
                MultiLinestringOut,
                detail::get_turns::get_turn_info_type
                <
                    MultiLinestringOut,
                    MultiLinestringOut,
                    AssignPolicy
                >
            >::apply(turns, mls1, mls2);

        MultiLinestringOut mls2_reverse = mls2;
        geometry::reverse(mls2_reverse);
        geometry::detail::relate::turns::get_turns
            <
                MultiLinestringOut,
                MultiLinestringOut,
                detail::get_turns::get_turn_info_type
                <
                    MultiLinestringOut,
                    MultiLinestringOut,
                    AssignPolicy
                >
            >::apply(reverse_turns, mls1, mls2_reverse);

        if ( turns.empty() )
        {
            // the two linestrings are disjoint; we return the first as is;
            //            canonical<MultiLinestringOut>::apply(mls1);
            std::cout << "NO INTERSECTIONS" << std::endl;
            std::copy(mls1.begin(), mls1.end(), oit);
            return oit;
        }

        // remove turns that have no added value
        filter_turns(turns);
        filter_turns(reverse_turns);

        // sort by seg_id, distance, and operation
        typedef detail::turns::less_seg_dist_other_op<> less;
        std::sort(boost::begin(turns), boost::end(turns), less());

        typedef
        detail::turns::less_seg_dist_other_op<std::greater<int> > rev_less;
        std::sort(boost::begin(reverse_turns), boost::end(reverse_turns),
                  rev_less());


#if 1
        detail::turns::print_turns(mls1, mls2, turns);
        std::cout << std::endl << std::endl;
        detail::turns::print_turns(mls1, mls2_reverse, reverse_turns);
#endif

        return detail::turns::following::follow
            <
                LinestringOut,
                MultiLinestringOut,
                MultiLinestringOut,
                overlay_difference
            >::apply(mls1, mls2, turns, reverse_turns, oit);
    }
};


//===========================================================================
//===========================================================================
//===========================================================================





template <typename LinestringOut>
struct linestring_linestring_linestring
{
    typedef typename point_type<LinestringOut>::type PointOut;
    typedef overlay::traversal_turn_info<PointOut> turn_info;


    template
    <
        typename Linestring1, typename Linestring2,
        typename OutputIterator, typename Strategy
    >
    static inline OutputIterator apply(Linestring1 const& linestring1,
                                       Linestring2 const& linestring2,
                                       OutputIterator oit,
                                       Strategy const& strategy)
    {
        std::cout << "Linestring-Linestring difference" << std::endl;

#if 1
        geometry::model::multi_linestring<Linestring1> mls1;
        geometry::model::multi_linestring<Linestring2> mls2;
        
        mls1.push_back(linestring1);
        mls2.push_back(linestring2);

        
        return
            multilinestring_multilinestring_linestring
            <
                LinestringOut
            >::apply(mls1, mls2, oit, strategy);

#else
        LinestringOut ls1, ls2;
        geometry::convert(linestring1, ls1);
        geometry::convert(linestring2, ls2);

        assert( boost::size(ls1) > 1 );
        assert( boost::size(ls2) > 1 );


        //        canonical<LinestringOut>::apply(ls1);
        //        canonical<LinestringOut>::apply(ls2);

        //        typedef typename point_type<LinestringOut>::type PointOut;

        //       typedef //overlay::assign_null_policy
        //            overlay::calculate_distance_policy AssignPolicy;
        typedef //overlay::assign_null_policy
            detail::union_::assign_union_policy AssignPolicy;

        typedef overlay::get_turn_info
            <
                PointOut, PointOut, turn_info, AssignPolicy
            > TurnPolicy;

        typedef std::vector<turn_info> Turns;
        typedef typename std::vector<turn_info>::iterator TurnIt;
        typedef detail::get_turns::no_interrupt_policy InterruptPolicy;
        //        typedef detail::disjoint::disjoint_interrupt_policy InterruptPolicy;

        typedef geometry::model::multi_linestring<LinestringOut> MultiLinestringOut;
        MultiLinestringOut mls_out;

        Turns turns;
        InterruptPolicy interrupt_policy;

        geometry::get_turns
            <
                false, false,
                AssignPolicy
            >(ls1, ls2, turns, interrupt_policy);


        if ( turns.empty() )
        {
            // the two linestrings are disjoint; we return the first as is;
            mls_out.push_back(ls1);
            //            canonical<MultiLinestringOut>::apply(mls_out);
            std::copy(mls_out.begin(), mls_out.end(), oit);
            return oit;
        }

        // sort by seg_id, distance, and operation
        typedef overlay::turns::less_seg_dist_op<overlay::turns::operation_order_uibc> less;
        std::sort(boost::begin(turns), boost::end(turns), less());



#if 1
        detail::turns::print_turns(ls1, ls2, turns);
#endif

        LinestringOut current_piece;
        geometry::segment_identifier current_segment_id(0, -1, -1, -1);

        // Iterate through all intersection points (they are ordered along the line)


        typedef typename boost::range_iterator
            <
                typename turn_info::container_type
            >::type turn_operation_iterator_type;

        typedef detail::overlay::following::action_selector
            <
                overlay_difference
            > action;

        bool entered = false;
        bool first = true;
        for (TurnIt it = boost::begin(turns); it != boost::end(turns); ++it)
        {
            turn_operation_iterator_type iit = boost::begin(it->operations);

#if 1
            if (was_entered(*it, *iit, first, ls1, ls2))
            {
                detail::overlay::debug_traverse(*it, *iit, "-> Was entered");
                entered = true;
            }
#endif

            if (is_staying_inside(*it, *iit, entered, first, ls1, ls2))
            {
                detail::overlay::debug_traverse(*it, *iit, "-> Staying inside");

                entered = true;
            }
            else if (/*!entered &&*/ is_entering(*it, *iit, ls1, ls2))
            {
                detail::overlay::debug_traverse(*it, *iit, "-> Entering");

                entered = true;
                action::enter(current_piece, linestring1, current_segment_id, iit->seg_id.segment_index, it->point, *iit, oit);
            }
            else if (is_leaving(*it, *iit, entered, first, ls1, ls2))
            {
                detail::overlay::debug_traverse(*it, *iit, "-> Leaving");

                entered = false;
                action::leave(current_piece, linestring1, current_segment_id, iit->seg_id.segment_index, it->point, *iit, oit);
            }
            first = false;
        }

#if 1
        if (action::is_entered(entered))
        {
            geometry::copy_segments<false>(ls1, current_segment_id,
                    boost::size(linestring1) - 1,
                    current_piece);
        }

        // Output the last one, if applicable
        if (::boost::size(current_piece) > 1)
        {
            *oit++ = current_piece;
        }
#endif
#endif
        return oit;
    }
};




}} // namespace detail::difference
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry



#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_DIFFERENCE_LINEAR_LINEAR_HPP
