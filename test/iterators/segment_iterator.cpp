// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Copyright Mateusz Loskot 2009, mateusz@loskot.net
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <list>
#include <sstream>
#include <string>
#include <vector>

#include <geometry_test_common.hpp>

#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/geometries/cartesian2d.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/iterators/segment_iterator.hpp>
#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>

template <typename C>
void test_linestring(const std::string& wkt, const std::string& expected)
{
    typedef C point_list;
    typedef typename C::value_type point;
    typedef typename C::iterator base_iterator;
    typedef boost::geometry::segment_iterator<base_iterator, point> segment_iterator;
    typedef typename segment_iterator::value_type segment;
    typedef boost::geometry::linestring<point> linestring;

    linestring g;
    boost::geometry::read_wkt(wkt, g);

    point_list v;
    std::copy(g.begin(), g.end(), std::back_insert_iterator<point_list>(v));
    BOOST_CHECK_EQUAL(g.size(), v.size());

    segment_iterator it(v.begin(), v.end());
    segment_iterator end(v.end());

    std::ostringstream oss;
    while (it != end)
    {
        segment const& s = *it;
        oss << boost::geometry::get<0>(s.first) << boost::geometry::get<1>(s.first)
            << boost::geometry::get<0>(s.second) << boost::geometry::get<1>(s.second);
        ++it;
    }
    BOOST_CHECK_EQUAL(oss.str(), expected);
}

int test_main(int, char* [])
{
    // Test std::vector
    typedef std::vector<boost::geometry::point_2d> points_v;
    test_linestring<points_v>("linestring empty", "");
    test_linestring<points_v>("linestring ()", "");
    test_linestring<points_v>("linestring (1 1)", "");
    test_linestring<points_v>("linestring (1 1, 2 2, 3 3)", "11222233");
    test_linestring<points_v>("linestring (1 1, 2 2, 3 3, 4 4)", "112222333344");
    test_linestring<points_v>("linestring (1 1, 2 2, 3 3, 4 4, 5 5, 6 6)", "11222233334444555566");

    // Test std::list
    typedef std::list<boost::geometry::point_2d> points_l;
    test_linestring<points_l>("linestring empty", "");
    test_linestring<points_l>("linestring ()", "");
    test_linestring<points_l>("linestring (1 1)", "");
    test_linestring<points_l>("linestring (1 1, 2 2, 3 3)", "11222233");
    test_linestring<points_l>("linestring (1 1, 2 2, 3 3, 4 4)", "112222333344");
    test_linestring<points_l>("linestring (1 1, 2 2, 3 3, 4 4, 5 5, 6 6)", "11222233334444555566");

    return 0;
}
