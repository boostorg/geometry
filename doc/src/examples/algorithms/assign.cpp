// Boost.Geometry (aka GGL, Generic Geometry Library)
// QuickBook Example

// Copyright (c) 2011 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//[assign
//` Shows how to assign a geometry to another

#include <iostream>

#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>

int main()
{
    typedef boost::geometry::model::d2::point_xy<double> point;
    typedef boost::geometry::model::box<point> box;
    typedef boost::geometry::model::polygon<point> polygon;

    point p1;
    box b;
    boost::geometry::assign_values(p1, 1, 1);
    boost::geometry::assign_values(b, 1, 1, 2, 2);
    
    // Assign a box to a polygon (conversion box->poly)
    polygon p;
    boost::geometry::assign(b, p);

    // Assign a point to another point type (conversion of point-type)
    boost::tuple<double, double> p2;
    boost::geometry::assign(p1, p2);
        
    
    using boost::geometry::dsv;
    std::cout
        << "box: " << dsv(b) << std::endl
        << "polygon: " << dsv(p) << std::endl
        << "point: " << dsv(p) << std::endl
        << "point tuples: " << dsv(p2) << std::endl
        ;

    return 0;
}

//]


//[assign_output
/*`
Output:
[pre
box: ((1, 1), (2, 2))
first: (1, 1)
second: (2, 2)
]
*/
//]
