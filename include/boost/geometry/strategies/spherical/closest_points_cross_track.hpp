// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_SPHERICAL_CLOSEST_POINTS_CROSS_TRACK_HPP
#define BOOST_GEOMETRY_STRATEGIES_SPHERICAL_CLOSEST_POINTS_CROSS_TRACK_HPP

#include <boost/geometry/algorithms/detail/closest_points/result.hpp>

#include <boost/geometry/strategies/closest_points.hpp>
#include <boost/geometry/strategies/geographic/distance_cross_track.hpp>
#include <boost/geometry/strategies/spherical/distance_cross_track.hpp>

#include <boost/geometry/formulas/spherical.hpp>

#include <iostream>

namespace boost { namespace geometry
{

namespace strategy { namespace closest_points
{

template
<
        typename CalculationType = void,
        typename Strategy = distance::haversine<double, CalculationType>
>
class cross_track
{
public :

    //TODO: not sure if all those are needed
    typedef within::spherical_point_point equals_point_point_strategy_type;

    typedef intersection::spherical_segments
        <
            CalculationType
        > relate_segment_segment_strategy_type;

    static inline relate_segment_segment_strategy_type get_relate_segment_segment_strategy()
    {
        return relate_segment_segment_strategy_type();
    }

    typedef within::spherical_winding
        <
            void, void, CalculationType
        > point_in_geometry_strategy_type;

    static inline point_in_geometry_strategy_type get_point_in_geometry_strategy()
    {
        return point_in_geometry_strategy_type();
    }
    //

    inline distance::comparable::cross_track<CalculationType, Strategy>
    get_cross_track() const
    {
        return distance::comparable::cross_track<CalculationType, Strategy>();
    }

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

    inline cross_track()
    {}

    explicit inline cross_track(typename Strategy::radius_type const& r)
        : m_strategy(r)
    {}

    explicit inline cross_track(Strategy const& s)
        : m_strategy(s)
    {}

