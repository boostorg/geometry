// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2011, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Quickbook Example

//[intersects
//` Check if two linestrings intersect each other

#include <iostream>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/extensions/gis/io/wkt/wkt.hpp>

int main()
{
    // Calculate the intersects of a cartesian polygon
    typedef boost::geometry::model::d2::point_xy<double> P;
    bg::model::linestring<P> line1, line2;

    boost::geometry::read_wkt("linestring(1 1,2 2)", line1);
    boost::geometry::read_wkt("linestring(2 1,1 2)", line2);

    bool b = boost::geometry::intersects(line1, line2);

    std::cout << "Intersects: " << (b ? "YES" : "NO") << std::endl;

    return 0;
}

//]


//[intersects_output
/*`
Output:
[pre
Intersects: YES
]
*/
//]
