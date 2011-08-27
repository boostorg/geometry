// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - ASSERT
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_ASSERT_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_ASSERT_HPP

#ifdef NDEBUG

#define BOOST_GEOMETRY_INDEX_ASSERT_UNUSED_PARAM(PARAM)

#define BOOST_GEOMETRY_INDEX_ASSERT(CONDITION, TEXT_MSG)

#else

#define BOOST_GEOMETRY_INDEX_ASSERT_UNUSED_PARAM(PARAM) PARAM

#define BOOST_GEOMETRY_INDEX_ASSERT(CONDITION, TEXT_MSG) assert((CONDITION) && (TEXT_MSG))

#endif

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_ASSERT_HPP
