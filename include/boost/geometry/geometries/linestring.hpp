// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_LINESTRING_HPP
#define BOOST_GEOMETRY_GEOMETRIES_LINESTRING_HPP

#include <memory>
#include <vector>

#include <boost/concept/assert.hpp>
#include <boost/range.hpp>

#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/geometries/concepts/point_concept.hpp>


namespace boost { namespace geometry
{

namespace model
{

/*!
    \brief A linestring (named so by OGC) is a collection (default a vector) of points.
    \ingroup geometries
    \tparam P point type
    \tparam V optional container type, for example std::vector, std::list, std::deque
    \tparam A optional container-allocator-type
    (see http://accu.org/index.php/journals/427#ftn.d0e249 )
    \par Concepts:
    All algorithms work on ranges, based on a container with point types fulfilling
    the point concepts. They will take linestring, but also vector, std::pair, or other containers.
*/
template
<
    typename P,
    template<typename,typename> class V = std::vector,
    template<typename> class A = std::allocator
>
class linestring : public V<P, A<P> >
{
    BOOST_CONCEPT_ASSERT( (concept::Point<P>) );
};

} // namespace model

#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{

template
<
    typename P,
    template<typename,typename> class V,
    template<typename> class A
>
struct tag<model::linestring<P, V, A> >
{
    typedef linestring_tag type;
};
} // namespace traits

#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_GEOMETRIES_LINESTRING_HPP
