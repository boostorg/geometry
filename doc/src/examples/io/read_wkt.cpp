// Boost.Geometry (aka GGL, Generic Geometry Library)
// QuickBook Example

// Copyright (c) 2014 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//[read_wkt
//` Shows the usage of read_wkt

#include <iostream>
#include <fstream>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>

int main()
{
    // Specify the basic type
    typedef boost::geometry::model::d2::point_xy<double> point_type;

    // Declare some geometries and set their values
    point_type a;
    boost::geometry::model::polygon<point_type> b;

    boost::geometry::read_wkt("POINT(1 2)", a);
    boost::geometry::read_wkt("POLYGON((0 0,0 7,4 2,2 0,0 0))", b);


    return 0;
}

//]

