// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2017, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_DISTANCE_CROSS_TRACK_BOX_BOX_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_DISTANCE_CROSS_TRACK_BOX_BOX_HPP

#include <boost/config.hpp>
#include <boost/concept_check.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_void.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/assert.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/formulas/cross_track_geo.hpp>
#include <boost/geometry/formulas/cross_track_box_box.hpp>

#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/concepts/distance_concept.hpp>
#include <boost/geometry/strategies/spherical/distance_cross_track.hpp>

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
    typename Strategy = cross_track_geo<>,
    typename Spheroid = srs::spheroid<double>,
    typename CalculationType = void
>
class cross_track_box_box_geo
{
public:
    template <typename Box1, typename Box2>
    struct return_type
        : services::return_type<Strategy, typename point_type<Box1>::type, typename point_type<Box2>::type>
    {};

    inline cross_track_box_box_geo()
    {}

    template <typename Box1, typename Box2>
    inline typename return_type<Box1, Box2>::type
    apply(Box1 const& box1, Box2 const& box2) const
    {
#if !defined(BOOST_MSVC)
        BOOST_CONCEPT_ASSERT
            (
                (concepts::PointSegmentDistanceStrategy
                    <
                        Strategy,
                        typename point_type<Box1>::type,
                        typename point_type<Box2>::type
                    >)
            );
#endif

        typedef typename return_type<Box1, Box2>::type return_type;
        return geometry::formula::cross_track_box_box
                                       <return_type>::apply(box1, box2, Strategy());
    }
};



#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <typename Strategy, typename Spheroid, typename CalculationType>
struct tag<cross_track_box_box_geo<Strategy, Spheroid, CalculationType> >
{
    typedef strategy_tag_distance_box_box type;
};


template <typename Strategy, typename Spheroid, typename CalculationType, typename P, typename Box>
struct return_type<cross_track_box_box_geo<Strategy, Spheroid, CalculationType>, P, Box>
    : cross_track_box_box_geo
        <
            Strategy, Spheroid, CalculationType
        >::template return_type<P, Box>
{};

template <typename Strategy, typename Spheroid, typename P, typename Box>
struct return_type<cross_track_box_box_geo<Strategy, Spheroid>, P, Box>
    : cross_track_box_box_geo
        <
            Strategy, Spheroid
        >::template return_type<P, Box>
{};

template <typename Strategy, typename P, typename Box>
struct return_type<cross_track_box_box_geo<Strategy>, P, Box>
    : cross_track_box_box_geo
        <
            Strategy
        >::template return_type<P, Box>
{};

template <typename Strategy, typename Spheroid, typename CalculationType>
struct comparable_type<cross_track_box_box_geo<Strategy, Spheroid, CalculationType> >
{
    typedef cross_track_box_box_geo
        <
            typename comparable_type<Strategy>::type, Spheroid, CalculationType
        > type;
};


template <typename Strategy, typename Spheroid, typename CalculationType>
struct get_comparable<cross_track_box_box_geo<Strategy, Spheroid, CalculationType> >
{
    typedef cross_track_box_box_geo<Strategy, Spheroid, CalculationType> this_strategy;
    typedef typename comparable_type<this_strategy>::type comparable_type;

public:
    static inline comparable_type apply(this_strategy const& strategy)
    {
        return comparable_type();
    }
};


template <typename Strategy, typename Spheroid, typename CalculationType, typename Box1, typename Box2>
struct result_from_distance
    <
        cross_track_box_box_geo<Strategy, Spheroid, CalculationType>, Box1, Box2
    >
{
private:
    typedef cross_track_box_box_geo<Strategy, Spheroid, CalculationType> this_strategy;

    typedef typename this_strategy::template return_type
        <
            Box1, Box2
        >::type return_type;

public:
    template <typename T>
    static inline return_type apply(this_strategy const& strategy,
                                    T const& distance)
    {
        result_from_distance
            <
                Strategy,
                typename point_type<Box1>::type,
                typename point_type<Box2>::type
            >::apply(strategy, distance);
    }
};


// define cross_track_box_box<default_point_segment_strategy> as
// default box-box strategy for the geographic coordinate system
template <typename Box1, typename Box2, typename Strategy>
struct default_strategy
    <
        box_tag, box_tag, Box1, Box2,
        geographic_tag, geographic_tag,
        Strategy
    >
{
    typedef cross_track_box_box_geo
        <
            typename boost::mpl::if_
                <
                    boost::is_void<Strategy>,
                    typename default_strategy
                        <
                            point_tag, segment_tag,
                            typename point_type<Box1>::type, typename point_type<Box2>::type,
                            geographic_tag, geographic_tag
                        >::type,
                    Strategy
                >::type
        > type;
};


} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::distance


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_DISTANCE_CROSS_TRACK_BOX_BOX_HPP
