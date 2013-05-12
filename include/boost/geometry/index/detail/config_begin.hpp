// Boost.Geometry Index
//
// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/config.hpp>

#ifdef BOOST_MSVC

    #define BOOST_GEOMETRY_INDEX_DETAIL_USE_PARAM(A) (void)A;

    #pragma warning (push)
    #pragma warning (disable : 4512) // assignment operator could not be generated
    #pragma warning (disable : 4127) // conditional expression is constant

    // temporary?
    #pragma warning (disable : 4180) // qualifier applied to function type has no meaning

#else //BOOST_MSVC

    #define BOOST_GEOMETRY_INDEX_DETAIL_USE_PARAM(A)

#endif   //BOOST_MSVC

