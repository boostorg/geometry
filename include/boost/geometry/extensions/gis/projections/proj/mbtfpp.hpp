#ifndef BOOST_GEOMETRY_PROJECTIONS_MBTFPP_HPP
#define BOOST_GEOMETRY_PROJECTIONS_MBTFPP_HPP

// Generic Geometry Library - projections (based on PROJ4)
// This file is automatically generated. DO NOT EDIT.

// Copyright Barend Gehrels (1995-2009), Geodan Holding B.V. Amsterdam, the Netherlands.
// Copyright Bruno Lalande (2008-2009)
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// This file is converted from PROJ4, http://trac.osgeo.org/proj
// PROJ4 is originally written by Gerald Evenden (then of the USGS)
// PROJ4 is maintained by Frank Warmerdam
// PROJ4 is converted to Geometry Library by Barend Gehrels (Geodan, Amsterdam)

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

#include <boost/math/special_functions/hypot.hpp>

#include <boost/geometry/extensions/gis/projections/impl/base_static.hpp>
#include <boost/geometry/extensions/gis/projections/impl/base_dynamic.hpp>
#include <boost/geometry/extensions/gis/projections/impl/projects.hpp>
#include <boost/geometry/extensions/gis/projections/impl/factory_entry.hpp>

namespace boost { namespace geometry { namespace projection
{
    #ifndef DOXYGEN_NO_DETAIL
    namespace detail { namespace mbtfpp{ 
            static const double CS = .95257934441568037152;
            static const double FXC = .92582009977255146156;
            static const double FYC = 3.40168025708304504493;
            static const double C23 = .66666666666666666666;
            static const double C13 = .33333333333333333333;
            static const double ONEEPS = 1.0000001;


            // template class, using CRTP to implement forward/inverse
            template <typename Geographic, typename Cartesian, typename Parameters>
            struct base_mbtfpp_spheroid : public base_t_fi<base_mbtfpp_spheroid<Geographic, Cartesian, Parameters>,
                     Geographic, Cartesian, Parameters>
            {

                 typedef double geographic_type;
                 typedef double cartesian_type;


                inline base_mbtfpp_spheroid(const Parameters& par)
                    : base_t_fi<base_mbtfpp_spheroid<Geographic, Cartesian, Parameters>,
                     Geographic, Cartesian, Parameters>(*this, par) {}

                inline void fwd(geographic_type& lp_lon, geographic_type& lp_lat, cartesian_type& xy_x, cartesian_type& xy_y) const
                {
                    lp_lat = asin(CS * sin(lp_lat));
                    xy_x = FXC * lp_lon * (2. * cos(C23 * lp_lat) - 1.);
                    xy_y = FYC * sin(C13 * lp_lat);
                }

                inline void inv(cartesian_type& xy_x, cartesian_type& xy_y, geographic_type& lp_lon, geographic_type& lp_lat) const
                {
                    lp_lat = xy_y / FYC;
                    if (fabs(lp_lat) >= 1.) {
                        if (fabs(lp_lat) > ONEEPS)    throw proj_exception();
                        else    lp_lat = (lp_lat < 0.) ? -HALFPI : HALFPI;
                    } else
                        lp_lat = asin(lp_lat);
                    lp_lon = xy_x / ( FXC * (2. * cos(C23 * (lp_lat *= 3.)) - 1.) );
                    if (fabs(lp_lat = sin(lp_lat) / CS) >= 1.) {
                        if (fabs(lp_lat) > ONEEPS)    throw proj_exception();
                        else    lp_lat = (lp_lat < 0.) ? -HALFPI : HALFPI;
                    } else
                        lp_lat = asin(lp_lat);
                }
            };

            // McBride-Thomas Flat-Polar Parabolic
            template <typename Parameters>
            void setup_mbtfpp(Parameters& par)
            {
                par.es = 0.;
                // par.inv = s_inverse;
                // par.fwd = s_forward;
            }

        }} // namespace detail::mbtfpp
    #endif // doxygen 

    /*!
        \brief McBride-Thomas Flat-Polar Parabolic projection
        \ingroup projections
        \tparam Geographic latlong point type
        \tparam Cartesian xy point type
        \tparam Parameters parameter type
        \par Projection characteristics
         - Cylindrical
         - Spheroid
        \par Example
        \image html ex_mbtfpp.gif
    */
    template <typename Geographic, typename Cartesian, typename Parameters = parameters>
    struct mbtfpp_spheroid : public detail::mbtfpp::base_mbtfpp_spheroid<Geographic, Cartesian, Parameters>
    {
        inline mbtfpp_spheroid(const Parameters& par) : detail::mbtfpp::base_mbtfpp_spheroid<Geographic, Cartesian, Parameters>(par)
        {
            detail::mbtfpp::setup_mbtfpp(this->m_par);
        }
    };

    #ifndef DOXYGEN_NO_DETAIL
    namespace detail
    {

        // Factory entry(s)
        template <typename Geographic, typename Cartesian, typename Parameters>
        class mbtfpp_entry : public detail::factory_entry<Geographic, Cartesian, Parameters>
        {
            public :
                virtual projection<Geographic, Cartesian>* create_new(const Parameters& par) const
                {
                    return new base_v_fi<mbtfpp_spheroid<Geographic, Cartesian, Parameters>, Geographic, Cartesian, Parameters>(par);
                }
        };

        template <typename Geographic, typename Cartesian, typename Parameters>
        inline void mbtfpp_init(detail::base_factory<Geographic, Cartesian, Parameters>& factory)
        {
            factory.add_to_factory("mbtfpp", new mbtfpp_entry<Geographic, Cartesian, Parameters>);
        }

    } // namespace detail 
    #endif // doxygen

}}} // namespace boost::geometry::projection

#endif // BOOST_GEOMETRY_PROJECTIONS_MBTFPP_HPP

