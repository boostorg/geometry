// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_CLOSEST_POINTS_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_CLOSEST_POINTS_HPP

#include <boost/geometry/algorithms/detail/closest_points/result.hpp>

#include <boost/geometry/strategies/closest_points.hpp>
#include <boost/geometry/strategies/cartesian/distance_pythagoras.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace closest_points
{

template
<
    typename CalculationType = void,
    typename Strategy = distance::pythagoras<CalculationType>
>
class cartesian
{
public :

    template <typename P1, typename P2>
    struct calculation_type
        : util::calculation_type::geometric::binary
          <
              P1,
              P2,
              CalculationType,
              double,
              double // promote integer to double
          >
    {};

    template <typename P1, typename P2>
    struct closest_point_result
    {
        typedef geometry::detail::closest_points::result
                <
                    typename calculation_type<P1, P2>::type
                > type;
    };

    template <typename Point1, typename Point2>
    typename closest_point_result<Point1, Point2>::type
    apply(Point1 const& p1, Point2 const& p2) const
    {
        typename closest_point_result<Point1, Point2>::type result;

        result.lon1 = get<0>(p1);
        result.lat1 = get<1>(p1);
        result.lon2 = get<0>(p2);
        result.lat2 = get<1>(p2);

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
        cartesian_tag, cartesian_tag
    >
{
    typedef cartesian<> type;
};

} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

} // namespace closest_points

namespace distance { namespace services {

//tags
template <typename CalculationType>
struct tag<closest_points::cartesian<CalculationType> >
{
    typedef strategy_tag_distance_point_point type;
};

template
<
        typename CalculationType,
        typename Strategy
>
struct tag<closest_points::cartesian<CalculationType, Strategy> >
{
    typedef strategy_tag_distance_point_point type;
};

//return types
template <typename CalculationType, typename P, typename PS>
struct return_type<closest_points::cartesian<CalculationType>, P, PS>
        :   closest_points::cartesian<CalculationType>
                          ::template closest_point_result<P, PS>
{};

//comparable types
template
<
        typename CalculationType,
        typename Strategy
>
struct comparable_type<closest_points::cartesian<CalculationType, Strategy> >
{
    typedef closest_points::cartesian<CalculationType, Strategy>  type;
};

template
<
        typename CalculationType,
        typename Strategy
>
struct get_comparable<closest_points::cartesian<CalculationType, Strategy> >
{
    typedef typename comparable_type
        <
            closest_points::cartesian<CalculationType, Strategy>
        >::type comparable_type;
public :
    static inline comparable_type
    apply(closest_points::cartesian<CalculationType, Strategy> const& strategy)
    {
        return comparable_type(strategy);
    }
};

template
<
    typename CalculationType,
    typename Strategy
>
struct swap_result_points
    <
        strategy::closest_points::cartesian<CalculationType, Strategy>
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
    strategy::closest_points::cartesian<CalculationType, Strategy>
>
{
    template <typename T, typename Point>
    static inline void apply(T& result, Point const& point)
    {
        result.set_unique_point(point);
    }
};

}}} // namespace strategy::distance::services

}} // namespace boost::geometry
#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_CLOSEST_POINTS_HPP
