// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_CLOSEST_POINTS_POINT_SEGMENT_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_CLOSEST_POINTS_POINT_SEGMENT_HPP

#include <boost/geometry/algorithms/detail/closest_points/result.hpp>

#include <boost/geometry/strategies/closest_points.hpp>
#include <boost/geometry/strategies/cartesian/comparable_point_segment_distance.hpp>
#include <boost/geometry/strategies/cartesian/distance_pythagoras.hpp>
#include <boost/geometry/strategies/cartesian/point_in_point.hpp>
#include <boost/geometry/strategies/cartesian/intersection.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace closest_points
{

template
<
    typename CalculationType = void,
    typename Strategy = distance::pythagoras<CalculationType>
>
class cartesian_point_segment
{
public :

    typedef within::cartesian_point_point equals_point_point_strategy_type;

    typedef intersection::cartesian_segments
        <
            CalculationType
        > relate_segment_segment_strategy_type;

    static inline relate_segment_segment_strategy_type get_relate_segment_segment_strategy()
    {
        return relate_segment_segment_strategy_type();
    }

    typedef within::cartesian_winding
        <
            void, void, CalculationType
        > point_in_geometry_strategy_type;

    static inline point_in_geometry_strategy_type get_point_in_geometry_strategy()
    {
        return point_in_geometry_strategy_type();
    }

    // Integer coordinates can still result in FP distances.
    // There is a division, which must be represented in FP.
    // So promote.
    template <typename Point, typename PointOfSegment>
    struct calculation_type
        : promote_floating_point
          <
              typename strategy::distance::services::return_type
                  <
                      Strategy,
                      Point,
                      PointOfSegment
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

    template <typename Point, typename PointOfSegment>
    inline typename closest_point_result<Point, PointOfSegment>::type
    apply(Point const& p, PointOfSegment const& p1, PointOfSegment const& p2) const
    {
        assert_dimension_equal<Point, PointOfSegment>();

        typename closest_point_result<Point, PointOfSegment>::type
                 cp_result;

        typedef typename calculation_type<Point, PointOfSegment>::type CT;

        typedef comparable_cartesian_point_segment_distance
            <
                CT,
                true
            > cartesian_ps_distance;

        typename cartesian_ps_distance::result_type result
            = cartesian_ps_distance::apply(p, p1, p2,
                                           distance::services::get_comparable<Strategy>
                                           ::apply(Strategy()));

        cp_result.lon1 = get<0>(p);
        cp_result.lat1 = get<1>(p);
        cp_result.lon2 = result.x2;
        cp_result.lat2 = result.y2;
        cp_result.distance = result.distance;

        return cp_result;
    }

    template <typename ResultType>
    ResultType apply(ResultType comparable_result) const
    {
        return comparable_result;
    }

    template <typename CT>
    geometry::detail::closest_points::result<CT>
    vertical_or_meridian(CT const& lat1, CT const& lat2, CT const& lon) const
    {
        geometry::detail::closest_points::result<CT> res;

        res.distance = math::abs(lat1 - lat2);
        res.lon1 = lon;
        res.lat1 = lat1;
        res.lon2 = lon;
        res.lat2 = lat2;

        return res;
    }
};


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <typename Point, typename PointOfSegment>
struct default_strategy
    <
        point_tag, segment_tag, Point, PointOfSegment,
        cartesian_tag, cartesian_tag
    >
{
    typedef cartesian_point_segment<> type;
};


template <typename PointOfSegment, typename Point>
struct default_strategy
    <
        segment_tag, point_tag, PointOfSegment, Point,
        cartesian_tag, cartesian_tag
    >
{
    typedef typename default_strategy
        <
            point_tag, segment_tag, Point, PointOfSegment,
            cartesian_tag, cartesian_tag
        >::type type;
};

} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

} // namespace closest_points

namespace distance {

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <typename CalculationType, typename Strategy>
struct tag<closest_points::cartesian_point_segment<CalculationType, Strategy> >
{
    typedef strategy_tag_distance_point_segment type;
};


template <typename CalculationType, typename Strategy, typename P, typename PS>
struct return_type<closest_points::cartesian_point_segment<CalculationType, Strategy>, P, PS>
    : closest_points::cartesian_point_segment<CalculationType, Strategy>
                    ::template closest_point_result<P, PS>
{};



template <typename CalculationType, typename Strategy>
struct comparable_type<closest_points::cartesian_point_segment<CalculationType, Strategy> >
{
    // Define a cartesian_point_segment strategy with its underlying point-point-strategy
    // being comparable
    typedef closest_points::cartesian_point_segment
        <
            CalculationType,
            typename comparable_type<Strategy>::type
        > type;
};


template <typename CalculationType, typename Strategy>
struct get_comparable<closest_points::cartesian_point_segment<CalculationType, Strategy> >
{
    typedef typename comparable_type
        <
            closest_points::cartesian_point_segment<CalculationType, Strategy>
        >::type comparable_type;
public :
    static inline comparable_type apply(closest_points::cartesian_point_segment
                                        <
                                            CalculationType,
                                            Strategy
                                        > const& )
    {
        return comparable_type();
    }
};


template <typename CalculationType, typename Strategy, typename P, typename PS>
struct result_from_distance
<
    closest_points::cartesian_point_segment
        <
            CalculationType,
            Strategy
        >,
    P, PS
>
{
private :
    typedef typename return_type<closest_points::cartesian_point_segment
                                                 <
                                                    CalculationType,
                                                    Strategy
                                                 >,
                                 P, PS>::type return_type;
public :
    template <typename T>
    static inline return_type apply(closest_points::cartesian_point_segment
                                                    <
                                                        CalculationType,
                                                        Strategy
                                                    > const& ,
                                    T const& value)
    {
        Strategy s;
        return result_from_distance<Strategy, P, PS>::apply(s, value);
    }
};

template
<
    typename CalculationType,
    typename Strategy
>
struct swap_result_points
    <
        strategy::closest_points::cartesian_point_segment<CalculationType, Strategy>
    >
{
    template <typename Result>
    static inline void apply(Result& res)
    {
        res.swap_points();
    }
};

template
<
    typename CalculationType,
    typename Strategy
>
struct result_set_unique_point
<
    strategy::closest_points::cartesian_point_segment<CalculationType, Strategy>
>
{
    template <typename T, typename Point>
    static inline void apply(T& result, Point const& point)
    {
        result.set_unique_point(point);
    }
};

} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

}} // namespace strategy::distance

}} // namespace boost::geometry
#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_CLOSEST_POINTS_POINT_SEGMENT_HPP
