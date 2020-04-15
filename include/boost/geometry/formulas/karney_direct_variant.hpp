// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fisikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_KARNEY_DIRECT_VARIANT_HPP
#define BOOST_GEOMETRY_FORMULAS_KARNEY_DIRECT_VARIANT_HPP


#include <boost/array.hpp>

#include <boost/math/constants/constants.hpp>
#include <boost/math/special_functions/hypot.hpp>

#include <boost/geometry/formulas/flattening.hpp>
#include <boost/geometry/formulas/result_direct.hpp>

#include <boost/geometry/util/condition.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/normalize_spheroidal_coordinates.hpp>
#include <boost/geometry/util/series_expansion.hpp>


namespace boost { namespace geometry { namespace formula
{

namespace se = series_expansion;

/*!
\brief The solution of a variant of direct problem of geodesics on latlong
       coordinates, where given is a point an azimuth and a latitude and
       we want to compute the corresponding longitude
\author See
- Charles F.F Karney, Algorithms for geodesics, 2011
https://arxiv.org/pdf/1109.4448.pdf
*/
template <
    typename CT,
    size_t SeriesOrder = 8
>
class karney_direct_variant
{

public:

    template <typename T, typename Azi, typename Spheroid>
    static inline CT apply(T const& lo1,
                           T const& la1,
                           T const& la2,
                           Azi const& azimuth12,
                           Spheroid const& spheroid)
    {
        CT lon1 = lo1;
        CT const lat1 = la1;
        CT const lat2 = la2;

        Azi azi12 = azimuth12;
        math::normalize_azimuth<degree, Azi>(azi12);

        CT const c0 = 0;
        CT const c1 = 1;
        CT const c2 = 2;

        CT const f = formula::flattening<CT>(spheroid);
        CT const one_minus_f = c1 - f;
        CT const two_minus_f = c2 - f;

        CT const n = f / two_minus_f;
        CT const e2 = f * two_minus_f;
        CT const ep2 = e2 / math::sqr(one_minus_f);

        CT sin_alpha1, cos_alpha1;
        math::sin_cos_degrees<CT>(azi12, sin_alpha1, cos_alpha1);

        // Find the reduced latitude.
        CT sin_beta1, cos_beta1;
        math::sin_cos_degrees<CT>(lat1, sin_beta1, cos_beta1);
        sin_beta1 *= one_minus_f;

        CT sin_beta2, cos_beta2;
        math::sin_cos_degrees<CT>(lat2, sin_beta2, cos_beta2);
        sin_beta2 *= one_minus_f;

        math::normalize_unit_vector<CT>(sin_beta1, cos_beta1);
        math::normalize_unit_vector<CT>(sin_beta2, cos_beta2);

        cos_beta1 = (std::max)(c0, cos_beta1);
        cos_beta2 = (std::max)(c0, cos_beta2);

        // Obtain alpha 0 by solving the spherical triangle.
        CT const sin_alpha0 = sin_alpha1 * cos_beta1;
        CT const cos_alpha0 = boost::math::hypot(cos_alpha1, sin_alpha1 * sin_beta1);

        CT const k2 = math::sqr(cos_alpha0) * ep2;

        CT const epsilon = k2 / (c2 * (c1 + math::sqrt(c1 + k2)) + k2);

        CT sin_sigma1 = sin_beta1;
        CT sin_omega1 = sin_alpha0 * sin_beta1;

        CT cos_sigma1, cos_omega1;
        cos_sigma1 = cos_omega1 = sin_beta1 != c0
                || cos_alpha1 != c0 ? cos_beta1 * cos_alpha1 : c1;
        math::normalize_unit_vector<CT>(sin_sigma1, cos_sigma1);


        CT const sin_alpha2 = sin_alpha0;

        CT sin_sigma2;
        CT cos_sigma2;

        if (cos_alpha0 != 0)
        {
            sin_sigma2 = sin_beta2 / cos_alpha0;
            cos_sigma2 = math::sqrt(cos_beta2 * cos_beta2
                                    - sin_alpha2 * sin_alpha2) / cos_alpha0;
        }
        else
        {
            sin_sigma2 = 1;
            cos_sigma2 = 0;
        }


        CT const sin_sigma12 = cos_sigma1 * sin_sigma2 - sin_sigma1 * cos_sigma2;
        CT const sigma12 = asin(sin_sigma12);

        CT const sin_omega2 = sin_alpha0 * sin_sigma2;
        CT const cos_omega2 = cos_sigma2;

        CT const omega12 = atan2(sin_omega2 * cos_omega1
                                 - cos_omega2 * sin_omega1,
                                 cos_omega2 * cos_omega1
                                 + sin_omega2 * sin_omega1);

        se::coeffs_A3<SeriesOrder, CT> const coeffs_A3(n);

        CT const A3 = math::horner_evaluate(epsilon,
                                            coeffs_A3.begin(),
                                            coeffs_A3.end());
        CT const A3c = -f * sin_alpha0 * A3;

        se::coeffs_C3<SeriesOrder, CT> const coeffs_C3(n, epsilon);

        CT const B31 = se::sin_cos_series(sin_sigma1, cos_sigma1, coeffs_C3);

        CT const lam12 = omega12 + A3c *
                (sigma12 + (se::sin_cos_series
                            (sin_sigma2,
                             cos_sigma2,
                             coeffs_C3) - B31));

        // Convert to radians to get the
        // longitudinal difference.
        CT lon12 = lam12 / math::d2r<CT>();

        // Add the longitude at first point to the longitudinal
        // difference and normalize the result.
        math::normalize_longitude<degree, CT>(lon1);
        math::normalize_longitude<degree, CT>(lon12);

        return lon1 + lon12;
    }
};

}}} // namespace boost::geometry::formula


#endif // BOOST_GEOMETRY_FORMULAS_KARNEY_DIRECT_VARIANT_HPP
