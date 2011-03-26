// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2011 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Quickbook Example

//[for_each_point_const
//` Sample using for_each_point, using a function to list coordinates

#include <iostream>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/domains/gis/io/wkt/wkt.hpp>


template <typename Point>
void list_coordinates(Point const& p)
{
    using boost::geometry::get;
    std::cout << "x = " << get<0>(p) << " y = " << get<1>(p) << std::endl;
}

int main()
{
    typedef boost::geometry::model::d2::point_xy<double> point;
    boost::geometry::model::polygon<point> poly;
    boost::geometry::read_wkt("POLYGON((0 0,0 4,4 0,0 0))", poly);
    boost::geometry::for_each_point(poly, list_coordinates<point>);
    return 0;
}

//]


//[for_each_point_const_output
/*`
Output:
[pre
x = 0 y = 0
x = 0 y = 4
x = 4 y = 0
x = 0 y = 0
]
*/
//]
