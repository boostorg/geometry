// Boost.Geometry

// Contributed and/or modified by Adeel Ahmad.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_KARNEY_INVERSE_HPP
#define BOOST_GEOMETRY_FORMULAS_KARNEY_INVERSE_HPP


#include <boost/math/constants/constants.hpp>
#include <boost/math/special_functions/hypot.hpp>

#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/series_expansion.hpp>

#include <boost/geometry/formulas/flattening.hpp>
#include <boost/geometry/formulas/result_inverse.hpp>


namespace boost { namespace geometry { namespace formula
{


/*!
\brief The solution of the inverse problem of geodesics on latlong coordinates,
       after Karney (2011).
\author See
- Charles F.F Karney, Algorithms for geodesics, 2011
https://arxiv.org/pdf/1109.4448.pdf
*/
template <
    typename CT,
    bool EnableDistance,
    bool EnableAzimuth,
    bool EnableReverseAzimuth = false,
    bool EnableReducedLength = false,
    bool EnableGeodesicScale = false,
    size_t SeriesOrder = 8
>
class karney_inverse
{
    static const bool CalcQuantities = EnableReducedLength || EnableGeodesicScale;
    static const bool CalcAzimuths = EnableAzimuth || EnableReverseAzimuth || CalcQuantities;
    static const bool CalcFwdAzimuth = EnableAzimuth || CalcQuantities;
    static const bool CalcRevAzimuth = EnableReverseAzimuth || CalcQuantities;

public:
    typedef result_inverse<CT> result_type;
    static CT constexpr c0 = 0;
    static CT constexpr c0_001 = 0.001;
    static CT constexpr c0_01 = 0.01;
    static CT constexpr c0_1 = 0.1;
    static CT constexpr c0_5 = 0.5;
    static CT constexpr c1 = 1;
    static CT constexpr c2 = 2;
    static CT constexpr c3 = 3;
    static CT constexpr c6 = 6;
    static CT constexpr c90 = 90;
    static CT constexpr c180 = 180;
    static CT constexpr c200 = 200;

