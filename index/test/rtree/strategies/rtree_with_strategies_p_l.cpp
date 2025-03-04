// Boost.Geometry Index
// Unit Test

// Copyright (c) 2025 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "rtree_with_strategies.hpp"

int test_main(int, char* [])
{
    test_rtree<point, bgi::linear<4>>();

    return 0;
}
