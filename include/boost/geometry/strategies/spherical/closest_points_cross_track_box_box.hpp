// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_SPHERICAL_CLOSEST_POINTS_CROSS_TRACK_BOX_BOX_HPP
#define BOOST_GEOMETRY_STRATEGIES_SPHERICAL_CLOSEST_POINTS_CROSS_TRACK_BOX_BOX_HPP

#include <boost/config.hpp>
#include <boost/concept_check.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_void.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/assert.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/algorithms/detail/assign_box_corners.hpp>
#include <boost/geometry/algorithms/detail/closest_points/result.hpp>

#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/strategies/concepts/distance_concept.hpp>
#include <boost/geometry/strategies/spherical/distance_cross_track.hpp>
#include <boost/geometry/strategies/spherical/distance_cross_track_point_box.hpp>

#include <boost/geometry/util/math.hpp>


namespace boost { namespace geometry
{

namespace strategy { namespace closest_points
{

class cross_track_box_box_generic
{
public:

    template <typename Point, typename Box, typename Result>
    static bool covered_by_box_with_info(Point const& box_corner,
                                         Box const& other_box,
                                         Result& result)
    {
        if (geometry::covered_by(box_corner, other_box))
        {
            result.lon1 = get_as_radian<0>(box_corner);
            result.lat1 = get_as_radian<1>(box_corner);
            result.lon2 = result.lon1;
            result.lat2 = result.lat1;
            result.distance = 0;
            return true;
        }
        return false;
    }

    template <typename Box1, typename Box2, typename Result>
    static bool box_box_overlap(Box1 const& box1,
                                Box2 const& box2,
                                Result& result)
    {
        //check first if boxes overlap
        typedef boost::array<typename point_type<Box1>::type, 4> array;

        array bp1;
        geometry::detail::assign_box_corners_oriented<true>(box1, bp1);

        for (typename array::const_iterator ait = bp1.begin();
             ait != bp1.end(); ait++)
        {
            if (covered_by_box_with_info(*ait, box2, result))
            {
                return true;
            }
        }

        array bp2;
        geometry::detail::assign_box_corners_oriented<true>(box2, bp2);

        for (typename array::const_iterator ait = bp2.begin();
             ait != bp2.end(); ait++)
        {
            if (covered_by_box_with_info(*ait, box1, result))
            {
                return true;
            }
        }
        return false;
    }
};

template
<
    typename CalculationType = void,
    typename Strategy = distance::haversine<double, CalculationType>
>
class cross_track_box_box
{
public:
    // point-point strategy getters
    struct closest_points_pp_strategy
    {
        typedef spherical<CalculationType, Strategy> type;
    };

    // point-segment strategy getters
    struct closest_points_ps_strategy
    {
        typedef cross_track<CalculationType, Strategy> type;
    };

    template <typename Box1, typename Box2>
    struct calculation_type
        : promote_floating_point
          <
              typename select_calculation_type
                  <
                      typename point_type<Box1>::type,
                      typename point_type<Box2>::type,
                      CalculationType
                  >::type
          >
    {};

    template <typename Box1, typename Box2>
    struct closest_point_result
    {
        typedef geometry::detail::closest_points::result
        <
        typename calculation_type<Box1, Box2>::type
        > type;
    };

    //constructors

    inline cross_track_box_box()
    {}

    explicit inline cross_track_box_box(typename Strategy::radius_type const& r)
        : m_strategy(r)
    {}

    inline cross_track_box_box(Strategy const& s)
        : m_strategy(s)
    {}

    template <typename Box1, typename Box2>
    typename closest_point_result<Box1, Box2>::type
    apply(Box1 const& box1, Box2 const& box2) const
    {
        typename closest_point_result<Box1, Box2>::type result;
        if (cross_track_box_box_generic::box_box_overlap(box1, box2, result))
        {
            return result;
        }

        return distance::details::cross_track_box_box_generic
               <
                    typename closest_point_result<Box1, Box2>::type
               >::apply(box1,
                        box2,
                        typename closest_points_pp_strategy::type(),
                        typename closest_points_ps_strategy::type(m_strategy));
    }

private :
    Strategy m_strategy;

};

namespace services {

template <typename Box1, typename Box2>
struct default_strategy
    <
        box_tag, box_tag, Box1, Box2,
        spherical_equatorial_tag, spherical_equatorial_tag
    >
{
    typedef closest_points::cross_track_box_box<> type;
};

}} // namespace closest_points::services

namespace distance { namespace services {

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

template <typename CalculationType, typename Strategy>
struct tag<closest_points::cross_track_box_box<CalculationType, Strategy> >
{
    typedef strategy_tag_distance_box_box type;
};

//return types

template <typename CalculationType, typename P, typename PS>
struct return_type<closest_points::cross_track_box_box<CalculationType>, P, PS>
        : closest_points::cross_track_box_box<CalculationType>
                        ::template closest_point_result<P, PS>
{};

template <typename CalculationType, typename Strategy, typename P, typename PS>
struct return_type<closest_points::cross_track_box_box<CalculationType, Strategy>, P, PS>
        : closest_points::cross_track_box_box<CalculationType, Strategy>
                        ::template closest_point_result<P, PS>
{};


template <typename CalculationType, typename Strategy>
struct comparable_type
       <
            closest_points::cross_track_box_box
            <
                CalculationType,
                Strategy
            >
       >
{
    typedef closest_points::cross_track_box_box
            <
                CalculationType, Strategy
            > type;
};


template <typename CalculationType, typename Strategy>
struct get_comparable
       <
            closest_points::cross_track_box_box
            <
                CalculationType,
                Strategy
            >
       >
{
public:
    static inline closest_points::cross_track_box_box
    <
        CalculationType,
        Strategy
    >
    apply(closest_points::cross_track_box_box
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
    typename Box1,
    typename Box2
>
struct result_from_distance
       <
            closest_points::cross_track_box_box
            <
                CalculationType,
                Strategy
            >,
            Box1,
            Box2
        >
{
private:
    typedef closest_points::cross_track_box_box
    <
        CalculationType,
        Strategy
    > this_strategy;

    typedef typename this_strategy::template calculation_type
                     <
                        Box1,
                        Box2
                     >::type calculation_type;

public:
    template <typename T>
    static inline calculation_type apply(this_strategy const& strategy,
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

template
<
    typename CalculationType,
    typename Strategy
>
struct swap<strategy::closest_points::cross_track_box_box<CalculationType, Strategy> >
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
struct result_set_unique_point
<
    strategy::closest_points::cross_track_box_box<CalculationType, Strategy>
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

#endif // BOOST_GEOMETRY_STRATEGIES_SPHERICAL_CLOSEST_POINTS_CROSS_TRACK_BOX_BOX_HPP
