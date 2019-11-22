// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_SPHERICAL_CLOSEST_POINTS_HPP
#define BOOST_GEOMETRY_STRATEGIES_SPHERICAL_CLOSEST_POINTS_HPP

#include <boost/geometry/strategies/closest_points.hpp>
#include <boost/geometry/strategies/spherical/distance_haversine.hpp>
#include <boost/geometry/formulas/point_segment_distance.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace closest_points
{

template
<
    typename CalculationType = void,
    typename Strategy = distance::haversine<double, CalculationType>
>
class spherical
{
public :

    template <typename Point, typename PointOfSegment>
    struct calculation_type
        : promote_floating_point
          <
              typename select_calculation_type
                  <
                      Point,
                      PointOfSegment,
                      CalculationType
                  >::type
          >
    {};

    template <typename Point, typename PointOfSegment>
    struct closest_point_result
    {
        typedef geometry::detail::closest_points::result
                <
                    typename calculation_type<Point, PointOfSegment>::type
                > type;
    };

    typedef typename Strategy::radius_type radius_type;

    template <typename Point1, typename Point2>
    typename closest_point_result<Point1, Point2>::type
    apply(Point1 const& p1, Point2 const& p2) const
    {
        typename closest_point_result<Point1, Point2>::type result;

        result.lon1 = get_as_radian<0>(p1);
        result.lat1 = get_as_radian<1>(p1);
        result.lon2 = get_as_radian<0>(p2);
        result.lat2 = get_as_radian<1>(p2);

        typedef typename distance::services::comparable_type<Strategy>::type
                comparable_strategy;

        result.distance = comparable_strategy().apply(p1,p2);

        return result;
    }

};

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <typename Point1, typename Point2>
struct default_strategy
    <
        point_tag, point_tag, Point1, Point2,
        spherical_equatorial_tag, spherical_equatorial_tag
    >
{
    typedef spherical<> type;
};

} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

} // namespace closest_points

namespace distance { namespace services {

//tags
template <typename CalculationType>
struct tag<closest_points::spherical<CalculationType> >
{
    typedef strategy_tag_distance_point_point type;
};

template
<
        typename CalculationType,
        typename Strategy
>
struct tag<closest_points::spherical<CalculationType, Strategy> >
{
    typedef strategy_tag_distance_point_point type;
};

//return types
template <typename CalculationType, typename P, typename PS>
struct return_type<closest_points::spherical<CalculationType>, P, PS>
        :   closest_points::spherical<CalculationType>
                          ::template closest_point_result<P, PS>
{};

/*
template <typename FormulaPolicy, typename P, typename PS>
struct return_type<closest_points::comparable::geographic<FormulaPolicy>, P, PS>
{
    typedef typename closest_points::geographic_cross_track<FormulaPolicy>
                          ::template point_segment_distance_closest_point<P, PS>
                          ::result_type type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename P,
        typename PS
>
struct return_type<closest_points::comparable::geographic_cross_track<FormulaPolicy,
                                                             Spheroid>, P, PS>
{
    typedef typename closest_points::geographic_cross_track<FormulaPolicy, Spheroid>
                     ::template point_segment_distance_closest_point<P, PS>
                     ::result_type type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType,
        typename P,
        typename PS
>
struct return_type<closest_points::comparable::geographic_cross_track
                   <
                        FormulaPolicy,
                        Spheroid,
                        CalculationType
                   >, P, PS>
{
    typedef typename closest_points::geographic_cross_track
                     <FormulaPolicy, Spheroid, CalculationType>
                    ::template point_segment_distance_closest_point<P, PS>
                    ::result_type type;
};
*/

//comparable types
template
<
        typename CalculationType,
        typename Strategy
>
struct comparable_type<closest_points::spherical<CalculationType, Strategy> >
{
    typedef closest_points::spherical<CalculationType, Strategy>  type;
};

template
<
        typename CalculationType,
        typename Strategy
>
struct get_comparable<closest_points::spherical<CalculationType, Strategy> >
{
    typedef typename comparable_type
        <
            closest_points::spherical<CalculationType, Strategy>
        >::type comparable_type;
public :
    static inline comparable_type
    apply(closest_points::spherical<CalculationType, Strategy> const& strategy)
    {
        return comparable_type(strategy);
    }
};

}}} // namespace strategy::distance::services

}} // namespace boost::geometry
#endif // BOOST_GEOMETRY_STRATEGIES_SPHERICAL_CLOSEST_POINTS_HPP
