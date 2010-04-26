// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <string>


#include <algorithms/test_intersection.hpp>
#include <algorithms/test_overlay.hpp>
#include <multi/algorithms/overlay/multi_overlay_cases.hpp>

#include <boost/geometry/multi/algorithms/intersection.hpp>

#include <boost/geometry/multi/geometries/multi_linestring.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>

#include <boost/geometry/extensions/gis/io/wkt/read_wkt_multi.hpp>


template <typename P>
void test_all()
{
    // polygon/multi polygon and ring/multi polygon are tested in union

    namespace bg = boost::geometry;
    typedef bg::polygon<P> polygon;
    typedef bg::multi_polygon<polygon> multi_polygon;

    test_one<polygon, multi_polygon, multi_polygon>("simplex_multi",
        case_multi_simplex[0], case_multi_simplex[1],
        2, 12, 6.42);

    test_one<polygon, multi_polygon, multi_polygon>("case_multi_no_ip",
        case_multi_no_ip[0], case_multi_no_ip[1],
        2, 8, 8.5);
    test_one<polygon, multi_polygon, multi_polygon>("case_multi_2",
        case_multi_2[0], case_multi_2[1],
        3, 12, 5.9);
}


int test_main(int, char* [])
{
    test_all<boost::geometry::point_xy<double> >();

    return 0;
}