    template <typename T1, typename T2, typename Spheroid>
    static inline result_type apply(T1 const& lo1,
                                    T1 const& la1,
                                    T2 const& lo2,
                                    T2 const& la2,
                                    Spheroid const& spheroid)
    {
        result_type result;

        CT lat1 = la1;
        CT lat2 = la2;

        CT lon1 = lo1;
        CT lon2 = lo2;

        CT const a = CT(get_radius<0>(spheroid));
        CT const b = CT(get_radius<2>(spheroid));
        CT const f = formula::flattening<CT>(spheroid);
        CT const one_minus_f = c1 - f;
        CT const two_minus_f = c2 - f;

        CT const tol0 = std::numeric_limits<CT>::epsilon();
        CT const tol1 = c200 * tol0;
        CT const tol2 = sqrt(tol0);
        CT const etol2 = c0_1 * tol2 /
            sqrt(std::max(c0_001, std::abs(f)) * std::min(c1, c1 - f / c2) / c2);

        CT tiny = std::sqrt(std::numeric_limits<CT>::min());

        CT const n = f / two_minus_f;
        CT const e2 = f * two_minus_f;
        CT const ep2 = e2 / math::sqr(one_minus_f);

        // Compute the longitudinal difference.
        CT lon12_error;
        CT lon12 = math::difference_angle(lon1, lon2, lon12_error);

        int lon12_sign = lon12 >= 0 ? 1 : -1;

        // Make points close to the meridian to lie on it.
        lon12 = lon12_sign * math::round_angle(lon12);
        lon12_error = math::round_angle((c180 - lon12) - lon12_sign * lon12_error);

        // Convert to radians.
        CT lam12 = lon12 * math::d2r<CT>();
        CT sin_lam12;
        CT cos_lam12;

        lon12 > c90 ? math::sin_cos_degrees(lon12_error, sin_lam12, cos_lam12)
                    : math::sin_cos_degrees(lon12, sin_lam12, cos_lam12);

        // Make points close to the equator to lie on it.
        lat1 = std::abs(lat1) > 90 ? math::NaN<CT>() : lat1;
        lat2 = std::abs(lat2) > 90 ? math::NaN<CT>() : lat2;

        lat1 = math::round_angle(lat1);
        lat2 = math::round_angle(lat2);

        // Arrange points in a canonical form, as explained in
        // paper, Algorithms for geodesics, Eq. (44):
        //
        //     0 <= lon12 <= 180
        //     -90 <= lat1 <= 0
        //     lat1 <= lat2 <= -lat1
        int swap_point = std::abs(lat1) < std::abs(lat2) ? -1 : 1;

        if (swap_point < 0)
        {
            lon12_sign *= -1;
            swap(lat1, lat2);
        }

        // Enforce lat1 to be <= 0.
        int lat_sign = lat1 < 0 ? 1 : -1;
        lat1 *= lat_sign;
        lat2 *= lat_sign;

        CT sin_beta1, cos_beta1;
        math::sin_cos_degrees(lat1, sin_beta1, cos_beta1);
        sin_beta1 *= one_minus_f;

        math::normalize(sin_beta1, cos_beta1);
        cos_beta1 = std::max(tiny, cos_beta1);

        CT sin_beta2, cos_beta2;
        math::sin_cos_degrees(lat2, sin_beta2, cos_beta2);
        sin_beta2 *= one_minus_f;

        math::normalize(sin_beta2, cos_beta2);
        cos_beta2 = std::max(tiny, cos_beta2);

        // If cos_beta1 < -sin_beta1, then cos_beta2 - cos_beta1 is a
        // sensitive measure of the |beta1| - |beta2|. Alternatively,
        // (cos_beta1 >= -sin_beta1), abs(sin_beta2) + sin_beta1 is
        // a better measure.
        // Sometimes these quantities vanish and in that case we
        // force beta2 = +/- bet1a exactly.
        if (cos_beta1 < -sin_beta1)
        {
            if (cos_beta1 == cos_beta2)
            {
                sin_beta2 = sin_beta2 < 0 ? sin_beta1 :
                                            -sin_beta1;
            }
        }
        else
        {
            if (std::abs(sin_beta2) == -sin_beta1)
            {
                cos_beta2 = cos_beta1;
            }
        }

        CT const dn1 = sqrt(c1 + ep2 * math::sqr(sin_beta1));
        CT const dn2 = sqrt(c1 + ep2 * math::sqr(sin_beta2));

        CT a12, sigma12;
        CT m12x, s12x, M21;

        // Index zero element of coeffs_C1 is unused.
        CT coeffs_C1[SeriesOrder + 1];

        bool meridian = lat1 == -90 || sin_lam12 == 0;

        CT cos_alpha1, sin_alpha1;
        CT cos_alpha2, sin_alpha2;

        if (meridian)
        {
            // Endpoints lie on a single full meridian.

            // Point to the target latitude.
            cos_alpha1 = cos_lam12;
            sin_alpha1 = sin_lam12;

            // Heading north at the target.
            cos_alpha2 = 1;
            sin_alpha2 = 0;

            CT sin_sigma1 = sin_beta1;
            CT cos_sigma1 = cos_alpha1 * cos_beta1;

            CT sin_sigma2 = sin_beta2;
            CT cos_sigma2 = cos_alpha2 * cos_beta2;

            CT sigma12 = std::atan2(std::max(c0, cos_sigma1 * sin_sigma2 - sin_sigma1 * cos_sigma2),
                                             cos_sigma1 * cos_sigma2 + sin_sigma1 * sin_sigma2);

            CT dummy;
            meridian_length(n, ep2, sigma12, sin_sigma1, cos_sigma1, dn1,
                                             sin_sigma2, cos_sigma2, dn2,
                                             cos_beta1, cos_beta2, s12x,
                                             m12x, dummy, result.geodesic_scale,
                                             M21, coeffs_C1);


            if (sigma12 < c1 || m12x >= c0)
            {
                if (sigma12 < c3 * tiny)
                {
                    sigma12  = m12x = s12x = c0;
                }

                m12x *= b;
                s12x *= b;
                a12 = sigma12 / math::d2r<CT>();
            }
            else
            {
                // m12 < 0, i.e., prolate and too close to anti-podal.
                meridian = false;
            }
        }

        CT omega12, sin_omega12, cos_omega12;

        if (!meridian && sin_beta1 == 0 &&
            (f <= 0 || lon12_error >= f * c180))
        {
            // Points lie on the equator.
            cos_alpha1 = cos_alpha2 = c0;
            sin_alpha1 = sin_alpha2 = c1;

            s12x = a * lam12;
            sigma12 = omega12 = lam12 / one_minus_f;
            m12x = b * sin(sigma12);

            if (BOOST_GEOMETRY_CONDITION(EnableGeodesicScale))
            {
                result.geodesic_scale = cos(sigma12);
            }
            a12 = lon12 / one_minus_f;
        }

        else if (!meridian)
        {
            // If point1 and point2 belong within a hemisphere bounded by a
            // meridian and geodesic is neither meridional nor equatorial.

            // Find the starting point for Newton's method.
            CT dnm;
            sigma12 = newton_start(sin_beta1, cos_beta1, dn1,
                                 sin_beta2, cos_beta2, dn2,
                                 lam12, sin_lam12, cos_lam12,
                                 sin_alpha1, cos_alpha1,
                                 sin_alpha2, cos_alpha2,
                                 dnm, coeffs_C1, ep2, etol2, n, f);
        }
    }

