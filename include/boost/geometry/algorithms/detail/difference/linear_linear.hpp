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


    struct TurnEqualsTo
    {
        template <typename Turn>
        bool operator()(Turn const& t1, Turn const& t2) const
        {
            return geometry::equals(t1.point, t2.point)
                && t1.operations[0].seg_id == t2.operations[0].seg_id
                && t1.operations[0].other_id == t2.operations[0].other_id;
        }
    };

    template <typename Turns>
    static inline void filter_turns(Turns& turns)
    {
        typedef typename Turns::iterator TurnIt;

        TurnIt new_end = std::remove_if(turns.begin(), turns.end(),
                                        IsContinueTurn());
        turns.resize( std::distance(turns.begin(), new_end) );
    }

    template <typename Turns>
    static inline void remove_duplicates(Turns& turns)
    {
        typedef typename Turns::iterator TurnIt;

        TurnIt new_end = std::unique(turns.begin(), turns.end(),
                                     TurnEqualsTo());
        turns.resize( std::distance(turns.begin(), new_end) );
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
        typedef geometry::model::multi_linestring
            <
                LinestringOut
            > MultiLinestringOut;

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
#ifdef PRINT_DEBUG
            std::cout << "NO INTERSECTIONS" << std::endl;
#endif
            std::copy(mls1.begin(), mls1.end(), oit);
            return oit;
        }

        // remove turns that have no added value
#if 1
        filter_turns(turns);
        filter_turns(reverse_turns);
#endif

        // sort by seg_id, distance, and operation
        typedef detail::turns::less_seg_dist_other_op<> less;
        std::sort(boost::begin(turns), boost::end(turns), less());

        typedef
        detail::turns::less_seg_dist_other_op<std::greater<int> > rev_less;
        std::sort(boost::begin(reverse_turns), boost::end(reverse_turns),
                  rev_less());

        remove_duplicates(turns);
        remove_duplicates(reverse_turns);

#ifdef PRINT_DEBUG
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
        geometry::model::multi_linestring<Linestring1> mls1;
        geometry::model::multi_linestring<Linestring2> mls2;
        
        mls1.push_back(linestring1);
        mls2.push_back(linestring2);

        
        return
            multilinestring_multilinestring_linestring
            <
                LinestringOut
            >::apply(mls1, mls2, oit, strategy);
    }
};




}} // namespace detail::difference
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry



#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_DIFFERENCE_LINEAR_LINEAR_HPP
