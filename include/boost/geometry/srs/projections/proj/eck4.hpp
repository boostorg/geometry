// Boost.Geometry - gis-projections (based on PROJ4)

// Copyright (c) 2008-2015 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2017, 2018, 2019.
// Modifications copyright (c) 2017-2019, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// This file is converted from PROJ4, http://trac.osgeo.org/proj
// PROJ4 is originally written by Gerald Evenden (then of the USGS)
// PROJ4 is maintained by Frank Warmerdam
// PROJ4 is converted to Boost.Geometry by Barend Gehrels

// Last updated version of proj: 5.0.0

// Original copyright notice:

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.

#ifndef BOOST_GEOMETRY_PROJECTIONS_ECK4_HPP
#define BOOST_GEOMETRY_PROJECTIONS_ECK4_HPP

#include <boost/geometry/srs/projections/impl/base_static.hpp>
#include <boost/geometry/srs/projections/impl/base_dynamic.hpp>
#include <boost/geometry/srs/projections/impl/projects.hpp>
#include <boost/geometry/srs/projections/impl/factory_entry.hpp>
#include <boost/geometry/srs/projections/impl/aasincos.hpp>

namespace boost { namespace geometry
{

namespace projections
{
    #ifndef DOXYGEN_NO_DETAIL
    namespace detail { namespace eck4
    {

            static const double C_x = .42223820031577120149;
            static const double C_y = 1.32650042817700232218;
            static const double RC_y = .75386330736002178205;
            static const double C_p = 3.57079632679489661922;
            static const double RC_p = .28004957675577868795;
            static const double epsilon = 1e-7;
            static const int n_iter = 6;

            template <typename T, typename Parameters>
            struct base_eck4_spheroid
            {
                // FORWARD(s_forward)  spheroid
                // Project coordinates from geographic (lon, lat) to cartesian (x, y)
                inline void fwd(Parameters const& , T const& lp_lon, T lp_lat, T& xy_x, T& xy_y) const
                {
                    static const T half_pi = detail::half_pi<T>();

                    T p, V, s, c;
                    int i;

                    if (fabs(lp_lat) > half_pi - 0.1) {
                        // Close to the poles the Newton iteration below converges only linearly.
                        // With theta = pi/2 - t and chi the colatitude, solve
                        // t - sin(2t)/2 + 2 * (1 - cos(t)) = 2 * C_p * sin(chi / 2)^2
                        T const sh = sin(0.5 * (half_pi - fabs(lp_lat)));
                        T const w = 2. * C_p * sh * sh;
                        T t = sqrt(w); // >= the root, as the left side is >= t^2
                        for (i = 0; i < 30; ++i) {
                            T const st = sin(t);
                            T const d = 2. * st * (1. + st);
                            if (d == 0.)
                                break;
                            T const sh2 = sin(0.5 * t);
                            T const dt = (0.5 * s_minus_sin_s(T(2. * t)) + 4. * sh2 * sh2 - w) / d;
                            t -= dt;
                            if (fabs(dt) <= 1e-12 * t) // quadratic convergence
                                break;
                        }
                        // cos(theta) = sin(t), sin(theta) = cos(t)
                        xy_x = C_x * lp_lon * (1. + sin(t));
                        xy_y = lp_lat < 0. ? T(-C_y * cos(t)) : T(C_y * cos(t));
                        return;
                    }

                    p = C_p * sin(lp_lat);
                    V = lp_lat * lp_lat;
                    lp_lat *= 0.895168 + V * ( 0.0218849 + V * 0.00826809 );
                    for (i = n_iter; i ; --i) {
                        c = cos(lp_lat);
                        s = sin(lp_lat);
                        lp_lat -= V = (lp_lat + s * (c + 2.) - p) /
                            (1. + c * (c + 2.) - s * s);
                        if (fabs(V) < epsilon)
                            break;
                    }
                    if (!i) {
                        xy_x = C_x * lp_lon;
                        xy_y = lp_lat < 0. ? -C_y : C_y;
                    } else {
                        xy_x = C_x * lp_lon * (1. + cos(lp_lat));
                        xy_y = C_y * sin(lp_lat);
                    }
                }

                // INVERSE(s_inverse)  spheroid
                // Project coordinates from cartesian (x, y) to geographic (lon, lat)
                inline void inv(Parameters const& , T const& xy_x, T const& xy_y, T& lp_lon, T& lp_lat) const
                {
                    static const T half_pi = detail::half_pi<T>();

                    T c;

                    if (fabs(xy_y) > 0.5 * C_y && fabs(xy_y) <= C_y) {
                        // Close to the poles: theta = pi/2 - t, with the colatitude chi from
                        // 2 * C_p * sin(chi / 2)^2 = t - sin(2t)/2 + 2 * (1 - cos(t))
                        T const t = 2. * asin(sqrt(0.5 * (C_y - fabs(xy_y)) * RC_y));
                        T const sh = sin(0.5 * t);
                        T const w = 0.5 * s_minus_sin_s(T(2. * t)) + 4. * sh * sh;
                        lp_lon = xy_x / (C_x * (1. + sin(t)));
                        lp_lat = half_pi - 2. * asin(sqrt(0.5 * w * RC_p));
                        if (xy_y < 0.)
                            lp_lat = -lp_lat;
                        return;
                    }

                    lp_lat = aasin(xy_y * RC_y);
                    lp_lon = xy_x / (C_x * (1. + (c = cos(lp_lat))));
                    lp_lat = aasin((lp_lat + sin(lp_lat) * (c + 2.)) * RC_p);
                }

                static inline std::string get_name()
                {
                    return "eck4_spheroid";
                }

            };

            // Eckert IV
            template <typename Parameters>
            inline void setup_eck4(Parameters& par)
            {
                par.es = 0.;
            }

    }} // namespace detail::eck4
    #endif // doxygen

    /*!
        \brief Eckert IV projection
        \ingroup projections
        \tparam T calculation type
        \tparam Parameters parameter type
        \par Projection characteristics
         - Pseudocylindrical
         - Spheroid
        \par Example
        \image html ex_eck4.gif
    */
    template <typename T, typename Parameters>
    struct eck4_spheroid : public detail::eck4::base_eck4_spheroid<T, Parameters>
    {
        template <typename Params>
        inline eck4_spheroid(Params const& , Parameters & par)
        {
            detail::eck4::setup_eck4(par);
        }
    };

    #ifndef DOXYGEN_NO_DETAIL
    namespace detail
    {

        // Static projection
        BOOST_GEOMETRY_PROJECTIONS_DETAIL_STATIC_PROJECTION_FI(srs::spar::proj_eck4, eck4_spheroid)

        // Factory entry(s)
        BOOST_GEOMETRY_PROJECTIONS_DETAIL_FACTORY_ENTRY_FI(eck4_entry, eck4_spheroid)

        BOOST_GEOMETRY_PROJECTIONS_DETAIL_FACTORY_INIT_BEGIN(eck4_init)
        {
            BOOST_GEOMETRY_PROJECTIONS_DETAIL_FACTORY_INIT_ENTRY(eck4, eck4_entry);
        }

    } // namespace detail
    #endif // doxygen

} // namespace projections

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_PROJECTIONS_ECK4_HPP

