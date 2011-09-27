// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - unit tests
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_TEST_MODULE test_module_boost_geometry_index
#include <boost/test/unit_test.hpp>

#include <ctime>

BOOST_AUTO_TEST_CASE( first_test_case )
{
    ::srand( (unsigned)::time(NULL) );
}

#define TEST_PRINT_INFO

#include <tests/translators.hpp>
#include <tests/rtree_function.hpp>
#include <tests/rtree_filters.hpp>

#include <boost/geometry/extensions/index/rtree/rtree.hpp>

BOOST_AUTO_TEST_CASE( last_test_case )
{
    tests_rtree_filters_hpp();

#ifdef _MSC_VER
    std::cin.get();
#endif

}
