#ifndef BOOST_GEOMETRY_GEOMETRIES_REGISTER_POLYHEDRALSURFACE_HPP
#define BOOST_GEOMETRY_GEOMETRIES_REGISTER_POLYHEDRALSURFACE_HPP


#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>

#define BOOST_GEOMETRY_REGISTER_POLYHEDRALSURFACE(PolyhedralSurface) \
namespace boost { namespace geometry { namespace traits {  \
    template<> struct tag<PolyhedralSurface> { typedef PolyhedralSurface_tag type; }; \
}}}


#define BOOST_GEOMETRY_REGISTER_POLYHEDRALSURFACE_TEMPLATED(PolyhedralSurface) \
namespace boost { namespace geometry { namespace traits {  \
    template<typename P> struct tag< PolyhedralSurface<P> > { typedef PolyhedralSurface_tag type; }; \
}}}