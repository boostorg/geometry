// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright (C) 2010 Mateusz Loskot <mateusz at loskot dot net>, London, UK
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Polygon Overlay Example

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include <boost/foreach.hpp>


#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/cartesian2d.hpp>
#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>

#include <boost/geometry/extensions/io/svg/svg_mapper.hpp>


int main(void)
{
    namespace bg = boost::geometry;

    std::ofstream stream("05_a_intersection_polygon_example.svg");
    bg::svg_mapper<bg::point_2d> svg(stream, 500, 500);

    // Define a polygons and fill the outer rings.
    bg::polygon_2d a;
    {
        const double c[][2] = {
            {160, 330}, {60, 260}, {20, 150}, {60, 40}, {190, 20}, {270, 130}, {260, 250}, {160, 330}
        };
        bg::assign(a, c);
    }
    bg::correct(a);
    std::cout << "A: " << bg::dsv(a) << std::endl;
    svg.add(a);

    bg::polygon_2d b;
    {
        const double c[][2] = {
            {300, 330}, {190, 270}, {150, 170}, {150, 110}, {250, 30}, {380, 50}, {380, 250}, {300, 330}
        };
        bg::assign(b, c);
    }
    bg::correct(b);
    std::cout << "B: " << bg::dsv(b) << std::endl;
    svg.add(b);

    svg.map(a, "opacity:0.6;fill:rgb(0,255,0);");
    svg.map(b, "opacity:0.6;fill:rgb(0,0,255);");


    // Calculate interesection(s)
    std::vector<bg::polygon_2d > intersection;
    bg::intersection<bg::polygon_2d>(a, b, intersection);

    std::cout << "Intersection of polygons A and B" << std::endl;
    BOOST_FOREACH(bg::polygon_2d const& polygon, intersection)
    {
        std::cout << bg::dsv(polygon) << std::endl;
        svg.map(polygon, "opacity:0.5;fill:none;stroke:rgb(255,0,0);stroke-width:6");
    }

    return 0;
}
