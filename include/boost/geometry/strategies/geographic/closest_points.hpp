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

    inline distance::geographic_cross_track<FormulaPolicy, Spheroid, CalculationType>
    get_geographic_cross_track() const
    {
        return distance::geographic_cross_track<FormulaPolicy, Spheroid, CalculationType>
                (m_spheroid);
    }

    template <typename Point, typename PointOfSegment>
    struct return_type
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
    struct point_segment_distance_closest_point
        : formula::point_segment_distance
            <
                typename return_type<Point, PointOfSegment>::type,
                true,
                FormulaPolicy,
                false
            >
    {};

    explicit geographic_closest_points(Spheroid const& spheroid = Spheroid())
        : m_spheroid(spheroid)
    {}

    template <typename Point, typename PointOfSegment>
    typename point_segment_distance_closest_point<Point, PointOfSegment>::result_type
    apply(Point const& p,
          PointOfSegment const& sp1,
          PointOfSegment const& sp2) const
    {
        return point_segment_distance_closest_point<Point, PointOfSegment>
                      ::apply(get_as_radian<0>(sp1), get_as_radian<1>(sp1),
                              get_as_radian<0>(sp2), get_as_radian<1>(sp2),
                              get_as_radian<0>(p), get_as_radian<1>(p),
                              this->m_spheroid);
    }

    Spheroid m_spheroid;
};

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

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

} // namespace closest_points

namespace distance { namespace services {

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
struct tag<closest_points::geographic_closest_points<FormulaPolicy,
                                                     Spheroid,
                                                     CalculationType> >
{
    typedef strategy_tag_distance_point_segment type;
};


//return types
template <typename FormulaPolicy, typename P, typename PS>
struct return_type<closest_points::geographic_closest_points<FormulaPolicy>, P, PS>
{
    typedef typename closest_points::geographic_closest_points<FormulaPolicy>
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
struct return_type<closest_points::geographic_closest_points<FormulaPolicy,
                                                             Spheroid>, P, PS>
{
    typedef typename closest_points::geographic_closest_points<FormulaPolicy, Spheroid>
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
struct return_type<closest_points::geographic_closest_points
                   <
                        FormulaPolicy,
                        Spheroid,
                        CalculationType
                   >, P, PS>
{
    typedef typename closest_points::geographic_closest_points
                     <FormulaPolicy, Spheroid, CalculationType>
                    ::template point_segment_distance_closest_point<P, PS>
                    ::result_type type;
};

//comparable types
template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType
>
struct comparable_type<closest_points::geographic_closest_points
                         <FormulaPolicy, Spheroid, CalculationType> >
{
    typedef closest_points::geographic_closest_points
        <
            FormulaPolicy, Spheroid, CalculationType
        >  type;
};

/*
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
*/

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType
>
struct get_comparable<closest_points::geographic_closest_points<FormulaPolicy, Spheroid, CalculationType> >
{
public :
    static inline closest_points::geographic_closest_points
                    <FormulaPolicy, Spheroid, CalculationType>
    apply(closest_points::geographic_closest_points
                    <FormulaPolicy, Spheroid, CalculationType> const& strategy)
    {
        //return strategy.get_geographic_cross_track();
        return strategy;
    }
};

/*
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
};*/


template
<
    typename FormulaPolicy,
    typename P,
    typename PS
>
struct result_from_distance<closest_points::geographic_closest_points<FormulaPolicy>, P, PS>
{
private :
    typedef typename closest_points::geographic_closest_points
        <
            FormulaPolicy
        >::template return_type<P, PS>::type return_type;
public :
    template <typename T>
    static inline return_type
    apply(closest_points::geographic_closest_points<FormulaPolicy> const& ,
          T const& distance)
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
struct result_from_distance<closest_points::geographic_closest_points
                  <FormulaPolicy, Spheroid, CalculationType>, P, PS>
{
private :
    typedef typename closest_points::geographic_closest_points
        <
            FormulaPolicy, Spheroid, CalculationType
        >::template return_type<P, PS>::type return_type;
public :
    template <typename T>
    static inline return_type
    apply(closest_points::geographic_closest_points<FormulaPolicy, Spheroid, CalculationType> const& ,
          T const& distance)
    {
        return distance;
    }
};

}}} // namespace strategy::distance::services

}} // namespace boost::geometry
#endif // BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_CLOSEST_POINTS_HPP
