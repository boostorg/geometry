// Boost.Geometry

// Copyright (c) 2017 Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_ELLIPTIC_ARC_LENGTH_HPP
#define BOOST_GEOMETRY_FORMULAS_ELLIPTIC_ARC_LENGTH_HPP

#include <boost/math/constants/constants.hpp>

#include <boost/geometry/core/radius.hpp>
#include <boost/geometry/core/srs.hpp>

#include <boost/geometry/util/condition.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/normalize_spheroidal_coordinates.hpp>

#include <boost/geometry/formulas/flattening.hpp>
#include <boost/geometry/formulas/elliptic_meridian_arc_length.hpp>

namespace boost { namespace geometry { namespace formula
{

/*!
\brief Compute the arc length of an ellipse i.e. the intersection of a spheroid
with a hyperplane defined by the center of spheroid and two points on its
surface.
*/

template <typename CT>
class elliptic_arc_length
{

public :

    // from [Tseng15]
    // Tseng - THE GEOMETRIC ALGORITHM OF INVERSE DIRECT PROBLEMS FOR GREAT ELLIPTIC ARCS
    template <typename T, typename Spheroid>
    static T dist_tseng(T lon1, T lat1, T lon2, T lat2, Spheroid const& spheroid)
    {
        CT c0 = 0;
        CT c1 = 1;
        CT c2 = 2;

        CT const f = formula::flattening<CT>(spheroid);
        CT const e2 = f * (CT(2) - f);
        CT const c1_e2 = c1 - e2;
        CT const e = sqrt(e2);
        CT const a = get_radius<0>(spheroid);

        CT sin_lat1 = sin(lat1);
        CT cos_lat1 = cos(lat1);
        CT sin_lat2 = sin(lat2);
        CT cos_lat2 = cos(lat2);

        CT sin_lon12 = sin(lon2 - lon1);
        CT cos_lon12 = cos(lon2 - lon1);

        CT bet1 = atan2(c1_e2 * sin_lat1, cos_lat1);
        CT bet2 = atan2(c1_e2 * sin_lat2, cos_lat2);

        CT sin_bet1 = sin(bet1);
        CT cos_bet1 = cos(bet1);

        CT sin_bet2 = sin(bet2);
        CT cos_bet2 = cos(bet2);

        CT gam1 = atan2(cos_bet2 * sin_lon12,
                        cos_bet1 * sin_bet2 - sin_bet1 * cos_bet2 * cos_lon12);
        CT sin_gam1 = sin(gam1);
        CT cos_gam1 = cos(gam1);

        CT lon_e = lon1 + atan2(-sin_bet1 * sin_gam1, cos_gam1);

        //CT cos_bet_v = cos_bet1 * sin_gam1;
        //CT cos_bet_v_sqr = cos_bet_v * cos_bet_v;
        //CT sin_bet_v = sqrt(c1 - cos_bet_v_sqr);

        CT bet_v = acos(cos_bet1 * sin_gam1);
        CT sin_bet_v = sin(bet_v);
        CT cos_bet_v = cos(bet_v);
        CT cos_bet_v_sqr = cos_bet_v * cos_bet_v;

        CT eps = (e * sin_bet_v) / sqrt(c1 - e2 * cos_bet_v_sqr);

        CT sig1 = atan2(sin(lon1 - lon_e) / cos_bet_v, cos(lon1 - lon_e));
        CT sig2 = atan2(sin(lon2 - lon_e) / cos_bet_v, cos(lon2 - lon_e));
        CT psi1 = atan2(sin(sig1), c1_e2 * cos(sig1));
        CT psi2 = atan2(sin(sig2), c1_e2 * cos(sig2));

        CT eps2 = eps * eps;
        CT eps4 = eps2 * eps2;

        //Order 2
        CT M0 = c1 + 0.75 * eps2 + 0.703125 * eps4;
        CT M2 = -0.375 * eps2 + -0.46875 * eps4;
        CT M4 = 0.05859375 * eps4;

        CT L_psi1 = a * c1_e2 * (M0 * psi1
                                  + M2 * sin(2*psi1)
                                  + M4 * sin(4*psi1));

        CT L_psi2 = a * c1_e2 * (M0 * psi2
                                  + M2 * sin(2*psi2)
                                  + M4 * sin(4*psi2));

        return L_psi2 - L_psi1;
    }

