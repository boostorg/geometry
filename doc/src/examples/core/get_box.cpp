// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2011, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Quickbook Example

//[get_box
//` Get the coordinate of a box

#include <iostream>
#include <boost/geometry/geometry.hpp>

namespace bg = boost::geometry;

int main()
{
    bg::model::box<bg::model::d2::point_xy<double> > box;

    bg::assign(box, 1, 3, 5, 6);

    std::cout << "Box:"
        << " " << bg::get<bg::min_corner, 0>(box)
        << " " << bg::get<bg::min_corner, 1>(box)
        << " " << bg::get<bg::max_corner, 0>(box)
        << " " << bg::get<bg::max_corner, 1>(box)
        << std::endl;

    return 0;
}

//]


//[get_box_output
/*`
Output:
[pre
Box: 1 3 5 6
]
*/
//]
