// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_EXTENSIONS_NSPHERE_CORE_TOPOLOGICAL_DIMENSION_HPP
#define BOOST_GEOMETRY_EXTENSIONS_NSPHERE_CORE_TOPOLOGICAL_DIMENSION_HPP


#include <boost/geometry/core/topological_dimension.hpp>

#include <boost/geometry/extensions/nsphere/core/tags.hpp>


namespace boost { namespace geometry {


#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch {



// nsphere: 2, but there is discussion. Is it CLOSED? Then 2, but
// then it should be called "disk"...
template <>
struct top_dim<nsphere_tag>    : boost::mpl::int_<2> {};




} // namespace core_dispatch
#endif



}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_EXTENSIONS_NSPHERE_CORE_TOPOLOGICAL_DIMENSION_HPP
