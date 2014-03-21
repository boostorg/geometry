// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle


#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_LINEAR_LINEAR_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_LINEAR_LINEAR_HPP

#include <algorithm>

#include <boost/geometry/algorithms/append.hpp>
#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/unique.hpp>

#include <boost/geometry/algorithms/detail/relate/turns.hpp>

#include <boost/geometry/algorithms/detail/turns/compare_turns.hpp>
#include <boost/geometry/algorithms/detail/turns/print_turns.hpp>
#include <boost/geometry/algorithms/detail/turns/filter_continue_turns.hpp>
#include <boost/geometry/algorithms/detail/turns/remove_duplicate_turns.hpp>

#include <boost/geometry/algorithms/detail/overlay/follow_linear_linear.hpp>



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
    typename Geometry,
    bool Enable = false,
    typename GeometryTag = typename tag<Geometry>::type
>
struct remove_extra_points
{
    static inline void apply(Geometry& ) {}
};


template <typename Linestring>
struct remove_extra_points<Linestring, true, linestring_tag>
{
    static inline void apply(Linestring& linestring)
    {
        geometry::unique(linestring);
        if ( boost::size(linestring) == 1 )
        {
            geometry::append(linestring, *boost::begin(linestring));
        }
    }
};


template <typename MultiLinestring>
struct remove_extra_points<MultiLinestring, true, multi_linestring_tag>
{
    static inline void apply(MultiLinestring& multilinestring)
    {
        BOOST_AUTO_TPL(it, boost::begin(multilinestring));
        for (; it != boost::end(multilinestring); ++it)
        {
            remove_extra_points
                <
                    typename boost::range_value<MultiLinestring>::type,
                    true
                >::apply(*it);
        }
    }
};









template
<
    typename Linear1,
    typename Linear2,
    typename LinestringOut,
    overlay_type OverlayType,
    bool EnableRemoveExtraPoints = true,
    bool EnableFilterContinueTurns = false,
    bool EnableRemoveDuplicateTurns = true,
    bool EnableDegenerateTurns = true
