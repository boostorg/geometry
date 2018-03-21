// Boost.Geometry (aka GGL, Generic Geometry Library)
// This file is manually converted from PROJ4 (projects.h)

// Copyright (c) 2008-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2017, 2018.
// Modifications copyright (c) 2017-2018, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

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

#ifndef BOOST_GEOMETRY_PROJECTIONS_IMPL_PROJECTS_HPP
#define BOOST_GEOMETRY_PROJECTIONS_IMPL_PROJECTS_HPP


#include <cstring>
#include <string>
#include <vector>

#include <boost/config.hpp>
#include <boost/geometry/srs/projections/exception.hpp>
#include <boost/math/constants/constants.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_pod.hpp>


namespace boost { namespace geometry { namespace projections
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

/* some useful constants */
template <typename T>
inline T ONEPI() { return boost::math::constants::pi<T>(); }
template <typename T>
inline T HALFPI() { return boost::math::constants::half_pi<T>(); }
template <typename T>
inline T FORTPI() { return boost::math::constants::pi<T>() / T(4); }
template <typename T>
inline T TWOPI() { return boost::math::constants::two_pi<T>(); }
template <typename T>
inline T TWO_D_PI() { return boost::math::constants::two_div_pi<T>(); }
template <typename T>
inline T HALFPI_SQR() { return 2.4674011002723396547086227499689; }
template <typename T>
inline T PI_SQR() { return boost::math::constants::pi_sqr<T>(); }
template <typename T>
inline T THIRD() { return 0.3333333333333333333333333333333; }
template <typename T>
inline T TWOTHIRD() { return 0.6666666666666666666666666666666; }
template <typename T>
inline T PI_HALFPI() { return 4.7123889803846898576939650749193; }
template <typename T>
inline T TWOPI_HALFPI() { return 7.8539816339744830961566084581988; }
template <typename T>
inline T PI_DIV_3() { return 1.0471975511965977461542144610932; }

/* datum_type values */
static const int PJD_UNKNOWN = 0;
static const int PJD_3PARAM = 1;
static const int PJD_7PARAM = 2;
static const int PJD_GRIDSHIFT = 3;
static const int PJD_WGS84 = 4;   /* WGS84 (or anything considered equivelent) */

/* library errors */
static const int PJD_ERR_GEOCENTRIC = -45;
static const int PJD_ERR_AXIS = -47;
static const int PJD_ERR_GRID_AREA = -48;
static const int PJD_ERR_CATALOG = -49;

template <typename T>
struct pvalue
{
    std::string param;
    std::string s;
    //int used;
};

// Originally defined in proj_internal.h
//enum pj_io_units {
//    PJ_IO_UNITS_WHATEVER  = 0,  /* Doesn't matter (or depends on pipeline neighbours) */
//    PJ_IO_UNITS_CLASSIC   = 1,  /* Scaled meters (right), projected system */
//    PJ_IO_UNITS_PROJECTED = 2,  /* Meters, projected system */
//    PJ_IO_UNITS_CARTESIAN = 3,  /* Meters, 3D cartesian system */
//    PJ_IO_UNITS_ANGULAR   = 4   /* Radians */
//};

// Originally defined in proj_internal.h
/* Maximum latitudinal overshoot accepted */
//static const double PJ_EPS_LAT = 1e-12;

template <typename T>
struct pj_consts
{
    // E L L I P S O I D     P A R A M E T E R S

    T a;                            /* semimajor axis (radius if eccentricity==0) */
    T ra;                           /* 1/a */

    T e;                            /* first  eccentricity */
    T es;                           /* first  eccentricity squared */
    T one_es;                       /* 1 - e^2 */
    T rone_es;                      /* 1/one_es */

    T es_orig, a_orig;              /* es and a before any +proj related adjustment */

    // C O O R D I N A T E   H A N D L I N G

    int over;                       /* over-range flag */
    int geoc;                       /* geocentric latitude flag */
    int is_latlong;                 /* proj=latlong ... not really a projection at all */
    int is_geocent;                 /* proj=geocent ... not really a projection at all */
    //int need_ellps;                 /* 0 for operations that are purely cartesian */

