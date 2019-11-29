// Boost.Geometry

// Copyright (c) 2016-2018, Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_SPHERICAL_HPP
#define BOOST_GEOMETRY_FORMULAS_SPHERICAL_HPP

#include <boost/geometry/srs/sphere.hpp>

#include <boost/geometry/core/coordinate_system.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/core/radius.hpp>

//#include <boost/geometry/arithmetic/arithmetic.hpp>
#include <boost/geometry/arithmetic/cross_product.hpp>
#include <boost/geometry/arithmetic/dot_product.hpp>

#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/normalize_spheroidal_coordinates.hpp>
#include <boost/geometry/util/select_coordinate_type.hpp>

#include <boost/geometry/formulas/result_direct.hpp>

//#define BOOST_GEOMETRY_DEBUG_PT_SEG
#include <iostream>

namespace boost { namespace geometry {
    
namespace formula {

template <typename T>
struct result_spherical
{
    result_spherical()
        : azimuth(0)
        , reverse_azimuth(0)
    {}

    T azimuth;
    T reverse_azimuth;
};

template <typename T>
static inline void sph_to_cart3d(T const& lon, T const& lat, T & x, T & y, T & z)
{
    T const cos_lat = cos(lat);
    x = cos_lat * cos(lon);
    y = cos_lat * sin(lon);
    z = sin(lat);
}

template <typename Point3d, typename PointSph>
static inline Point3d sph_to_cart3d(PointSph const& point_sph)
{
    typedef typename coordinate_type<Point3d>::type calc_t;

    calc_t const lon = get_as_radian<0>(point_sph);
    calc_t const lat = get_as_radian<1>(point_sph);
    calc_t x, y, z;
    sph_to_cart3d(lon, lat, x, y, z);

    Point3d res;
    set<0>(res, x);
    set<1>(res, y);
    set<2>(res, z);

    return res;
}

template <typename T>
static inline void cart3d_to_sph(T const& x, T const& y, T const& z, T & lon, T & lat)
{
    lon = atan2(y, x);
    lat = asin(z);
}

template <typename PointSph, typename Point3d>
static inline PointSph cart3d_to_sph(Point3d const& point_3d)
{
    typedef typename coordinate_type<PointSph>::type coord_t;
    typedef typename coordinate_type<Point3d>::type calc_t;

    calc_t const x = get<0>(point_3d);
    calc_t const y = get<1>(point_3d);
    calc_t const z = get<2>(point_3d);
    calc_t lonr, latr;
    cart3d_to_sph(x, y, z, lonr, latr);

    PointSph res;
    set_from_radian<0>(res, lonr);
    set_from_radian<1>(res, latr);

    coord_t lon = get<0>(res);
    coord_t lat = get<1>(res);

    math::normalize_spheroidal_coordinates
        <
            typename detail::cs_angular_units<PointSph>::type,
            coord_t
        >(lon, lat);

    set<0>(res, lon);
    set<1>(res, lat);

    return res;
}

// -1 right
// 1 left
// 0 on
template <typename Point3d1, typename Point3d2>
static inline int sph_side_value(Point3d1 const& norm, Point3d2 const& pt)
{
    typedef typename select_coordinate_type<Point3d1, Point3d2>::type calc_t;
    calc_t c0 = 0;
    calc_t d = dot_product(norm, pt);
    return math::equals(d, c0) ? 0
        : d > c0 ? 1
        : -1; // d < 0
}

template <typename CT, bool ReverseAzimuth, typename T1, typename T2>
static inline result_spherical<CT> spherical_azimuth(T1 const& lon1,
                                                     T1 const& lat1,
                                                     T2 const& lon2,
                                                     T2 const& lat2)
{
    typedef result_spherical<CT> result_type;
    result_type result;

    // http://williams.best.vwh.net/avform.htm#Crs
    // https://en.wikipedia.org/wiki/Great-circle_navigation
    CT dlon = lon2 - lon1;

    // An optimization which should kick in often for Boxes
    //if ( math::equals(dlon, ReturnType(0)) )
    //if ( get<0>(p1) == get<0>(p2) )
    //{
    //    return - sin(get_as_radian<1>(p1)) * cos_p2lat);
    //}

    CT const cos_dlon = cos(dlon);
    CT const sin_dlon = sin(dlon);
    CT const cos_lat1 = cos(lat1);
    CT const cos_lat2 = cos(lat2);
    CT const sin_lat1 = sin(lat1);
    CT const sin_lat2 = sin(lat2);

    {
        // "An alternative formula, not requiring the pre-computation of d"
        // In the formula below dlon is used as "d"
        CT const y = sin_dlon * cos_lat2;
        CT const x = cos_lat1 * sin_lat2 - sin_lat1 * cos_lat2 * cos_dlon;
        result.azimuth = atan2(y, x);
    }

    if (ReverseAzimuth)
    {
        CT const y = sin_dlon * cos_lat1;
        CT const x = sin_lat2 * cos_lat1 * cos_dlon - cos_lat2 * sin_lat1;
        result.reverse_azimuth = atan2(y, x);
    }

    return result;
}

template <typename ReturnType, typename T1, typename T2>
inline ReturnType spherical_azimuth(T1 const& lon1, T1 const& lat1,
                                    T2 const& lon2, T2 const& lat2)
{
    return spherical_azimuth<ReturnType, false>(lon1, lat1, lon2, lat2).azimuth;
}

template <typename T>
inline T spherical_azimuth(T const& lon1, T const& lat1, T const& lon2, T const& lat2)
{
    return spherical_azimuth<T, false>(lon1, lat1, lon2, lat2).azimuth;
}

template <typename T>
inline int azimuth_side_value(T const& azi_a1_p, T const& azi_a1_a2)
{
    T const c0 = 0;
    T const pi = math::pi<T>();
    T const two_pi = math::two_pi<T>();

    // instead of the formula from XTD
    //calc_t a_diff = asin(sin(azi_a1_p - azi_a1_a2));

    T a_diff = azi_a1_p - azi_a1_a2;
    // normalize, angle in [-pi, pi]
    while (a_diff > pi)
        a_diff -= two_pi;
    while (a_diff < -pi)
        a_diff += two_pi;

    // NOTE: in general it shouldn't be required to support the pi/-pi case
    // because in non-cartesian systems it makes sense to check the side
    // only "between" the endpoints.
    // However currently the winding strategy calls the side strategy
    // for vertical segments to check if the point is "between the endpoints.
    // This could be avoided since the side strategy is not required for that
    // because meridian is the shortest path. So a difference of
    // longitudes would be sufficient (of course normalized to [-pi, pi]).

    // NOTE: with the above said, the pi/-pi check is temporary
    // however in case if this was required
    // the geodesics on ellipsoid aren't "symmetrical"
    // therefore instead of comparing a_diff to pi and -pi
    // one should probably use inverse azimuths and compare
    // the difference to 0 as well

    // positive azimuth is on the right side
    return math::equals(a_diff, c0)
        || math::equals(a_diff, pi)
        || math::equals(a_diff, -pi) ? 0
        : a_diff > 0 ? -1 // right
        : 1; // left
}

template
<
    bool Coordinates,
    bool ReverseAzimuth,
    typename CT,
    typename Sphere
>
inline result_direct<CT> spherical_direct(CT const& lon1,
                                          CT const& lat1,
                                          CT const& sig12,
                                          CT const& alp1,
                                          Sphere const& sphere)
{
    result_direct<CT> result;

    CT const sin_alp1 = sin(alp1);
    CT const sin_lat1 = sin(lat1);
    CT const cos_alp1 = cos(alp1);
    CT const cos_lat1 = cos(lat1);

    CT const norm = math::sqrt(cos_alp1 * cos_alp1 + sin_alp1 * sin_alp1
                                                   * sin_lat1 * sin_lat1);
    CT const alp0 = atan2(sin_alp1 * cos_lat1, norm);
    CT const sig1 = atan2(sin_lat1, cos_alp1 * cos_lat1);
    CT const sig2 = sig1 + sig12 / get_radius<0>(sphere);

    CT const cos_sig2 = cos(sig2);
    CT const sin_alp0 = sin(alp0);
    CT const cos_alp0 = cos(alp0);

    if (Coordinates)
    {
        CT const sin_sig2 = sin(sig2);
        CT const sin_sig1 = sin(sig1);
        CT const cos_sig1 = cos(sig1);

        CT const norm2 = math::sqrt(cos_alp0 * cos_alp0 * cos_sig2 * cos_sig2
                                    + sin_alp0 * sin_alp0);
        CT const lat2 = atan2(cos_alp0 * sin_sig2, norm2);

        CT const omg1 = atan2(sin_alp0 * sin_sig1, cos_sig1);
        CT const lon2 = atan2(sin_alp0 * sin_sig2, cos_sig2);

        result.lon2 = lon1 + lon2 - omg1;
        result.lat2 = lat2;
    }

    if (ReverseAzimuth)
    {
        CT const alp2 = atan2(sin_alp0, cos_alp0 * cos_sig2);
        result.reverse_azimuth = alp2;
    }

    return result;
}

template <
    typename CalculationType,
    bool EnableClosestPoint = false
>
class spherical_point_segment_distance
{

public :

