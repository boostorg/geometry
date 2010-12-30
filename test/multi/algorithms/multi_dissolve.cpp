// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <string>


#include <algorithms/test_dissolve.hpp>

#include <boost/geometry/multi/core/is_multi.hpp>

#include <boost/geometry/multi/algorithms/area.hpp>
#include <boost/geometry/multi/algorithms/length.hpp>
#include <boost/geometry/multi/algorithms/dissolve.hpp>
#include <boost/geometry/multi/algorithms/envelope.hpp>
#include <boost/geometry/multi/algorithms/num_points.hpp>
#include <boost/geometry/multi/core/interior_rings.hpp>

#include <boost/geometry/multi/geometries/multi_linestring.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>

#include <boost/geometry/extensions/gis/io/wkt/read_wkt_multi.hpp>
#include <boost/geometry/extensions/io/svg/write_svg_multi.hpp>


template <typename P>
void test_all()
{
    typedef bg::model::polygon<P> polygon;
    typedef bg::model::multi_polygon<polygon> multi_polygon;

    test_one<multi_polygon, polygon>("three_triangles",
        "MULTIPOLYGON(((1 1,5 5,8 0,1 1)),((4 2,0 8,5 9,4 2)),((5 3,4 8,10 4,5 3)))" ,
        1, 13, 42.614078674948232);

    test_one<multi_polygon, polygon>("simplex_two",
        "MULTIPOLYGON(((0 0,1 4,4 1,0 0)),((2 2,3 6,6 3,2 2)))",
        0, 8, 14.7);
    test_one<multi_polygon, polygon>("simplex_three",
        "MULTIPOLYGON(((0 0,1 4,4 1,0 0)),((2 2,3 6,6 3,2 2)),((3 4,5 6,6 2,3 4)))",
        0, 14, 16.7945);
    test_one<multi_polygon, polygon>("simplex_four",
        "MULTIPOLYGON(((0 0,1 4,4 1,0 0)),((2 2,3 6,6 3,2 2)),((3 4,5 6,6 2,3 4)),((5 5,7 7,8 4,5 5)))",
        0, 18, 20.7581);

    // disjoint
    test_one<multi_polygon, polygon>("simplex_disjoint",
        "MULTIPOLYGON(((0 0,1 4,4 1,0 0)),((1 6,2 10,5 7,1 6)),((3 4,5 6,6 2,3 4)),((6 5,8 7,9 4,6 5)))",
        0, 16, 24.0);

    // new hole of four
    test_one<multi_polygon, polygon>("new_hole",
        "MULTIPOLYGON(((0 0,1 4,4 1,0 0)),((2 2,3 6,6 3,2 2)),((3 4,5 6,6 2,3 4)),((3 1,5 4,8 4,3 1)))",
        1, 18, 19.5206);

}


int test_main(int, char* [])
{
    test_all<bg::model::d2::point_xy<double> >();

    return 0;
}
