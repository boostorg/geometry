// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2016-2017 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fisikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_SPHERICAL_DISTANCE_CROSS_TRACK_BOX_BOX_HPP
#define BOOST_GEOMETRY_STRATEGIES_SPHERICAL_DISTANCE_CROSS_TRACK_BOX_BOX_HPP

#include <boost/config.hpp>
#include <boost/concept_check.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_void.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/assert.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/core/tags.hpp>

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
\brief Strategy functor for distance box to box calculation
\ingroup strategies
\details Class which calculates the distance of a box to a box, for
boxes on a sphere or globe
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
    typename CalculationType = void,
        typename Strategy = cross_track<CalculationType>
//        typename Strategy = haversine<CalculationType>
>
class cross_track_box_box
{
public:
    template <typename Box1, typename Box2>
    struct return_type
        : services::return_type<Strategy,
                                typename point_type<Box1>::type,
                                typename point_type<Box2>::type>
    {};

    typedef typename Strategy::radius_type radius_type;

    inline cross_track_box_box()
    {}

    explicit inline cross_track_box_box(typename Strategy::radius_type const& r)
        : m_ps_strategy(r)
    {}

    inline cross_track_box_box(Strategy const& s)
        : m_ps_strategy(s)
    {}


    // It might be useful in the future
    // to overload constructor with strategy info.
    // crosstrack(...) {}

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
                                       <return_type>::apply(box1, box2, m_ps_strategy);
    }

    inline typename Strategy::radius_type radius() const
    {
        return m_ps_strategy.radius();
    }

private:
    Strategy m_ps_strategy;
};



#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <typename CalculationType, typename Strategy>
struct tag<cross_track_box_box<CalculationType, Strategy> >
{
    typedef strategy_tag_distance_box_box type;
};


template <typename CalculationType, typename Strategy, typename Box1, typename Box2>
struct return_type<cross_track_box_box<CalculationType, Strategy>, Box1, Box2>
    : cross_track_box_box
        <
            CalculationType, Strategy
        >::template return_type<Box1, Box2>
{};


template <typename CalculationType, typename Strategy>
struct comparable_type<cross_track_box_box<CalculationType, Strategy> >
{
    typedef cross_track_box_box
        <
            CalculationType, typename comparable_type<Strategy>::type
        > type;
};


template <typename CalculationType, typename Strategy>
struct get_comparable<cross_track_box_box<CalculationType, Strategy> >
{
    typedef cross_track_box_box<CalculationType, Strategy> this_strategy;
    typedef typename comparable_type<this_strategy>::type comparable_type;

public:
    static inline comparable_type apply(this_strategy const& strategy)
    {
        return comparable_type(strategy.radius());
    }
};


template <typename CalculationType, typename Strategy, typename Box1, typename Box2>
struct result_from_distance
    <
        cross_track_box_box<CalculationType, Strategy>, Box1, Box2
    >
{
private:
    typedef cross_track_box_box<CalculationType, Strategy> this_strategy;

    typedef typename this_strategy::template return_type
        <
            Box1, Box2
        >::type return_type;

public:
    template <typename T>
    static inline return_type apply(this_strategy const& strategy,
                                    T const& distance)
    {
        Strategy s(strategy.radius());

        return result_from_distance
            <
                Strategy,
                typename point_type<Box1>::type,
                typename point_type<Box2>::type
            >::apply(s, distance);
    }
};


// define cross_track_box_box<default_point_segment_strategy> as
// default box-box strategy for the spherical equatorial coordinate system
template <typename Box1, typename Box2, typename Strategy>
struct default_strategy
    <
        box_tag, box_tag, Box1, Box2,
        spherical_equatorial_tag, spherical_equatorial_tag,
        Strategy
    >
{
    typedef cross_track_box_box
        <
            void,
            typename boost::mpl::if_
                <
                    boost::is_void<Strategy>,
                    typename default_strategy
                        <
                            point_tag, segment_tag,
                            typename point_type<Box1>::type, typename point_type<Box2>::type,
                            spherical_equatorial_tag, spherical_equatorial_tag
                        >::type,
                    Strategy
                >::type
        > type;
};

} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::distance


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_SPHERICAL_DISTANCE_CROSS_TRACK_BOX_BOX_HPP