    //enum pj_io_units left;          /* Flags for input/output coordinate types */
    //enum pj_io_units right;

    // C A R T O G R A P H I C       O F F S E T S

    T lam0, phi0;                   /* central longitude, latitude */
    T x0, y0/*, z0, t0*/;           /* false easting and northing (and height and time) */

    // S C A L I N G

    T k0;                           /* general scaling factor */
    T to_meter, fr_meter;           /* cartesian scaling */
    T vto_meter, vfr_meter;         /* Vertical scaling. Internal unit [m] */

    // D A T U M S   A N D   H E I G H T   S Y S T E M S    

    int datum_type;                 /* PJD_UNKNOWN/3PARAM/7PARAM/GRIDSHIFT/WGS84 */
    T datum_params[7];              /* Parameters for 3PARAM and 7PARAM */

    T from_greenwich;               /* prime meridian offset (in radians) */
    T long_wrap_center;             /* 0.0 for -180 to 180, actually in radians*/
    bool is_long_wrap_set;

    // Initialize all variables
    pj_consts()
        : a(0), ra(0)
        , e(0), es(0), one_es(0), rone_es(0)
        , es_orig(0), a_orig(0)
        , over(0), geoc(0), is_latlong(0), is_geocent(0)
        //, need_ellps(1)
        //, left(PJ_IO_UNITS_ANGULAR), right(PJ_IO_UNITS_CLASSIC)
        , lam0(0), phi0(0)
        , x0(0), y0(0)/*, z0(0), t0(0)*/
        , k0(0) , to_meter(0), fr_meter(0), vto_meter(0), vfr_meter(0)
        , datum_type(PJD_UNKNOWN)
#ifndef BOOST_NO_CXX11_UNIFIED_INITIALIZATION_SYNTAX
        , datum_params{0, 0, 0, 0, 0, 0, 0}
#endif
        , from_greenwich(0), long_wrap_center(0), is_long_wrap_set(false)
    {
#ifdef BOOST_NO_CXX11_UNIFIED_INITIALIZATION_SYNTAX
        std::fill(datum_params, datum_params + 7, T(0));
#endif
    }
};

// PROJ4 complex. Might be replaced with std::complex
template <typename T>
struct COMPLEX { T r, i; };

struct PJ_ELLPS
{
    std::string id;    /* ellipse keyword name */
    std::string major;    /* a= value */
    std::string ell;    /* elliptical parameter */
    std::string name;    /* comments */
};

struct PJ_DATUMS
{
    std::string id;     /* datum keyword */
    std::string defn;   /* ie. "to_wgs84=..." */
    std::string ellipse_id; /* ie from ellipse table */
    std::string comments; /* EPSG code, etc */
};

struct PJ_PRIME_MERIDIANS
{
    std::string id;     /* prime meridian keyword */
    std::string defn;   /* offset from greenwich in DMS format. */
};

struct PJ_UNITS
{
    std::string id;    /* units keyword */
    std::string to_meter;    /* multiply by value to get meters */
    std::string name;    /* comments */
};

template <typename T>
struct DERIVS
{
    T x_l, x_p; /* derivatives of x for lambda-phi */
    T y_l, y_p; /* derivatives of y for lambda-phi */
};

template <typename T>
struct FACTORS
{
    DERIVS<T> der;
    T h, k;    /* meridinal, parallel scales */
    T omega, thetap;    /* angular distortion, theta prime */
    T conv;    /* convergence */
    T s;        /* areal scale factor */
    T a, b;    /* max-min scale error */
    int code;        /* info as to analytics, see following */
};

} // namespace detail
#endif // DOXYGEN_NO_DETAIL

/*!
    \brief parameters, projection parameters
    \details This structure initializes all projections
    \ingroup projection
*/
template <typename T>
struct parameters : public detail::pj_consts<T>
{
    typedef T type;

    std::string name;
    std::vector<detail::pvalue<T> > params;
};

}}} // namespace boost::geometry::projections
#endif // BOOST_GEOMETRY_PROJECTIONS_IMPL_PROJECTS_HPP
