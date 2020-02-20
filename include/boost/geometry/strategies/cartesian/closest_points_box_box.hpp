// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_CLOSEST_POINTS_BOX_BOX_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_CLOSEST_POINTS_BOX_BOX_HPP

#include <boost/geometry/algorithms/detail/closest_points/result.hpp>

#include <boost/geometry/formulas/cartesian.hpp>

#include <boost/geometry/strategies/closest_points.hpp>
#include <boost/geometry/strategies/cartesian/distance_pythagoras.hpp>
#include <boost/geometry/strategies/cartesian/distance_pythagoras_point_box.hpp>
#include <boost/geometry/strategies/cartesian/point_in_point.hpp>
#include <boost/geometry/strategies/cartesian/intersection.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace closest_points
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

template <typename CT>
struct result_type
{
    result_type()
        : distance(0)
        , x1(0)
        , y1(0)
        , x2(0)
        , y2(0)
    {}

    struct CoordinateType
    {
        typedef CT type;
    };

    CT distance;
    CT x1;
    CT y1;
    CT x2;
    CT y2;
};

template <size_t I>
struct compute_pythagoras_box_box
{
    template <typename Box1, typename Box2, typename T>
    static inline void apply(Box1 const& box1, Box2 const& box2, T& result)
    {
        typedef typename T::CoordinateType::type CT;
        CT const b1_min_coord =
            boost::numeric_cast<CT>(geometry::get<min_corner, I-1>(box1));
        CT const b1_max_coord =
            boost::numeric_cast<CT>(geometry::get<max_corner, I-1>(box1));

        CT const b2_min_coord =
            boost::numeric_cast<CT>(geometry::get<min_corner, I-1>(box2));
        CT const b2_max_coord =
            boost::numeric_cast<CT>(geometry::get<max_corner, I-1>(box2));

        CT closest_point1_coordinate = CT(0);
        CT closest_point2_coordinate = CT(0);

        if ( b1_max_coord < b2_min_coord )
        {
            CT diff = b2_min_coord - b1_max_coord;
            result.distance += diff * diff;
            closest_point1_coordinate = b1_max_coord;
            closest_point2_coordinate = b2_min_coord;
        }
        else if ( b1_min_coord > b2_max_coord )
        {
            CT diff = b1_min_coord - b2_max_coord;
            result.distance += diff * diff;
            closest_point1_coordinate = b1_min_coord;
            closest_point2_coordinate = b2_max_coord;
        }
        else
        {
            CT max_min_coord = (std::max)(b1_min_coord, b2_min_coord);
            closest_point1_coordinate = max_min_coord;
            closest_point2_coordinate = max_min_coord;
        }

        result.x1 = I==1 ? closest_point1_coordinate : result.x1;
        result.y1 = I==2 ? closest_point1_coordinate : result.y1;

        result.x2 = I==1 ? closest_point2_coordinate : result.x2;
        result.y2 = I==2 ? closest_point2_coordinate : result.y2;

        compute_pythagoras_box_box<I-1>::apply(box1, box2, result);
    }
};

template <>
struct compute_pythagoras_box_box<0>
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
class cartesian_box_box
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
    template <typename Box1, typename Box2>
    static inline typename closest_point_result<Box1, Box2>::type
    apply(Box1 const& point, Box2 const& box)
    {
        assert_dimension_equal<Box1, Box2>();

        //typename calculation_type<Point, Box>::type result(0);
        detail::result_type<typename calculation_type<Box1, Box2>::type> result;

        detail::compute_pythagoras_box_box
            <
                dimension<Box1>::value
            >::apply(point, box, result);

        typename closest_point_result<Box1, Box2>::type
                 closest_point_result;

        closest_point_result.lon1 = result.x1;
        closest_point_result.lat1 = result.y1;
        closest_point_result.lon2 = result.x2;
        closest_point_result.lat2 = result.y2;
        closest_point_result.distance = result.distance;

        return closest_point_result;
    }
};


#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <typename PointOfBox1, typename PointOfBox2>
struct default_strategy
    <
        box_tag, box_tag, PointOfBox1, PointOfBox2,
        cartesian_tag, cartesian_tag
    >
{
    typedef cartesian_box_box<> type;
};

} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

} // namespace closest_points

namespace distance {

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <typename CalculationType, typename Strategy>
struct tag<closest_points::cartesian_box_box<CalculationType, Strategy> >
{
    typedef strategy_tag_distance_box_box type;
};


template <typename CalculationType, typename Strategy, typename Box1, typename Box2>
struct return_type<closest_points::cartesian_box_box<CalculationType, Strategy>,
                   Box1, Box2>
    : closest_points::cartesian_box_box<CalculationType, Strategy>
                    ::template closest_point_result<Box1, Box2>
{};



template <typename CalculationType, typename Strategy>
struct comparable_type<closest_points::cartesian_box_box<CalculationType, Strategy> >
{
    // Define a projected_point strategy with its underlying point-point-strategy
    // being comparable
    typedef closest_points::cartesian_box_box
        <
            CalculationType,
            typename comparable_type<Strategy>::type
        > type;
};


template <typename CalculationType, typename Strategy>
struct get_comparable<closest_points::cartesian_box_box<CalculationType, Strategy> >
{
    typedef typename comparable_type
        <
            closest_points::cartesian_box_box<CalculationType, Strategy>
        >::type comparable_type;
public :
    static inline comparable_type apply(closest_points::cartesian_box_box
                                        <
                                            CalculationType,
                                            Strategy
                                        > const& )
    {
        return comparable_type();
    }
};


template <typename CalculationType, typename Strategy, typename P, typename PS>
struct result_from_distance<closest_points::cartesian_box_box
                                            <
                                                CalculationType,
                                                Strategy
                                            >,
                            P, PS>
{
private :
    typedef typename return_type<closest_points::cartesian_box_box
                                                 <
                                                    CalculationType,
                                                    Strategy
                                                 >,
                                 P, PS>::type return_type;
public :
    template <typename T>
    static inline return_type apply(closest_points::cartesian_box_box
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
struct swap<strategy::closest_points::cartesian_box_box<CalculationType, Strategy> >
{
    template <typename Result>
    static inline void apply(Result& res)
    {
        res.swap();
    }
};

template
<
    typename CalculationType,
    typename Strategy
>
struct result_init
<
    strategy::closest_points::cartesian_box_box<CalculationType, Strategy>
>
{
    template <typename T, typename Point>
    static inline void apply(T& result, Point const& point)
    {
        result.init(point);
    }
};

} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

}} // namespace strategy::distance

}} // namespace boost::geometry
#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_CLOSEST_POINTS_BOX_BOX_HPP
