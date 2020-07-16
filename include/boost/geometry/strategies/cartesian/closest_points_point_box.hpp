// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_CLOSEST_POINTS_POINT_BOX_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_CLOSEST_POINTS_POINT_BOX_HPP

#include <boost/geometry/algorithms/detail/closest_points/result.hpp>

#include <boost/geometry/formulas/cartesian.hpp>

#include <boost/geometry/strategies/closest_points.hpp>
#include <boost/geometry/strategies/cartesian/distance_pythagoras.hpp>
#include <boost/geometry/strategies/cartesian/distance_pythagoras_point_box.hpp>
#include <boost/geometry/strategies/cartesian/point_in_point.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace closest_points
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

template <size_t I>
struct compute_pythagoras_point_box
{
    template <typename Point, typename Box, typename T>
    static inline void apply(Point const& point, Box const& box, T& result)
    {
        typedef typename T::CoordinateType::type CT;
        CT const p_coord = boost::numeric_cast<CT>(geometry::get<I-1>(point));
        CT const b_min_coord =
            boost::numeric_cast<CT>(geometry::get<min_corner, I-1>(box));
        CT const b_max_coord =
            boost::numeric_cast<CT>(geometry::get<max_corner, I-1>(box));

        CT closest_point_coordinate = CT(0);

        if ( p_coord < b_min_coord )
        {
            CT diff = b_min_coord - p_coord;
            result.distance += diff * diff;
            closest_point_coordinate = b_min_coord;
        }
        else if ( p_coord > b_max_coord )
        {
            CT diff = p_coord - b_max_coord;
            result.distance += diff * diff;
            closest_point_coordinate = b_max_coord;
        }
        else
        {
            closest_point_coordinate = p_coord;
        }

        result.x2 = I==1 ? closest_point_coordinate : result.x2;
        result.y2 = I==2 ? closest_point_coordinate : result.y2;

        compute_pythagoras_point_box<I-1>::apply(point, box, result);
    }
};

template <>
struct compute_pythagoras_point_box<0>
{
    template <typename Point, typename Box, typename T>
    static inline void apply(Point const&, Box const&, T&)
    {
    }
};


} // namespace detail
#endif // DOXYGEN_NO_DETAIL

template
<
    typename CalculationType = void,
    typename Strategy = distance::pythagoras<CalculationType>
>
class cartesian_point_box
{
public :

    template <typename Point, typename Box>
    struct calculation_type
        : util::calculation_type::geometric::binary
          <
              Point,
              Box,
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

    /*!
    \brief applies the distance calculation using pythagoras
    \return the calculated distance (including taking the square root)
    \param point point
    \param box box
    */
    template <typename Point, typename Box>
    static inline typename closest_point_result<Point, Box>::type
    apply(Point const& point, Box const& box)
    {
        assert_dimension_equal<Point, Box>();

        //typename calculation_type<Point, Box>::type result(0);
        detail::result_type<typename calculation_type<Point, Box>::type> result;

        detail::compute_pythagoras_point_box
            <
                dimension<Point>::value
            >::apply(point, box, result);

        typename closest_point_result<Point, Box>::type
                 closest_point_result;

        closest_point_result.lon1 = get_as_radian<0>(point);
        closest_point_result.lat1 = get_as_radian<1>(point);
        closest_point_result.lon2 = result.x2;
        closest_point_result.lat2 = result.y2;
        closest_point_result.distance = result.distance;

        return closest_point_result;
    }
};


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <typename Point, typename PointOfBox>
struct default_strategy
    <
        point_tag, box_tag, Point, PointOfBox,
        cartesian_tag, cartesian_tag
    >
{
    typedef cartesian_point_box<> type;
};


template <typename PointOfBox, typename Point>
struct default_strategy
    <
        box_tag, point_tag, PointOfBox, Point,
        cartesian_tag, cartesian_tag
    >
{
    typedef typename default_strategy
        <
            point_tag, box_tag, Point, PointOfBox,
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
struct tag<closest_points::cartesian_point_box<CalculationType, Strategy> >
{
    typedef strategy_tag_distance_point_box type;
};


template <typename CalculationType, typename Strategy, typename P, typename PS>
struct return_type<closest_points::cartesian_point_box<CalculationType, Strategy>, P, PS>
    : closest_points::cartesian_point_box<CalculationType, Strategy>
                    ::template closest_point_result<P, PS>
{};



template <typename CalculationType, typename Strategy>
struct comparable_type<closest_points::cartesian_point_box<CalculationType, Strategy> >
{
    // Define a projected_point strategy with its underlying point-point-strategy
    // being comparable
    typedef closest_points::cartesian_point_box
        <
            CalculationType,
            typename comparable_type<Strategy>::type
        > type;
};


template <typename CalculationType, typename Strategy>
struct get_comparable<closest_points::cartesian_point_box<CalculationType, Strategy> >
{
    typedef typename comparable_type
        <
            closest_points::cartesian_point_box<CalculationType, Strategy>
        >::type comparable_type;
public :
    static inline comparable_type apply(closest_points::cartesian_point_box
                                        <
                                            CalculationType,
                                            Strategy
                                        > const& )
    {
        return comparable_type();
    }
};


template <typename CalculationType, typename Strategy, typename P, typename PS>
struct result_from_distance<closest_points::cartesian_point_box
                                            <
                                                CalculationType,
                                                Strategy
                                            >,
                            P, PS>
{
private :
    typedef typename return_type<closest_points::cartesian_point_box
                                                 <
                                                    CalculationType,
                                                    Strategy
                                                 >,
                                 P, PS>::type return_type;
public :
    template <typename T>
    static inline return_type apply(closest_points::cartesian_point_box
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
        strategy::closest_points::cartesian_point_box<CalculationType, Strategy>
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
    strategy::closest_points::cartesian_point_box<CalculationType, Strategy>
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
#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_CLOSEST_POINTS_POINT_BOX_HPP
