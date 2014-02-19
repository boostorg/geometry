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


template <typename LineStringOut, typename Geometry, typename GeometryTag>
struct difference_no_intersections;


template <typename LineStringOut, typename LineString>
struct difference_no_intersections<LineStringOut, LineString, linestring_tag>
{
    template <typename OutputIterator>
    static inline OutputIterator apply(LineString const& linestring,
                                       OutputIterator oit)
    {
        LineStringOut ls_out;
        geometry::convert(linestring, ls_out);
        *oit++ = ls_out;
        return oit;
    }
};


template <typename LineStringOut, typename MultiLineString>
struct difference_no_intersections
    <
        LineStringOut, MultiLineString, multi_linestring_tag
    >
{
    template <typename OutputIterator>
    static inline OutputIterator apply(MultiLineString const& multilinestring,
                                       OutputIterator oit)
    {
        BOOST_AUTO_TPL(it, boost::begin(multilinestring));
        for (; it != boost::end(multilinestring); ++it)
        {
            LineStringOut ls_out;
            geometry::convert(*it, ls_out);
            *oit++ = ls_out;
        }
        return oit;
    }
};

template <typename LinestringOut>
struct linear_linear_linestring
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
        typedef typename Turns::iterator TurnIt;

        TurnIt new_end = std::remove_if(turns.begin(), turns.end(),
                                        IsContinueTurn());
        turns.resize( std::distance(turns.begin(), new_end) );
    }


    template
    <
        typename Linear1, typename Linear2,
        typename OutputIterator, typename Strategy
    >
    static inline OutputIterator apply(Linear1 const& linear1,
                                       Linear2 const& linear2,
                                       OutputIterator oit,
                                       Strategy const& )
    {
        typedef geometry::model::multi_linestring
            <
                LinestringOut
            > MultiLinestringOut;

        //        MultiLinestringOut mls1, mls2;
        //        geometry::convert(multilinestring1, mls1);
        //        geometry::convert(multilinestring2, mls2);

        //        assert( boost::size(mls1) > 0 );
        //        assert( boost::size(mls2) > 0 );


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
                Linear1,
                Linear2,
                detail::get_turns::get_turn_info_type
                <
                    Linear1,
                    Linear2,
                    AssignPolicy
                >
            >::apply(turns, linear1, linear2);

        Linear2 linear2_reverse = linear2;
        geometry::reverse(linear2_reverse);
        geometry::detail::relate::turns::get_turns
            <
                Linear1,
                Linear2,
                detail::get_turns::get_turn_info_type
                <
                    Linear1,
                    Linear2,
                    AssignPolicy
                >
            >::apply(reverse_turns, linear1, linear2_reverse);

        if ( turns.empty() )
        {
            // the two linestrings are disjoint; we return the first as is;
            //            canonical<MultiLinestringOut>::apply(mls1);
#ifdef GEOMETRY_TEST_DEBUG
            std::cout << "NO INTERSECTIONS" << std::endl;
#endif
            // MK:: need to think about this
            //            std::copy(linear1.begin(), linear1.end(), oit);
            oit = difference_no_intersections
                <
                    LinestringOut, Linear1, typename tag<Linear1>::type
                >::apply(linear1, oit);
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

#ifdef GEOMETRY_TEST_DEBUG
        detail::turns::print_turns(linear1, linear2, turns);
        std::cout << std::endl << std::endl;
        detail::turns::print_turns(linear1, linear2_reverse, reverse_turns);
#endif

        return detail::turns::following::follow
            <
                LinestringOut,
                Linear1,
                Linear2,
                overlay_difference
            >::apply(linear1, linear2, turns, reverse_turns, oit);
    }
};



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
        typedef typename Turns::iterator TurnIt;

        TurnIt new_end = std::remove_if(turns.begin(), turns.end(),
                                        IsContinueTurn());
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
#ifdef GEOMETRY_TEST_DEBUG
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

#ifdef GEOMETRY_TEST_DEBUG
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


//===========================================================================
//===========================================================================
//===========================================================================





template <typename LinestringOut>
struct linestring_multilinestring_linestring
{
    typedef typename point_type<LinestringOut>::type PointOut;
    typedef overlay::traversal_turn_info<PointOut> turn_info;


    template
    <
        typename Linestring, typename MultiLinestring,
        typename OutputIterator, typename Strategy
    >
    static inline OutputIterator apply(Linestring const& linestring,
                                       MultiLinestring const& multilinestring,
                                       OutputIterator oit,
                                       Strategy const& strategy)
    {
        geometry::model::multi_linestring<Linestring> mls;
        
        mls.push_back(linestring);
        
        return
            multilinestring_multilinestring_linestring
            <
                LinestringOut
            >::apply(mls, multilinestring, oit, strategy);
    }
};




}} // namespace detail::difference
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry



#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_DIFFERENCE_LINEAR_LINEAR_HPP
