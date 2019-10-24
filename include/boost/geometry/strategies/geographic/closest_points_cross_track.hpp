// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_CLOSEST_POINTS_CROSS_TRACK_HPP
#define BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_CLOSEST_POINTS_CROSS_TRACK_HPP

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
class geographic_cross_track
{
public :

    typedef within::spherical_point_point equals_point_point_strategy_type;

    typedef intersection::geographic_segments
        <
            FormulaPolicy,
            strategy::default_order<FormulaPolicy>::value,
            Spheroid,
            CalculationType
        > relate_segment_segment_strategy_type;

    inline relate_segment_segment_strategy_type get_relate_segment_segment_strategy() const
    {
        return relate_segment_segment_strategy_type(m_spheroid);
    }

    typedef within::geographic_winding
        <
            void, void, FormulaPolicy, Spheroid, CalculationType
        > point_in_geometry_strategy_type;

    inline point_in_geometry_strategy_type get_point_in_geometry_strategy() const
    {
        return point_in_geometry_strategy_type(m_spheroid);
    }


    inline distance::geographic_cross_track<FormulaPolicy, Spheroid, CalculationType>
    get_geographic_cross_track() const
    {
        return distance::geographic_cross_track<FormulaPolicy, Spheroid, CalculationType>
                (m_spheroid);
    }

    Spheroid get_spheroid() const
    {
        return m_spheroid;
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

    explicit geographic_cross_track(Spheroid const& spheroid = Spheroid())
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

    template <typename ResultType>
    ResultType
    apply(ResultType comparable_result) const
    {
        return comparable_result;
    }

    // points on a meridian not crossing poles
    template <typename CT>
    typename formula::point_segment_distance<CT, true, FormulaPolicy, false>::result_type
    vertical_or_meridian(CT const& lat1, CT const& lat2, CT const& lon) const
    {
        typedef typename formula::meridian_inverse
        <
            CT,
            strategy::default_order<FormulaPolicy>::value
        > meridian_inverse;

        CT distance = meridian_inverse::meridian_not_crossing_pole_dist(lat1, lat2,
                                                                 m_spheroid);
        typename formula::point_segment_distance
                <
                    CT,
                    true,
                    FormulaPolicy,
                    false
                >::result_type res;

        res.distance = distance;
        res.lon1 = lon;
        res.lat1 = lat1;
        res.lon2 = lon;
        res.lat2 = lat2;

        return res;

    }

    Spheroid m_spheroid;
};

namespace comparable {

template
<
    typename FormulaPolicy = strategy::andoyer,
    typename Spheroid = srs::spheroid<double>,
    typename CalculationType = void
>
class geographic_cross_track
        : public closest_points::geographic_cross_track
            <
                FormulaPolicy,
                Spheroid,
                CalculationType
            >
{
public :
    explicit geographic_cross_track(Spheroid const& spheroid = Spheroid())
        :
        closest_points::geographic_cross_track<
                FormulaPolicy,
                Spheroid,
                CalculationType
            >(spheroid)
        {}
};

}

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
    typedef geographic_cross_track<> type;
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
struct tag<closest_points::geographic_cross_track<FormulaPolicy> >
{
    typedef strategy_tag_distance_point_segment type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid
>
struct tag<closest_points::geographic_cross_track<FormulaPolicy, Spheroid> >
{
    typedef strategy_tag_distance_point_segment type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType
>
struct tag<closest_points::geographic_cross_track<FormulaPolicy,
                                                     Spheroid,
                                                     CalculationType> >
{
    typedef strategy_tag_distance_point_segment type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType
>
struct tag<closest_points::comparable::geographic_cross_track<FormulaPolicy,
                                                                 Spheroid,
                                                                 CalculationType> >
{
    typedef strategy_tag_distance_point_segment type;
};


//return types
template <typename FormulaPolicy, typename P, typename PS>
struct return_type<closest_points::geographic_cross_track<FormulaPolicy>, P, PS>
{
    typedef typename closest_points::geographic_cross_track<FormulaPolicy>
                          ::template point_segment_distance_closest_point<P, PS>
                          ::result_type type;
};

template <typename FormulaPolicy, typename P, typename PS>
struct return_type<closest_points::comparable::geographic_cross_track<FormulaPolicy>, P, PS>
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

//comparable types
template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType
>
struct comparable_type<closest_points::geographic_cross_track
                         <FormulaPolicy, Spheroid, CalculationType> >
{
    typedef closest_points::comparable::geographic_cross_track
        <
            FormulaPolicy, Spheroid, CalculationType
        >  type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType
>
struct comparable_type<closest_points::comparable::geographic_cross_track
                         <FormulaPolicy, Spheroid, CalculationType> >
{
    typedef closest_points::comparable::geographic_cross_track
        <
            FormulaPolicy, Spheroid, CalculationType
        >  type;
};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType
>
struct get_comparable
        <
            closest_points::geographic_cross_track
                            <
                                FormulaPolicy,
                                Spheroid,
                                CalculationType
                            >
        >
{
    typedef typename comparable_type
        <
            closest_points::geographic_cross_track
                    <
                        FormulaPolicy,
                        Spheroid,
                        CalculationType
                    >
        >::type comparable_type;
public :
    static inline comparable_type
    apply(closest_points::geographic_cross_track
                    <FormulaPolicy, Spheroid, CalculationType> const& strategy)
    {
        return comparable_type(strategy.get_spheroid());
    }
};

template
<
        typename FormulaPolicy,
        typename Spheroid,
        typename CalculationType
>
struct get_comparable
        <
            closest_points::comparable::geographic_cross_track
                            <
                                FormulaPolicy,
                                Spheroid,
                                CalculationType
                            >
        >
{
    typedef typename closest_points::comparable::geographic_cross_track
    <
        FormulaPolicy,
        Spheroid,
        CalculationType
    > comparable_type;
public :
    static inline comparable_type
    apply(closest_points::comparable::geographic_cross_track
                    <FormulaPolicy, Spheroid, CalculationType> const& strategy)
    {
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
struct result_from_distance<closest_points::geographic_cross_track<FormulaPolicy>, P, PS>
{
private :
    typedef typename closest_points::geographic_cross_track
        <
            FormulaPolicy
        >::template return_type<P, PS>::type return_type;
public :
    template <typename T>
    static inline return_type
    apply(closest_points::geographic_cross_track<FormulaPolicy> const& ,
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
struct result_from_distance<closest_points::geographic_cross_track
                  <FormulaPolicy, Spheroid, CalculationType>, P, PS>
{
private :
    typedef typename closest_points::geographic_cross_track
        <
            FormulaPolicy, Spheroid, CalculationType
        >::template return_type<P, PS>::type return_type;
public :
    template <typename T>
    static inline return_type
    apply(closest_points::geographic_cross_track<FormulaPolicy, Spheroid, CalculationType> const& ,
          T const& distance)
    {
        return distance;
    }
};

}}} // namespace strategy::distance::services

}} // namespace boost::geometry
#endif // BOOST_GEOMETRY_STRATEGIES_GEOGRAPHIC_CLOSEST_POINTS_CROSS_TRACK_HPP
