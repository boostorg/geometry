// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_MULTI_CORE_IS_MULTI_HPP
#define BOOST_GEOMETRY_MULTI_CORE_IS_MULTI_HPP


#include <boost/type_traits.hpp>


#include <boost/geometry/core/is_multi.hpp>
#include <boost/geometry/multi/core/tags.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{

template <>
struct is_multi<multi_point_tag> : boost::true_type {};


template <>
struct is_multi<multi_linestring_tag> : boost::true_type {};


template <>
struct is_multi<multi_polygon_tag> : boost::true_type {};


} // namespace core_dispatch
#endif


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_MULTI_CORE_IS_MULTI_HPP
