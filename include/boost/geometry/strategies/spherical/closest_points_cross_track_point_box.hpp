// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_SPHERICAL_CLOSEST_POINTS_CROSS_TRACK_POINT_BOX_HPP
#define BOOST_GEOMETRY_STRATEGIES_SPHERICAL_CLOSEST_POINTS_CROSS_TRACK_POINT_BOX_HPP

#include <boost/config.hpp>
#include <boost/concept_check.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_void.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/assert.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/algorithms/detail/closest_points/result.hpp>

#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/concepts/distance_concept.hpp>
#include <boost/geometry/strategies/spherical/distance_cross_track.hpp>
#include <boost/geometry/strategies/geographic/distance_cross_track.hpp>
#include <boost/geometry/strategies/spherical/distance_cross_track_point_box.hpp>

#include <boost/geometry/util/math.hpp>
#include <boost/geometry/algorithms/detail/assign_box_corners.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace closest_points
{


template
<
    typename CalculationType = void,
    typename Strategy = distance::haversine<double, CalculationType>
>
class cross_track_point_box
{
public:
    // point-segment strategy getters
    struct closest_points_ps_strategy
    {
        typedef cross_track<CalculationType, Strategy>
        type;
    };

    template <typename Point, typename Box>
    struct calculation_type
        : promote_floating_point
          <
              typename select_calculation_type
                  <
                      Point,
                      typename point_type<Box>::type,
                      CalculationType
                  >::type
          >
    {};

    template <typename Point, typename Box>
    struct closest_point_result
    {
        typedef geometry::detail::closest_points::result
        <
            typename calculation_type<Point, Box>::type
        > type;
    };

    //constructors

    inline cross_track_point_box()
    {}

    explicit inline cross_track_point_box(typename Strategy::radius_type const& r)
        : m_strategy(r)
    {}

    inline cross_track_point_box(Strategy const& s)
        : m_strategy(s)
    {}

    template <typename Point, typename Box>
    typename closest_point_result<Point, Box>::type
    apply(Point const& point, Box const& box) const
    {
        return distance::details::cross_track_point_box_generic
               <
                    typename closest_point_result<Point, Box>::type
               >::apply(point,
                        box,
                        //typename closest_points_ps_strategy::type(m_strategy)
                        closest_points::cross_track<CalculationType, Strategy>(m_strategy));
    }

private :
    Strategy m_strategy;

};

namespace services {

template <typename Point, typename Box>
struct default_strategy
    <
        point_tag, box_tag, Point, Box,
        spherical_equatorial_tag, spherical_equatorial_tag
    >
{
    typedef closest_points::cross_track_point_box<> type;
};

template <typename Box, typename Point>
struct default_strategy
    <
        box_tag, point_tag, Box, Point,
        spherical_equatorial_tag, spherical_equatorial_tag
    >
{
    typedef typename default_strategy
        <
            point_tag, box_tag, Point, Box,
            spherical_equatorial_tag, spherical_equatorial_tag
        >::type type;
};

}} // namespace closest_points::services

namespace distance { namespace services {

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

template <typename CalculationType, typename Strategy>
struct tag<closest_points::cross_track_point_box<CalculationType, Strategy> >
{
    typedef strategy_tag_distance_point_box type;
};

//return types

template <typename CalculationType, typename P, typename PS>
struct return_type<closest_points::cross_track_point_box<CalculationType>, P, PS>
        : closest_points::cross_track_point_box<CalculationType>
                        ::template closest_point_result<P, PS>
{};

template <typename CalculationType, typename Strategy, typename P, typename PS>
struct return_type<closest_points::cross_track_point_box<CalculationType, Strategy>, P, PS>
        : closest_points::cross_track_point_box<CalculationType, Strategy>
                        ::template closest_point_result<P, PS>
{};


template <typename CalculationType, typename Strategy>
struct comparable_type
       <
            closest_points::cross_track_point_box
            <
                CalculationType,
                Strategy
            >
       >
{
    typedef closest_points::cross_track_point_box
            <
                CalculationType, Strategy
            > type;
};


template <typename CalculationType, typename Strategy>
struct get_comparable
       <
            closest_points::cross_track_point_box
            <
                CalculationType,
                Strategy
            >
       >
{
public:
    static inline closest_points::cross_track_point_box
    <
        CalculationType,
        Strategy
    >
    apply(closest_points::cross_track_point_box
          <
              CalculationType,
              Strategy
          > const& str)
    {
        return str;
    }
};


template
<
    typename CalculationType,
    typename Strategy,
    typename P,
    typename Box
>
struct result_from_distance
       <
            closest_points::cross_track_point_box
            <
                CalculationType,
                Strategy
            >,
            P,
            Box
        >
{
private:
    typedef closest_points::geographic_cross_track_point_box
    <
        CalculationType,
        Strategy
    > this_strategy;

    typedef typename this_strategy::template calculation_type
                     <
                        P,
                        Box
                     >::type calculation_type;

public:
    template <typename T>
    static inline calculation_type apply(this_strategy const& strategy,
                                         T const& distance)
    {
        result_from_distance
            <
                Strategy, P, typename point_type<Box>::type
            >::apply(strategy, distance);
    }
};

} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS


}} // namespace strategy::distance


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_STRATEGIES_SPHERICAL_CLOSEST_POINTS_CROSS_TRACK_POINT_BOX_HPP
