// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <iterator>
#include <sstream>
#include <string>

#include <geometry_test_common.hpp>

#include <boost/geometry/geometries/cartesian2d.hpp>
#include <boost/geometry/ranges/segment_range.hpp>
#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>

#include <test_geometries/custom_segment.hpp>


template <typename Segment>
void test_geometry(std::string const& wkt, std::string const& expected)
{
    namespace bg = boost::geometry;

    Segment segment;
    bg::read_wkt(wkt, segment);

    typedef bg::segment_range<Segment> range_type;
    range_type range(segment);

    {
        std::ostringstream out;
        for (typename boost::range_iterator<range_type>::type it = boost::begin(range); 
            it != boost::end(range); ++it)
        {
            out << " " << boost::geometry::get<0>(*it) << boost::geometry::get<1>(*it);
        }
        BOOST_CHECK_EQUAL(out.str(), expected);
    }

    {
        // Check forward/backward behaviour
        std::ostringstream out;
        typename boost::range_iterator<range_type>::type it = boost::begin(range); 
        it++;
        it--;
        out << " " << boost::geometry::get<0>(*it) << boost::geometry::get<1>(*it);
        typename boost::range_iterator<range_type>::type it2 = boost::end(range);
        it2--;
        out << " " << boost::geometry::get<0>(*it2) << boost::geometry::get<1>(*it2);
        BOOST_CHECK_EQUAL(out.str(), expected);
    }
}


template <typename P>
void test_all()
{
    test_geometry<test::custom_segment>("linestring(1 1,2 2)", " 11 22");
    test_geometry<test::custom_segment>("linestring(4 4,3 3)", " 44 33");
}


int test_main(int, char* [])
{
    test_all<boost::geometry::point_2d>();
    return 0;
}
