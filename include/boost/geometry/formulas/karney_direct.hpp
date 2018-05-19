// Boost.Geometry

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_KARNEY_DIRECT_HPP
#define BOOST_GEOMETRY_FORMULAS_KARNEY_DIRECT_HPP


#include <boost/math/constants/constants.hpp>

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
    std::size_t SeriesOrder = 8,
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
        std::size_t n = SeriesOrder;

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
        Azi azi12 = azimuth12;

        if (math::equals(distance, Dist(0)) || distance < Dist(0))
        {
            result.lon2 = lon1;
            result.lat2 = lat1;
            return result;
        }

        CT const c1 = 1;
        CT const c2 = 2;

        CT const a = CT(get_radius<0>(spheroid));
        CT const b = CT(get_radius<2>(spheroid));
        CT const f = formula::flattening<CT>(spheroid);
        CT const one_minus_f = c1 - f;
        CT const two_minus_f = c2 - f;

        CT const e2 = f * two_minus_f;
        CT const ep2 = e2 / math::sqr(one_minus_f);

        azi12 = math::normalize_angle<CT>(azi12);
        CT sin_alpha1, cos_alpha1;
        math::sin_cos_degrees<CT>(math::round_angle<CT>(azi12), sin_alpha1, cos_alpha1);

        // Find the reduced latitude.
        CT sin_beta1, cos_beta1;
        math::sin_cos_degrees<CT>(math::round_angle<CT>(lat1), sin_beta1, cos_beta1);
        sin_beta1 *= one_minus_f;

        // Obtain alpha 0 by solving the spherical triangle.
        CT cos_alpha0 = boost::math::hypot(cos_alpha1, sin_alpha1 * sin_beta1);

        CT k2 = math::sqr(cos_alpha0) * ep2;

        CT epsilon = k2 / (c2 * (c1 + std::sqrt(c1 + k2)) + k2);

        // Find the coefficients for Aj by computing the
        // series expansion using Horner scehme.
        CT expansion_A1 = series_expansion::evaluate_series_A1<CT, SeriesOrder>(epsilon);

        // Index zero element of coeffs_C1 is unused.
        CT coeffs_C1[SeriesOrder + 1];
        series_expansion::evaluate_coeffs_C1<CT, SeriesOrder>(epsilon, coeffs_C1);

        // Tau is an integration variable.
        CT tau12 = distance / (b + (c1 + expansion_A1));
        CT sin_tau12 = sin(tau12);
        CT cos_tau12 = cos(tau12);
    }

};

}}} // namespace boost::geometry::formula


#endif // BOOST_GEOMETRY_FORMULAS_KARNEY_DIRECT_HPP
