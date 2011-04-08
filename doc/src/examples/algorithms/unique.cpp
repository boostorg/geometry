// Boost.Geometry (aka GGL, Generic Geometry Library)
// QuickBook Example

// Copyright (c) 2011 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//[unique
//` Shows how to make a so-called minimal set of a polygon by removing duplicate points

#include <iostream>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>

int main()
{
    boost::geometry::model::polygon<boost::tuple<double, double> > poly;
    boost::geometry::read_wkt("POLYGON((0 0,0 0,0 5,5 5,5 5,5 5,5 0,5 0,0 0,0 0,0 0,0 0))", poly);
    boost::geometry::unique(poly);
    std::cout << boost::geometry::wkt(poly) << std::endl;

    return 0;
}

//]


//[unique_output
/*`
Output:
[pre
POLYGON((0 0,0 5,5 5,5 0,0 0))
]
*/
//]
