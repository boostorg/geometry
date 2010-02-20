// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSION_NSPHERE_CARTESIAN3D_HPP
#define BOOST_GEOMETRY_EXTENSION_NSPHERE_CARTESIAN3D_HPP


#include <boost/geometry/geometries/cartesian3d.hpp>
#include <boost/geometry/extensions/nsphere/geometries/nsphere.hpp>

namespace boost { namespace geometry
{

typedef nsphere<point_3d, double> sphere;

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSION_NSPHERE_CARTESIAN3D_HPP
