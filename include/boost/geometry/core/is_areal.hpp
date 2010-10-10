// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_CORE_IS_AREAL_HPP
#define BOOST_GEOMETRY_CORE_IS_AREAL_HPP


#include <boost/type_traits.hpp>


#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{

template <typename GeometryTag> struct is_areal : boost::false_type {};

template <> struct is_areal<ring_tag> : boost::true_type {};
template <> struct is_areal<box_tag> : boost::true_type {};
template <> struct is_areal<polygon_tag> : boost::true_type {};


} // namespace core_dispatch
#endif



/*!
    \brief Meta-function defining "true" for areal types (box, (multi)polygon, ring),
    \note Used for tag dispatching and meta-function finetuning
    \note Even though linear_ring is called linear, it has areal properties.
    \ingroup core
*/
template <typename Geometry>
struct is_areal : core_dispatch::is_areal<typename tag<Geometry>::type>
{};


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_CORE_IS_AREAL_HPP
