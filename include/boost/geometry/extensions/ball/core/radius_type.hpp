// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2012 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2012 Mateusz Loskot, London, UK.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_EXTENSIONS_BALL_CORE_RADIUS_TYPE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_BALL_CORE_RADIUS_TYPE_HPP


#include <cstddef>


#include <boost/type_traits/remove_const.hpp>

#include <boost/geometry/core/tag.hpp>

#include <boost/geometry/extensions/ball/core/tags.hpp>


namespace boost { namespace geometry
{

namespace traits
{

/*!
    \brief Traits class indicating the type (double,float,...) of the radius of a circle or a sphere
    \par Geometries:
        - n-sphere (circle,sphere)
        - upcoming ellipse
    \par Specializations should provide:
        - typedef T type (double,float,int,etc)
    \ingroup traits
*/
template <typename G>
struct radius_type {};

} // namespace traits


#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{

template <typename Tag, typename G>
struct radius_type
{
    //typedef core_dispatch_specialization_required type;
};

template <typename S>
struct radius_type<ball_tag, S>
{
    typedef typename traits::radius_type<S>::type type;
};

} // namespace core_dispatch
#endif // DOXYGEN_NO_DISPATCH


template <typename G>
struct radius_type
{
    typedef typename boost::remove_const<G>::type rconst;
    typedef typename core_dispatch::radius_type<typename tag<G>::type, rconst>::type type;
};


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_EXTENSIONS_BALL_CORE_RADIUS_TYPE_HPP
