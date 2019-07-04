// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_CLOSEST_POINTS_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_CLOSEST_POINTS_HPP

#include <boost/geometry/strategies/closest_points.hpp>
#include <boost/geometry/strategies/geographic/distance_cross_track.hpp>

namespace boost { namespace geometry
{

namespace strategy { namespace closest_points
{


template
<
    typename FormulaPolicy = strategy::andoyer,
    typename Spheroid = srs::spheroid<double>,
    typename CalculationType = void
>
class geographic_closest_points
    : public distance::detail::geographic_cross_track
        <
            FormulaPolicy,
            Spheroid,
            CalculationType,
            false,
            true
        >
{
public :
    explicit geographic_closest_points(Spheroid const& spheroid = Spheroid())
        :
        distance::detail::geographic_cross_track<
                FormulaPolicy,
                Spheroid,
                CalculationType,
                false,
                true
            >(spheroid)
        {}
};

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{
/*
//tags
template <typename FormulaPolicy>
struct tag<geographic_closest_points<FormulaPolicy> >
{
    typedef strategy_tag_distance_point_segment type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid
>
struct tag<geographic_closest_points<FormulaPolicy, Spheroid> >
{
    typedef strategy_tag_distance_point_segment type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType
>
struct tag<geographic_closest_points<FormulaPolicy, Spheroid, CalculationType> >
{
    typedef strategy_tag_distance_point_segment type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType,
        bool Bisection
>
struct tag<detail::geographic_closest_points<FormulaPolicy, Spheroid, CalculationType, Bisection> >
{
    typedef strategy_tag_distance_point_segment type;
};

//return types
template <typename FormulaPolicy, typename P, typename PS>
struct return_type<geographic_closest_points<FormulaPolicy>, P, PS>
    : geographic_closest_points<FormulaPolicy>::template return_type<P, PS>
{};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename P,
        typename PS
>
struct return_type<geographic_closest_points<FormulaPolicy, Spheroid>, P, PS>
    : geographic_closest_points<FormulaPolicy, Spheroid>::template return_type<P, PS>
{};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType,
        typename P,
        typename PS
>
struct return_type<geographic_closest_points<FormulaPolicy, Spheroid, CalculationType>, P, PS>
    : geographic_closest_points<FormulaPolicy, Spheroid, CalculationType>::template return_type<P, PS>
{};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType,
        bool Bisection,
        typename P,
        typename PS
>
struct return_type<detail::geographic_closest_points<FormulaPolicy, Spheroid, CalculationType, Bisection>, P, PS>
    : detail::geographic_closest_points<FormulaPolicy, Spheroid, CalculationType, Bisection>::template return_type<P, PS>
{};

//comparable types
template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType
>
struct comparable_type<geographic_closest_points<FormulaPolicy, Spheroid, CalculationType> >
{
    typedef geographic_closest_points
        <
            FormulaPolicy, Spheroid, CalculationType
        >  type;
};


template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType,
        bool Bisection
>
struct comparable_type<detail::geographic_cross_track<FormulaPolicy, Spheroid, CalculationType, Bisection> >
{
    typedef detail::geographic_cross_track
        <
            FormulaPolicy, Spheroid, CalculationType, Bisection
        >  type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType
>
struct get_comparable<geographic_closest_points<FormulaPolicy, Spheroid, CalculationType> >
{
public :
    static inline geographic_closest_points<FormulaPolicy, Spheroid, CalculationType>
    apply(geographic_closest_points<FormulaPolicy, Spheroid, CalculationType> const& strategy)
    {
        return strategy;
    }
};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType,
        bool Bisection
>
struct get_comparable<detail::geographic_cross_track<FormulaPolicy, Spheroid, CalculationType, Bisection> >
{
public :
    static inline detail::geographic_cross_track<FormulaPolicy, Spheroid, CalculationType, Bisection>
    apply(detail::geographic_cross_track<FormulaPolicy, Spheroid, CalculationType, Bisection> const& strategy)
    {
        return strategy;
    }
};


template
<
    typename FormulaPolicy,
    typename P,
    typename PS
>
struct result_from_distance<geographic_closest_points<FormulaPolicy>, P, PS>
{
private :
    typedef typename geographic_closest_points
        <
            FormulaPolicy
        >::template return_type<P, PS>::type return_type;
public :
    template <typename T>
    static inline return_type
    apply(geographic_closest_points<FormulaPolicy> const& , T const& distance)
    {
        return distance;
    }
};

template
<
    typename FormulaPolicy,
    typename Spheroid,
    typename CalculationType,
    typename P,
    typename PS
>
struct result_from_distance<geographic_closest_points<FormulaPolicy, Spheroid, CalculationType>, P, PS>
{
private :
    typedef typename geographic_closest_points
        <
            FormulaPolicy, Spheroid, CalculationType
        >::template return_type<P, PS>::type return_type;
public :
    template <typename T>
    static inline return_type
    apply(geographic_closest_points<FormulaPolicy, Spheroid, CalculationType> const& , T const& distance)
    {
        return distance;
    }
};
*/

template <typename Point, typename PointOfSegment>
struct default_strategy
    <
        point_tag, segment_tag, Point, PointOfSegment,
        geographic_tag, geographic_tag
    >
{
    typedef geographic_closest_points<> type;
};


template <typename PointOfSegment, typename Point>
struct default_strategy
    <
        segment_tag, point_tag, PointOfSegment, Point,
        geographic_tag, geographic_tag
    >
{
    typedef typename default_strategy
        <
            point_tag, segment_tag, Point, PointOfSegment,
            geographic_tag, geographic_tag
        >::type type;
};

} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

}} // namespace strategy::closest_points

}} // namespace boost::geometry
#endif // BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_CLOSEST_POINTS_HPP
