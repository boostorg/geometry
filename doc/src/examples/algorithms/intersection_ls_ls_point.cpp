// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2011, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Quickbook Example

//[intersection_ls_ls_point
//` Calculate the intersection of two linestrings

#include <iostream>
#include <deque>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/domains/gis/io/wkt/wkt.hpp>
#include <boost/geometry/geometries/adapted/std_as_linestring.hpp> /*< Adapts std::vector to linestring concept >*/

#include <boost/foreach.hpp>


int main()
{
    typedef boost::geometry::model::d2::point_xy<double> P;
    std::vector<P> line1, line2;
    boost::geometry::read_wkt("linestring(1 1,2 2,3 1)", line1);
    boost::geometry::read_wkt("linestring(1 2,2 1,3 2)", line2);

    std::deque<P> intersection_points;
    boost::geometry::intersection(line1, line2, intersection_points);

    BOOST_FOREACH(P const& p, intersection_points)
    {
        std::cout << " " << boost::geometry::wkt(p);
    }
    std::cout << std::endl;

    return 0;
}

//]


//[intersection_ls_ls_point_output
/*`
Output:
[pre
 POINT(1.5 1.5) POINT(2.5 1.5)
]
*/
//]
