// Boost.Geometry

// Copyright (c) 2017 Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Based on code for GSoC17 project "Filtering of compare distance predicates"
// by Ruoyun Jing, Northwest University, China.
// See https://github.com/BoostGSoC17/geometry/wiki

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_LAMBERT_INVERSE_HPP
#define BOOST_GEOMETRY_FORMULAS_LAMBERT_INVERSE_HPP

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
\brief Lambert inverse formula
*/
template <typename CT>
class lambert_inverse
{

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

        CT const c4 = CT(4);
        CT const c1 = CT(1);
        CT const a = get_radius<0>(spheroid);
        CT const f = formula::flattening<CT>(spheroid);

        CT const sin_lat1 = sin(lat1);
        CT const cos_lat1 = cos(lat1);
        CT const sin_lat2 = sin(lat2);
        CT const cos_lat2 = cos(lat2);

        //CT const phi_1 = atan((c1 - spher_f) * tan(lat1));
        //CT const phi_2 = atan((c1 - spher_f) * tan(la2));
        CT const P = math::sqr(sin_lat1 + sin_lat2);
        CT const Q = math::sqr(sin_lat1 - sin_lat2);

        CT const theta = acos(sin_lat1 * sin_lat2 +
                              cos_lat1 * cos_lat2 * cos(lon1 - lon2));
        CT const X = (theta - sin(theta)) /
                c4 * (c1 + cos(theta));
        CT const Y = (theta + sin(theta)) /
                c4 * (c1 - cos(theta));

        result.distance = a * (theta - f * (P * X + Q * Y));

        return result;
    }
};

}}} // namespace boost::geometry::formula


#endif // BOOST_GEOMETRY_FORMULAS_LAMBERT_INVERSE_HPP