    template <typename Point, typename PointOfSegment>
    typename closest_point_result<Point, PointOfSegment>::type
    apply(Point const& p,
          PointOfSegment const& sp1,
          PointOfSegment const& sp2) const
    {
        typename closest_point_result<Point, PointOfSegment>::type
                 closest_point_result;

        typedef typename calculation_type<Point, PointOfSegment>::type CT;
/*
        typedef model::point<CT, 3, cs::cartesian> point3d_t;

        point3d_t vp1 = formula::sph_to_cart3d<point3d_t>(sp1);
        point3d_t vp2 = formula::sph_to_cart3d<point3d_t>(sp2);
        point3d_t vp = formula::sph_to_cart3d<point3d_t>(p);

        point3d_t G = cross_product(vp1, vp2);
        point3d_t F = cross_product(G, vp);
        set<0>(G, get<0>(G)*(-1));
        set<1>(G, get<1>(G)*(-1));
        set<2>(G, get<2>(G)*(-1));
        point3d_t X = cross_product(G, F);
        geometry::detail::vec_normalize(X);
        Point result_p = formula::cart3d_to_sph<Point>(X);

        closest_point_result.lon1 = get_as_radian<0>(p);
        closest_point_result.lat1 = get_as_radian<1>(p);
        closest_point_result.lon2 = get_as_radian<0>(result_p);
        closest_point_result.lat2 = get_as_radian<1>(result_p);

        closest_point_result.distance
                = typename distance::services::comparable_type<Strategy>
                                   ::type().apply(p, result_p);

        geometry::detail::vec_normalize(vp);
        geometry::detail::vec_normalize(vp1);
        geometry::detail::vec_normalize(vp2);

        std::cout << std::setprecision(10)
                  << get_as_radian<0>(p) * math::r2d<CT>() << ","
                  << get_as_radian<1>(p) * math::r2d<CT>() << " ("
                  << get_as_radian<0>(sp1) * math::r2d<CT>() << ","
                  << get_as_radian<1>(sp1) * math::r2d<CT>() << ")--("
                  << get_as_radian<0>(sp2) * math::r2d<CT>() << ","
                  << get_as_radian<1>(sp2) * math::r2d<CT>() << ") "
                  << get_as_radian<0>(result_p) * math::r2d<CT>() << ","
                  << get_as_radian<1>(result_p) * math::r2d<CT>() << ": "
                  << dot_product(vp,vp1) << " "
                  << dot_product(vp,vp2) << " "
                  << dot_product(vp,X) << " "
                  << std::endl;

        CT lon1 = get_as_radian<0>(sp1);
        CT lat1 = get_as_radian<1>(sp1);
        CT lon2 = get_as_radian<0>(sp2);
        CT lat2 = get_as_radian<1>(sp2);

        Strategy distance_strategy = Strategy(m_strategy);
        CT XTD = distance::cross_track<CT,Strategy>().apply(p, sp1, sp2);
        std::cout << "dist_cross_track=" << XTD << std::endl;
        std::cout << "dist_cross_track_comp=" << distance::comparable::cross_track<CT,Strategy>().apply(p, sp1, sp2) << std::endl;
        std::cout << "dist_cross_track radius=" << distance_strategy.radius() << std::endl;
        std::cout << "dist_cross_track asin=" <<
                     asin(math::sqrt(distance::comparable::cross_track<CT,Strategy>().apply(p, sp1, sp2))) << std::endl;
        CT dist_AD = distance_strategy.apply(p, sp1);
        CT ATD = acos(cos(dist_AD)/cos(XTD));
        CT a12 = geometry::formula::spherical_azimuth<>(lon1, lat1, lon2, lat2);
        geometry::formula::result_direct<CT> res
                = geometry::formula::spherical_direct<true, false>
                  (lon1, lat1, ATD, a12, srs::sphere<CT>(distance_strategy.radius()));
*/
        Strategy distance_strategy = Strategy(m_strategy);
        typedef geometry::formula::spherical_point_segment_distance<CT>
                spherical_point_segment_distance;
        typename spherical_point_segment_distance::result_type result
                = spherical_point_segment_distance()
                  .apply(p,
                         sp1,
                         sp2,
                         distance::services::get_comparable<Strategy>
                                 ::apply(distance_strategy));

        closest_point_result.lon1 = get_as_radian<0>(p);
        closest_point_result.lat1 = get_as_radian<1>(p);
        closest_point_result.lon2 = result.lon2;
        closest_point_result.lat2 = result.lat2;
        closest_point_result.distance = result.distance;
/*
        std::cout << std::setprecision(10)
                  << get_as_radian<0>(p) * math::r2d<CT>() << ","
                  << get_as_radian<1>(p) * math::r2d<CT>() << " ("
                  << get_as_radian<0>(sp1) * math::r2d<CT>() << ","
                  << get_as_radian<1>(sp1) * math::r2d<CT>() << ")--("
                  << get_as_radian<0>(sp2) * math::r2d<CT>() << ","
                  << get_as_radian<1>(sp2) * math::r2d<CT>() << ") "
                  << res.lon2 * math::r2d<CT>() << ","
                  << res.lat2 * math::r2d<CT>() << ": "
                  << XTD
                  << std::endl;
*/
        return closest_point_result;
    }

    template <typename ResultType>
    ResultType
    apply(ResultType comparable_result) const
    {
        return comparable_result;
    }

