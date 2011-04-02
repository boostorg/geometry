// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2010 Alfredo Correa
// Copyright (c) 2010-2011 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_ARRAY_GEOGRAPHIC_HPP
#define BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_ARRAY_GEOGRAPHIC_HPP

#ifdef BOOST_GEOMETRY_ADAPTED_BOOST_ARRAY_COORDINATE_SYSTEM_DEFINED
#error Include only one headerfile to register coordinate coordinate_system for adapted boost array
#endif

#define BOOST_GEOMETRY_ADAPTED_BOOST_ARRAY_COORDINATE_SYSTEM_DEFINED


#include <boost/geometry/geometries/adapted/boost_array.hpp>

namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{
    template <typename T, std::size_t N>
    struct coordinate_system<boost::array<T, N> >
    { typedef cs::geographic type; };

}
#endif


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_GEOMETRIES_ADAPTED_BOOST_ARRAY_GEOGRAPHIC_HPP
