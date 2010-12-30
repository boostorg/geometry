// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_EXTENSIONS_NSPHERE_CORE_GEOMETRY_ID_HPP
#define BOOST_GEOMETRY_EXTENSIONS_NSPHERE_CORE_GEOMETRY_ID_HPP


#include <boost/geometry/core/geometry_id.hpp>

#include <boost/geometry/extensions/nsphere/core/tags.hpp>


namespace boost { namespace geometry {


#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{


template <>
struct geometry_id<nsphere_tag>    : boost::mpl::int_<91> {};



} // namespace core_dispatch
#endif




}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_EXTENSIONS_NSPHERE_CORE_GEOMETRY_ID_HPP
