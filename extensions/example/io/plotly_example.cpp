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
#include <boost/geometry/geometries/point_xyz.hpp>

int main()
{
    typedef boost::geometry::model::d3::point_xyz<double> point_t;
    
    boost::geometry::model::linestring<point_t> a;

    boost::geometry::append(a, point_t(0, 0, 0));
    boost::geometry::append(a, point_t(0, 1, 2));
    boost::geometry::append(a, point_t(1, 2, 3));

    std::ofstream json("my_plot.json");
    boost::geometry::json_plotter plotter(json);
    plotter.plot(a, "rgb(5, 200, 64)");

    return 0;
}