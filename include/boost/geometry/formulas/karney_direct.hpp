// Boost.Geometry

// Contributed and/or modified by Adeel Ahmad.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_KARNEY_DIRECT_HPP
#define BOOST_GEOMETRY_FORMULAS_KARNEY_DIRECT_HPP


#include <boost/math/constants/constants.hpp>
#include <boost/math/special_functions/hypot.hpp>

#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/series_expansion.hpp>

#include <boost/geometry/formulas/flattening.hpp>
#include <boost/geometry/formulas/result_direct.hpp>


namespace boost { namespace geometry { namespace formula
{


/*!
\brief The solution of the direct problem of geodesics on latlong coordinates,
       after Karney (2011).
\author See
- Charles F.F Karney, Algorithms for geodesics, 2011
https://arxiv.org/pdf/1109.4448.pdf
*/
template <
    typename CT,
    size_t SeriesOrder = 8,
    bool EnableCoordinates = true,
    bool EnableReverseAzimuth = false,
    bool EnableReducedLength = false,
    bool EnableGeodesicScale = false
>
class karney_direct
{
    static const bool CalcQuantities = EnableReducedLength || EnableGeodesicScale;
    static const bool CalcCoordinates = EnableCoordinates || CalcQuantities;
    static const bool CalcRevAzimuth = EnableReverseAzimuth || CalcCoordinates || CalcQuantities;

public:
    typedef result_direct<CT> result_type;

    /*
     Evaluate

     y = sum(c[i] * sin(2*i * x), i, 1, n)

     using Clenshaw summation.
    */
    static inline CT sin_cos_series(CT sinx,
                                    CT cosx,
                                    const CT coeffs[])
    {
        size_t n = SeriesOrder;

        // Point to one beyond last element.
        coeffs += (n + 1);
        CT ar = 2 * (cosx - sinx) * (cosx + sinx);

        CT k0 = n & 1 ? *--coeffs : 0;
        CT k1 = 0;

        // Make n even.
        n /= 2;
        while (n--) {
          // Unroll loop x 2, so accumulators return to their original role.
          k1 = ar * k0 - k1 + *--coeffs;
          k0 = ar * k1 - k0 + *--coeffs;
        }

        return 2 * sinx * cosx * k0;
    }


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
        Azi const azi12 = math::normalize_angle<CT>(azimuth12);

        if (math::equals(distance, Dist(0)) || distance < Dist(0))
        {
            result.lon2 = lon1;
            result.lat2 = lat1;
            return result;
        }

        CT const c1 = 1;
        CT const c2 = 2;

        CT const b = CT(get_radius<2>(spheroid));
        CT const f = formula::flattening<CT>(spheroid);
        CT const one_minus_f = c1 - f;
        CT const two_minus_f = c2 - f;

        CT const n = f / two_minus_f;
        CT const e2 = f * two_minus_f;
        CT const ep2 = e2 / math::sqr(one_minus_f);

        CT sin_alpha1, cos_alpha1;
        math::sin_cos_degrees<CT>(math::round_angle<CT>(azi12), sin_alpha1, cos_alpha1);

        // Find the reduced latitude.
        CT sin_beta1, cos_beta1;
        math::sin_cos_degrees<CT>(math::round_angle<CT>(lat1), sin_beta1, cos_beta1);
        sin_beta1 *= one_minus_f;

        math::normalize<CT>(sin_beta1, cos_beta1);
        cos_beta1 = std::max(sqrt(std::numeric_limits<CT>::min()), cos_beta1);

        // Obtain alpha 0 by solving the spherical triangle.
        CT const sin_alpha0
            = sin_alpha1 * cos_beta1;
        CT const cos_alpha0
            = boost::math::hypot(cos_alpha1, sin_alpha1 * sin_beta1);

        CT const k2 = math::sqr(cos_alpha0) * ep2;

        CT const epsilon = k2 / (c2 * (c1 + sqrt(c1 + k2)) + k2);

        // Find the coefficients for A1 by computing the
        // series expansion using Horner scehme.
        CT const expansion_A1
            = series_expansion::evaluate_series_A1<CT, SeriesOrder>(epsilon);

        // Index zero element of coeffs_C1 is unused.
        CT coeffs_C1[SeriesOrder + 1];
        series_expansion::evaluate_coeffs_C1<CT, SeriesOrder>(epsilon, coeffs_C1);

        // Tau is an integration variable.
        CT const tau12 = distance / (b * (c1 + expansion_A1));

        CT const sin_tau12 = sin(tau12);
        CT const cos_tau12 = cos(tau12);

        CT sin_sigma1 = sin_beta1;
        CT sin_omega1 = sin_alpha0 * sin_beta1;

        CT cos_sigma1, cos_omega1;
        cos_sigma1 = cos_omega1 = sin_beta1 != 0 || cos_alpha1 != 0 ? cos_beta1 * cos_alpha1 : 1;
        math::normalize<CT>(sin_sigma1, cos_sigma1);

        CT const B11 = sin_cos_series(sin_sigma1, cos_sigma1, coeffs_C1);
        CT const sin_B11 = sin(B11);
        CT const cos_B11 = cos(B11);

        CT const sin_tau1
            = sin_sigma1 * cos_B11 + cos_sigma1 * sin_B11;
        CT const cos_tau1
            = cos_sigma1 * cos_B11 - sin_sigma1 * sin_B11;

        // Index zero element of coeffs_C1p is unused.
        CT coeffs_C1p[SeriesOrder + 1];
        series_expansion::evaluate_coeffs_C1p<CT, SeriesOrder>(epsilon, coeffs_C1p);

