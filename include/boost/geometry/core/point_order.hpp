// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_CORE_POINT_ORDER_HPP
#define BOOST_GEOMETRY_CORE_POINT_ORDER_HPP

#include <boost/range/functions.hpp>
#include <boost/range/metafunctions.hpp>
#include <boost/type_traits/remove_const.hpp>

#include <boost/geometry/core/ring_type.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>

namespace boost { namespace geometry {


enum order_selector { clockwise = 1, counterclockwise = 2, order_undetermined = 0 };

namespace traits {

/*!
    \brief Traits class indicating the order of contained points within a
        ring or (multi)polygon, clockwise, counter clockwise or not known.
    \ingroup traits
    \par Geometries:
        - ring
        - polygon
        - multi polygon
    \par Specializations should provide:
        - typedef P type (where P should fulfil the Point concept)
    \tparam G geometry
*/
template <typename G>
struct point_order
{
    static const order_selector value = clockwise;
};


} // namespace traits


#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{

template <typename Tag, typename Geometry>
struct point_order
{
    static const order_selector value = clockwise;
};



template <typename Ring>
struct point_order<ring_tag, Ring>
{
    static const order_selector value = geometry::traits::point_order<Ring>::value;
};

// Specialization for polygon: the order is the order of its rings
template <typename Polygon>
struct point_order<polygon_tag, Polygon>
{
    static const order_selector value = core_dispatch::point_order
        <
            ring_tag,
            typename ring_type<polygon_tag, Polygon>::type
        >::value ;
};

} // namespace core_dispatch
#endif // DOXYGEN_NO_DISPATCH


/*!
    \brief Meta-function which defines point type of any geometry
    \ingroup core
*/
template <typename Geometry>
struct point_order
{
    typedef typename boost::remove_const<Geometry>::type ncg;
    static const order_selector value = core_dispatch::point_order
        <
            typename tag<Geometry>::type,
            ncg
        >::value;
};

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_CORE_POINT_ORDER_HPP
