// Boost.Geometry

// Copyright (c) 2017 Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Based on code for GSoC17 project "Filtering of compare distance predicates"
// by Ruoyun Jing, Northwest University, China.
// See https://github.com/BoostGSoC17/geometry/wiki

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_ANDOYER_INVERSE_ALT_HPP
#define BOOST_GEOMETRY_FORMULAS_ANDOYER_INVERSE_ALT_HPP

#include <boost/math/constants/constants.hpp>

#include <boost/geometry/core/radius.hpp>
#include <boost/geometry/srs/srs.hpp>

#include <boost/geometry/util/condition.hpp>
#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/normalize_spheroidal_coordinates.hpp>

#include <boost/geometry/formulas/flattening.hpp>
#include <boost/geometry/formulas/elliptic_meridian_arc_length.hpp>

namespace boost { namespace geometry { namespace formula
{

/*!
\brief Andoyer inverse laternative formula
*/
template <typename CT>
class andoyer_inverse_alt
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

        CT const c0 = CT(0);
        CT const c1 = CT(1);
        CT const c2 = CT(2);
        CT const c3 = CT(3);
        if ( math::equals(lon1, lon2) && math::equals(lat1, lat2) )
        {
            result.distance = c0;
            return result;
        }

        CT const a = get_radius<0>(spheroid);
        CT const f = formula::flattening<CT>(spheroid);

        CT const F = (lat1 + lat2) / c2;
        CT const G = (lat1 - lat2) / c2;
        CT const L = (lon1 - lon2) / c2;

        CT const sin_g = sin(G);
        CT const sin_l = sin(L);
        CT const sin_f = sin(F);
        CT const cos_g = cos(G);
        CT const cos_l = cos(L);
        CT const cos_f = cos(F);

        CT const S = math::sqr(sin_g) * math::sqr(cos_l)
                + math::sqr(cos_f) * math::sqr(sin_l);
        CT const C = math::sqr(cos_g) * math::sqr(cos_l)
                + math::sqr(sin_f) * math::sqr(sin_l);
        CT const W = atan(math::sqrt(S / C));
        CT const R = sqrt(S * C) / W;
        CT const D = c2 * W * a;
        CT const H_1 = (c3 * R - c1) / (c2 * C);
        CT const H_2 = (c3 * R + c1) / (c2 * S);

        result.distance =  D * (c1 + f * H_1 * math::sqr(sin_f) * math::sqr(cos_g)
                                - f * H_2 * math::sqr(cos_f) * math::sqr(sin_g));

        return result;
    }
};

}}} // namespace boost::geometry::formula


#endif // BOOST_GEOMETRY_FORMULAS_ANDOYER_INVERSE_ALT_HPP