    // from Technical Report:
    // PAUL D. THOMAS, MATHEMATICAL MODELS FOR NAVIGATION SYSTEMS, 1965
    // http://www.dtic.mil/docs/citations/AD0627893
    template <typename T, typename Spheroid>
    static inline T apply(T lon1, T lat1, T lon2, T lat2, Spheroid const& spheroid)
    {
/*
        // If the segment is meridian
        // TODO: do not use manual Order
        // maybe throw an exception here?
        typedef typename formula::elliptic_meridian_arc_length
                <
                    CT, 4
                > elliptic_meridian_arc_length;

        typename elliptic_meridian_arc_length::result res =
                 elliptic_meridian_arc_length::apply(lon1, lat1, lon2, lat2, spheroid);

        if (res.meridian)
        {
            return res.distance;
        }
*/
        CT const c0 = 0;
        CT const c1 = 1;
        CT const c2 = 2;
        CT const pi =  math::pi<CT>();
        CT const f = formula::flattening<CT>(spheroid);
        CT const one_minus_f = c1 - f;
        CT const a = get_radius<0>(spheroid);
/*
        CT const theta1 = math::equals(lat1, pi_half) ? lat1 :
                          math::equals(lat1, -pi_half) ? lat1 :
                          atan(one_minus_f * tan(lat1));
        CT const theta2 = math::equals(lat2, pi_half) ? lat2 :
                          math::equals(lat2, -pi_half) ? lat2 :
                          atan(one_minus_f * tan(lat2));
*/
        CT const theta1 = atan(one_minus_f * tan(lat1));
        CT const theta2 = atan(one_minus_f * tan(lat2));

        CT const dlon = lon2 - lon1;
        CT const cosdl = cos(dlon);

        CT const cos_th1 = cos(theta1);
        CT const cos_th2 = cos(theta2);

        // Equatorial segment
        if (math::equals(lat1, c0) && math::equals(lat2, c0))
        {
            CT const sin_th1 = sin(theta1);
            CT const sin_th2 = sin(theta2);
            CT const d1_d2 = acos(sin_th1 * sin_th2 + cos_th1 * cos_th2 * cosdl);
            return a * d1_d2;
        }

        CT const sindl = sin(dlon);

        CT const tan_th1 = tan(theta1);
        CT const tan_th2 = tan(theta2);

        CT const A = tan_th1 - tan_th2 * cosdl;
        CT const B = tan_th2 - tan_th1 * cosdl;
        CT const K = (A * tan_th1 + B * tan_th2) / (sindl * sindl);
        CT const sin_th0_sq = K/(K+c1);

        CT const cos2d1 = c2 * (c1 - cos_th1 * cos_th1) / sin_th0_sq - c1;
        CT const cos2d2 = c2 * (c1 - cos_th2 * cos_th2) / sin_th0_sq - c1;

        CT const d1 = -acos(cos2d1) / c2;
        CT d2 = (lat1 * lat2 > 0) ? acos(cos2d2) / c2
                                  : pi - acos(cos2d2) / c2;

        CT const H = d1 + d2;
        CT const P = d1 - d2;

        CT const e2 = f * (CT(2) - f);
        CT const k2 = sin_th0_sq * e2;
        CT const N1 = k2 / 4;
        CT const N2 = N1 * N1 / 8;
        CT const N3 = N1 * N2 / 3;

        CT const Q1 = sin(H) * cos(P);
        CT const Q2 = sin(2*H) * cos(2*P);
        CT const Q3 = sin(3*H) * cos(3*P);

        CT const U1 = -N1 * (H - Q1);
        CT const U2 = -N2 * (6*H - 8*Q1 + Q2);
        CT const U3 = -N3 * (30*H - 45*Q1 + 9*Q2 - Q3);

        CT const s = a * (H + U1 + U2 + U3);

        return math::abs(s);
    }
};

}}} // namespace boost::geometry::formula


#endif // BOOST_GEOMETRY_FORMULAS_ELLIPTIC_ARC_LENGTH_HPP
