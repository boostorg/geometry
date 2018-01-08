// Boost.Geometry

// Copyright (c) 2017 Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_FORMULAS_ELLIPTIC_MERIDIAN_ARC_DIRECT_HPP
#define BOOST_GEOMETRY_FORMULAS_ELLIPTIC_MERIDIAN_ARC_DIRECT_HPP

#include <boost/math/constants/constants.hpp>

#include <boost/geometry/core/radius.hpp>
#include <boost/geometry/core/srs.hpp>

#include <boost/geometry/util/condition.hpp>
#include <boost/geometry/util/math.hpp>

#include <boost/geometry/formulas/flattening.hpp>


namespace boost { namespace geometry { namespace formula
{

/*!
\brief Compute the direct geodesic problem on a meridian
*/

template <typename CT, unsigned int Order = 1>
class elliptic_meridian_arc_direct
{

public :

    // https://en.wikipedia.org/wiki/Meridian_arc#The_inverse_meridian_problem_for_the_ellipsoid
    // latitudes are assumed to be in radians and in [-pi/2,pi/2]
    template <typename T, typename Spheroid>
    static CT apply(T m, Spheroid const& spheroid)
    {
        CT const f = formula::flattening<CT>(spheroid);
        CT n = f / (CT(2) - f);
        CT mp = 10001965.729;
        CT mu = geometry::math::pi<CT>()/CT(2) * m / mp;

        if (Order == 0)
        {
            return mu;
        }

        CT H2 = 1.5 * n;

        if (Order == 1)
        {
            return mu + H2 * sin(2*mu);
        }

        CT n2 = n * n;
        CT H4 = 1.3125 * n2;

        if (Order == 2)
        {
            return mu + H2 * sin(2*mu) + H4 * sin(4*mu);
        }

        CT n3 = n2 * n;
        H2 -= 0.84375 * n3;
        CT H6 = 1.572916667 * n3;

        if (Order == 3)
        {
            return mu + H2 * sin(2*mu) + H4 * sin(4*mu) + H6 * sin(6*mu);
        }

        CT n4 = n2 * n2;
        H4 -= 1.71875 * n4;
        CT H8 = 2.142578125 * n4;

        // Order 4 or higher
        return mu + H2 * sin(2*mu) + H4 * sin(4*mu) + H6 * sin(6*mu) + H8 * sin(8*mu);
    }
};

}}} // namespace boost::geometry::formula

#endif // BOOST_GEOMETRY_FORMULAS_ELLIPTIC_MERIDIAN_ARC_DIRECT_HPP
