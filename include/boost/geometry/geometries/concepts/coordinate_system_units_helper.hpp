// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_GEOMETRIES_CONCEPTS_COORDINATE_SYSTEM_UNITS_HELPER_HPP
#define BOOST_GEOMETRY_GEOMETRIES_CONCEPTS_COORDINATE_SYSTEM_UNITS_HELPER_HPP

#include <boost/geometry/core/cs.hpp>


namespace boost { namespace geometry
{

namespace concept { namespace detail
{


template <typename CoordinateSystem>
struct coordinate_system_units_helper
{
    typedef typename CoordinateSystem::units type;
};

template <>
struct coordinate_system_units_helper<geometry::cs::cartesian>
{
    typedef void type;
};


}} // namespace concept::detail

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_GEOMETRIES_CONCEPTS_COORDINATE_SYSTEM_UNITS_HELPER_HPP
