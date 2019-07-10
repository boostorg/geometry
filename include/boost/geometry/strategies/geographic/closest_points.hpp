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
{
public :

    template <typename Point, typename PointOfSegment>
    struct return_type
        : formula::point_segment_distance<double>::result_type
    {};

    explicit geographic_closest_points(Spheroid const& spheroid = Spheroid())
        : m_spheroid(spheroid)
    {}

    template <typename Point, typename PointOfSegment>
    formula::point_segment_distance<double, true>::result_type
    apply(Point const& p,
          PointOfSegment const& sp1,
          PointOfSegment const& sp2) const
    {
        //result_distance_point_segment<typename return_type<Point, PointOfSegment>::type>
        formula::point_segment_distance<double, true>::result_type res =
                formula::point_segment_distance<double, true>
                                       ::apply(get_as_radian<0>(sp1), get_as_radian<1>(sp1),
                                               get_as_radian<0>(sp2), get_as_radian<1>(sp2),
                                               get_as_radian<0>(p), get_as_radian<1>(p),
                                               this->m_spheroid);
        return res;
        //Point point;
        /*
        set_from_radian<0,0>(s, get_as_radian<0>(p));
        set_from_radian<0,1>(s, get_as_radian<1>(p));
        set_from_radian<1,0>(s, res.closest_point_lon);
        set_from_radian<1,1>(s, res.closest_point_lat);
        */
    }

    Spheroid m_spheroid;

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
*/



/*
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

namespace strategy { namespace distance {
namespace services {

//tags
template <typename FormulaPolicy>
struct tag<closest_points::geographic_closest_points<FormulaPolicy> >
{
    typedef strategy_tag_distance_point_segment type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid
>
struct tag<closest_points::geographic_closest_points<FormulaPolicy, Spheroid> >
{
    typedef strategy_tag_distance_point_segment type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType
>
struct tag<closest_points::geographic_closest_points<FormulaPolicy, Spheroid, CalculationType> >
{
    typedef strategy_tag_distance_point_segment type;
};


//return types
template <typename FormulaPolicy, typename P, typename PS>
struct return_type<closest_points::geographic_closest_points<FormulaPolicy>, P, PS>
{
    typedef formula::point_segment_distance<double, true>::result_type type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename P,
        typename PS
>
struct return_type<closest_points::geographic_closest_points<FormulaPolicy, Spheroid>, P, PS>
{
    typedef formula::point_segment_distance<double, true>::result_type type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType,
        typename P,
        typename PS
>
struct return_type<closest_points::geographic_closest_points<FormulaPolicy, Spheroid, CalculationType>, P, PS>
{
    typedef formula::point_segment_distance<double, true>::result_type type;
};

}}} // namespace strategy::distance::services

}} // namespace boost::geometry
#endif // BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_CLOSEST_POINTS_HPP
