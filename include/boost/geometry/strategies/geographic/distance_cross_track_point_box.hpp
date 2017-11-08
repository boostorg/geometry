// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2017, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_DISTANCE_CROSS_TRACK_POINT_BOX_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_DISTANCE_CROSS_TRACK_POINT_BOX_HPP

#include <boost/config.hpp>
#include <boost/concept_check.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_void.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/assert.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/core/tags.hpp>

//#include <boost/geometry/formulas/distance_point_segment.hpp>
//#include <boost/geometry/formulas/cross_track_point_box.hpp>

#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/concepts/distance_concept.hpp>
#include <boost/geometry/strategies/spherical/distance_cross_track.hpp>
#include <boost/geometry/strategies/geographic/distance_cross_track.hpp>
#include <boost/geometry/strategies/distance_cross_track_point_box.hpp>

#include <boost/geometry/util/math.hpp>
#include <boost/geometry/algorithms/detail/assign_box_corners.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace distance
{


/*!
\brief Strategy functor for distance point to box calculation
\ingroup strategies
\details Class which calculates the distance of a point to a box, for
points and boxes on a sphere or globe
\tparam CalculationType \tparam_calculation
\tparam Strategy underlying point-segment distance strategy, defaults
to cross track
\qbk{
[heading See also]
[link geometry.reference.algorithms.distance.distance_3_with_strategy distance (with strategy)]
}
*/
template
<
    typename FormulaPolicy = strategy::andoyer,
    //typename Strategy = geographic_cross_track<>,
    typename Spheroid = srs::spheroid<double>,
    typename CalculationType = void
>
class geographic_cross_track_point_box
{
public:
    typedef geographic_cross_track<FormulaPolicy, Spheroid, CalculationType> Strategy;

    template <typename Point, typename Box>
    struct return_type
        : services::return_type<Strategy, Point, typename point_type<Box>::type>
    {};

    inline geographic_cross_track_point_box()
    {}

    template <typename Point, typename Box>
    inline typename return_type<Point, Box>::type
    apply(Point const& point, Box const& box) const
    {
/*
#if !defined(BOOST_MSVC)
        BOOST_CONCEPT_ASSERT
            (
                (concepts::PointSegmentDistanceStrategy
                    <
                        Strategy, Point, typename point_type<Box>::type
                    >)
            );
#endif
*/

        typedef typename return_type<Point, Box>::type return_type;

        return cross_track_point_box_generic
                        <return_type>::apply(point, box, Strategy());
    }
};



#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <typename Strategy, typename Spheroid, typename CalculationType>
struct tag<geographic_cross_track_point_box<Strategy, Spheroid, CalculationType> >
{
    typedef strategy_tag_distance_point_box type;
};


template <typename Strategy, typename Spheroid, typename CalculationType, typename P, typename Box>
struct return_type<geographic_cross_track_point_box<Strategy, Spheroid, CalculationType>, P, Box>
    : geographic_cross_track_point_box
        <
            Strategy, Spheroid, CalculationType
        >::template return_type<P, Box>
{};

template <typename Strategy, typename Spheroid, typename P, typename Box>
struct return_type<geographic_cross_track_point_box<Strategy, Spheroid>, P, Box>
    : geographic_cross_track_point_box
        <
            Strategy, Spheroid
        >::template return_type<P, Box>
{};

template <typename Strategy, typename P, typename Box>
struct return_type<geographic_cross_track_point_box<Strategy>, P, Box>
    : geographic_cross_track_point_box
        <
            Strategy
        >::template return_type<P, Box>
{};

template <typename Strategy, typename Spheroid, typename CalculationType>
struct comparable_type<geographic_cross_track_point_box<Strategy, Spheroid, CalculationType> >
{
    typedef geographic_cross_track_point_box
        <
            typename comparable_type<Strategy>::type, Spheroid, CalculationType
        > type;
};


template <typename Strategy, typename Spheroid, typename CalculationType>
struct get_comparable<geographic_cross_track_point_box<Strategy, Spheroid, CalculationType> >
{
    typedef geographic_cross_track_point_box<Strategy, Spheroid, CalculationType> this_strategy;
    typedef typename comparable_type<this_strategy>::type comparable_type;

public:
    static inline comparable_type apply(this_strategy const& strategy)
    {
        return comparable_type();
    }
};


template <typename Strategy, typename Spheroid, typename CalculationType, typename P, typename Box>
struct result_from_distance
    <
        geographic_cross_track_point_box<Strategy, Spheroid, CalculationType>, P, Box
    >
{
private:
    typedef geographic_cross_track_point_box<Strategy, Spheroid, CalculationType> this_strategy;

    typedef typename this_strategy::template return_type
        <
            P, Box
        >::type return_type;

public:
    template <typename T>
    static inline return_type apply(this_strategy const& strategy,
                                    T const& distance)
    {
        Strategy s();

        result_from_distance
            <
                Strategy, P, typename point_type<Box>::type
            >::apply(s, distance);
    }
};

/*
// define cross_track_point_box<default_point_segment_strategy> as
// default point-box strategy for geographic coordinate system
template <typename Point, typename Box, typename Strategy>
struct default_strategy
    <
        point_tag, box_tag, Point, Box,
        geographic_tag, geographic_tag,
        Strategy
    >
{
    typedef geographic_cross_track_point_box
        <
            typename boost::mpl::if_
                <
                    boost::is_void<Strategy>,
                    typename default_strategy
                        <
                            point_tag, segment_tag,
                            Point, typename point_type<Box>::type,
                            geographic_tag, geographic_tag
                        >::type,
                    Strategy
                >::type
        > type;
};
*/

template <typename Point, typename Box>
struct default_strategy
    <
        point_tag, box_tag, Point, Box,
        geographic_tag, geographic_tag
    >
{
    typedef geographic_cross_track_point_box<> type;
};

template <typename Box, typename Point>
struct default_strategy
    <
        box_tag, point_tag, Box, Point,
        geographic_tag, geographic_tag
    >
{
    typedef typename default_strategy
        <
            point_tag, box_tag, Point, Box,
            geographic_tag, geographic_tag
        >::type type;
};
/*
template <typename Box, typename Point, typename Strategy>
struct default_strategy
    <
        box_tag, point_tag, Box, Point,
        geographic_tag, geographic_tag,
        Strategy
    >
{
    typedef typename default_strategy
        <
            point_tag, box_tag, Point, Box,
            geographic_tag, geographic_tag,
            Strategy
        >::type type;
};
*/

} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::distance


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_DISTANCE_CROSS_TRACK_POINT_BOX_HPP