    static inline void meridian_length(CT epsilon, CT ep2, CT sigma12,
                                       CT sin_sigma1, CT cos_sigma1, CT dn1,
                                       CT sin_sigma2, CT cos_sigma2, CT dn2,
                                       CT cos_beta1, CT cos_beta2,
                                       CT& s12x, CT& m12x, CT& m0,
                                       CT& M12, CT& M21, CT coeffs_C1[])
    {
        CT A12x = 0, J12 = 0;
        CT expansion_A1, expansion_A2;

        // Index zero element of coeffs_C2 is unused.
        CT coeffs_C2[SeriesOrder + 1];

        if (BOOST_GEOMETRY_CONDITION(EnableDistance) ||
            BOOST_GEOMETRY_CONDITION(EnableReducedLength) ||
            BOOST_GEOMETRY_CONDITION(EnableGeodesicScale))
        {
            // Find the coefficients for A1 by computing the
            // series expansion using Horner scehme.
            expansion_A1
                = series_expansion::evaluate_series_A1<CT, SeriesOrder>(epsilon);

            // Evaluate the coefficients for C1.
            series_expansion::evaluate_coeffs_C1<CT, SeriesOrder>(epsilon, coeffs_C1);

            if (BOOST_GEOMETRY_CONDITION(EnableReducedLength) ||
                BOOST_GEOMETRY_CONDITION(EnableGeodesicScale))
            {
                // Find the coefficients for A2 by computing the
                // series expansion using Horner scehme.
                expansion_A2
                    = series_expansion::evaluate_series_A2<CT, SeriesOrder>(epsilon);

                // Evaluate the coefficients for C2.
                series_expansion::evaluate_coeffs_C2<CT, SeriesOrder>(epsilon, coeffs_C2);

                A12x = expansion_A1 - expansion_A2;
                expansion_A2 += c1;
            }
            expansion_A1 += c1;
        }

        if (BOOST_GEOMETRY_CONDITION(EnableDistance))
        {
            CT B1 = series_expansion::sin_cos_series<CT, SeriesOrder>
                                      (sin_sigma2, cos_sigma2, coeffs_C1)
                  - series_expansion::sin_cos_series<CT, SeriesOrder>
                                      (sin_sigma1, cos_sigma1, coeffs_C1);

            m12x = expansion_A1 * (sigma12 + B1);

            if (BOOST_GEOMETRY_CONDITION(EnableReducedLength) ||
                BOOST_GEOMETRY_CONDITION(EnableGeodesicScale))
            {
                CT B2 = series_expansion::sin_cos_series<CT, SeriesOrder>
                                          (sin_sigma2, cos_sigma2, coeffs_C2)
                      - series_expansion::sin_cos_series<CT, SeriesOrder>
                                          (sin_sigma1, cos_sigma1, coeffs_C2);

                J12 = A12x * sigma12 + (expansion_A1 * B1 -
                                        expansion_A2 * B2);
            }
        }
        else if (BOOST_GEOMETRY_CONDITION(EnableReducedLength) ||
                 BOOST_GEOMETRY_CONDITION(EnableGeodesicScale))
        {
            for (size_t i = 1; i <= SeriesOrder; ++i)
            {
                coeffs_C2[i] = expansion_A1 * coeffs_C1[i] -
                               expansion_A2 * coeffs_C2[i];
            }

            J12 = A12x * sigma12 +
                   (series_expansion::sin_cos_series<CT, SeriesOrder>
                                      (sin_sigma2,
                                       cos_sigma2,
                                       coeffs_C2)
                  - series_expansion::sin_cos_series<CT, SeriesOrder>
                                      (sin_sigma1,
                                       cos_sigma1,
                                       coeffs_C2));
        }

        if (BOOST_GEOMETRY_CONDITION(EnableReducedLength))
        {
            m0 = A12x;

            m12x = dn2 * (cos_sigma1 * sin_sigma2) -
                   dn1 * (sin_sigma1 * cos_sigma2) -
                   cos_sigma1 * cos_sigma2 * J12;
        }

        if (BOOST_GEOMETRY_CONDITION(EnableGeodesicScale))
        {
            CT cos_sigma12 = cos_sigma1 * cos_sigma2 + sin_sigma1 * sin_sigma2;
            CT t = ep2 * (cos_beta1 - cos_beta2) *
                         (cos_beta1 + cos_beta2) / (dn1 + dn2);

            M12 = cos_sigma12 + (t * sin_sigma2 - cos_sigma2 * J12) * sin_sigma1 / dn1;
            M21 = cos_sigma12 - (t * sin_sigma1 - cos_sigma1 * J12) * sin_sigma2 / dn2;
        }
    }