>
class linear_linear_linestring
{
protected:
    struct AssignPolicy
    {
        static bool const include_no_turn = false;
        static bool const include_degenerate = EnableDegenerateTurns;
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
            >::apply(linear1, linear2, turns, oit);
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
                          LinearGeometry1 const& linear1,
                          LinearGeometry2 const& linear2,
                          OutputIterator oit)
    {
        // remove turns that have no added value
        turns::filter_continue_turns
            <
                Turns, EnableFilterContinueTurns
            >::apply(turns);

        // sort by seg_id, distance, and operation
        std::sort(boost::begin(turns), boost::end(turns),
                  detail::turns::less_seg_dist_other_op<>());

        // remove duplicate turns
        turns::remove_duplicate_turns
            <
                Turns, EnableRemoveDuplicateTurns
            >::apply(turns);

#ifdef GEOMETRY_TEST_DEBUG
        Linear2 linear2_reverse(linear2);
        geometry::reverse(linear2_reverse);
        Turns reverse_turns;
        compute_turns(reverse_turns, linear1, linear2_reverse);


        Turns turns_copy(turns);
        Turns reverse_turns_copy(reverse_turns);

        turns::filter_continue_turns
            <
                Turns, true //EnableFilterContinueTurns
            >::apply(turns_copy);

        turns::filter_continue_turns
            <
                Turns, EnableFilterContinueTurns
            >::apply(reverse_turns);

        turns::filter_continue_turns
            <
                Turns, true //EnableFilterContinueTurns
            >::apply(reverse_turns_copy);

        std::sort(boost::begin(turns_copy), boost::end(turns_copy),
                  detail::turns::less_seg_dist_other_op<>());

        std::sort(boost::begin(reverse_turns), boost::end(reverse_turns),
                  detail::turns::less_seg_dist_other_op<std::greater<int> >());

        std::sort(boost::begin(reverse_turns_copy), boost::end(reverse_turns_copy),
                  detail::turns::less_seg_dist_other_op<std::greater<int> >());

        turns::remove_duplicate_turns
            <
                Turns, EnableRemoveDuplicateTurns
            >::apply(reverse_turns);

       turns::remove_duplicate_turns
            <
                Turns, EnableRemoveDuplicateTurns
            >::apply(reverse_turns_copy);

        BOOST_ASSERT( boost::size(turns_copy) == boost::size(reverse_turns_copy) );

        std::cout << std::endl << std::endl;
        std::cout << "### ORIGINAL TURNS ###" << std::endl;
        detail::turns::print_turns(linear1, linear2, turns);
        std::cout << std::endl << std::endl;
        std::cout << "### ORIGINAL REVERSE TURNS ###" << std::endl;
        detail::turns::print_turns(linear1, linear2, reverse_turns);
        std::cout << std::endl << std::endl;
        std::cout << "### TURNS W/O CONTINUE TURNS ###" << std::endl;
        detail::turns::print_turns(linear1, linear2, turns_copy);
        std::cout << std::endl << std::endl;
        std::cout << "### REVERSE TURNS W/O CONTINUE TURNS ###" << std::endl;
        detail::turns::print_turns(linear1, linear2_reverse, reverse_turns_copy);
        std::cout << std::endl << std::endl;
#endif

        return follow_turns
            <
                OverlayTypeForFollow, FollowIsolatedPoints
            >(turns, linear1, linear2, oit);
    }

public:
    template
    <
        typename OutputIterator, typename Strategy
    >
    static inline OutputIterator apply(Linear1 const& lineargeometry1,
                                       Linear2 const& lineargeometry2,
                                       OutputIterator oit,
                                       Strategy const& )
    {
        Linear1 linear1(lineargeometry1);
        Linear2 linear2(lineargeometry2);

        remove_extra_points<Linear1, EnableRemoveExtraPoints>::apply(linear1);
        remove_extra_points<Linear2, EnableRemoveExtraPoints>::apply(linear2);

        typedef typename detail::relate::turns::get_turns
            <
                Linear1, Linear2
            >::turn_info turn_info;

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

        return sort_and_follow_turns
            <
                OverlayType, OverlayType == overlay_intersection
            >(turns, linear1, linear2, oit);
    }
};




template
<
    typename Linear1,
    typename Linear2,
    typename LinestringOut,
    bool EnableRemoveExtraPoints,
    bool EnableFilterContinueTurns,
    bool EnableRemoveDuplicateTurns,
    bool EnableDegenerateTurns
>
struct linear_linear_linestring
    <
        Linear1, Linear2, LinestringOut, overlay_union,
        EnableRemoveExtraPoints, EnableFilterContinueTurns,
        EnableRemoveDuplicateTurns, EnableDegenerateTurns
    >
{
    template
    <
        typename OutputIterator, typename Strategy
    >
    static inline OutputIterator apply(Linear1 const& lineargeometry1,
                                       Linear2 const& lineargeometry2,
                                       OutputIterator oit,
                                       Strategy const& strategy)
    {
        Linear1 linear1(lineargeometry1);
        Linear2 linear2(lineargeometry2);

        remove_extra_points<Linear1, EnableRemoveExtraPoints>::apply(linear1);
        remove_extra_points<Linear2, EnableRemoveExtraPoints>::apply(linear2);

        oit = linear_linear_no_intersections
            <
                LinestringOut,
                overlay_difference,
                Linear1,
                typename tag<Linear1>::type
            >::apply(linear1, oit);

        return linear_linear_linestring
            <
                Linear2, Linear1, LinestringOut, overlay_difference,
                false, EnableFilterContinueTurns,
                EnableRemoveDuplicateTurns, EnableDegenerateTurns
            >::apply(linear2, linear1, oit, strategy);
    }
};




}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry



#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_LINEAR_LINEAR_HPP
