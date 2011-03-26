// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2011 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Code to create SVG for examples

#ifndef CREATE_SVG_TWO_HPP
#define CREATE_SVG_TWO_HPP

#include <fstream>
#include <boost/algorithm/string.hpp>
#include <boost/geometry/extensions/io/svg/svg_mapper.hpp>

template <typename Geometry1, typename Geometry2>
void create_svg(std::string const& filename, Geometry1 const& a, Geometry2 const& b)
{
    std::cout  << std::endl << "[$img/algorithms/" << boost::replace_all_copy(filename, ".svg", ".png") << "]" << std::endl << std::endl;

    typedef typename boost::geometry::point_type<Geometry1>::type point_type;
    std::ofstream svg(filename.c_str());

    boost::geometry::svg_mapper<point_type> mapper(svg, 400, 400);
    mapper.add(a);
    mapper.add(b);

    mapper.map(a, "fill-opacity:0.5;fill:rgb(153,204,0);stroke:rgb(153,204,0);stroke-width:2");
    mapper.map(b, "opacity:0.8;fill:none;stroke:rgb(255,128,0);stroke-width:4;stroke-dasharray:1,7;stroke-linecap:round");
}

// NOTE: convert manually from svg to png using Inkscape ctrl-shift-E
// and copy png to html/img/algorithms/


#endif // CREATE_SVG_TWO_HPP

