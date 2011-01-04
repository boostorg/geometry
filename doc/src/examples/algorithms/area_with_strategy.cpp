// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2011, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Quickbook Example

//[area_with_strategy
//` Calculate the area of a polygon specifying a strategy

#include <iostream>
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/extensions/gis/io/wkt/wkt.hpp>

namespace bg = boost::geometry;

int main()
{
    // Define a spherical point
    typedef bg::model::point<double, 2, bg::cs::spherical<bg::degree> > pnt_type;

    bg::model::polygon<pnt_type> hawaii;
    bg::read_wkt("POLYGON((-155.86 18.93,-155.84 20.30,-154.80 19.52,-155.86 18.93))" /*< Rough appromation of [@http://en.wikipedia.org/wiki/Hawaii_%28island%29 Hawaii Island] >*/
        , hawaii);
    double const mean_radius = 6371.0; /*< [@http://en.wikipedia.org/wiki/Earth_radius Wiki]  >*/


    // Construct the strategy (Huiller) and calculate the area
    bg::strategy::area::huiller<pnt_type> in_square_kilometers(mean_radius);
    double area = bg::area(hawaii, in_square_kilometers);

    std::cout << area << " km2" << std::endl;

    return 0;
}

//]


//[area_with_strategy_output
/*`
Output:
[pre
Area: 8393.22 km2
]
*/
//]
