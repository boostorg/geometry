// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2008-2015 Bruno Lalande, Paris, France.
// Copyright (c) 2008-2015 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2009-2015 Mateusz Loskot, London, UK.

// This file was modified by Oracle on 2014-2017.
// Modifications copyright (c) 2014-2017, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_SPHERICAL_DISTANCE_CROSS_TRACK_POINT_BOX_HPP
#define BOOST_GEOMETRY_STRATEGIES_SPHERICAL_DISTANCE_CROSS_TRACK_POINT_BOX_HPP

#include <boost/config.hpp>
#include <boost/concept_check.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_void.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/assert.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/formulas/cross_track_point_box.hpp>

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
    typename CalculationType = void,
    typename Strategy = cross_track<CalculationType>
>
class cross_track_point_box
{
public:
    template <typename Point, typename Box>
    struct return_type
        : services::return_type<Strategy, Point, typename point_type<Box>::type>
    {};

    typedef typename Strategy::radius_type radius_type;

    inline cross_track_point_box()
    {}

    explicit inline cross_track_point_box(typename Strategy::radius_type const& r)
        : m_ps_strategy(r)
    {}

    inline cross_track_point_box(Strategy const& s)
        : m_ps_strategy(s)
    {}


    // It might be useful in the future
    // to overload constructor with strategy info.
    // crosstrack(...) {}

    template <typename Point, typename Box>
    inline typename return_type<Point, Box>::type
    apply(Point const& point, Box const& box) const
    {
#if !defined(BOOST_MSVC)
        BOOST_CONCEPT_ASSERT
            (
                (concepts::PointSegmentDistanceStrategy
                    <
                        Strategy, Point, typename point_type<Box>::type
                    >)
            );
#endif
        typedef typename return_type<Point, Box>::type return_type;
        return geometry::formula::cross_track_point_box
                                       <return_type>::apply(point, box, m_ps_strategy);
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
struct tag<cross_track_point_box<CalculationType, Strategy> >
{
    typedef strategy_tag_distance_point_box type;
};


template <typename CalculationType, typename Strategy, typename P, typename Box>
struct return_type<cross_track_point_box<CalculationType, Strategy>, P, Box>
    : cross_track_point_box
        <
            CalculationType, Strategy
        >::template return_type<P, Box>
{};


template <typename CalculationType, typename Strategy>
struct comparable_type<cross_track_point_box<CalculationType, Strategy> >
{
    typedef cross_track_point_box
        <
            CalculationType, typename comparable_type<Strategy>::type
        > type;
};


template <typename CalculationType, typename Strategy>
struct get_comparable<cross_track_point_box<CalculationType, Strategy> >
{
    typedef cross_track_point_box<CalculationType, Strategy> this_strategy;
    typedef typename comparable_type<this_strategy>::type comparable_type;

public:
    static inline comparable_type apply(this_strategy const& strategy)
    {
        return comparable_type(strategy.radius());
    }
};


template <typename CalculationType, typename Strategy, typename P, typename Box>
struct result_from_distance
    <
        cross_track_point_box<CalculationType, Strategy>, P, Box
    >
{
private:
    typedef cross_track_point_box<CalculationType, Strategy> this_strategy;

    typedef typename this_strategy::template return_type
        <
            P, Box
        >::type return_type;

public:
    template <typename T>
    static inline return_type apply(this_strategy const& strategy,
                                    T const& distance)
    {
        Strategy s(strategy.radius());

        return result_from_distance
            <
                Strategy, P, typename point_type<Box>::type
            >::apply(s, distance);
    }
};


// define cross_track_point_box<default_point_segment_strategy> as
// default point-box strategy for the spherical equatorial coordinate system
template <typename Point, typename Box, typename Strategy>
struct default_strategy
    <
        point_tag, box_tag, Point, Box,
        spherical_equatorial_tag, spherical_equatorial_tag,
        Strategy
    >
{
    typedef cross_track_point_box
        <
            void,
            typename boost::mpl::if_
                <
                    boost::is_void<Strategy>,
                    typename default_strategy
                        <
                            point_tag, segment_tag,
                            Point, typename point_type<Box>::type,
                            spherical_equatorial_tag, spherical_equatorial_tag
                        >::type,
                    Strategy
                >::type
        > type;
};


template <typename Box, typename Point, typename Strategy>
struct default_strategy
    <
        box_tag, point_tag, Box, Point,
        spherical_equatorial_tag, spherical_equatorial_tag,
        Strategy
    >
{
    typedef typename default_strategy
        <
            point_tag, box_tag, Point, Box,
            spherical_equatorial_tag, spherical_equatorial_tag,
            Strategy
        >::type type;
};


} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::distance


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES_SPHERICAL_DISTANCE_CROSS_TRACK_POINT_BOX_HPP
