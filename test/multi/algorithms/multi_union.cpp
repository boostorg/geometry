// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <string>


#include <algorithms/test_union.hpp>
#include <algorithms/test_overlay.hpp>
#include <multi/algorithms/overlay/multi_overlay_cases.hpp>

#include <boost/geometry/multi/algorithms/correct.hpp>
#include <boost/geometry/multi/algorithms/union.hpp>

#include <boost/geometry/multi/geometries/multi_linestring.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>

#include <boost/geometry/extensions/gis/io/wkt/read_wkt_multi.hpp>


template <typename Ring, typename Polygon, typename MultiPolygon>
void test_areal()
{
    test_one<Polygon, MultiPolygon, MultiPolygon>("simplex_multi",
        case_multi_simplex[0], case_multi_simplex[1],
        1, 0, 20, 14.58);

    test_one<Polygon, Polygon, MultiPolygon>("simplex_multi_p_mp",
        case_single_simplex, case_multi_simplex[0],
        1, 0, 20, 14.58);
    test_one<Polygon, MultiPolygon, Polygon>("simplex_multi_mp_p",
        case_multi_simplex[0], case_single_simplex,
        1, 0, 20, 14.58);

    test_one<Polygon, Ring, MultiPolygon>("simplex_multi_r_mp",
        case_single_simplex, case_multi_simplex[0],
        1, 0, 20, 14.58);
    test_one<Ring, MultiPolygon, Polygon>("simplex_multi_mp_r",
        case_multi_simplex[0], case_single_simplex,
        1, 0, 20, 14.58);


    // Normal test cases
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_multi_no_ip",
        case_multi_no_ip[0], case_multi_no_ip[1],
        4, 0, 16, 66.5);
    test_one<Polygon, MultiPolygon, MultiPolygon>("case_multi_2",
        case_multi_2[0], case_multi_2[1],
        3, 0, 16, 59.1);
}

template <typename P>
void test_all()
{
    namespace bg = boost::geometry;

    {
        typedef bg::linear_ring<P> ring;
        typedef bg::polygon<P> polygon;
        typedef bg::multi_polygon<polygon> multi_polygon;
        test_areal<ring, polygon, multi_polygon>();
    }

    {
        typedef bg::linear_ring<P, std::vector, false> ring_ccw;
        typedef bg::polygon<P, std::vector, std::vector, false> polygon_ccw;
        typedef bg::multi_polygon<polygon_ccw> multi_polygon_ccw;
        test_areal<ring_ccw, polygon_ccw, multi_polygon_ccw>();
    }

}


int test_main(int, char* [])
{
    test_all<boost::geometry::point_xy<double> >();

    return 0;
}
