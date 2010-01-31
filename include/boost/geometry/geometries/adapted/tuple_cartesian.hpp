// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
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
    template <typename T>
    struct coordinate_system<boost::tuple<T, T> >
    { typedef cs::cartesian type; };

    template <typename T>
    struct coordinate_system<boost::tuple<T, T, T> >
    { typedef cs::cartesian type; };

}
#endif


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_GEOMETRIES_ADAPTED_TUPLE_CARTESIAN_HPP
