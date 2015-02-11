// Boost.Geometry

// Copyright (c) 2015 Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_UTIL_CONDITION_HPP
#define BOOST_GEOMETRY_UTIL_CONDITION_HPP


#include <boost/config.hpp>


#ifdef BOOST_MSVC

//namespace boost { namespace geometry { namespace detail {
//BOOST_FORCEINLINE bool condition(bool const b) { return b; }
//}}} // boost::geometry::detail
//#define BOOST_GEOMETRY_CONDITION(CONDITION) boost::geometry::detail::condition(CONDITION)

#define BOOST_GEOMETRY_CONDITION(CONDITION) ((void)0, (CONDITION))

#else

#define BOOST_GEOMETRY_CONDITION(CONDITION) (CONDITION)

#endif


#endif // BOOST_GEOMETRY_UTIL_CONDITION_HPP
