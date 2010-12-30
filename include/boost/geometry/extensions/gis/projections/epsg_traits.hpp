#ifndef _PROJECTIONS_EPSG_TRAITS_HPP
#define _PROJECTIONS_EPSG_TRAITS_HPP

// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <boost/geometry/extensions/gis/projections/impl/projects.hpp>


namespace boost { namespace geometry { namespace projection
{

/*!
    \brief EPSG traits
    \details With help of the EPSG traits library users can statically use projections
        or coordinate systems specifying an EPSG code. The correct projections for transformations
        are used automically then, still keeping static polymorphism.
    \ingroup projection
    \tparam EPSG epsg code
    \tparam LL latlong point type
    \tparam XY xy point type
    \tparam PAR parameter type, normally not specified
*/
template <size_t EPSG, typename LLR, typename XY, typename PAR = parameters>
struct epsg_traits
{
    // Specializations define:
    // - type to get projection type
    // - function par to get parameters
};


}}} // namespace boost::geometry::projection


#endif

