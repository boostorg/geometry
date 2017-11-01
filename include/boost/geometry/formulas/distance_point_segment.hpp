// Boost.Geometry

// Copyright (c) 2016-2017 Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_CROSS_TRACK_GEO_HPP
#define BOOST_GEOMETRY_FORMULAS_CROSS_TRACK_GEO_HPP

#include <boost/geometry/formulas/result_direct.hpp>
#include <boost/geometry/formulas/mean_radius.hpp>

#ifndef BOOST_GEOMETRY_DETAIL_POINT_SEGMENT_DISTANCE_MAX_STEPS
#define BOOST_GEOMETRY_DETAIL_POINT_SEGMENT_DISTANCE_MAX_STEPS 100
#endif

#ifdef BOOST_GEOMETRY_DISTANCE_POINT_SEGMENT_DEBUG
#include <iostream>
#endif

/*!
\brief Algorithm to compute the distance between a segment and a point using
       direct and inverse geodesic problems as subroutines. The algorithm
       approximates the distance by an iterative Newton method.
\see C.F.F.Karney - Geodesics on an ellipsoid of revolution,
      https://arxiv.org/abs/1102.1215
*/

namespace boost { namespace geometry { namespace formula
{

template
<
        typename CT,
        typename Units,
        typename FormulaPolicy,
        bool EnableClosestPoint = false
>
class distance_point_segment{

public:

    typedef typename FormulaPolicy::template inverse<CT, true, false, false, true, true> inverse_distance_quantities_type;
    typedef typename FormulaPolicy::template inverse<CT, false, true, false, false, false> inverse_azimuth_type;
    typedef typename FormulaPolicy::template inverse<CT, false, true, true, false, false> inverse_azimuth_reverse_type;
    typedef typename FormulaPolicy::template direct<CT, true, false, false, false> direct_distance_type;

    struct result_distance_point_segment
    {
        result_distance_point_segment()
            : distance(0)
            , closest_point_lon(0)
            , closest_point_lat(0)
        {}

        CT distance;
        CT closest_point_lon;
        CT closest_point_lat;
    };

    result_distance_point_segment
    static inline non_iterative_case(CT lon, CT lat, CT distance)
    {
        result_distance_point_segment result;
        result.distance = distance;

        if (EnableClosestPoint)
        {
            result.closest_point_lon = lon;
            result.closest_point_lat = lat;
        }
        return result;
    }

    template <typename Spheroid>
    result_distance_point_segment
    static inline non_iterative_case(CT lon1, CT lat1, //p1
                                     CT lon2, CT lat2, //p2
                                     Spheroid const& spheroid)
    {
        CT distance = geometry::strategy::distance::geographic<FormulaPolicy, Spheroid, CT>
                              ::apply(lon1, lat1, lon2, lat2, spheroid);

        return non_iterative_case(lon1, lat1, distance);
    }

    template <typename T>
    T static inline normalize(T g4)
    {
        CT const pi = math::pi<CT>();
        if (g4 < 0 && g4 < -pi)//close to -270
        {
            return g4 + 1.5 * pi;
        }
        else if (g4 > 0 && g4 > pi)//close to 270
        {
            return - g4 + 1.5 * pi;
        }
        else if (g4 < 0 && g4 > -pi)//close to -90
        {
            return -g4 - pi/2;
        }
        return g4 - pi/2;
    }

