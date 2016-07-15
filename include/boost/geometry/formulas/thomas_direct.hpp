// Boost.Geometry

// Copyright (c) 2016 Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_THOMAS_DIRECT_HPP
#define BOOST_GEOMETRY_FORMULAS_THOMAS_DIRECT_HPP


#include <boost/math/constants/constants.hpp>

#include <boost/geometry/core/radius.hpp>
#include <boost/geometry/core/srs.hpp>

#include <boost/geometry/util/condition.hpp>
#include <boost/geometry/util/math.hpp>

#include <boost/geometry/algorithms/detail/flattening.hpp>

#include <boost/geometry/formulas/differential_quantities.hpp>
#include <boost/geometry/formulas/result_direct.hpp>


namespace boost { namespace geometry { namespace formula
{


/*!
\brief The solution of the direct problem of geodesics on latlong coordinates,
       Forsyth-Andoyer-Lambert type approximation with second order terms.
\author See
    - Technical Report: PAUL D. THOMAS, MATHEMATICAL MODELS FOR NAVIGATION SYSTEMS, 1965
      http://www.dtic.mil/docs/citations/AD0627893
    - Technical Report: PAUL D. THOMAS, SPHEROIDAL GEODESICS, REFERENCE SYSTEMS, AND LOCAL GEOMETRY, 1970
      http://www.dtic.mil/docs/citations/AD0703541

*/
template <
    typename CT,
    bool EnableCoordinates = true,
    bool EnableReverseAzimuth = false,
    bool EnableReducedLength = false,
    bool EnableGeodesicScale = false
>
class thomas_direct
{
    static const bool CalcQuantities = EnableReducedLength || EnableGeodesicScale;
    static const bool CalcCoordinates = EnableCoordinates || CalcQuantities;
    static const bool CalcRevAzimuth = EnableReverseAzimuth || CalcCoordinates || CalcQuantities;

public:
    typedef result_direct<CT> result_type;

    template <typename T, typename Dist, typename Azi, typename Spheroid>
    static inline result_type apply(T const& lo1,
                                    T const& la1,
                                    Dist const& distance,
                                    Azi const& azimuth12,
                                    Spheroid const& spheroid)
    {
        result_type result;

        CT const lon1 = lo1;
        CT const lat1 = la1;

        if ( math::equals(distance, Dist(0)) || distance < Dist(0) )
        {
            result.lon2 = lon1;
            result.lat2 = lat1;
            return result;
        }

        CT const c0 = 0;
        CT const c1 = 1;
        CT const c2 = 2;
        CT const c4 = 4;

        CT const a = CT(get_radius<0>(spheroid));
        CT const b = CT(get_radius<2>(spheroid));
        CT const f = detail::flattening<CT>(spheroid);
        CT const one_minus_f = c1 - f;

        CT const pi_half = math::pi<CT>() / c2;

        CT const theta1 = math::equals(lat1, pi_half) ? lat1 :
                          math::equals(lat1, -pi_half) ? lat1 :
                          atan(one_minus_f * tan(lat1));
        CT const sin_theta1 = sin(theta1);
        CT const cos_theta1 = cos(theta1);

        CT const sin_a12 = sin(azimuth12);
        CT const cos_a12 = cos(azimuth12);

        CT const M = cos_theta1 * sin_a12; // cos_theta0
        CT const theta0 = acos(M);
        CT const sin_theta0 = sin(theta0);

        CT const N = cos_theta1 * cos_a12;
        CT const C1 = f * M; // lower-case c1 in the technical report
        CT const C2 = f * (c1 - math::sqr(M)) / c4; // lower-case c2 in the technical report
        CT const D = (c1 - C2) * (c1 - C2 - C1 * M);
        CT const P = C2 * (c1 + C1 * M / c2) / D;

        CT const cos_sigma1 = sin_theta1 / sin_theta0;
        CT const sigma1 = acos(cos_sigma1);
        CT const d = distance / (a * D);
        CT const u = 2 * (sigma1 - d);
        CT const sin_sigma1 = sin(sigma1);
        CT const cos_d = cos(d);
        CT const sin_d = sin(d);
        CT const cos_u = cos(u);
        CT const sin_u = sin(u);

        CT const W = c1 - c2 * P * cos_u;
        CT const V = cos_u * cos_d - sin_u * sin_d;
        CT const X = math::sqr(C2) * sin_d * cos_d * (2 * math::sqr(V) - c1);
        CT const Y = c2 * P * V * W * sin_d;
        CT const d_sigma = d + X - Y;
        CT const sin_d_sigma = sin(d_sigma);
        CT const cos_d_sigma = cos(d_sigma);

        if (BOOST_GEOMETRY_CONDITION(CalcRevAzimuth))
        {
            result.reverse_azimuth = atan2(M, N * cos_d_sigma - sin_theta1 * sin_d_sigma);
        }

        if (BOOST_GEOMETRY_CONDITION(CalcCoordinates))
        {
            CT const sin_a21 = sin(result.reverse_azimuth);
            CT const cos_a21 = cos(result.reverse_azimuth);

            CT const tan_lat2 = (sin_theta1 * cos_d_sigma + N * sin_d_sigma) * sin_a21 / (one_minus_f * M);

            result.lat2 = atan(tan_lat2);

            CT const S_sigma = c2 * sigma1 - d_sigma;
            CT const cos_S_sigma = cos(S_sigma);
            CT const d_eta = atan2(sin_d_sigma * sin_a12, cos_theta1 * cos_d_sigma - sin_theta1 * sin_d_sigma * cos_a12);
            CT const H = C1 * (c1 - C2) * d_sigma - C1 * C2 * sin_d_sigma * cos_S_sigma;
            CT const d_lambda = d_eta - H;
            
            result.lon2 = lon1 + d_lambda;
        }        

        if (BOOST_GEOMETRY_CONDITION(CalcQuantities))
        {
            typedef differential_quantities<CT, EnableReducedLength, EnableGeodesicScale> quantities;
            quantities::apply(lon1, lat1, result.lon2, result.lat2,
                              azimuth12, result.reverse_azimuth,
                              b, f,
                              result.reduced_length, result.geodesic_scale,
                              quantities::J12_calc_f2);
        }

        return result;
    }

};

}}} // namespace boost::geometry::formula


#endif // BOOST_GEOMETRY_FORMULAS_THOMAS_DIRECT_HPP
