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

#ifndef BOOST_GEOMETRY_PROJECTIONS_PUTP2_HPP
#define BOOST_GEOMETRY_PROJECTIONS_PUTP2_HPP

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
    namespace detail { namespace putp2
    {

            static const double C_x = 1.89490;
            static const double C_y = 1.71848;
            static const double C_p = 0.6141848493043784;
            static const double epsilon = 1e-10;
            static const int n_iter = 10;
            //static const double third_pi = 1.0471975511965977;

            // Close to the poles, where theta tends to pi/3 and the Newton iteration converges
            // only linearly: with theta = pi/3 - t and F(theta) = theta + sin(theta) * (cos(theta) - 1),
            // so that C_p = F(pi/3),
            // F(pi/3) - F(pi/3 - t) = t + cos(2pi/3 - t) * sin(t) - 2 * cos(pi/3 - t/2) * sin(t/2)
            template <typename T>
            inline T putp2_polar_d(T const& t)
            {
                static const T third_pi = detail::third_pi<T>();
                return t + cos(2. * third_pi - t) * sin(t) - 2. * cos(third_pi - 0.5 * t) * sin(0.5 * t);
            }

            template <typename T, typename Parameters>
            struct base_putp2_spheroid
            {
                // FORWARD(s_forward)  spheroid
                // Project coordinates from geographic (lon, lat) to cartesian (x, y)
                inline void fwd(Parameters const& , T const& lp_lon, T lp_lat, T& xy_x, T& xy_y) const
                {
                    static const T third_pi = detail::third_pi<T>();

                    static const T half_pi = detail::half_pi<T>();

                    T p, c, s, V;
                    int i;

                    if (fabs(lp_lat) > half_pi - 0.1) {
                        // solve F(pi/3) - F(pi/3 - t) = C_p * (1 - sin|phi|) = 2 * C_p * sin(chi / 2)^2
                        T const sh = sin(0.5 * (half_pi - fabs(lp_lat)));
                        T const w = 2. * C_p * sh * sh;
                        T t = sqrt(w / (0.25 * sqrt(3.))); // D(t) ~ sqrt(3)/4 * t^2
                        for (i = 0; i < 30; ++i) {
                            // derivative of D: cos(theta) * (2 * cos(theta) - 1),
                            // with 2 * cos(theta) - 1 = 4 * sin(pi/3 - t/2) * sin(t/2)
                            T const d = cos(third_pi - t) * 4. * sin(third_pi - 0.5 * t) * sin(0.5 * t);
                            if (d == 0.)
                                break;
                            T const dt = (putp2_polar_d(t) - w) / d;
                            t -= dt;
                            if (t < 0.)
                                t = 0.;
                            // D(t) carries a rounding error of about eps * t, so also stop on an
                            // absolute step
                            if (fabs(dt) <= 1e-12 * t || fabs(dt) < 1e-15)
                                break;
                        }
                        // cos(theta) - 1/2 = 2 * sin(pi/3 - t/2) * sin(t/2)
                        xy_x = C_x * lp_lon * 2. * sin(third_pi - 0.5 * t) * sin(0.5 * t);
                        xy_y = C_y * sin(third_pi - t);
                        if (lp_lat < 0.)
                            xy_y = -xy_y;
                        return;
                    }

                    p = C_p * sin(lp_lat);
                    s = lp_lat * lp_lat;
                    lp_lat *= 0.615709 + s * ( 0.00909953 + s * 0.0046292 );
                    for (i = n_iter; i ; --i) {
                        c = cos(lp_lat);
                        s = sin(lp_lat);
                        lp_lat -= V = (lp_lat + s * (c - 1.) - p) /
                            (1. + c * (c - 1.) - s * s);
                        if (fabs(V) < epsilon)
                            break;
                    }
                    if (!i)
                        lp_lat = lp_lat < 0 ? - third_pi : third_pi;
                    xy_x = C_x * lp_lon * (cos(lp_lat) - 0.5);
                    xy_y = C_y * sin(lp_lat);
                }

                // INVERSE(s_inverse)  spheroid
                // Project coordinates from cartesian (x, y) to geographic (lon, lat)
                inline void inv(Parameters const& , T const& xy_x, T const& xy_y, T& lp_lon, T& lp_lat) const
                {
                    static const T half_pi = detail::half_pi<T>();
                    static const T third_pi = detail::third_pi<T>();

                    T c;

                    lp_lat = aasin(xy_y / C_y);
                    if (fabs(lp_lat) > third_pi - 0.5) {
                        // Close to the poles: theta = +/-(pi/3 - t), with the colatitude chi from
                        // 2 * C_p * sin(chi / 2)^2 = F(pi/3) - F(pi/3 - t)
                        T t = third_pi - fabs(lp_lat);
                        if (t < 0.)
                            t = 0.;
                        T const den = C_x * 2. * sin(third_pi - 0.5 * t) * sin(0.5 * t);
                        lp_lon = den == 0. ? T(0) : T(xy_x / den);
                        T const phi = half_pi - 2. * asin(sqrt(0.5 * putp2_polar_d(t) / C_p));
                        lp_lat = xy_y < 0. ? T(-phi) : phi;
                        return;
                    }
                    lp_lon = xy_x / (C_x * ((c = cos(lp_lat)) - 0.5));
                    lp_lat = aasin((lp_lat + sin(lp_lat) * (c - 1.)) / C_p);
                }

                static inline std::string get_name()
                {
                    return "putp2_spheroid";
                }

            };

            // Putnins P2
            template <typename Parameters>
            inline void setup_putp2(Parameters& par)
            {
                par.es = 0.;
            }

    }} // namespace detail::putp2
    #endif // doxygen

    /*!
        \brief Putnins P2 projection
        \ingroup projections
        \tparam T calculation type
        \tparam Parameters parameter type
        \par Projection characteristics
         - Pseudocylindrical
         - Spheroid
        \par Example
        \image html ex_putp2.gif
    */
    template <typename T, typename Parameters>
    struct putp2_spheroid : public detail::putp2::base_putp2_spheroid<T, Parameters>
    {
        template <typename Params>
        inline putp2_spheroid(Params const& , Parameters & par)
        {
            detail::putp2::setup_putp2(par);
        }
    };

    #ifndef DOXYGEN_NO_DETAIL
    namespace detail
    {

        // Static projection
        BOOST_GEOMETRY_PROJECTIONS_DETAIL_STATIC_PROJECTION_FI(srs::spar::proj_putp2, putp2_spheroid)

        // Factory entry(s)
        BOOST_GEOMETRY_PROJECTIONS_DETAIL_FACTORY_ENTRY_FI(putp2_entry, putp2_spheroid)

        BOOST_GEOMETRY_PROJECTIONS_DETAIL_FACTORY_INIT_BEGIN(putp2_init)
        {
            BOOST_GEOMETRY_PROJECTIONS_DETAIL_FACTORY_INIT_ENTRY(putp2, putp2_entry)
        }

    } // namespace detail
    #endif // doxygen

} // namespace projections

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_PROJECTIONS_PUTP2_HPP

