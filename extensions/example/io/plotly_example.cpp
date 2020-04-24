// Boost.Geometry (aka GGL, Generic Geometry Library)
// QuickBook Example

// Copyright (c) 2013 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <fstream>
#include <boost/geometry.hpp>
#include <boost/geometry/extensions/io/plotly/plotter.hpp>
#include <boost/geometry/geometries/point_xy.hpp>

namespace bg = boost::geometry;

int main()
{
    typedef bg::model::point<double, 2, bg::cs::cartesian> point_t;
    typedef bg::model::polygon<point_t> polygon_t;

    polygon_t poly1;

    bg::append(poly1.outer(), point_t(0.0, 0.0));
    bg::append(poly1.outer(), point_t(0.0, 5.0));
    bg::append(poly1.outer(), point_t(5.0, 5.0));
    bg::append(poly1.outer(), point_t(5.0, 0.0));
    bg::append(poly1.outer(), point_t(0.0, 0.0));

    poly1.inners().resize(1);
    bg::append(poly1.inners()[0], point_t(1.0, 1.0));
    bg::append(poly1.inners()[0], point_t(4.0, 1.0));
    bg::append(poly1.inners()[0], point_t(4.0, 4.0));
    bg::append(poly1.inners()[0], point_t(1.0, 4.0));
    bg::append(poly1.inners()[0], point_t(1.0, 1.0));
    
    bg::plotly_plotter<std::ofstream> plotter("my_plot.json");
    plotter.plot(poly1, bg::plotly_color(255, 39, 0));

    return 0;
}