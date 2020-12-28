#ifndef BOOST_GEOMETRY_GEOMETRIES_REGISTER_POLYHEDRON_HPP
#define BOOST_GEOMETRY_GEOMETRIES_REGISTER_POLYHEDRON_HPP


#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>

#define BOOST_GEOMETRY_REGISTER_POLYHEDRON(Polyhedron) \
namespace boost { namespace geometry { namespace traits {  \
    template<> struct tag<Polyhedron> { typedef polyhedron_tag type; }; \
}}}


#define BOOST_GEOMETRY_REGISTER_POLYHEDRON_TEMPLATED(Polyhedron) \
namespace boost { namespace geometry { namespace traits {  \
    template<typename P> struct tag< Polyhedron<P> > { typedef polyhedron_tag type; }; \
}}}