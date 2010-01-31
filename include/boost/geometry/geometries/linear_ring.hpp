// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_LINEAR_RING_HPP
#define BOOST_GEOMETRY_GEOMETRIES_LINEAR_RING_HPP

#include <memory>
#include <vector>

#include <boost/concept/assert.hpp>

#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>
#include <boost/geometry/core/point_order.hpp>

#include <boost/geometry/geometries/concepts/point_concept.hpp>


namespace boost { namespace geometry
{

/*!
    \brief A linear_ring (linear linear_ring) is a closed line which should not be selfintersecting
    \ingroup geometries
    \tparam P point type
    \tparam V optional container type, for example std::vector, std::list, std::deque
    \tparam A optional container-allocator-type
*/
template
<
    typename P,
    template<typename, typename> class V = std::vector,
    bool ClockWise = true,
    template<typename> class A = std::allocator
>
class linear_ring : public V<P, A<P> >
{
    BOOST_CONCEPT_ASSERT( (concept::Point<P>) );
};

#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{

template
<
    typename P,
    template<typename, typename> class V,
    bool ClockWise,
    template<typename> class A
>
struct tag< linear_ring<P, V, ClockWise, A> >
{
    typedef ring_tag type;
};


template
<
    typename P,
    template<typename, typename> class V,
    template<typename> class A
>
struct point_order< linear_ring<P, V, false, A> >
{
    static const order_selector value = counterclockwise;
};


template
<
    typename P,
    template<typename, typename> class V,
    template<typename> class A
>
struct point_order< linear_ring<P, V, true, A> >
{
    static const order_selector value = clockwise;
};




} // namespace traits
#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_GEOMETRIES_LINEAR_RING_HPP
