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

#ifndef BOOST_GEOMETRY_PROJECTIONS_FOUC_S_HPP
#define BOOST_GEOMETRY_PROJECTIONS_FOUC_S_HPP

#include <boost/geometry/util/math.hpp>

#include <boost/geometry/srs/projections/impl/base_static.hpp>
#include <boost/geometry/srs/projections/impl/base_dynamic.hpp>
#include <boost/geometry/srs/projections/impl/projects.hpp>
#include <boost/geometry/srs/projections/impl/factory_entry.hpp>
#include <boost/geometry/srs/projections/impl/aasincos.hpp>

namespace boost { namespace geometry
{

namespace srs { namespace par4
{
    struct fouc_s {}; // Foucaut Sinusoidal

}} //namespace srs::par4

namespace projections
{
    #ifndef DOXYGEN_NO_DETAIL
    namespace detail { namespace fouc_s
    {

            static const int MAX_ITER = 10;
            static const double LOOP_TOL = 1e-7;

            template <typename T>
            struct par_fouc_s
            {
                T n, n1;
            };

            // template class, using CRTP to implement forward/inverse
            template <typename CalculationType, typename Parameters>
            struct base_fouc_s_spheroid : public base_t_fi<base_fouc_s_spheroid<CalculationType, Parameters>,
                     CalculationType, Parameters>
            {

                typedef CalculationType geographic_type;
                typedef CalculationType cartesian_type;

                par_fouc_s<CalculationType> m_proj_parm;

                inline base_fouc_s_spheroid(const Parameters& par)
                    : base_t_fi<base_fouc_s_spheroid<CalculationType, Parameters>,
                     CalculationType, Parameters>(*this, par) {}

                // FORWARD(s_forward)  spheroid
                // Project coordinates from geographic (lon, lat) to cartesian (x, y)
                inline void fwd(geographic_type& lp_lon, geographic_type& lp_lat, cartesian_type& xy_x, cartesian_type& xy_y) const
                {
                    CalculationType t;

                    t = cos(lp_lat);
                    xy_x = lp_lon * t / (this->m_proj_parm.n + this->m_proj_parm.n1 * t);
                    xy_y = this->m_proj_parm.n * lp_lat + this->m_proj_parm.n1 * sin(lp_lat);
                }

                // INVERSE(s_inverse)  spheroid
                // Project coordinates from cartesian (x, y) to geographic (lon, lat)
                inline void inv(cartesian_type& xy_x, cartesian_type& xy_y, geographic_type& lp_lon, geographic_type& lp_lat) const
                {
                    static CalculationType const HALFPI = detail::HALFPI<CalculationType>();

                    CalculationType V;
                    int i;

                    if (this->m_proj_parm.n != 0.0) {
                        lp_lat = xy_y;
                        for (i = MAX_ITER; i ; --i) {
                            lp_lat -= V = (this->m_proj_parm.n * lp_lat + this->m_proj_parm.n1 * sin(lp_lat) - xy_y ) /
                                (this->m_proj_parm.n + this->m_proj_parm.n1 * cos(lp_lat));
                            if (fabs(V) < LOOP_TOL)
                                break;
                        }
                        if (!i)
                            lp_lat = xy_y < 0. ? -HALFPI : HALFPI;
                    } else
                        lp_lat = aasin(xy_y);
                    V = cos(lp_lat);
                    lp_lon = xy_x * (this->m_proj_parm.n + this->m_proj_parm.n1 * V) / V;
                }

                static inline std::string get_name()
                {
                    return "fouc_s_spheroid";
                }

            };

            // Foucaut Sinusoidal
            template <typename Parameters, typename T>
            inline void setup_fouc_s(Parameters& par, par_fouc_s<T>& proj_parm)
            {
                proj_parm.n = pj_param(par.params, "dn").f;
                if (proj_parm.n < 0. || proj_parm.n > 1.)
                    BOOST_THROW_EXCEPTION( projection_exception(-40) );

                proj_parm.n1 = 1. - proj_parm.n;
                par.es = 0;
            }

    }} // namespace detail::fouc_s
    #endif // doxygen

    /*!
        \brief Foucaut Sinusoidal projection
        \ingroup projections
        \tparam Geographic latlong point type
        \tparam Cartesian xy point type
        \tparam Parameters parameter type
        \par Projection characteristics
         - Pseudocylindrical
         - Spheroid
        \par Projection parameters
         - n (real)
        \par Example
        \image html ex_fouc_s.gif
    */
    template <typename CalculationType, typename Parameters>
    struct fouc_s_spheroid : public detail::fouc_s::base_fouc_s_spheroid<CalculationType, Parameters>
    {
        inline fouc_s_spheroid(const Parameters& par) : detail::fouc_s::base_fouc_s_spheroid<CalculationType, Parameters>(par)
        {
            detail::fouc_s::setup_fouc_s(this->m_par, this->m_proj_parm);
        }
    };

    #ifndef DOXYGEN_NO_DETAIL
    namespace detail
    {

        // Static projection
        BOOST_GEOMETRY_PROJECTIONS_DETAIL_STATIC_PROJECTION(srs::par4::fouc_s, fouc_s_spheroid, fouc_s_spheroid)

        // Factory entry(s)
        template <typename CalculationType, typename Parameters>
        class fouc_s_entry : public detail::factory_entry<CalculationType, Parameters>
        {
            public :
                virtual base_v<CalculationType, Parameters>* create_new(const Parameters& par) const
                {
                    return new base_v_fi<fouc_s_spheroid<CalculationType, Parameters>, CalculationType, Parameters>(par);
                }
        };

        template <typename CalculationType, typename Parameters>
        inline void fouc_s_init(detail::base_factory<CalculationType, Parameters>& factory)
        {
            factory.add_to_factory("fouc_s", new fouc_s_entry<CalculationType, Parameters>);
        }

    } // namespace detail
    #endif // doxygen

} // namespace projections

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_PROJECTIONS_FOUC_S_HPP