    struct result_type
    {
        result_type()
            : distance(0)
            , lon1(0)
            , lat1(0)
            , lon2(0)
            , lat2(0)
        {}

        CalculationType distance;
        CalculationType lon1;
        CalculationType lat1;
        CalculationType lon2;
        CalculationType lat2;
    };

    template <typename Point, typename PointOfSegment, typename Strategy>
    inline result_type
    apply(Point const& p, PointOfSegment const& sp1, PointOfSegment const& sp2,
          Strategy const& comparable_distance_strategy) const
    {

#if !defined(BOOST_MSVC)
        BOOST_CONCEPT_ASSERT
            (
                (concepts::PointDistanceStrategy<Strategy, Point, PointOfSegment>)
            );
#endif


        result_type res;
        res.lon1 = get_as_radian<0>(p);
        res.lat1 = get_as_radian<1>(p);

        typedef CalculationType return_type;

        // http://williams.best.vwh.net/avform.htm#XTE
        return_type d1 = comparable_distance_strategy.apply(sp1, p);
        return_type d3 = comparable_distance_strategy.apply(sp1, sp2);

        if (geometry::math::equals(d3, 0.0))
        {
            // "Degenerate" segment, return either d1 or d2
            res.distance = d1;
            res.lon2 = get_as_radian<0>(sp1);
            res.lat2 = get_as_radian<1>(sp1);
            return res;
        }

        return_type d2 = comparable_distance_strategy.apply(sp2, p);

        return_type lon1 = geometry::get_as_radian<0>(sp1);
        return_type lat1 = geometry::get_as_radian<1>(sp1);
        return_type lon2 = geometry::get_as_radian<0>(sp2);
        return_type lat2 = geometry::get_as_radian<1>(sp2);
        return_type lon = geometry::get_as_radian<0>(p);
        return_type lat = geometry::get_as_radian<1>(p);

        return_type crs_AD = spherical_azimuth<return_type, false>
                             (lon1, lat1, lon, lat).azimuth;

        result_spherical<return_type> result = spherical_azimuth<return_type, true>
                                               (lon1, lat1, lon2, lat2);
        return_type crs_AB = result.azimuth;
        return_type crs_BA = result.reverse_azimuth - geometry::math::pi<return_type>();

        return_type crs_BD = geometry::formula::spherical_azimuth<return_type, false>
                             (lon2, lat2, lon, lat).azimuth;

        return_type d_crs1 = crs_AD - crs_AB;
        return_type d_crs2 = crs_BD - crs_BA;

        // d1, d2, d3 are in principle not needed, only the sign matters
        return_type projection1 = cos( d_crs1 ) * d1 / d3;
        return_type projection2 = cos( d_crs2 ) * d2 / d3;

#ifdef BOOST_GEOMETRY_DEBUG_PT_SEG
        /*
        std::cout << "Course " << dsv(sp1) << " to " << dsv(p) << " "
                  << crs_AD * geometry::math::r2d<return_type>() << std::endl;
        std::cout << "Course " << dsv(sp1) << " to " << dsv(sp2) << " "
                  << crs_AB * geometry::math::r2d<return_type>() << std::endl;
        std::cout << "Course " << dsv(sp2) << " to " << dsv(sp1) << " "
                  << crs_BA * geometry::math::r2d<return_type>() << std::endl;
        std::cout << "Course " << dsv(sp2) << " to " << dsv(p) << " "
                  << crs_BD * geometry::math::r2d<return_type>() << std::endl;*/
        std::cout << "Projection AD-AB " << projection1 << " : "
                  << d_crs1 * geometry::math::r2d<return_type>() << std::endl;
        std::cout << "Projection BD-BA " << projection2 << " : "
                  << d_crs2 * geometry::math::r2d<return_type>() << std::endl;
        std::cout << " d1: " << (d1 )
                  << " d2: " << (d2 )
                  << std::endl;
#endif

        if (projection1 > 0.0 && projection2 > 0.0)
        {
#ifdef BOOST_GEOMETRY_DEBUG_PT_SEG
            return_type XTD = comparable_distance_strategy.radius() * geometry::math::abs( asin( sin( d1 ) * sin( d_crs1 ) ));

            std::cout << "Projection ON the segment" << std::endl;
            std::cout << "XTD: " << XTD
                      << " d1: " << (d1 * comparable_distance_strategy.radius())
                      << " d2: " << (d2 * comparable_distance_strategy.radius())
                      << std::endl;
#endif
            return_type const half(0.5);
            return_type const quarter(0.25);

            return_type sin_d_crs1 = sin(d_crs1);
            /*
              This is the straightforward obvious way to continue:

              return_type discriminant
                  = 1.0 - 4.0 * (d1 - d1 * d1) * sin_d_crs1 * sin_d_crs1;
              return 0.5 - 0.5 * math::sqrt(discriminant);

              Below we optimize the number of arithmetic operations
              and account for numerical robustness:
            */
            return_type d1_x_sin = d1 * sin_d_crs1;
            return_type d = d1_x_sin * (sin_d_crs1 - d1_x_sin);
            res.distance = d / (half + math::sqrt(quarter - d));
#ifdef BOOST_GEOMETRY_DEBUG_PT_SEG
            //boost::geometry::strategy::distance::cross_track<return_type,Strategy> str;
            //std::cout << str.apply(p, sp1, sp2)
            //        << std::endl;
            std::cout << "dist=" << res.distance << std::endl;
            std::cout << "dist_cross_track radius=" << comparable_distance_strategy.radius() << std::endl;
            std::cout << "dist_cross_track asin=" <<
                         asin(math::sqrt(res.distance)) << std::endl;


#endif
            return_type dist = return_type(2)
                               * asin(math::sqrt(res.distance))
                               * comparable_distance_strategy.radius();
            return_type dist_d1 = return_type(2)
                                  * asin(math::sqrt(d1))
                                  * comparable_distance_strategy.radius();
            // this is similar to spherical computation in geographic
            // point_segment_distance formula
            return_type earth_radius = comparable_distance_strategy.radius();
            return_type cos_frac = cos(dist_d1 / earth_radius)
                                 / cos(dist / earth_radius);
            return_type s14_sph = cos_frac >= 1 ? return_type(0)
                         : cos_frac <= -1 ? math::pi<return_type>()
                                            * earth_radius
                         : acos(cos_frac) * earth_radius;

            return_type a12 = spherical_azimuth<>(lon1, lat1, lon2, lat2);
            result_direct<return_type> res_direct
                    = geometry::formula::spherical_direct<true, false>
                      (lon1, lat1, s14_sph, a12,
                       srs::sphere<return_type>(earth_radius));

            res.lon2 = res_direct.lon2;
            res.lat2 = res_direct.lat2;

            return res;
        }
        else
        {
#ifdef BOOST_GEOMETRY_DEBUG_PT_SEG
            std::cout << "Projection OUTSIDE the segment" << std::endl;
#endif

            // Return shortest distance, project either on point sp1 or sp2
            if (d1 < d2)
            {
                res.distance = return_type(d1);
                res.lon2 = get_as_radian<0>(sp1);
                res.lat2 = get_as_radian<1>(sp1);
            }
            else
            {
                res.distance = return_type(d2);
                res.lon2 = get_as_radian<0>(sp2);
                res.lat2 = get_as_radian<1>(sp2);
            }
            return res;
        }
    }
};

} // namespace formula

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_FORMULAS_SPHERICAL_HPP
