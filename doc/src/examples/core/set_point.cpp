// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2011, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Quickbook Example

//[set_point
//` Set the coordinate of a point

#include <iostream>
#include <boost/geometry/geometry.hpp>

namespace bg = boost::geometry;

int main()
{
    bg::model::d2::point_xy<double> point;

    bg::set<0>(point, 1);
    bg::set<1>(point, 2);

    std::cout << "Location: " << bg::dsv(point) << std::endl;

    return 0;
}

//]


//[set_point_output
/*`
Output:
[pre
Location: (1, 2)
]
*/
//]
