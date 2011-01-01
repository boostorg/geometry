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
*/
template
<
    typename P,
    template<typename, typename> class V = std::vector,
    template<typename> class A = std::allocator
>
class multi_polygon : public V<P, A<P> >
{
    BOOST_CONCEPT_ASSERT( (concept::Polygon<P>) );
};


} // namespace model


#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{

template
<
    typename P,
    template<typename, typename> class V,
    template<typename> class A
>
struct tag< model::multi_polygon<P, V, A> >
{
    typedef multi_polygon_tag type;
};

} // namespace traits
#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_MULTI_GEOMETRIES_MULTI_POLYGON_HPP
