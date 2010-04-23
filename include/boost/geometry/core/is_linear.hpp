// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_CORE_IS_LINEAR_HPP
#define BOOST_GEOMETRY_CORE_IS_LINEAR_HPP


#include <boost/type_traits.hpp>


#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/core/tags.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{

template <typename GeometryTag>
struct is_linear : boost::false_type {};


template <>
struct is_linear<linestring_tag> : boost::true_type {};


template <>
struct is_linear<ring_tag>       : boost::true_type {};


} // namespace core_dispatch
#endif



/*!
    \brief Meta-function defining "true" for linear types (linestring,ring),
        "false" for non-linear typse
    \note Used for tag dispatching and meta-function finetuning
    \ingroup core
*/
template <typename Geometry>
struct is_linear : core_dispatch::is_linear<typename tag<Geometry>::type>
{};


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_CORE_IS_LINEAR_HPP
