// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2008-2011 Bruno Lalande, Paris, France.
// Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2009-2011 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_ADAPTED_C_ARRAY_CARTESIAN_HPP
#define BOOST_GEOMETRY_GEOMETRIES_ADAPTED_C_ARRAY_CARTESIAN_HPP

#ifdef BOOST_GEOMETRY_ADAPTED_C_ARRAY_COORDINATE_SYSTEM_DEFINED
#error Include only one headerfile to register coordinate coordinate_system for adapted c array
#endif

#define BOOST_GEOMETRY_ADAPTED_C_ARRAY_COORDINATE_SYSTEM_DEFINED


#include <boost/geometry/geometries/adapted/c_array.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{
    template <typename T, int N>
    struct coordinate_system<T[N]>
    { typedef cs::cartesian type; };

}
#endif


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_GEOMETRIES_ADAPTED_C_ARRAY_CARTESIAN_HPP
