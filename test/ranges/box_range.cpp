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
#include <boost/geometry/ranges/box_range.hpp>
#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>



template <typename Box>
void test_geometry(std::string const& wkt, std::string const& expected)
{
    namespace bg = boost::geometry;

    Box box;
    bg::read_wkt(wkt, box);

    typedef bg::box_range<Box> range_type;
    range_type range(box);

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
        typename boost::range_iterator<range_type>::type it = boost::begin(range);
        it++;
        it--;
        // Not verified further, same as segment
    }
}


template <typename P>
void test_all()
{
    test_geometry<boost::geometry::box<P> >("polygon((1 1,2 2))", " 11 12 22 21 11");
    test_geometry<boost::geometry::box<P> >("polygon((3 3,5 5))", " 33 35 55 53 33");
}


int test_main(int, char* [])
{
    test_all<boost::geometry::point_2d>();
    return 0;
}
