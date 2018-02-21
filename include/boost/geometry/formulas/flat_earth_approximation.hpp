// Boost.Geometry

// Copyright (c) 2017 Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Based on code for GSoC17 project "Filtering of compare distance predicates"
// by Ruoyun Jing, Northwest University, China.
// See https://github.com/BoostGSoC17/geometry/wiki

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_FLAT_EARTH_APPROXIMATION_HPP
#define BOOST_GEOMETRY_FORMULAS_FLAT_EARTH_APPROXIMATION_HPP

#include <boost/math/constants/constants.hpp>

#include <boost/geometry/core/radius.hpp>

#include <boost/geometry/util/condition.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/normalize_spheroidal_coordinates.hpp>

#include <boost/geometry/formulas/flattening.hpp>
#include <boost/geometry/formulas/elliptic_meridian_arc_length.hpp>

namespace boost { namespace geometry { namespace formula
{

/*!
\brief Computations on a flat approximation of spheroidal Earth model
*/
template <
    typename CT,
    bool EnableDistance,
    bool EnableAzimuth,
    bool EnableReverseAzimuth = false,
    bool EnableReducedLength = false,
    bool EnableGeodesicScale = false
>
class flat_earth_approximation
{
    static const bool CalcQuantities = EnableReducedLength || EnableGeodesicScale;
    static const bool CalcFwdAzimuth = EnableAzimuth || CalcQuantities;
    static const bool CalcRevAzimuth = EnableReverseAzimuth || CalcQuantities;

public :

    typedef result_inverse<CT> result_type;

    template <typename T, typename Spheroid>
    static inline result_type apply(T lon1,
                                    T lat1,
                                    T lon2,
                                    T lat2,
                                    Spheroid const& spheroid)
    {
        result_type result;

        CT const c1 = 1;
        CT const c2 = 2;
        CT const pi = math::pi<CT>();
        CT const f = formula::flattening<CT>(spheroid);
        CT const e2 = f * (c2 - f);

        CT const dlat = lat2 - lat1;
        CT const dlon = lon2 - lon1;

        CT const sin_lat1 = sin(lat1);
        CT const sin2_lat1 = math::sqr(sin_lat1);
        CT const sin_lat_e2 = c1 - e2 * sin2_lat1;
        CT const sin_lat_e2_sqrt = math::sqrt(sin_lat_e2);

        CT const cos_lat1 = cos(lat1);
        CT const r = get_radius<0>(spheroid);

        // radius r for given latitude
        //CT const b = get_radius<2>(spheroid);
        //CT const a_cos_lat1 = a * cos_lat1;
        //CT const b_sin_lat1 = b * sin_lat1;
        //CT const r =
        // math::sqrt((math::sqr(a * a_cos_lat1) + math::sqr(b * b_sin_lat1))
        //          / (math::sqr(a * cos_lat1) + math::sqr(b * sin_lat1)));

        CT const R1 = r * (c1 - e2) / sin_lat_e2_sqrt * sin_lat_e2;
        CT const R2 = r / sin_lat_e2_sqrt;

        CT const dis_North = R1 * dlat;
        CT const dis_East = R2 * cos_lat1 * dlon;

        if ( BOOST_GEOMETRY_CONDITION(EnableDistance) )
        {
            result.distance = math::sqrt(math::sqr(dis_North) + math::sqr(dis_East));
        }
        if (BOOST_GEOMETRY_CONDITION(CalcFwdAzimuth))
        {
            result.azimuth = std::fmod(atan2(dis_East, dis_North), c2 * pi);
        }
        if (BOOST_GEOMETRY_CONDITION(CalcRevAzimuth))
        {
            CT const sin_lat2 = sin(lat2);
            CT const sin2_lat2 = math::sqr(sin_lat2);
            CT const sin_lat_e2 = c1 - e2 * sin2_lat2;
            CT const sin_lat_e2_sqrt = math::sqrt(sin_lat_e2);

            CT const R1 = r * (c1 - e2) / sin_lat_e2_sqrt * sin_lat_e2;
            CT const R2 = r / sin_lat_e2_sqrt;

            CT const dis_North = R1 * -dlat;
            CT const dis_East = R2 * cos(lat2) * -dlon;

            result.reverse_azimuth = pi + std::fmod(atan2(dis_East, dis_North), c2 * pi);
        }
        if (BOOST_GEOMETRY_CONDITION(CalcQuantities))
        {
            typedef differential_quantities<CT,
                                            EnableReducedLength,
                                            EnableGeodesicScale,
                                            1> quantities;
            quantities::apply(lon1, lat1, lon2, lat2,
                              result.azimuth, result.reverse_azimuth,
                              get_radius<2>(spheroid), f,
                              result.reduced_length, result.geodesic_scale);
        }
        return result;
    }
};

}}} // namespace boost::geometry::formula


#endif // BOOST_GEOMETRY_FORMULAS_FLAT_EARTH_APPROXIMATION_HPP
