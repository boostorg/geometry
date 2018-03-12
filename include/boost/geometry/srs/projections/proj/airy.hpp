// Boost.Geometry - gis-projections (based on PROJ4)

// Copyright (c) 2008-2015 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2017, 2018.
// Modifications copyright (c) 2017-2018, Oracle and/or its affiliates.
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

// Purpose:  Implementation of the airy (Airy) projection.
// Author:   Gerald Evenden (1995)
//           Thomas Knudsen (2016) - revise/add regression tests
// Copyright (c) 1995, Gerald Evenden

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

#ifndef BOOST_GEOMETRY_PROJECTIONS_AIRY_HPP
#define BOOST_GEOMETRY_PROJECTIONS_AIRY_HPP

#include <boost/geometry/util/math.hpp>

#include <boost/geometry/srs/projections/impl/base_static.hpp>
#include <boost/geometry/srs/projections/impl/base_dynamic.hpp>
#include <boost/geometry/srs/projections/impl/projects.hpp>
#include <boost/geometry/srs/projections/impl/factory_entry.hpp>

#include <boost/geometry/srs/projections/par4.hpp> // airy tag

namespace boost { namespace geometry
{

namespace srs { namespace par4
{
    // already defined in par4.hpp as ellps name
    //struct airy {};

}} //namespace srs::par4

namespace projections
{
    #ifndef DOXYGEN_NO_DETAIL
    namespace detail { namespace airy
    {

            static const double EPS = 1.e-10;
            enum Mode {
                N_POLE = 0,
                S_POLE = 1,
                EQUIT  = 2,
                OBLIQ  = 3
            };

            template <typename T>
            struct par_airy
            {
                T    p_halfpi;
                T    sinph0;
                T    cosph0;
                T    Cb;
                enum Mode mode;
                int  no_cut;    /* do not cut at hemisphere limit */
            };

            // template class, using CRTP to implement forward/inverse
            template <typename CalculationType, typename Parameters>
            struct base_airy_spheroid : public base_t_f<base_airy_spheroid<CalculationType, Parameters>,
                     CalculationType, Parameters>
            {

                typedef CalculationType geographic_type;
                typedef CalculationType cartesian_type;

                par_airy<CalculationType> m_proj_parm;

                inline base_airy_spheroid(const Parameters& par)
                    : base_t_f<base_airy_spheroid<CalculationType, Parameters>,
                     CalculationType, Parameters>(*this, par) {}

                // FORWARD(s_forward)  spheroid
                // Project coordinates from geographic (lon, lat) to cartesian (x, y)
                inline void fwd(geographic_type& lp_lon, geographic_type& lp_lat, cartesian_type& xy_x, cartesian_type& xy_y) const
                {
                    static const CalculationType HALFPI = detail::HALFPI<CalculationType>();

                    CalculationType  sinlam, coslam, cosphi, sinphi, t, s, Krho, cosz;

                    sinlam = sin(lp_lon);
                    coslam = cos(lp_lon);
                    switch (this->m_proj_parm.mode) {
                    case EQUIT:
                    case OBLIQ:
                        sinphi = sin(lp_lat);
                        cosphi = cos(lp_lat);
                        cosz = cosphi * coslam;
                        if (this->m_proj_parm.mode == OBLIQ)
                            cosz = this->m_proj_parm.sinph0 * sinphi + this->m_proj_parm.cosph0 * cosz;
                        if (!this->m_proj_parm.no_cut && cosz < -EPS) {
                            BOOST_THROW_EXCEPTION( projection_exception(-20) );
                        }
                        if (fabs(s = 1. - cosz) > EPS) {
                            t = 0.5 * (1. + cosz);
                            Krho = -log(t)/s - this->m_proj_parm.Cb / t;
                        } else
                            Krho = 0.5 - this->m_proj_parm.Cb;
                        xy_x = Krho * cosphi * sinlam;
                        if (this->m_proj_parm.mode == OBLIQ)
                            xy_y = Krho * (this->m_proj_parm.cosph0 * sinphi -
                                this->m_proj_parm.sinph0 * cosphi * coslam);
                        else
                            xy_y = Krho * sinphi;
                        break;
                    case S_POLE:
                    case N_POLE:
                        lp_lat = fabs(this->m_proj_parm.p_halfpi - lp_lat);
                        if (!this->m_proj_parm.no_cut && (lp_lat - EPS) > HALFPI)
                            BOOST_THROW_EXCEPTION( projection_exception(-20) );
                        if ((lp_lat *= 0.5) > EPS) {
                            t = tan(lp_lat);
                            Krho = -2.*(log(cos(lp_lat)) / t + t * this->m_proj_parm.Cb);
                            xy_x = Krho * sinlam;
                            xy_y = Krho * coslam;
                            if (this->m_proj_parm.mode == N_POLE)
                                xy_y = -xy_y;
                        } else
                            xy_x = xy_y = 0.;
                    }
                }

                static inline std::string get_name()
                {
                    return "airy_spheroid";
                }

            };

