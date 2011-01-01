// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_MULTI_GEOMETRIES_LINESTRING_HPP
#define BOOST_GEOMETRY_MULTI_GEOMETRIES_LINESTRING_HPP

#include <memory>
#include <vector>

#include <boost/concept/requires.hpp>

#include <boost/geometry/geometries/concepts/linestring_concept.hpp>

#include <boost/geometry/multi/core/tags.hpp>

namespace boost { namespace geometry
{


namespace model
{

/*!
    \brief multi_line, a collection of linestring
    \details Multi-linestring can be used to group lines belonging to each other,
            e.g. a highway (with interruptions)
    \ingroup geometries
*/
template
<
    typename L,
    template<typename, typename> class V = std::vector,
    template<typename> class A = std::allocator
>
class multi_linestring : public V<L, A<L> >
{
    BOOST_CONCEPT_ASSERT( (concept::Linestring<L>) );
};


} // namespace model


#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{

template
<
    typename L,
    template<typename, typename> class V,
    template<typename> class A
>
struct tag< model::multi_linestring<L, V, A> >
{
    typedef multi_linestring_tag type;
};

} // namespace traits
#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_MULTI_GEOMETRIES_LINESTRING_HPP
