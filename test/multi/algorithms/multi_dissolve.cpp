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
#include <boost/geometry/multi/algorithms/dissolve.hpp>
#include <boost/geometry/multi/algorithms/envelope.hpp>
#include <boost/geometry/multi/algorithms/num_points.hpp>
#include <boost/geometry/multi/core/interior_rings.hpp>
	
#include <boost/geometry/multi/geometries/multi_polygon.hpp>

#include <boost/geometry/extensions/gis/io/wkt/read_wkt_multi.hpp>
#include <boost/geometry/extensions/io/svg/write_svg_multi.hpp>


template <typename P>
void test_all()
{
    namespace bg = boost::geometry;
    typedef bg::polygon<P> polygon;
    typedef bg::multi_polygon<polygon> multi_polygon;

    test_one<multi_polygon>("three_triangles",
		"MULTIPOLYGON(((1 1,5 5,8 0,1 1)),((4 2,0 8,5 9,4 2)),((5 3,4 8,10 4,5 3)))" ,
		1, 15, 42.614078674948232);
}


int test_main(int, char* [])
{
    test_all<boost::geometry::point_xy<double> >();

    return 0;
}
