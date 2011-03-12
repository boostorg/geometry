// Boost.Geometry (aka GGL, Generic Segment Library)
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

#include <geometry_test_common.hpp>

#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/iterators/segment_range_iterator.hpp>
#include <boost/geometry/domains/gis/io/wkt/read_wkt.hpp>

#include <test_geometries/custom_segment.hpp>


template <typename Segment>
void test_geometry(std::string const& wkt, std::string const& expected)
{
    Segment segment;
    bg::read_wkt(wkt, segment);

    std::ostringstream out;
    bg::segment_range_iterator<Segment> it(segment), end(segment, true);
    for ( ; it != end; ++it)
    {
        out << " " << bg::get<0>(*it) << bg::get<1>(*it);
    }
    BOOST_CHECK_EQUAL(out.str(), expected);
}


template <typename P>
void test_all()
{
    test_geometry<test::custom_segment>("linestring(1 1,2 2)", " 11 22");
    test_geometry<test::custom_segment>("linestring(4 4,3 3)", " 44 33");
}


int test_main(int, char* [])
{
    test_all<bg::model::d2::point_xy<double> >();

    return 0;
}
