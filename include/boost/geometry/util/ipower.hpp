// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2014, Oracle and/or its affiliates.

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_UTIL_IPOWER_HPP
#define BOOST_GEOMETRY_UTIL_IPOWER_HPP

#include <cstddef>

#include <boost/static_assert.hpp>

#include <boost/mpl/equal_to.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/integral_c.hpp>
#include <boost/mpl/size_t.hpp>
#include <boost/mpl/times.hpp>


namespace boost { namespace geometry
{

namespace util
{


template <typename T>
struct is_odd
    : boost::mpl::equal_to
        <
            boost::mpl::integral_c<typename T::value_type, T::value % 2>,
            boost::mpl::integral_c<typename T::value_type, 1>
        >
{};


template <typename T>
struct square : boost::mpl::times<T, T>
{};


// compile time computation of the power of an integral constant to
// a non-negative integral power
//
// implements the basic exponentiation by squaring algorithm; see:
//      http://en.wikipedia.org/wiki/Exponentiation_by_squaring
//
template <typename T, std::size_t Power>
struct ipower
    : boost::mpl::if_
        <
            is_odd<boost::mpl::size_t<Power> >,
            boost::mpl::times<ipower<square<T>, (Power - 1) / 2>, T>,
            ipower<square<T>, Power / 2>
        >::type
{};

template <typename T>
struct ipower<T, 1> : T
{};

template <typename T>
struct ipower<T, 0>
    : boost::mpl::integral_c<typename T::value_type, 1>
{
    BOOST_STATIC_ASSERT( (T::value != 0) );
};


} // namespace util

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_UTIL_IPOWER_HPP
