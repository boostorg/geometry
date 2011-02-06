// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2011, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Quickbook Example

//[get_point
//` Get the coordinate of a point

#include <iostream>
#include <boost/geometry/geometry.hpp>

namespace bg = boost::geometry;

int main()
{
    bg::model::d2::point_xy<double> point(1, 2);

    double x = bg::get<0>(point);
    double y = bg::get<1>(point);

    std::cout << "x=" << x << " y=" << y << std::endl;

    return 0;
}

//]


//[get_point_output
/*`
Output:
[pre
x=1 y=2
]
*/
//]