    template <typename Spheroid>
    result_distance_point_segment
    static inline apply(CT lon1, CT lat1, //p1
                        CT lon2, CT lat2, //p2
                        CT lon3, CT lat3, //query point p3
                        Spheroid const& spheroid)
    {
        CT const earth_radius = geometry::formula::mean_radius<CT>(spheroid);

        result_distance_point_segment result;

        // Constants
        CT const f = flattening<CT>(spheroid);
        CT const pi = math::pi<CT>();
        CT const half_pi = pi / CT(2);
        CT const c0 = CT(0);

        // Convert to radians
        lon1 = math::as_radian<Units>(lon1);
        lat1 = math::as_radian<Units>(lat1);
        lon2 = math::as_radian<Units>(lon2);
        lat2 = math::as_radian<Units>(lat2);
        lon3 = math::as_radian<Units>(lon3);
        lat3 = math::as_radian<Units>(lat3);

        if (lon1 > lon2)
        {
            std::swap(lon1, lon2);
            std::swap(lat1, lat2);
        }

        //segment on equator
        //Note: antipodal points on equator does not define segment on equator
        //but pass by the pole
        CT diff = math::longitude_distance_signed<geometry::radian>(lon1, lon2);
        if (math::equals(lat1, c0) && math::equals(lat2, c0)
            && !math::equals(math::abs(diff), pi))
        {
#ifdef BOOST_GEOMETRY_DISTANCE_POINT_SEGMENT_DEBUG
            std::cout << "Equatorial segment" << std::endl;
#endif
            if (lon3 <= lon1)
            {
                return non_iterative_case(lon1, lat1, lon3, lat3, spheroid);
            }
            if (lon3 >= lon2)
            {
                return non_iterative_case(lon2, lat2, lon3, lat3, spheroid);
            }
            return non_iterative_case(lon3, lat1, lon3, lat3, spheroid);
        }

        CT d1 = geometry::strategy::distance::geographic<FormulaPolicy, Spheroid, CT>
                ::apply(lon1, lat1, lon3, lat3, spheroid);

        CT d3 = geometry::strategy::distance::geographic<FormulaPolicy, Spheroid, CT>
                ::apply(lon1, lat1, lon2, lat2, spheroid);

        if (geometry::math::equals(d3, c0))
        {
#ifdef BOOST_GEOMETRY_DISTANCE_POINT_SEGMENT_DEBUG
            std::cout << "Degenerate segment" << std::endl;
            std::cout << "distance between points=" << d1 << std::endl;
#endif
            return non_iterative_case(lon1, lat2, d1);
        }

        CT d2 = geometry::strategy::distance::geographic<FormulaPolicy, Spheroid, CT>
                ::apply(lon2, lat2, lon3, lat3, spheroid);

        // Compute a12 (GEO)
        geometry::formula::result_inverse<CT> res12 =
                inverse_azimuth_reverse_type::apply(lon1, lat1, lon2, lat2, spheroid);
        CT a12 = res12.azimuth;
        CT a13 = inverse_azimuth_type::apply(lon1, lat1, lon3, lat3, spheroid).azimuth;

        CT a312 = a13 - a12;

        if (geometry::math::equals(a312, c0))
        {
#ifdef BOOST_GEOMETRY_DISTANCE_POINT_SEGMENT_DEBUG
            std::cout << "point on segment" << std::endl;
#endif
            return non_iterative_case(lon3, lat3, c0);
        }

        CT projection1 = cos( a312 ) * d1 / d3;

#ifdef BOOST_GEOMETRY_DISTANCE_POINT_SEGMENT_DEBUG
        std::cout << "segment=(" << lon1 * math::r2d<CT>();
        std::cout << "," << lat1 * math::r2d<CT>();
        std::cout << "),(" << lon2 * math::r2d<CT>();
        std::cout << "," << lat2 * math::r2d<CT>();
        std::cout << ")\np=(" << lon3 * math::r2d<CT>();
        std::cout << "," << lat3 * math::r2d<CT>();
        std::cout << ")\na1=" << a12 * math::r2d<CT>() << std::endl;
        std::cout << "a13=" << a13 * math::r2d<CT>() << std::endl;
        std::cout << "a312=" << a312 * math::r2d<CT>() << std::endl;
        std::cout << "cos(a312)=" << cos(a312) << std::endl;
#endif
        if (projection1 < 0.0)
        {
#ifdef BOOST_GEOMETRY_DISTANCE_POINT_SEGMENT_DEBUG
            std::cout << "projection closer to p1" << std::endl;
#endif
            // projection of p3 on geodesic spanned by segment (p1,p2) fall
            // outside of segment on the side of p1
            return non_iterative_case(lon1, lat1, lon3, lat3, spheroid);
        }

        CT a21 = res12.reverse_azimuth - pi;
        CT a23 = inverse_azimuth_type::apply(lon2, lat2, lon3, lat3, spheroid).azimuth;

        CT a321 = a23 - a21;

#ifdef BOOST_GEOMETRY_DISTANCE_POINT_SEGMENT_DEBUG
        std::cout << "a21=" << a21 * math::r2d<CT>() << std::endl;
        std::cout << "a23=" << a23 * math::r2d<CT>() << std::endl;
        std::cout << "a321=" << a321 * math::r2d<CT>() << std::endl;
        std::cout << "cos(a321)=" << cos(a321) << std::endl;
#endif
        CT projection2 = cos( a321 ) * d2 / d3;

        if (projection2 < 0.0)
        {
#ifdef BOOST_GEOMETRY_DISTANCE_POINT_SEGMENT_DEBUG
            std::cout << "projection closer to p2" << std::endl;
#endif
            // projection of p3 on geodesic spanned by segment (p1,p2) fall
            // outside of segment on the side of p2
            return non_iterative_case(lon2, lat2, lon3, lat3, spheroid);
        }

        // Guess s14 (SPHERICAL)
        typedef geometry::model::point
                <
                    CT, 2,
                    geometry::cs::spherical_equatorial<geometry::radian>
                > point;

        CT bet1 = atan((1 - f) * tan(lon1));
        CT bet2 = atan((1 - f) * tan(lon2));
        CT bet3 = atan((1 - f) * tan(lon3));
        point p1 = point(bet1, lat1);
        point p2 = point(bet2, lat2);
        point p3 = point(bet3, lat3);

        geometry::strategy::distance::cross_track<CT> cross_track(earth_radius);
        CT s34 = cross_track.apply(p3, p1, p2);

        geometry::strategy::distance::haversine<CT> str(earth_radius);
        CT s13 = str.apply(p1, p3);
        CT s14 = acos( cos(s13/earth_radius) / cos(s34/earth_radius) ) * earth_radius;

#ifdef BOOST_GEOMETRY_DISTANCE_POINT_SEGMENT_DEBUG
        std::cout << "s34=" << s34 << std::endl;
        std::cout << "s13=" << s13 << std::endl;
        std::cout << "s14=" << s14 << std::endl;
        std::cout << "===============" << std::endl;
#endif

        // Update s14 (using Newton method)
        CT prev_distance = 0;
        geometry::formula::result_direct<CT> res14;
        geometry::formula::result_inverse<CT> res34;

        int counter = 0; // robustness
        CT g4;
        CT delta_g4;

        do{
            prev_distance = res34.distance;

            // Solve the direct problem to find p4 (GEO)
            res14 = direct_distance_type::apply(lon1, lat1, s14, a12, spheroid);

            // Solve an inverse problem to find g4
            // g4 is the angle between segment (p1,p2) and segment (p3,p4) that meet on p4 (GEO)

            CT a4 = inverse_azimuth_type::apply(res14.lon2, res14.lat2,
                                                lon2, lat2, spheroid).azimuth;
            res34 = inverse_distance_quantities_type::apply(res14.lon2, res14.lat2,
                                                            lon3, lat3, spheroid);
            g4 = res34.azimuth - a4;

            delta_g4 = normalize(g4);

            CT M43 = res34.geodesic_scale; // cos(s14/earth_radius) is the spherical limit
            CT m34 = res34.reduced_length;
            CT der = (M43 / m34) * sin(g4);
            s14 = s14 - delta_g4 / der;

#ifdef BOOST_GEOMETRY_DISTANCE_POINT_SEGMENT_DEBUG
            std::cout << "p4=" << res14.lon2 * math::r2d<CT>() <<
                         "," << res14.lat2 * math::r2d<CT>() << std::endl;
            std::cout << "delta_g4=" << delta_g4  << std::endl;
            std::cout << "g4=" << g4 * math::r2d<CT>() << std::endl;
            std::cout << "der=" << der  << std::endl;
            std::cout << "M43=" << M43 << std::endl;
            std::cout << "spherical limit=" << cos(s14/earth_radius) << std::endl;
            std::cout << "m34=" << m34 << std::endl;
            std::cout << "new_s14=" << s14 << std::endl;
            std::cout << std::setprecision(16) << "dist     =" << res34.distance << std::endl;
            std::cout << "---------end of step " << counter << std::endl<< std::endl;
#endif
            result.distance = prev_distance;

#ifdef BOOST_GEOMETRY_DISTANCE_POINT_SEGMENT_DEBUG
            if (g4 == half_pi)
            {
                std::cout << "Stop msg: g4 == half_pi" << std::endl;
            }
            if (res34.distance >= prev_distance && prev_distance != 0)
            {
                std::cout << "Stop msg: res34.distance >= prev_distance" << std::endl;
            }
            if (delta_g4 == 0)
            {
                std::cout << "Stop msg: delta_g4 == 0" << std::endl;
            }
            if (counter == 19)
            {
                std::cout << "Stop msg: counter" << std::endl;
            }
#endif

        } while (g4 != half_pi
                 && (prev_distance > res34.distance || prev_distance == 0)
                 && delta_g4 != 0
                 && ++counter < BOOST_GEOMETRY_DETAIL_POINT_SEGMENT_DISTANCE_MAX_STEPS ) ;

#ifdef BOOST_GEOMETRY_DISTANCE_POINT_SEGMENT_DEBUG
        std::cout << "distance=" << res34.distance << std::endl;

        point p4(res14.lon2, res14.lat2);
        CT s34_sph = str.apply(p4, p3);

        std::cout << "s34(sph) =" << s34_sph << std::endl;
        std::cout << "s34(geo) ="
                  << inverse_distance_quantities_type::apply(get<0>(p4), get<1>(p4), lon3, lat3, spheroid).distance
                  << ", p4=(" << get<0>(p4) * math::r2d<double>() << ","
                              << get<1>(p4) * math::r2d<double>() << ")"
                  << std::endl;

        CT s31 = inverse_distance_quantities_type::apply(lon3, lat3, lon1, lat1, spheroid).distance;
        CT s32 = inverse_distance_quantities_type::apply(lon3, lat3, lon2, lat2, spheroid).distance;

        CT a4 = inverse_azimuth_type::apply(get<0>(p4), get<1>(p4), lon2, lat2, spheroid).azimuth;
        geometry::formula::result_direct<CT> res4 = direct_distance_type::apply(get<0>(p4), get<1>(p4), .04, a4, spheroid);
        CT p4_plus = inverse_distance_quantities_type::apply(res4.lon2, res4.lat2, lon3, lat3, spheroid).distance;

        geometry::formula::result_direct<CT> res1 = direct_distance_type::apply(lon1, lat1, s14-.04, a12, spheroid);
        CT p4_minus = inverse_distance_quantities_type::apply(res1.lon2, res1.lat2, lon3, lat3, spheroid).distance;

        std::cout << "s31=" << s31 << "\ns32=" << s32
                  << "\np4_plus=" << p4_plus << ", p4=(" << res4.lon2 * math::r2d<double>() << "," << res4.lat2 * math::r2d<double>() << ")"
                  << "\np4_minus=" << p4_minus << ", p4=(" << res1.lon2 * math::r2d<double>() << "," << res1.lat2 * math::r2d<double>() << ")"
                  << std::endl;

        if (res34.distance <= p4_plus && res34.distance <= p4_minus)
        {
            std::cout << "Closest point computed" << std::endl;
        }
        else
        {
            std::cout << "There is a closer point nearby" << std::endl;
        }
#endif

        return result;
    }

};

}}} // namespace boost::geometry::formula
#endif // BOOST_GEOMETRY_FORMULAS_CROSS_TRACK_GEO_HPP