    // points on a meridian not crossing poles
    template <typename CT>
    geometry::detail::closest_points::result<CT>
    vertical_or_meridian(CT const& lat1, CT const& lat2, CT const& lon) const
    {
        CT distance = distance::comparable
                      ::cross_track<CalculationType, Strategy>()
                        .vertical_or_meridian(lat1, lat2, lon);

        geometry::detail::closest_points::result<CT> res;

        res.distance = distance;
        res.lon1 = lon;
        res.lat1 = lat1;
        res.lon2 = lon;
        res.lat2 = lat2;

        return res;
    }

private :
    Strategy m_strategy;
};

namespace comparable {

template
<
        typename CalculationType = void,
        typename Strategy = distance::haversine<double, CalculationType>
>
class cross_track
        : public closest_points::cross_track
            <
                CalculationType,
                Strategy
            >
{};

}

#ifndef DOXYGEN_NO_STRATEGY_SPECIALIZATIONS
namespace services
{

template <typename Point, typename PointOfSegment>
struct default_strategy
    <
        point_tag, segment_tag, Point, PointOfSegment,
        spherical_equatorial_tag, spherical_equatorial_tag
    >
{
    typedef cross_track<> type;
};


template <typename PointOfSegment, typename Point>
struct default_strategy
    <
        segment_tag, point_tag, PointOfSegment, Point,
        spherical_equatorial_tag, spherical_equatorial_tag
    >
{
    typedef typename default_strategy
        <
            point_tag, segment_tag, Point, PointOfSegment,
            spherical_equatorial_tag, spherical_equatorial_tag
        >::type type;
};

} // namespace services
#endif // DOXYGEN_NO_STRATEGY_SPECIALIZATIONS

} // namespace closest_points

namespace distance { namespace services {

//tags
template <typename CalculationType>
struct tag<closest_points::cross_track<CalculationType> >
{
    typedef strategy_tag_distance_point_segment type;
};

template
<
        typename CalculationType,
        typename Strategy
>
struct tag<closest_points::cross_track<CalculationType, Strategy> >
{
    typedef strategy_tag_distance_point_segment type;
};

template
<
        typename CalculationType,
        typename Strategy
>
struct tag<closest_points::comparable::cross_track<CalculationType, Strategy> >
{
    typedef strategy_tag_distance_point_segment type;
};


//return types
template <typename CalculationType, typename P, typename PS>
struct return_type<closest_points::cross_track<CalculationType>, P, PS>
        : closest_points::cross_track<CalculationType>
                        ::template closest_point_result<P, PS>
{};

template <typename CalculationType, typename P, typename PS>
struct return_type<closest_points::comparable::cross_track<CalculationType>, P, PS>
{
    typedef geometry::detail::closest_points::result
                <
                    typename closest_points::cross_track<CalculationType>
                                           ::template calculation_type<P, PS>
                                           ::type
                > type;
};

template
<
        typename CalculationType,
        typename Strategy,
        typename P,
        typename PS
>
struct return_type
       <
            closest_points::comparable::cross_track<CalculationType, Strategy>,
            P,
            PS
       >
{
    typedef geometry::detail::closest_points::result
                <
                    typename closest_points::cross_track<CalculationType, Strategy>
                                           ::template calculation_type<P, PS>
                                           ::type
                > type;
};

//comparable types
template
<
        typename CalculationType,
        typename Strategy
>
struct comparable_type<closest_points::cross_track<CalculationType, Strategy> >
{
    typedef closest_points::comparable::cross_track<CalculationType, Strategy>
            type;
};

template
<
        typename CalculationType,
        typename Strategy
>
struct comparable_type<closest_points::comparable
                                     ::cross_track<CalculationType, Strategy> >
{
    typedef closest_points::comparable::cross_track
        <
            CalculationType, Strategy
        >  type;
};

template
<
        typename CalculationType,
        typename Strategy
>
struct get_comparable<closest_points::cross_track<CalculationType, Strategy> >
{
    typedef typename comparable_type
        <
            closest_points::cross_track<CalculationType, Strategy>
        >::type comparable_type;
public :
    static inline comparable_type
    apply(closest_points::cross_track<CalculationType, Strategy> const&)
    {
        return comparable_type();
    }
};

template
<
        typename CalculationType,
        typename Strategy
>
struct get_comparable
        <
            closest_points::comparable::cross_track<CalculationType, Strategy>
        >
{
    typedef typename closest_points::comparable::geographic_cross_track
                     <
                        CalculationType,
                        Strategy
                     > comparable_type;
public :
    static inline comparable_type
    apply(closest_points::comparable::cross_track
                    <CalculationType, Strategy> const& strategy)
    {
        return strategy;
    }
};

template
<
    typename CalculationType,
    typename P,
    typename PS
>
struct result_from_distance<closest_points::cross_track<CalculationType>, P, PS>
{
private :
    typedef typename closest_points::cross_track
        <
            CalculationType
        >::template calculation_type<P, PS>::type calculation_type;
public :
    template <typename T>
    static inline calculation_type
    apply(closest_points::cross_track<CalculationType> const& ,
          T const& distance)
    {
        return distance;
    }
};

template
<
    typename CalculationType,
    typename Strategy,
    typename P,
    typename PS
>
struct result_from_distance<closest_points::cross_track
                  <CalculationType, Strategy>, P, PS>
{
private :
    typedef typename closest_points::cross_track
        <
            CalculationType, Strategy
        >::template calculation_type<P, PS>::type calculation_type;
public :
    template <typename T>
    static inline calculation_type
    apply(closest_points::cross_track<CalculationType, Strategy> const& ,
          T const& distance)
    {
        return distance;
    }
};

}}} // namespace strategy::distance::services

}} // namespace boost::geometry
#endif // BOOST_GEOMETRY_STRATEGIES_SPHERICAL_CLOSEST_POINTS_CROSS_TRACK_HPP

