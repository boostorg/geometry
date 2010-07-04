// Boost.Geometry (aka GGL, Generic Geometry Library) - projections (based on PROJ4)
// This file is manually converted from PROJ4

// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
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

#ifndef BOOST_GEOMETRY_PROJECTIONS_IMPL_PJ_FWD_HPP
#define BOOST_GEOMETRY_PROJECTIONS_IMPL_PJ_FWD_HPP

#include <cmath>

#include <boost/geometry/core/radian_access.hpp>
#include <boost/geometry/util/math.hpp>

#include <boost/geometry/extensions/gis/projections/impl/adjlon.hpp>

/* general forward projection */

namespace boost { namespace geometry { namespace projection {

namespace detail {

namespace forwrd
{
    static const double EPS = 1.0e-12;
}

/* forward projection entry */
template <typename Prj, typename LL, typename XY, typename P>
inline void pj_fwd(Prj const& prj, P const& par, LL const& ll, XY& xy)
{
    using namespace detail;

    double lp_lon = geometry::get_as_radian<0>(ll);
    double lp_lat = geometry::get_as_radian<1>(ll);
    const double t = geometry::math::abs(lp_lat) - HALFPI;

    /* check for forward and latitude or longitude overange */
    if (t > forwrd::EPS || geometry::math::abs(lp_lon) > 10.)
    {
        throw proj_exception();
    }

    if (geometry::math::abs(t) <= forwrd::EPS)
    {
        lp_lat = lp_lat < 0. ? -HALFPI : HALFPI;
    }
    else if (par.geoc)
    {
        lp_lat = atan(par.rone_es * tan(lp_lat));
    }

    lp_lon -= par.lam0;    /* compute del lp.lam */
    if (! par.over)
    {
        lp_lon = adjlon(lp_lon); /* post_forward del longitude */
    }

    double x = 0;
    double y = 0;

    prj.fwd(lp_lon, lp_lat, x, y);
    geometry::set<0>(xy, par.fr_meter * (par.a * x + par.x0));
    geometry::set<1>(xy, par.fr_meter * (par.a * y + par.y0));
}

} // namespace detail
}}} // namespace boost::geometry::projection

#endif // BOOST_GEOMETRY_PROJECTIONS_IMPL_PJ_FWD_HPP
