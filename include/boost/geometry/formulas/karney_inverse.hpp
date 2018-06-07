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
    static CT constexpr c1 = 1;
    static CT constexpr c2 = 2;
    static CT constexpr c3 = 3;
    static CT constexpr c180 = 180;
    static CT constexpr c90 = 90;

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

        CT tiny = std::sqrt(std::numeric_limits<CT>::min());

        CT const b = CT(get_radius<2>(spheroid));
        CT const f = formula::flattening<CT>(spheroid);
        CT const one_minus_f = c1 - f;
        CT const two_minus_f = c2 - f;

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
        // paper Algorithms for geodesics, Eq. (44):
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
        // sensitive measure of the |beta1| - |beta2|.  Alternatively,
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

        if (meridian)
        {
            // Endpoints lie on a single full meridian.

            // Point to the target latitude.
            CT cos_alpha1 = cos_lam12;
            CT sin_alpha1 = sin_lam12;

            // Heading north at the target.
            CT cos_alpha2 = 1;
            CT sin_alpha2 = 0;

            CT sin_sigma1 = sin_beta1;
            CT cos_sigma1 = cos_alpha1 * cos_beta1;

            CT sin_sigma2 = sin_beta2;
            CT cos_sigma2 = cos_alpha2 * cos_beta2;

            CT sigma12 = std::atan2(std::max(c0, cos_sigma1 * sin_sigma2 - sin_sigma1 * cos_sigma2),
                                             cos_sigma1 * cos_sigma2 + sin_sigma1 * sin_sigma2);

            meridian_length(n, ep2, sigma12, sin_sigma1, cos_sigma1, dn1,
                                             sin_sigma2, cos_sigma2, dn2,
                                             cos_beta1, cos_beta2, s12x,
                                             m12x, result.geodesic_scale, M21);


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
    }

    static inline void meridian_length(CT epsilon, CT ep2, CT sigma12,
                                       CT sin_sigma1, CT cos_sigma1, CT dn1,
                                       CT sin_sigma2, CT cos_sigma2, CT dn2,
                                       CT cos_beta1, CT cos_beta2,
                                       CT& s12x, CT& m12x,
                                       CT& M12, CT& M21)
    {
        CT A12x = 0, J12 = 0;
        CT expansion_A1, expansion_A2;

        // Index zero element of coeffs_C1 and coeffs_C2 is unused.
        CT coeffs_C1[SeriesOrder + 1];
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

};

}}} // namespace boost::geometry::formula


#endif // BOOST_GEOMETRY_FORMULAS_KARNEY_INVERSE_HPP
