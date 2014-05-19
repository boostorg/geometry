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

#include <boost/geometry/core/coordinate_system.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>

#include <boost/geometry/algorithms/not_implemented.hpp>


namespace boost { namespace geometry
{

namespace util
{

namespace detail
{

template <typename Geometry, typename Algorithm>
struct is_implemented1
    : mpl::not_
        <
            typename is_base_of
            <
                nyi::not_implemented_tag,
                //typename mpl::apply1<Algorithm, Geometry>::type
                Algorithm                
            >::type
        >
{};


template <typename Geometry1, typename Geometry2, typename Algorithm>
struct is_implemented2
    : mpl::not_
        <
            typename is_base_of
            <
                nyi::not_implemented_tag,
                //typename mpl::apply2<Algorithm, Geometry1, Geometry2>::type
                Algorithm
            >::type
        >
{};


} // namespace detail


/*!
    \brief Meta-function to check whether two geometry types
        have the same dimensions and coordinate systems
    \ingroup utility
*/
template <typename Geometry1, typename Geometry2>
struct is_compatible2
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


/*!
    \brief Meta-function to check whether an Unary Algorithm
        is implemented for the given geometry types
    \ingroup utility
*/
template <typename Geometry, typename Algorithm>
struct is_implemented1
    : mpl::and_
        <
            typename detail::is_implemented1<Geometry, Algorithm>::type
        >
{};


/*!
    \brief Meta-function to check whether an Binary Algorithm 
        is implemented for the given geometry types
    \ingroup utility
*/
template <typename Geometry1, typename Geometry2, typename Algorithm>
struct is_implemented2
    : mpl::and_
        <
            // is_compatible is necessary because of some MPL_ASSERT
            // shortcuting the not_implemented class when dimension / coordinate system differs
            typename is_compatible2<Geometry1, Geometry2>::type,
            typename detail::is_implemented2<Geometry1, Geometry2, Algorithm>::type
        >
{};


} // namespace util


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_UTIL_IS_IMPLEMENTED_HPP