        CT const B12 = - sin_cos_series(sin_tau1 * cos_tau12 + cos_tau1 * sin_tau12,
                                      cos_tau1 * cos_tau12 - sin_tau1 * sin_tau12,
                                      coeffs_C1p);

        CT const sigma12 = tau12 - (B12 - B11);
        CT const sin_sigma12 = sin(sigma12);
        CT const cos_sigma12 = cos(sigma12);

        CT const sin_sigma2
            = sin_sigma1 * cos_sigma12 + cos_sigma1 * sin_sigma12;
        CT const cos_sigma2
            = cos_sigma1 * cos_sigma12 - sin_sigma1 * sin_sigma12;

        if (BOOST_GEOMETRY_CONDITION(CalcRevAzimuth))
        {
            CT const sin_alpha2 = sin_alpha0;
            CT const cos_alpha2 = cos_alpha0 * cos_sigma2;

            result.reverse_azimuth = atan2(sin_alpha2, cos_alpha2);

            // Convert the angle to radians.
            result.reverse_azimuth /= math::d2r<T>();
        }

        if (BOOST_GEOMETRY_CONDITION(CalcCoordinates))
        {
            // Find the latitude at the second point.
            CT const sin_beta2
                = cos_alpha0 * sin_sigma2;
            CT const cos_beta2
                = boost::math::hypot(sin_alpha0, cos_alpha0 * cos_sigma2);

            result.lat2 = atan2(sin_beta2, one_minus_f * cos_beta2);

            // Convert the coordinate to radians.
            result.lat2 /= math::d2r<T>();

            // Find the longitude at the second point.
            CT const sin_omega2 = sin_alpha0 * sin_sigma2;
            CT const cos_omega2 = cos_sigma2;

            CT const omega12 = atan2(sin_omega2 * cos_omega1 - cos_omega2 * sin_omega1,
                                     cos_omega2 * cos_omega1 + sin_omega2 * sin_omega1);

            CT coeffs_A3[SeriesOrder];
            series_expansion::evaluate_coeffs_A3<double, SeriesOrder>(n, coeffs_A3);

            CT const A3 = math::horner_evaluate(epsilon, coeffs_A3, coeffs_A3 + SeriesOrder);
            CT const A3c = -f * sin_alpha0 * A3;

            // Compute the size of coefficient array.
            size_t const coeffs_C3_size = (SeriesOrder * (SeriesOrder - 1)) / 2;
            CT coeffs_C3x[coeffs_C3_size];
            series_expansion::evaluate_coeffs_C3x<double, SeriesOrder>(n, coeffs_C3x);

            // Evaluate C3 coefficients.
            CT coeffs_C3[SeriesOrder];
            series_expansion::evaluate_coeffs_C3<double, SeriesOrder>(epsilon, coeffs_C3, coeffs_C3x);

            CT const B31 = sin_cos_series(sin_sigma1, cos_sigma1, coeffs_C3);

            CT const lam12 = omega12 + A3c *
                             (sigma12 + (sin_cos_series(sin_sigma2,
                                                        cos_sigma2,
                                                        coeffs_C3) - B31));

            // Convert to radians to get the
            // longitudinal difference.
            CT const lon12 = lam12 / math::d2r<T>();

            // Add the longitude at first point to the longitudinal
            // difference and normalize the result.
            result.lon2 = math::normalize_angle(math::normalize_angle(lon1) +
                                                math::normalize_angle(lon12));
        }

        if (BOOST_GEOMETRY_CONDITION(CalcQuantities))
        {
            // Evaluate the coefficients for C2.
            // Index zero element of coeffs_C2 is unused.
            CT coeffs_C2[SeriesOrder + 1];
            series_expansion::evaluate_coeffs_C2<CT, SeriesOrder>(epsilon, coeffs_C2);

            CT const B21 = sin_cos_series(sin_sigma1, cos_sigma1, coeffs_C2);
            CT const B22 = sin_cos_series(sin_sigma2, cos_sigma2, coeffs_C2);

            // Find the coefficients for A2 by computing the
            // series expansion using Horner scehme.
            CT const expansion_A2
                = series_expansion::evaluate_series_A2<CT, SeriesOrder>(epsilon);

            CT const AB1 = (c1 + expansion_A1) * (B12 - B11);
            CT const AB2 = (c1 + expansion_A2) * (B22 - B21);
            CT const J12 = (expansion_A1 - expansion_A2) * sigma12 + (AB1 - AB2);

            CT const dn1 = sqrt(1 + ep2 * math::sqr(sin_beta1));
            CT const dn2 = sqrt(1 + k2 * math::sqr(sin_sigma2));

            // Find the reduced length.
            result.reduced_length = b * ((dn2 * (cos_sigma1 * sin_sigma2) -
                                          dn1 * (sin_sigma1 * cos_sigma2)) -
                                          cos_sigma1 * cos_sigma2 * J12);

            // Find the geodesic scale.
            CT const t = k2 * (sin_sigma2 - sin_sigma1) *
                              (sin_sigma2 * sin_sigma1) / (dn1 + dn2);

            result.geodesic_scale = cos_sigma12 +
                                    (t * sin_sigma2 - cos_sigma2 * J12) *
                                    sin_sigma1 / dn1;
        }

        return result;
    }
};

}}} // namespace boost::geometry::formula


#endif // BOOST_GEOMETRY_FORMULAS_KARNEY_DIRECT_HPP
