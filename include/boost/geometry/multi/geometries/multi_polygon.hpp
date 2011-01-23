// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_MULTI_GEOMETRIES_MULTI_POLYGON_HPP
#define BOOST_GEOMETRY_MULTI_GEOMETRIES_MULTI_POLYGON_HPP

#include <memory>
#include <vector>

#include <boost/concept/requires.hpp>

#include <boost/geometry/multi/core/tags.hpp>

#include <boost/geometry/geometries/concepts/polygon_concept.hpp>

namespace boost { namespace geometry
{

namespace model
{

/*!
\brief multi_polygon, a collection of polygons
\details Multi-polygon can be used to group polygons belonging to each other,
        e.g. Hawaii
\ingroup geometries

\qbk{before.synopsis,
[heading Model of]
[link geometry.reference.concepts.concept_multi_polygon MultiPolygon Concept]
}
*/
template
<
    typename Polygon,
    template<typename, typename> class Container = std::vector,
    template<typename> class Allocator = std::allocator
>
class multi_polygon : public Container<Polygon, Allocator<Polygon> >
{
    BOOST_CONCEPT_ASSERT( (concept::Polygon<Polygon>) );
};


} // namespace model


#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{

template
<
    typename Polygon,
    template<typename, typename> class Container,
    template<typename> class Allocator
>
struct tag< model::multi_polygon<Polygon, Container, Allocator> >
{
    typedef multi_polygon_tag type;
};

} // namespace traits
#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_MULTI_GEOMETRIES_MULTI_POLYGON_HPP
