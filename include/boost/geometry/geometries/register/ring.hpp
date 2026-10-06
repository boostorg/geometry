// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_GEOMETRIES_REGISTER_RING_HPP
#define BOOST_GEOMETRY_GEOMETRIES_REGISTER_RING_HPP

#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/point_order.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>

#ifndef DOXYGEN_NO_SPECIALIZATIONS
#define BOOST_GEOMETRY_DETAIL_SPECIALIZE_RING_1(Ring) \
namespace boost { namespace geometry { namespace traits {  \
    template<> struct tag<Ring> { using type = ring_tag; }; \
}}}

#define BOOST_GEOMETRY_DETAIL_SPECIALIZE_RING_3(Ring, Clockwise, Closed) \
namespace boost { namespace geometry { namespace traits {  \
    template<> struct tag<Ring> { using type = ring_tag; }; \
    template<> struct point_order<Ring> { static const order_selector value = (Clockwise) ? clockwise : counterclockwise; }; \
    template<> struct closure<Ring> { static const closure_selector value = (Closed) ? closed : open; }; \
}}}

#define BOOST_GEOMETRY_DETAIL_SPECIALIZE_RING_TEMPLATED_1(Ring) \
namespace boost { namespace geometry { namespace traits {  \
    template<typename P> struct tag< Ring<P> > { using type = ring_tag; }; \
}}}

#define BOOST_GEOMETRY_DETAIL_SPECIALIZE_RING_TEMPLATED_3(Ring, Clockwise, Closed) \
namespace boost { namespace geometry { namespace traits {  \
    template<typename P> struct tag< Ring<P> > { using type = ring_tag; }; \
    template<typename P> struct point_order< Ring<P> > { static const order_selector value = (Clockwise) ? clockwise : counterclockwise; }; \
    template<typename P> struct closure< Ring<P> > { static const closure_selector value = (Closed) ? closed : open; }; \
}}}

#define BOOST_GEOMETRY_DETAIL_REGISTER_RING_GET_MACRO(_1, _2, _3, NAME, ...) NAME
#define BOOST_GEOMETRY_DETAIL_REGISTER_RING_EXPAND(x) x
#endif // DOXYGEN_NO_SPECIALIZATIONS

/*!
\brief \brief_macro{ring}
\ingroup register
\details \details_macro{BOOST_GEOMETRY_REGISTER_RING, ring} The
    ring may contain template parameters, which must be specified then.
    Optionally accepts Clockwise and Closed booleans to specify orientation and closure.
\param ... \param_macro_type{ring} or `Ring, Clockwise, Closed`

\qbk{
[heading Example]
[register_ring]
[register_ring_output]
}
*/
#define BOOST_GEOMETRY_REGISTER_RING(...) \
    BOOST_GEOMETRY_DETAIL_REGISTER_RING_EXPAND( \
        BOOST_GEOMETRY_DETAIL_REGISTER_RING_GET_MACRO(__VA_ARGS__, \
            BOOST_GEOMETRY_DETAIL_SPECIALIZE_RING_3, \
            BOOST_GEOMETRY_DETAIL_SPECIALIZE_RING_UNUSED, \
            BOOST_GEOMETRY_DETAIL_SPECIALIZE_RING_1)(__VA_ARGS__))


/*!
\brief \brief_macro{templated ring}
\ingroup register
\details \details_macro{BOOST_GEOMETRY_REGISTER_RING_TEMPLATED, templated ring}
    \details_macro_templated{ring, point}
    Optionally accepts Clockwise and Closed booleans to specify orientation and closure.
\param ... \param_macro_type{ring (without template parameters)} or `Ring, Clockwise, Closed`

\qbk{
[heading Example]
[register_ring_templated]
[register_ring_templated_output]
}
*/
#define BOOST_GEOMETRY_REGISTER_RING_TEMPLATED(...) \
    BOOST_GEOMETRY_DETAIL_REGISTER_RING_EXPAND( \
        BOOST_GEOMETRY_DETAIL_REGISTER_RING_GET_MACRO(__VA_ARGS__, \
            BOOST_GEOMETRY_DETAIL_SPECIALIZE_RING_TEMPLATED_3, \
            BOOST_GEOMETRY_DETAIL_SPECIALIZE_RING_TEMPLATED_UNUSED, \
            BOOST_GEOMETRY_DETAIL_SPECIALIZE_RING_TEMPLATED_1)(__VA_ARGS__))


#endif // BOOST_GEOMETRY_GEOMETRIES_REGISTER_RING_HPP
