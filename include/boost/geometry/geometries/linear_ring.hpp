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

#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/point_order.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>

#include <boost/geometry/geometries/concepts/point_concept.hpp>


namespace boost { namespace geometry
{

namespace model
{
/*!
    \brief A linear_ring (linear linear_ring) is a closed line which should not be selfintersecting
    \ingroup geometries
    \tparam Point point type
    \tparam Container optional container type, for example std::vector, std::list, std::deque
    \tparam Allocator optional container-allocator-type
*/
template
<
    typename Point,
    bool ClockWise = true, bool Closed = true,
    template<typename, typename> class Container = std::vector,
    template<typename> class Allocator = std::allocator
>
class linear_ring : public Container<Point, Allocator<Point> >
{
    BOOST_CONCEPT_ASSERT( (concept::Point<Point>) );
};

} // namespace model


#ifndef DOXYGEN_NO_TRAITS_SPECIALIZATIONS
namespace traits
{

template
<
    typename Point,
    bool ClockWise, bool Closed,
    template<typename, typename> class Container,
    template<typename> class Allocator
>
struct tag<model::linear_ring<Point, ClockWise, Closed, Container, Allocator> >
{
    typedef ring_tag type;
};


template
<
    typename Point,
    bool Closed,
    template<typename, typename> class Container,
    template<typename> class Allocator
>
struct point_order<model::linear_ring<Point, false, Closed, Container, Allocator> >
{
    static const order_selector value = counterclockwise;
};


template
<
    typename Point,
    bool Closed,
    template<typename, typename> class Container,
    template<typename> class Allocator
>
struct point_order<model::linear_ring<Point, true, Closed, Container, Allocator> >
{
    static const order_selector value = clockwise;
};

template
<
    typename Point,
    bool PointOrder,
    template<typename, typename> class Container,
    template<typename> class Allocator
>
struct closure<model::linear_ring<Point, PointOrder, true, Container, Allocator> >
{
    static const closure_selector value = closed;
};

template
<
    typename Point,
    bool PointOrder,
    template<typename, typename> class Container,
    template<typename> class Allocator
>
struct closure<model::linear_ring<Point, PointOrder, false, Container, Allocator> >
{
    static const closure_selector value = open;
};


} // namespace traits
#endif // DOXYGEN_NO_TRAITS_SPECIALIZATIONS


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_GEOMETRIES_LINEAR_RING_HPP
