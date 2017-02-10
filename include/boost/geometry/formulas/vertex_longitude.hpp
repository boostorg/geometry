// Boost.Geometry

// Copyright (c) 2016 Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_MAXIMUM_LONGITUDE_HPP
#define BOOST_GEOMETRY_FORMULAS_MAXIMUM_LONGITUDE_HPP

#include <boost/geometry/formulas/spherical.hpp>
#include <boost/geometry/algorithms/detail/flattening.hpp>
#include <boost/geometry/core/srs.hpp>
#include <boost/mpl/assert.hpp>

#include <boost/math/special_functions/hypot.hpp>

namespace boost { namespace geometry { namespace formula
{

/*!
\brief Algorithm to compute the vertex longitude of a geodesic segment. Vertex is
a point on the geodesic that maximizes (or minimizes) the latitude. The algorithm
is given the vertex latitude.
*/

template <typename CT>
class vertex_longitude_on_sphere
{

public:

    template <typename T>
    static inline CT apply(T const lat1,
                           T const lat2,
                           T const lat3,
                           T const dlon,
                           T const)
    {
        //https://en.wikipedia.org/wiki/Great-circle_navigation#Finding_way-points
        CT const A = sin(lat1) * cos(lat2) * cos(lat3) * sin(dlon);
        CT const B = sin(lat1) * cos(lat2) * cos(lat3) * cos(dlon)
                   - cos(lat1) * sin(lat2) * cos(lat3);
        CT const dlon_max = atan2(B, A);
        return dlon_max;
    }
};

template <typename CT>
class vertex_longitude_on_spheroid
{

public:

    template<typename T>
    static inline void normalize(T& x, T& y)
    {
        T h = boost::math::hypot(x, y);
        x /= h;
        y /= h;
    }

    template <typename T>
    static inline CT apply(T const lat1,
                           T const lat2,
                           T const lat3,
                           T const,
                           T const alp1)
    {
        // Constants

        geometry::srs::spheroid<CT> spheroid;
        CT const f = detail::flattening<CT>(spheroid);

        // First, compute longitude on auxiliary sphere

        CT const bet1 = atan((CT(1) - f) * tan(lat1));
        CT const bet2 = atan((CT(1) - f) * tan(lat2));
        CT const bet3 = atan((CT(1) - f) * tan(lat3));

        CT const cos_bet1 = cos(bet1);
        CT const cos_bet2 = cos(bet2);
        CT const sin_bet1 = math::sqrt(CT(1) - math::sqr(cos_bet1));
        CT const sin_bet2 = math::sqrt(CT(1) - math::sqr(cos_bet2));

        CT const sin_alp1 = sin(alp1);
        CT const sin_alp0 = sin_alp1 * cos_bet1;
        CT const sin_alp2 = sin_alp1 * cos_bet1 / cos_bet2;

        CT const cos_alp0 = math::sqrt(CT(1) - math::sqr(sin_alp0));
        CT const cos_alp1 = math::sqrt(CT(1) - math::sqr(sin_alp1));
        CT const cos_alp2 = math::sqrt(CT(1) - math::sqr(sin_alp2));

        CT const sig1 = atan2(sin_bet1, cos_alp1 * cos_bet1);
        CT const sig2 = atan2(sin_bet2, cos_alp2 * cos_bet2);

        CT const cos_sig1 = cos(sig1);
        CT const sin_sig1 = math::sqrt(CT(1) - math::sqr(cos_sig1));

        CT const cos_sig2 = cos(sig2);
        CT const sin_sig2 = math::sqrt(CT(1) - math::sqr(cos_sig2));

        CT const omg1 = atan2(sin_alp0 * sin_sig1, cos_sig1);
        CT const omg2 = atan2(sin_alp0 * sin_sig2, cos_sig2);

        //version without atan2()
        //CT const omg1 = asin(tan(bet1) * sin_alp0 / cos_alp0);
        //CT const omg2 = asin(tan(bet2) * sin_alp0 / cos_alp0);

        CT const omg3 = vertex_longitude_on_sphere<CT>
                            ::apply(bet1, bet2, bet3, omg2 - omg1, CT());

        // Second, compute the ellipsodal longitude

        CT const e2 = f * (CT(2) - f);

        CT ep = math::sqrt(e2 / (CT(1.0) - e2));
        CT k2 = math::sqr(ep * cos_alp0);
        CT sqrt_k2_plus_one = math::sqrt(CT(1) + k2);
        CT eps = (sqrt_k2_plus_one - CT(1)) / (sqrt_k2_plus_one + CT(1));
        CT eps2 = eps * eps;
        CT n = f / (CT(2) - f);

        // Order 2 approximation
        CT c0 = 0.5;
        CT c1 = 0.25;
        CT c2 = 0.125;
        CT c3 = 0.0625;

        CT const A3 = 1 - (c0 - c0 * n) * eps - c1 * eps2;
        CT const C31 = (c1 - c1 * n) * eps + c2 * eps2;
        CT const C32 = c3 * eps2;

        CT const sin2_sig2 = CT(2) * cos_sig2 * sin_sig2;
        CT const sin4_sig2 = sin_sig2 * (-CT(4) * cos_sig2
                                      + CT(8) * cos_sig2 * cos_sig2 * cos_sig2);
        CT const I3 = A3 * (sig2 + C31 * sin2_sig2 + C32 * sin4_sig2);

        CT dlon_max = omg3 - f * sin_alp0 * I3;

        if (lat1 > lat2)
        {
            dlon_max -= math::pi<CT>();
        }

        return dlon_max;
    }

};


template <typename CT, typename CS_Tag>
struct vertex_longitude
{
    BOOST_MPL_ASSERT_MSG
         (
             false, NOT_IMPLEMENTED_FOR_THIS_COORDINATE_SYSTEM, (types<CS_Tag>)
         );

};

template <typename CT>
struct vertex_longitude<CT, spherical_equatorial_tag>
        : vertex_longitude_on_sphere<CT>
{};

template <typename CT>
struct vertex_longitude<CT, geographic_tag>
        : vertex_longitude_on_spheroid<CT>
{};


}}} // namespace boost::geometry::formula
#endif // BOOST_GEOMETRY_FORMULAS_MAXIMUM_LONGITUDE_HPP

