// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Bruno Lalande 2008, 2009
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_EXTENSIONS_NSPHERE_CORE_REPLACE_POINT_TYPE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_NSPHERE_CORE_REPLACE_POINT_TYPE_HPP


#include <boost/geometry/core/replace_point_type.hpp>

#include <boost/geometry/extensions/nsphere/core/tags.hpp>
#include <boost/geometry/extensions/nsphere/geometries/nsphere.hpp>



namespace boost { namespace geometry {

#ifndef DOXYGEN_NO_DISPATCH
namespace core_dispatch
{


template <typename Geometry, typename NewPointType>
struct replace_point_type<nsphere_tag, Geometry, NewPointType>
{
    typedef typename geometry::coordinate_type<Geometry>::type coortype;
    typedef model::nsphere<NewPointType, coortype> type;
};


} // namespace core_dispatch
#endif // DOXYGEN_NO_DISPATCH



}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_NSPHERE_CORE_REPLACE_POINT_TYPE_HPP
