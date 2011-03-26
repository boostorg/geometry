// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2011 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2011 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_GEOMETRIES_REGISTER_RING_HPP
#define BOOST_GEOMETRY_GEOMETRIES_REGISTER_RING_HPP

#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>

#define BOOST_GEOMETRY_REGISTER_RING(Ring) \
namespace boost { namespace geometry { namespace traits {  \
    template<> struct tag<Ring> { typedef ring_tag type; }; \
}}}


#define BOOST_GEOMETRY_REGISTER_RING_TEMPLATIZED(Ring) \
namespace boost { namespace geometry { namespace traits {  \
    template<typename P> struct tag< Ring<P> > { typedef ring_tag type; }; \
}}}


#endif // BOOST_GEOMETRY_GEOMETRIES_REGISTER_RING_HPP
