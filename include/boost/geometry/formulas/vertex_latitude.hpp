// Boost.Geometry

// Copyright (c) 2016 Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_MAXIMUM_LATITUDE_HPP
#define BOOST_GEOMETRY_FORMULAS_MAXIMUM_LATITUDE_HPP

#include <boost/geometry/formulas/spherical.hpp>

namespace boost { namespace geometry { namespace formula
{

/*!
\brief Algorithm to compute the vertex latitude of a geodesic segment. Vertex is
a point on the geodesic that maximizes (or minimizes) the latitude.
\author See
    Wood - Vertex Latitudes on Ellipsoid Geodesics, SIAM Rev., 38(4), 637–644,
    1996
*/

template <typename CT>
class vertex_latitude
{

public:

    template <
            bool north,
            typename T1,
            typename T2
            >
    static inline CT spherical(T1 const& lon1,
                               T1 const& lat1,
                               T2 const& lon2,
                               T2 const& lat2)
    {
        CT const half_pi = math::pi<CT>() / CT(2);

        // signbit returns a non-zero value (true) if the sign is negative;
        // and zero (false) otherwise.
        bool sign = std::signbit(std::abs(lat1) > std::abs(lat2) ? lat1 : lat2);

        geometry::formula::result_spherical<CT> result = geometry::formula::
                spherical_azimuth<CT, true>(lon1, lat1, lon2, lat2);

        CT const alp1 = std::abs(result.azimuth);
        CT const alp2 = std::abs(result.reverse_azimuth);

        // if the segment does not contain the vertex of the geodesic
        // then return the endpoint of max (min) latitude
        if ((alp1 < half_pi && alp2 < half_pi)
                || (alp1 > half_pi && alp2 > half_pi)
                || (north == sign))
        {
            return north ? std::max(lat1, lat2) : std::min(lat1, lat2);
        }

        CT const cos2_lat1 = math::sqr(cos(lat1));
        CT const sin2_alp1 = math::sqr(sin(alp1));
        CT const vertex_lat = std::asin(math::sqrt(1 - cos2_lat1 * sin2_alp1));

        return (sign ? vertex_lat * CT(-1) : vertex_lat);
    }


    template <
            template <typename, bool, bool, bool, bool, bool> class Inverse,
            bool north,
            typename T1,
            typename T2,
            typename Spheroid
            >
    static inline CT geographic(T1 const& lon1,
                                T1 const& lat1,
                                T2 const& lon2,
                                T2 const& lat2,
                                Spheroid const& spheroid)
    {
        typedef Inverse<CT, false, true, true, false, false> inverse_type;
        typedef typename inverse_type::result_type inverse_result;

        CT const a = get_radius<0>(spheroid);
        CT const f = detail::flattening<CT>(spheroid);
        CT const e2 = f * (CT(2) - f);
        CT const half_pi = math::pi<CT>() / CT(2);

        // signbit returns a non-zero value (true) if the sign is negative;
        // and zero (false) otherwise.
        bool sign = std::signbit(std::abs(lat1) > std::abs(lat2) ? lat1 : lat2);

        inverse_result i_res = inverse_type::apply(lon1, lat1, lon2, lat2, spheroid);

        CT const alp1 = std::abs(i_res.azimuth);
        CT const alp2 = std::abs(i_res.reverse_azimuth);

        // if the segment does not contain the vertex of the geodesic
        // then return the endpoint of max (min) latitude
        if ((alp1 < half_pi && alp2 < half_pi)
                || (alp1 > half_pi && alp2 > half_pi)
                || (north == sign))
        {
            return north ? std::max(lat1, lat2) : std::min(lat1, lat2);
        }

        CT const sin_alp1 = sin(alp1);
        CT const sin2_lat1 = math::sqr(sin(lat1));
        CT const cos_lat1 = math::sqrt(CT(1) - sin2_lat1);

        // normal radius at point p1(lon1,lat1)
        CT const n_b1 = a / (math::sqrt(CT(1) - e2 * sin2_lat1));

        // the invariant of the geodesic
        CT const c = n_b1 * cos_lat1 * sin_alp1;

        CT const a_c2 = math::sqr(a / c);
        CT const vertex_lat = std::asin(math::sqrt((a_c2 - 1) / (a_c2 - e2)));

        return (sign ? vertex_lat * CT(-1) : vertex_lat);
    }
};


}}} // namespace boost::geometry::formula

#endif // BOOST_GEOMETRY_FORMULAS_MAXIMUM_LATITUDE_HPP
