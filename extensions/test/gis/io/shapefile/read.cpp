// Boost.Geometry
// Unit Test

// Copyright (c) 2019, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <geometry_test_common.hpp>

#include <boost/geometry/extensions/gis/io/shapefile/read.hpp>


int test_main(int, char*[])
{
    bg::detail::shapefile::double_endianness_check();

    return 0;
}