    /*
     Return a starting point for Newton's method in sin_alpha1 and
     cos_alpha1 (function value is -1). If Newton's method
     doesn't need to be used, return also sin_alpha2 and
     cos_alpha2 and function value is sig12.
    */
    static inline CT newton_start(CT sin_beta1, CT cos_beta1, CT dn1,
                                  CT sin_beta2, CT cos_beta2, CT dn2,
                                  CT lam12, CT sin_lam12, CT cos_lam12,
                                  CT& sin_alpha1, CT& cos_alpha1,
                                  CT& sin_alpha2, CT& cos_alpha2,
                                  CT& dnm, CT coeffs_C1[], CT ep2,
                                  CT etol2, CT n, CT f)
    {
        CT const one_minus_f = c1 - f;
        CT sig12 = -c1;

        CT sin_beta12 = sin_beta2 * cos_beta1 - cos_beta2 * sin_beta1;
        CT cos_beta12 = cos_beta2 * cos_beta1 + sin_beta2 * sin_beta1;

        CT sin_beta12a = sin_beta2 * cos_beta1 + cos_beta2 * sin_beta1;

        bool shortline = cos_beta12 >= c0 && sin_beta12 < c0_5 &&
                         cos_beta2 * lam12 < c0_5;

        CT sin_omega12, cos_omega12;

        if (shortline)
        {
            CT sin_beta_m2 = math::sqr(sin_beta1 + sin_beta2);

            sin_beta_m2 /= sin_beta_m2 + math::sqr(cos_beta1 + cos_beta2);
            dnm = math::sqrt(c1 + ep2 * sin_beta_m2);

            CT omega12 = lam12 / (one_minus_f * dnm);

            sin_omega12 = sin(omega12);
            cos_omega12 = cos(omega12);
        }
        else
        {
            sin_omega12 = sin_lam12;
            cos_omega12 = cos_lam12;
        }

        sin_alpha1 = cos_beta2 * sin_omega12;
        // TODO: adl1995 - Resolve inaccuracy with
        // cos_alpha1 calculation.
        cos_alpha1 = cos_omega12 >= c0 ?
            sin_beta12 + cos_beta2 * sin_beta1 * math::sqr(sin_omega12) / (c1 + cos_omega12) :
            sin_beta12a - cos_beta2 * sin_beta1 * math::sqr(sin_omega12) / (c1 - cos_omega12);

        CT sin_sigma12 = boost::math::hypot(sin_alpha1, cos_alpha1);
        CT cos_sigma12 = sin_beta1 * sin_beta2 + cos_beta1 * cos_beta2 * cos_omega12;

        if (shortline && sin_sigma12 < etol2)
        {
            sin_alpha2 = cos_beta1 * sin_omega12;
            cos_alpha2 = sin_beta12 - cos_beta1 * sin_beta2 *
                (cos_omega12 >= c0 ? math::sqr(sin_omega12) /
                (c1 + cos_omega12) : c1 - cos_omega12);
        }
        // Skip astroid calculation if too eccentric.
        else if (std::abs(n) > c0_1 ||
                 cos_sigma12 >= c0 ||
                 sin_sigma12 >= c6 * std::abs(n) * math::pi<CT>() *
                 math::sqr(cos_beta1))
        {
            // Nothing to do (?).
        }
        else
        {
            // Scale lam12 and beta2 to x, y coordinate system where antipodal
            // point is at origin and singular point is at y = 0, x = -1.
            CT lambda_scale, beta_scale;

            CT y;
            volatile CT x;

            CT lam12x = atan2(-sin_lam12, -cos_lam12);
            if (f >= 0)
            {
                CT k2 = math::sqr(sin_beta1) * ep2;
                CT epsilon = k2 / (c2 * (c1 * sqrt(c1 + k2)) + k2);

                CT coeffs_A3[SeriesOrder];
                series_expansion::evaluate_coeffs_A3<double, SeriesOrder>(n, coeffs_A3);

                CT const A3 = math::horner_evaluate(epsilon, coeffs_A3, coeffs_A3 + SeriesOrder);

                lambda_scale = f * cos_beta1 * A3 * math::pi<CT>();

                beta_scale = lambda_scale * cos_beta1;

                x = lam12x / lambda_scale;
                y = sin_beta12a / beta_scale;
            }
            else
            {
                CT cos_beta12a = cos_beta2 * cos_beta1 - sin_beta2 * sin_beta1;
                CT beta12a = atan2(sin_beta12a, cos_beta12a);

                CT m12b, m0, dummy;
                meridian_length(n, ep2, math::pi<CT>() + beta12a,
                                sin_beta1, -cos_beta1, dn1,
                                sin_beta2, cos_beta2, dn2,
                                cos_beta1, cos_beta2, dummy,
                                m12b, m0, dummy, dummy, coeffs_C1);

                x = -c1 + m12b / (cos_beta1 * cos_beta2 * m0 * math::pi<CT>());
                beta_scale = x < -c0_01 ? sin_beta12a / x :
                    -f * math::sqr(cos_beta1) * math::pi<CT>();
                lambda_scale = beta_scale / cos_beta1;

                y = lam12x / lambda_scale;
            }
        }
    }
};

}}} // namespace boost::geometry::formula


#endif // BOOST_GEOMETRY_FORMULAS_KARNEY_INVERSE_HPP
