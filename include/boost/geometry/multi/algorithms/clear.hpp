// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_MULTI_ALGORITHMS_CLEAR_HPP
#define BOOST_GEOMETRY_MULTI_ALGORITHMS_CLEAR_HPP


#include <boost/geometry/multi/core/tags.hpp>
#include <boost/geometry/algorithms/clear.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{

template <typename Geometry>
struct clear<multi_point_tag, Geometry>
    : detail::clear::collection_clear<Geometry>
{};


template <typename Geometry>
struct clear<multi_linestring_tag, Geometry>
    : detail::clear::collection_clear<Geometry>
{};

template <typename Geometry>
struct clear<multi_polygon_tag, Geometry>
    : detail::clear::collection_clear<Geometry>
{};

} // namespace dispatch
#endif


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_MULTI_ALGORITHMS_CLEAR_HPP
