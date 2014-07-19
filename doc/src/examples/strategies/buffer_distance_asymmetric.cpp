// Boost.Geometry (aka GGL, Generic Geometry Library)
// QuickBook Example

// Copyright (c) 2014 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//[buffer_distance_asymmetric
//` Shows how the buffer_distance_asymmetric strategy can be used as a EndStrategy to create rounded ends

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/geometries.hpp>
/*<-*/ #include "../examples_utils/create_svg_buffer.hpp" /*->*/

int main()
{
    typedef boost::geometry::model::d2::point_xy<double> point;
    typedef boost::geometry::model::linestring<point> linestring;
    typedef boost::geometry::model::polygon<point> polygon;

    // Declare the asymmetric distance strategy
    boost::geometry::strategy::buffer::distance_asymmetric<double> distance_strategy(1.0, 0.5);

    // Declare other strategies
    const int points_per_circle = 36;
    boost::geometry::strategy::buffer::buffer_side side_strategy;
    boost::geometry::strategy::buffer::join_round join_strategy(points_per_circle);
    boost::geometry::strategy::buffer::end_round end_strategy(points_per_circle);
    boost::geometry::strategy::buffer::buffer_circle point_strategy(points_per_circle);

    // Declare/fill a multi linestring
    boost::geometry::model::multi_linestring<linestring> ml;
    boost::geometry::read_wkt("MULTILINESTRING((3 5,5 10,7 5),(7 7,11 10,15 7,19 10))", ml);

    // Create the buffered geometry with left/right a different distance
    boost::geometry::model::multi_polygon<polygon> result;
    boost::geometry::buffer(ml, result,
                distance_strategy, side_strategy,
                join_strategy, end_strategy, point_strategy);
    /*<-*/ create_svg_buffer("buffer_distance_asymmetric.svg", ml, result); /*->*/

    return 0;
}

//]

