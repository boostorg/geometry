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
    
    point_t a, b;
    boost::geometry::assign_values(a, 1, 1, 1);
    boost::geometry::assign_values(b, 0, 0, 0);

    std::ofstream json("my_plot.json");
    boost::geometry::json_plotter plotter(json);
    plotter.plot(a);
    plotter.plot(b);

    return 0;
}