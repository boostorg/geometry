// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle


#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_LINEAR_LINEAR_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_LINEAR_LINEAR_HPP

#include <algorithm>

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/detail/relate/turns.hpp>

#include <boost/geometry/algorithms/detail/turns/compare_turns.hpp>
#include <boost/geometry/algorithms/detail/turns/print_turns.hpp>
#include <boost/geometry/algorithms/detail/overlay/follow_linear_linear.hpp>

#include <boost/geometry/multi/geometries/multi_linestring.hpp>

namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{


//===========================================================================
//===========================================================================
//===========================================================================


template
<
    typename LineStringOut,
    overlay_type OverlayType,
    typename Geometry,
    typename GeometryTag
>
struct linear_linear_no_intersections;


template <typename LineStringOut, typename LineString>
struct linear_linear_no_intersections
    <
        LineStringOut, overlay_difference, LineString, linestring_tag
    >
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
struct linear_linear_no_intersections
    <
        LineStringOut,
        overlay_difference,
        MultiLineString,
        multi_linestring_tag
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


template <typename LineStringOut, typename Geometry, typename GeometryTag>
struct linear_linear_no_intersections
    <
        LineStringOut, overlay_intersection, Geometry, GeometryTag
    >
{
    template <typename OutputIterator>
    static inline OutputIterator apply(Geometry const&,
                                       OutputIterator oit)
    {
        return oit;
    }
};











template
<
    typename Linear1,
    typename Linear2,
    typename LinestringOut,
    overlay_type OverlayType
>
class linear_linear_linestring
{
protected:
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
            calculate_distance_policy::apply(info, p1, p2, ii, di);
        }
    };


    class IsContinueTurn
    {
    private:
        template <typename Operation>
        inline bool is_continue_or_opposite(Operation const& operation) const
        {
            return operation == operation_continue
                || operation == operation_opposite;
        }

    public:
        template <typename Turn>
        bool operator()(Turn const& turn) const
        {
            if ( turn.method != method_collinear
                 && turn.method != method_equal )
            {
                return false;
            }

            return is_continue_or_opposite(turn.operations[0].operation)
                && is_continue_or_opposite(turn.operations[1].operation);
        }
    };


#ifndef BOOST_GEOMETRY_DIFFERENCE_DO_NOT_REMOVE_DUPLICATE_TURNS
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
#endif


    template <typename Turns>
    static inline void filter_turns(Turns& turns)
    {
        typedef typename Turns::iterator TurnIt;

        TurnIt new_end = std::remove_if(turns.begin(), turns.end(),
                                        IsContinueTurn());
        turns.resize( std::distance(turns.begin(), new_end) );
    }


#ifndef BOOST_GEOMETRY_DIFFERENCE_DO_NOT_REMOVE_DUPLICATE_TURNS
    template <typename Turns>
    static inline void remove_duplicates(Turns& turns)
    {
        typedef typename Turns::iterator TurnIt;

        TurnIt new_end = std::unique(turns.begin(), turns.end(),
                                     TurnEqualsTo());
        turns.resize( std::distance(turns.begin(), new_end) );
    }
