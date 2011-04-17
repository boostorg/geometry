// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2008-2011 Bruno Lalande, Paris, France.
// Copyright (c) 2008-2011 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2009-2011 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_ADAPTED_TUPLE_CARTESIAN_HPP
#define BOOST_GEOMETRY_GEOMETRIES_ADAPTED_TUPLE_CARTESIAN_HPP

#ifdef BOOST_GEOMETRY_ADAPTED_TUPLE_COORDINATE_SYSTEM_DEFINED
#error Include only one headerfile to register coordinate coordinate_system for adapted tuple
#endif

#define BOOST_GEOMETRY_ADAPTED_TUPLE_COORDINATE_SYSTEM_DEFINED


#include <boost/geometry/geometries/adapted/tuple.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{


template <typename T1, typename T2, typename T3, typename T4, typename T5,
          typename T6, typename T7, typename T8, typename T9, typename T10>
struct coordinate_system<boost::tuple<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10> >
{ typedef cs::cartesian type; };


}
#endif


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_GEOMETRIES_ADAPTED_TUPLE_CARTESIAN_HPP