            // Airy
            template <typename Parameters, typename T>
            inline void setup_airy(Parameters& par, par_airy<T>& proj_parm)
            {
                static const T HALFPI = detail::HALFPI<T>();

                T beta;

                proj_parm.no_cut = pj_param(par.params, "bno_cut").i;
                beta = 0.5 * (HALFPI - pj_param(par.params, "rlat_b").f);
                if (fabs(beta) < EPS)
                    proj_parm.Cb = -0.5;
                else {
                    proj_parm.Cb = 1./tan(beta);
                    proj_parm.Cb *= proj_parm.Cb * log(cos(beta));
                }

                if (fabs(fabs(par.phi0) - HALFPI) < EPS)
                    if (par.phi0 < 0.) {
                        proj_parm.p_halfpi = -HALFPI;
                        proj_parm.mode = S_POLE;
                    } else {
                        proj_parm.p_halfpi =  HALFPI;
                        proj_parm.mode = N_POLE;
                    }
                else {
                    if (fabs(par.phi0) < EPS)
                        proj_parm.mode = EQUIT;
                    else {
                        proj_parm.mode = OBLIQ;
                        proj_parm.sinph0 = sin(par.phi0);
                        proj_parm.cosph0 = cos(par.phi0);
                    }
                }
                par.es = 0.;
            }

    }} // namespace detail::airy
    #endif // doxygen

    /*!
        \brief Airy projection
        \ingroup projections
        \tparam Geographic latlong point type
        \tparam Cartesian xy point type
        \tparam Parameters parameter type
        \par Projection characteristics
         - Miscellaneous
         - Spheroid
         - no inverse
        \par Projection parameters
         - no_cut: Do not cut at hemisphere limit (boolean)
         - lat_b (degrees)
        \par Example
        \image html ex_airy.gif
    */
    template <typename CalculationType, typename Parameters>
    struct airy_spheroid : public detail::airy::base_airy_spheroid<CalculationType, Parameters>
    {
        inline airy_spheroid(const Parameters& par) : detail::airy::base_airy_spheroid<CalculationType, Parameters>(par)
        {
            detail::airy::setup_airy(this->m_par, this->m_proj_parm);
        }
    };

    #ifndef DOXYGEN_NO_DETAIL
    namespace detail
    {

        // Static projection
        BOOST_GEOMETRY_PROJECTIONS_DETAIL_STATIC_PROJECTION(srs::par4::airy, airy_spheroid, airy_spheroid)

        // Factory entry(s)
        template <typename CalculationType, typename Parameters>
        class airy_entry : public detail::factory_entry<CalculationType, Parameters>
        {
            public :
                virtual base_v<CalculationType, Parameters>* create_new(const Parameters& par) const
                {
                    return new base_v_f<airy_spheroid<CalculationType, Parameters>, CalculationType, Parameters>(par);
                }
        };

        template <typename CalculationType, typename Parameters>
        inline void airy_init(detail::base_factory<CalculationType, Parameters>& factory)
        {
            factory.add_to_factory("airy", new airy_entry<CalculationType, Parameters>);
        }

    } // namespace detail
    #endif // doxygen

} // namespace projections

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_PROJECTIONS_AIRY_HPP
