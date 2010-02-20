// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


// Include the single-geometry version
#define BOOST_GEOMETRY_TEST_MULTI
#include "traverse.cpp"


#include <boost/geometry/multi/core/ring_type.hpp>

#include <boost/geometry/multi/algorithms/num_points.hpp>
#include <boost/geometry/multi/algorithms/overlay/get_turns.hpp>
#include <boost/geometry/multi/algorithms/overlay/copy_segments.hpp>
#include <boost/geometry/multi/algorithms/overlay/copy_segment_point.hpp>
#include <boost/geometry/multi/algorithms/detail/sections/get_full_section.hpp>

#include <boost/geometry/multi/geometries/multi_linestring.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>

#include <boost/geometry/extensions/gis/io/wkt/read_wkt_multi.hpp>


#include "multi_overlay_cases.hpp"
#include "multi_svg_mapper.hpp"


// Because multi-intersections are similar to single,
// we don't have to test all cases.
// We only check "double" and the simplex (two triangles)
// and cases which are specific to multi (if any)


template <typename T>
void test_all()
{
    namespace bg = boost::geometry;
    namespace ov = boost::geometry::detail::overlay;

    typedef bg::multi_polygon
        <
            bg::polygon
                <
                    bg::point<T, 2, bg::cs::cartesian>
                >
        > multi_polygon;

    typedef boost::tuple<int, double> Tuple;

    test_overlay<multi_polygon, multi_polygon,
        test_traverse<ov::operation_intersection>,  Tuple>
        (
            "simplex", boost::make_tuple(2, 6.42),
            case_multi_simplex[0], case_multi_simplex[1]
        );

    test_overlay<multi_polygon, multi_polygon,
        test_traverse<ov::operation_union>,  Tuple>
        (
            "simplex", boost::make_tuple(1, 14.58),
            case_multi_simplex[0], case_multi_simplex[1]
        );

    // Bug found with "recursive boxes"
    // TODO: Solve this bug
    /***
    test_overlay<multi_polygon, multi_polygon,
        test_traverse<ov::operation_union>,  Tuple>
        (
            "recursive_box1", boost::make_tuple(1, 14.58),
            "MULTIPOLYGON(((3 9,3 10,4 10,4 9,3 9)),((1 1,1 2,2 2,2 1,1 1)),((7 8,7 9,8 9,8 8,7 8)),((8 0,8 1,9 1,9 0,8 0)))", 
            "MULTIPOLYGON(((0 1,0 2,1 2,1 1,0 1)),((7 5,7 6,8 6,8 5,7 5)),((1 1,1 1,2 1,2 1,2 0,2 0,1 0,1 0,1 0,1 1)))"
        );
    ***/
}


int test_main(int, char* [])
{
    test_all<double>();

    return 0;
}


