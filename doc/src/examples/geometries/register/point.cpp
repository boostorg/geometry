// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2011, Geodan, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Quickbook Example

//[register_point_2d
//` Show the use of the macro BOOST_GEOMETRY_REGISTER_POINT_2D

#include <iostream>
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/register/point.hpp>

/*< Somewhere, any legacy point struct is defined >*/
struct legacy_point
{
    double x, y;
};

BOOST_GEOMETRY_REGISTER_POINT_2D(legacy_point, double, cs::cartesian, x, y) /*< The magic: adapt it to Boost.Geometry Point Concept >*/

int main()
{
    legacy_point p1, p2;

    namespace bg = boost::geometry;

    /*< Any Boost.Geometry function can be used for legacy point now. Here: assign and distance >*/
    bg::assign(p1, 1, 1);
    bg::assign(p2, 2, 2);

    double d = bg::distance(p1, p2);

    std::cout << "Distance: " << d << std::endl;

    return 0;
}

//]


//[register_point_2d_output
/*`
Output:
[pre
Distance: 1.41421
]
*/
//]
