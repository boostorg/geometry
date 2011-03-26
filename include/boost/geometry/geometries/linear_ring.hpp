// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2011 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2011 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2011 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRIES_LINEAR_RING_HPP
#define BOOST_GEOMETRY_GEOMETRIES_LINEAR_RING_HPP

#include <boost/config.hpp>

#if defined(BOOST_MSVC_FULL_VER)
#pragma message ("linear_ring is renamed to 'ring', so deprecated. Prefer using ring")
#else
#warning "linear_ring is renamed to 'ring', so deprecated. Prefer using ring"
#endif

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
    \tparam Container container type, for example std::vector, std::deque
    \tparam Allocator container-allocator-type
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

    typedef Container<Point, Allocator<Point> > base_type;

public :
    inline linear_ring()
        : base_type()
    {}

    template <typename Iterator>
    inline linear_ring(Iterator begin, Iterator end)
        : base_type(begin, end)
    {}
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
