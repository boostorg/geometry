// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <iterator>
#include <string>


#include <ggl_test_common.hpp>

#include <boost/geometry/extensions/algorithms/midpoints.hpp>
#include <boost/geometry/algorithms/make.hpp>
#include <boost/geometry/geometries/geometries.hpp>

#include <boost/geometry/util/write_dsv.hpp>


#include <boost/geometry/extensions/gis/io/wkt/wkt.hpp>


#include <test_common/test_point.hpp>

template <typename G>
void test_midpoints(const G& g, bool start_and_end)
{
    G processed;
    boost::geometry::midpoints(g, start_and_end, std::back_inserter(processed));
    std::cout << dsv(processed) << std::endl;
}

template <typename G>
void test_midpoints(const std::string& wkt)
{
    G g;
    boost::geometry::read_wkt(wkt, g);
    test_midpoints(g, true);
    test_midpoints(g, false);
}

template <typename P>
void test_all()
{
    test_midpoints<boost::geometry::linestring<P> >("LINESTRING(1 1,2 2,3 3)");
}

int test_main(int, char* [])
{
    //test_all<boost::geometry::point_xy<float> >();
    test_all<boost::geometry::point_xy<double> >();

    return 0;
}
