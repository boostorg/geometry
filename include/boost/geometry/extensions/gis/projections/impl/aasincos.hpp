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

#ifndef BOOST_GEOMETRY_PROJECTIONS_IMPL_AASINCOS_HPP
#define BOOST_GEOMETRY_PROJECTIONS_IMPL_AASINCOS_HPP


#include <cmath>

#include <boost/geometry/util/math.hpp>


namespace boost { namespace geometry { namespace projection
{

namespace detail
{

namespace aasincos
{
    static const double ONE_TOL= 1.00000000000001;
    //static const double TOL = 0.000000001;
    static const double ATOL = 1e-50;
}

inline double aasin(double v)
{
    double av = 0;

    if ((av = geometry::math::abs(v)) >= 1.0)
    {
        if (av > aasincos::ONE_TOL)
        {
            throw proj_exception(-19);
        }
        return (v < 0.0 ? -HALFPI : HALFPI);
    }

    return asin(v);
}

inline double aacos(double v)
{
    double av = 0;

    if ((av = geometry::math::abs(v)) >= 1.0)
    {
        if (av > aasincos::ONE_TOL)
        {
            throw proj_exception(-19);
        }
        return (v < 0.0 ? PI : 0.0);
    }

    return acos(v);
}

inline double asqrt(double v)
{
    return ((v <= 0) ? 0 : sqrt(v));
}

inline double aatan2(double n, double d)
{
    return ((geometry::math::abs(n) < aasincos::ATOL
        && geometry::math::abs(d) < aasincos::ATOL) ? 0.0 : atan2(n, d));
}


} // namespace detail


}}} // namespace boost::geometry::projection


#endif // BOOST_GEOMETRY_PROJECTIONS_IMPL_AASINCOS_HPP
