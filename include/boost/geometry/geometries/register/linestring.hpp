// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_GEOMETRIES_REGISTER_LINESTRING_HPP
#define BOOST_GEOMETRY_GEOMETRIES_REGISTER_LINESTRING_HPP


#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>


#define BOOST_GEOMETRY_REGISTER_LINESTRING(Linestring) \
namespace boost { namespace geometry { namespace traits {  \
    template<> struct tag<Linestring> { typedef linestring_tag type; }; \
}}}



#endif // BOOST_GEOMETRY_GEOMETRIES_REGISTER_LINESTRING_HPP
