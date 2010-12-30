// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_MULTI_GEOMETRIES_MULTI_POINT_HPP
#define BOOST_GEOMETRY_MULTI_GEOMETRIES_MULTI_POINT_HPP

#include <memory>
#include <vector>

#include <boost/concept/requires.hpp>

#include <boost/geometry/geometries/concepts/point_concept.hpp>

#include <boost/geometry/multi/core/tags.hpp>

namespace boost { namespace geometry
{

namespace model
{


/*!
    \brief multi_point, a collection of points
    \details Multipoint can be used to group points belonging to each other,
            e.g. a constellation
    \ingroup geometries
*/
template
<
    typename P,
    template<typename, typename> class V = std::vector,
    template<typename> class A = std::allocator
>
struct multi_point : public V<P, A<P> >
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
    template<typename, typename> class V,
    template<typename> class A
>
struct tag< model::multi_point<P, V, A> >
{
    typedef multi_point_tag type;
};

} // namespace traits
#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_MULTI_GEOMETRIES_MULTI_POINT_HPP
