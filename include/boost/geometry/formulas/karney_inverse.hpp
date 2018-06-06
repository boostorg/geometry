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

        CT const c1 = 1;
        CT const c2 = 2;
        CT const c180 = 180;
        CT const c90 = 90;

        // Compute the longitudinal difference.
        CT lon12_error;
        CT lon12 = math::difference_angle(lon1, lon2, lon12_error);

        int lon12_sign = lon12 >= 0 ? 1 : -1;

        // Make points close to the meridian to lie on it.
        lon12 = lon12_sign * math::round_angle(lon12);
        lon12_error = math::round_angle((c180 - lon12) - lon12_sign * lon12_error);

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
    }
};

}}} // namespace boost::geometry::formula


#endif // BOOST_GEOMETRY_FORMULAS_KARNEY_INVERSE_HPP
