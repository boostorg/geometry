// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_MULTI_TOPOLOGICAL_DIMENSION_HPP
#define BOOST_GEOMETRY_MULTI_TOPOLOGICAL_DIMENSION_HPP


#include <boost/mpl/int.hpp>


#include <boost/geometry/core/topological_dimension.hpp>
#include <boost/geometry/multi/core/tags.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{

template <>
struct top_dim<multi_point_tag> : boost::mpl::int_<0> {};


template <>
struct top_dim<multi_linestring_tag> : boost::mpl::int_<1> {};


template <>
struct top_dim<multi_polygon_tag> : boost::mpl::int_<2> {};


} // namespace core_dispatch
#endif


}} // namespace boost::geometry


#endif
