// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014 Samuel Debionne, Grenoble, France.

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef BOOST_GEOMETRY_UTIL_IS_IMPLEMENTED_HPP
#define BOOST_GEOMETRY_UTIL_IS_IMPLEMENTED_HPP

#include <boost/mpl/and.hpp>
#include <boost/mpl/not.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/type_traits/is_same.hpp>

namespace boost { namespace geometry
{

namespace util
{


template <typename Geometry1, typename Geometry2>
struct is_same_dimension_and_coordinate_system
    : mpl::and_
        <
            typename is_same
            <
                typename coordinate_system<Geometry1>::type,
                typename coordinate_system<Geometry2>::type
            >::type,
            typename is_same
            <
                typename dimension<Geometry1>::type,
                typename dimension<Geometry2>::type
            >::type
        >
{};


template <typename Geometry1, typename Geometry2>
struct is_implemented
    : mpl::not_
        <
            typename is_base_of
            <
                nyi::not_implemented_tag,
                geometry::dispatch::distance<Geometry1, Geometry2>
            >::type
        >
{};


template <typename Geometry1, typename Geometry2>
struct is_compatible
    : mpl::and_
        <
            typename is_same_dimension_and_coordinate_system<Geometry1, Geometry2>::type,
            typename is_implemented<Geometry1, Geometry2>::type
        >
{};


} // namespace util


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_UTIL_IS_IMPLEMENTED_HPP
