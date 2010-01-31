// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_CORE_COORDINATE_TYPE_HPP
#define BOOST_GEOMETRY_CORE_COORDINATE_TYPE_HPP

#include <boost/type_traits/remove_const.hpp>

#include <boost/geometry/core/point_type.hpp>

namespace boost { namespace geometry {

namespace traits {

/*!
    \brief Traits class which indicate the coordinate type (double,float,...) of a point
    \ingroup traits
    \par Geometries:
        - point
    \par Specializations should provide:
        - typedef T type; (double,float,int,etc)
*/
template <typename P>
struct coordinate_type {};

} // namespace traits

#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{

template <typename GeometryTag, typename G>
struct coordinate_type
{
    typedef typename point_type<GeometryTag, G>::type point_type;

    // Call its own specialization on point-tag
    typedef typename coordinate_type<point_tag, point_type>::type type;
};

template <typename P>
struct coordinate_type<point_tag, P>
{
    typedef typename traits::coordinate_type<P>::type type;
};

} // namespace core_dispatch
#endif // DOXYGEN_NO_DISPATCH

/*!
    \brief Meta-function which defines coordinate type (int, float, double, etc) of any geometry
    \ingroup core
*/
template <typename G>
struct coordinate_type
{
    typedef typename boost::remove_const<G>::type ncg;
    typedef typename core_dispatch::coordinate_type
        <
            typename tag<G>::type,
            ncg
        >::type type;
};

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_CORE_COORDINATE_TYPE_HPP
