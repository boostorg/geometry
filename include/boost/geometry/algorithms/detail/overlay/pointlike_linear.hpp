// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle


#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_POINTLIKE_LINEAR_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_POINTLIKE_LINEAR_HPP

#include <boost/geometry/policies/compare.hpp>
#include <boost/geometry/algorithms/append.hpp>
#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/within.hpp>
#include <boost/geometry/multi/algorithms/within.hpp>
#include <boost/geometry/algorithms/detail/overlay/pointlike_pointlike.hpp>
#include <boost/geometry/algorithms/detail/overlay/linear_linear.hpp>

#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/multi/geometries/multi_linestring.hpp>

namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{


template
<
    typename Point,
    typename Linear,
    typename PointOut,
    overlay_type OverlayType
>
struct point_linear_point
{
    template
    <
        typename OutputIterator, typename Strategy
    >
    static inline OutputIterator apply(Point const& point,
                                       Linear const& linear,
                                       OutputIterator oit,
                                       Strategy const&)
    {
        bool contained = geometry::within(point, linear);
        action_selector_pl_pl
            <
                PointOut, OverlayType
            >::apply(point, contained, oit);
        return oit;
    }
};




//===========================================================================
//===========================================================================
//===========================================================================




template
<
    typename MultiPoint1,
    typename Linear2,
    typename PointOut,
    overlay_type OverlayType
>
class multipoint_linear_point
{
private:
    typedef typename point_type<MultiPoint1>::type Point1;
    typedef model::linestring<Point1> Linestring1;
    typedef model::multi_linestring<Linestring1> MultiLinestring1;


    template <typename MultiPoint, typename MultiLinestring>
    static inline
    void multipoint_to_multilinestring(MultiPoint const& multipoint,
                                       MultiLinestring& multilinestring)
    {
        typedef typename boost::range_value
            <
                MultiLinestring
            >::type Linestring;

        BOOST_AUTO_TPL(it, boost::begin(multipoint));
        for (; it != boost::end(multipoint); ++it)
        {
            Linestring linestring;
            geometry::append(linestring, *it);
            geometry::append(linestring, *it);
            traits::push_back<MultiLinestring>::apply(multilinestring,
                                                      linestring);
        }
    }


    struct AssignPolicy
    {
        static bool const include_no_turn = false;
        static bool const include_degenerate = true;
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

    template <typename Point>
    struct TurnLess
    {
        geometry::less<Point> turn_point_less;

        template <typename Turn>
        bool operator()(Turn const& turn1, Turn const& turn2) const
        {
            return turn_point_less(turn1.point, turn2.point);
        }
    };


public:
    template
    <
        typename OutputIterator, typename Strategy
    >
    static inline OutputIterator apply(MultiPoint1 const& multipoint1,
                                       Linear2 const& linear2,
                                       OutputIterator oit,
                                       Strategy const& strategy)
    {
        typedef traversal_turn_info<Point1> turn_info;

        typedef std::vector<turn_info> Turns;

        MultiLinestring1 multilinestring1;
        multipoint_to_multilinestring(multipoint1, multilinestring1);

        Turns turns;
        compute_turns(turns, multilinestring1, linear2);

#ifdef GEOMETRY_TEST_DEBUG
        detail::turns::print_turns(multilinestring1, linear2, turns);
#endif

        MultiPoint1 turn_points;
        for (typename Turns::const_iterator it = turns.begin();
             it != turns.end(); ++it)
        {
            geometry::append(turn_points, it->point);
        }

        return multipoint_multipoint_point
            <
                MultiPoint1, MultiPoint1, PointOut, OverlayType
            >::apply(multipoint1, turn_points, oit, strategy);
    }
};




}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry



#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_POINTLIKE_LINEAR_HPP
