// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright Barend Gehrels 2011, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Quickbook Example

//[area
//` Calculate the area of a polygon

#include <iostream>
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/extensions/gis/io/wkt/wkt.hpp>

namespace bg = boost::geometry; /*< Convenient namespace alias >*/

int main()
{
    // Calculate the area of a cartesian polygon
    bg::model::polygon<bg::model::d2::point_xy<double> > poly;
    bg::read_wkt("POLYGON((0 0,0 7,4 2,2 0,0 0))", poly);
    double area = bg::area(poly);
    std::cout << "Area: " << area << std::endl;

    // Calculate the area of a spherical polygon
    bg::model::polygon<bg::model::point<float, 2, bg::cs::spherical<bg::degree> > > sph_poly;
    bg::read_wkt("POLYGON((0 0,0 45,45 0,0 0))", sph_poly);
    area = bg::area(sph_poly);
    std::cout << "Area: " << area << std::endl;

    return 0;
}

//]


//[area_output
/*`
Output:
[pre
Area: 16
Area: 0.339837
]
*/
//]
