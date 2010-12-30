// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_MULTI_ALGORITHMS_REVERSE_HPP
#define BOOST_GEOMETRY_MULTI_ALGORITHMS_REVERSE_HPP


#include <boost/range/metafunctions.hpp>

#include <boost/geometry/algorithms/reverse.hpp>
#include <boost/geometry/multi/algorithms/detail/modify.hpp>

#include <boost/geometry/multi/core/tags.hpp>



namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DISPATCH
namespace dispatch
{


template <typename Geometry>
struct reverse<multi_linestring_tag, Geometry>
    : detail::multi_modify
        <
            Geometry,
            detail::reverse::range_reverse
                <
                    typename boost::range_value<Geometry>::type
                >
        >
{};


template <typename Geometry>
struct reverse<multi_polygon_tag, Geometry>
    : detail::multi_modify
        <
            Geometry,
            detail::reverse::polygon_reverse
                <
                    typename boost::range_value<Geometry>::type
                >
        >
{};


} // namespace dispatch
#endif // DOXYGEN_NO_DISPATCH


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_MULTI_ALGORITHMS_REVERSE_HPP
