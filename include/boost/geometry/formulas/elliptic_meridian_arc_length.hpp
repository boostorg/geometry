// Boost.Geometry

// Copyright (c) 2017 Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_ELLIPTIC_MERIDIAN_ARC_LENGTH_HPP
#define BOOST_GEOMETRY_FORMULAS_ELLIPTIC_MERIDIAN_ARC_LENGTH_HPP

#include <boost/math/constants/constants.hpp>

#include <boost/geometry/core/radius.hpp>

#include <boost/geometry/util/condition.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/normalize_spheroidal_coordinates.hpp>

#include <boost/geometry/formulas/flattening.hpp>

namespace boost { namespace geometry { namespace formula
{

/*!
\brief Compute the arc length of a meridian ellipse.
*/

template <typename CT, unsigned int Order = 1>
class elliptic_meridian_arc_length
{

public :

    struct result
    {
        result()
            : distance(0)
            , meridian(false)
        {}

        CT distance;
        bool meridian;
    };

    template <typename T, typename Spheroid>
    static result apply(T lon1, T lat1, T lon2, T lat2, Spheroid const& spheroid)
    {
        result res;

        CT c0 = 0;
        CT pi = math::pi<CT>();
        CT half_pi = pi/CT(2);
        CT diff = geometry::math::longitude_distance_signed<geometry::radian>(lon1, lon2);

        if (lat1 > lat2)
        {
            std::swap(lat1, lat2);
        }

        if ( math::equals(diff, c0) ||
            (math::equals(lat2, half_pi) && math::equals(lat1, -half_pi)) )
        {
            // single meridian not crossing pole
            res.distance = apply(lat2, spheroid) - apply(lat1, spheroid);
            res.meridian = true;
        }

        if (math::equals(math::abs(diff), pi))
        {
            // meridian crosses pole
            CT lat_sign = 1;
            if (lat1+lat2 < c0)
            {
                lat_sign = CT(-1);
            }
            res.distance = math::abs(lat_sign * CT(2) * apply(half_pi, spheroid)
                               - apply(lat1, spheroid) - apply(lat2, spheroid));
            res.meridian = true;
        }
        return res;
    }

    // Distance computation on meridians using series approximations
    // to elliptic integrals. Formula to compute distance from lattitude 0 to lat
    // cf. https://en.wikipedia.org/wiki/Meridian_arc
    // latitudes are assumed to be in radians and in [-pi/2,pi/2]

    //https://en.wikipedia.org/wiki/Meridian_arc#Expansions_in_the_third_flattening_(n)
    /* Maxima code to generate the series
        ataylor(expr,var,ord):=expand(ratdisrep(taylor(expr,var,0,ord)));
        int:(1-e2*sin(phi)^2)^(-3/2);
        maxpow:6;
        intexp:ataylor(subst([e2=4*n/((1+n)^2)],int),n,maxpow);
        m:trigreduce(integrate(intexp,phi));
        m:a*(1-4*n/((1+n)^2))*m;
        m:m-subst(phi=0,m);
        m:ataylor(m*(1+n)/a,n,maxpow);
    */
    template <typename T, typename Spheroid>
    static CT apply(T lat, Spheroid const& spheroid)
    {
        CT const a = get_radius<0>(spheroid);
        CT const f = formula::flattening<CT>(spheroid);

        CT n = f / (CT(2) - f);
        CT M = a/(1+n);
        CT C0 = 1;

        if (Order == 0)
        {
           return M * C0 * lat;
        }

        CT C2 = -CT(3)/CT(2) * n;

        if (Order == 1)
        {
            return M * (C0 * lat + C2 * sin(2*lat));
        }

        CT n2 = n * n;
        C0 += CT(1)/CT(4) * n2;
        CT C4 = CT(15)/CT(16) * n2;

        if (Order == 2)
        {
            return M * (C0 * lat + C2 * sin(2*lat) + C4 * sin(4*lat));
        }

        CT n3 = n2 * n;
        C2 += CT(3)/CT(16) * n3;
        CT C6 = -CT(35)/CT(48) * n3;

        if (Order == 3)
        {
            return M * (C0 * lat + C2 * sin(2*lat) + C4 * sin(4*lat)
                      + C6 * sin(6*lat));
        }

        CT n4 = n2 * n2;
        C0 += CT(1)/CT(64) * n4;
        C4 -= CT(15)/CT(64) * n4;
        CT C8 = CT(315)/CT(512) * n4;

        if (Order == 4)
        {
            return M * (C0 * lat + C2 * sin(2*lat) + C4 * sin(4*lat)
                      + C6 * sin(6*lat) + C8 * sin(8*lat));
        }

        CT n5 = n4 * n;
        C2 += CT(3)/CT(128) * n5;
        C6 += CT(175)/CT(768) * n5;
        CT C10 = -CT(693)/CT(1280) * n5;

        if (Order == 5)
        {
            return M * (C0 * lat + C2 * sin(2*lat) + C4 * sin(4*lat)
                      + C6 * sin(6*lat) + C8 * sin(8*lat) + C10 * sin(10*lat));
        }

        CT n6 = n5 * n;
        C0 += CT(1)/CT(256) * n6;
        C4 -= CT(75)/CT(2048) * n6;
        C8 -= CT(441)/CT(2048) * n6;
        CT C12 = CT(1001)/CT(2048) * n6;

        // Order 6 or higher
        return M * (C0 * lat + C2 * sin(2*lat) + C4 * sin(4*lat)
                  + C6 * sin(6*lat) + C8 * sin(8*lat) + C10 * sin(10*lat)
                  + C12 * sin(12*lat));
    }

    //https://en.wikipedia.org/wiki/Meridian_arc#Series_in_terms_of_the_parametric_latitude
    template <typename T, typename Spheroid>
    static CT apply_par(T lat, Spheroid const& spheroid)
    {
        CT const a = get_radius<0>(spheroid);
        CT const f = formula::flattening<CT>(spheroid);

        CT const bet = lat;

        CT n = f / (CT(2) - f);
        CT M = a/(1+n);
        CT C0 = 1;

        if (Order == 0)
        {
           return M * C0 * bet;
        }

        CT C2 = -CT(1)/CT(2) * n;

        if (Order == 1)
        {
            return M * (C0 * bet + C2 * sin(2*bet));
        }

        CT n2 = n * n;
        C0 += CT(1)/CT(4) * n2;
        CT C4 = -CT(1)/CT(16) * n2;

        if (Order == 2)
        {
            return M * (C0 * bet + C2 * sin(2*bet) + C4 * sin(4*bet));
        }

        CT n3 = n2 * n;
        C2 += CT(1)/CT(16) * n3;
        CT C6 = -CT(1)/CT(48) * n3;

        if (Order == 3)
        {
            return M * (C0 * bet + C2 * sin(2*bet) + C4 * sin(4*bet)
                      + C6 * sin(6*bet));
        }

        CT n4 = n2 * n2;
        C0 += CT(1)/CT(64) * n4;
        C4 += CT(1)/CT(64) * n4;
        CT C8 = -CT(5)/CT(512) * n4;

        //order 4 or higher
        return M * (C0 * bet + C2 * sin(2*bet) + C4 * sin(4*bet)
                  + C6 * sin(6*bet) + C8 * sin(8*bet));
    }
};

}}} // namespace boost::geometry::formula


#endif // BOOST_GEOMETRY_FORMULAS_ELLIPTIC_MERIDIAN_ARC_LENGTH_HPP