#endif


    template
    <
        typename Turns,
        typename LinearGeometry1,
        typename LinearGeometry2
    >
    static inline void compute_turns(Turns& turns,
                                     LinearGeometry1 const& linear1,
                                     LinearGeometry2 const& linear2)
    {
        turns.clear();
        geometry::detail::relate::turns::get_turns
            <
                LinearGeometry1,
                LinearGeometry2,
                detail::get_turns::get_turn_info_type
                <
                    LinearGeometry1,
                    LinearGeometry2,
                    AssignPolicy
                >
            >::apply(turns, linear1, linear2);
    }


   template
    <
        overlay_type OverlayTypeForFollow,
        bool FollowIsolatedPoints,
        typename Turns,
        typename LinearGeometry1,
        typename LinearGeometry2,
        typename OutputIterator
    >
    static inline OutputIterator follow_turns(Turns const& turns,
                                              Turns const& reverse_turns,
                                              LinearGeometry1 const& linear1,
                                              LinearGeometry2 const& linear2,
                                              OutputIterator oit)
    {
        return detail::overlay::following::linear::follow
            <
                LinestringOut,
                LinearGeometry1,
                LinearGeometry2,
                OverlayTypeForFollow,
                FollowIsolatedPoints
            >::apply(linear1, linear2, turns, reverse_turns, oit);
    }


    template
    <
        overlay_type OverlayTypeForFollow,
        bool FollowIsolatedPoints,
        typename Turns,
        typename LinearGeometry1,
        typename LinearGeometry2,
        typename OutputIterator
    >
    static inline OutputIterator
    sort_and_follow_turns(Turns& turns,
                          Turns& reverse_turns,
                          LinearGeometry1 const& linear1,
                          LinearGeometry2 const& linear2,
                          OutputIterator oit)
    {
        // remove turns that have no added value
        filter_turns(turns);
        filter_turns(reverse_turns);

        // sort by seg_id, distance, and operation
        std::sort(boost::begin(turns), boost::end(turns),
                  detail::turns::less_seg_dist_other_op<>());

        std::sort(boost::begin(reverse_turns), boost::end(reverse_turns),
                  detail::turns::less_seg_dist_other_op<std::greater<int> >());

#ifndef BOOST_GEOMETRY_DIFFERENCE_DO_NOT_REMOVE_DUPLICATE_TURNS
        // remove duplicate turns
        remove_duplicates(turns);
        remove_duplicates(reverse_turns);
#endif

#ifdef GEOMETRY_TEST_DEBUG
        detail::turns::print_turns(linear1, linear2, turns);
        std::cout << std::endl << std::endl;
        Linear2 linear2_reverse = linear2;
        geometry::reverse(linear2_reverse);
        detail::turns::print_turns(linear1, linear2_reverse, reverse_turns);
#endif

        return follow_turns
            <
                OverlayTypeForFollow, FollowIsolatedPoints
            >(turns, reverse_turns, linear1, linear2, oit);
    }

public:
    template
    <
        typename OutputIterator, typename Strategy
    >
    static inline OutputIterator apply(Linear1 const& linear1,
                                       Linear2 const& linear2,
                                       OutputIterator oit,
                                       Strategy const& )
    {
        typedef traversal_turn_info
            <
                typename point_type<LinestringOut>::type
            > turn_info;

        typedef std::vector<turn_info> Turns;

        Turns turns;
        compute_turns(turns, linear1, linear2);

        if ( turns.empty() )
        {
            // the two linear geometries are disjoint
#ifdef GEOMETRY_TEST_DEBUG
            std::cout << "NO INTERSECTIONS" << std::endl;
#endif
            return linear_linear_no_intersections
                <
                    LinestringOut,
                    OverlayType,
                    Linear1,
                    typename tag<Linear1>::type
                >::apply(linear1, oit);
        }


        Turns reverse_turns;
        Linear2 linear2_reverse = linear2;
        geometry::reverse(linear2_reverse);
        compute_turns(reverse_turns, linear1, linear2_reverse);

        return sort_and_follow_turns
            <
                OverlayType, OverlayType == overlay_intersection
            >(turns, reverse_turns, linear1, linear2, oit);
    }
};




template
<
    typename Linear1,
    typename Linear2,
    typename LinestringOut
>
struct linear_linear_linestring<Linear1, Linear2, LinestringOut, overlay_union>
{
    template
    <
        typename OutputIterator, typename Strategy
    >
    static inline OutputIterator apply(Linear1 const& linear1,
                                       Linear2 const& linear2,
                                       OutputIterator oit,
                                       Strategy const& strategy)
    {
        oit = linear_linear_no_intersections
            <
                LinestringOut,
                overlay_difference,
                Linear1,
                typename tag<Linear1>::type
            >::apply(linear1, oit);

        return linear_linear_linestring
            <
                Linear2, Linear1, LinestringOut, overlay_difference
            >::apply(linear2, linear1, oit, strategy);
    }
};




}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry



#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_LINEAR_LINEAR_HPP
