// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2010, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <geometry_test_common.hpp>


#include<boost/geometry/geometry.hpp>
#include<boost/geometry/geometries/adapted/boost_polygon/point.hpp>
#include<boost/geometry/geometries/adapted/boost_polygon/box.hpp>
#include<boost/geometry/geometries/adapted/boost_polygon/ring.hpp>
#include<boost/geometry/geometries/adapted/boost_polygon/polygon.hpp>
#include<boost/geometry/extensions/gis/io/wkt/wkt.hpp>
#include<iostream>


int test_main(int, char* [])
{
    // 1a: Check if Boost.Polygon's point fulfills Boost.Geometry's point concept
    bg::concept::check<boost::polygon::point_data<double> >();

    // 1b: use a Boost.Polygon point in Boost.Geometry, calc. distance with two point types
    boost::polygon::point_data<double> p1(1, 2);

    typedef bg::model::point<double, 2, bg::cs::cartesian> bg_point;
    bg_point p2(3, 4);
    BOOST_CHECK_CLOSE(bg::distance(p1, p2), 2 * std::sqrt(2.0), 0.001);

    // 2a: Check if Boost.Polygon's box fulfills Boost.Geometry's box concept
    bg::concept::check<boost::polygon::rectangle_data<double> >();

    // 2b: use a Boost.Polygon rectangle in Boost.Geometry, compare with boxes
    boost::polygon::rectangle_data<double> b1;
    bg::model::box<bg_point> b2;

    bg::assign(b1, 0, 1, 5, 6);
    bg::assign(b2, 0, 1, 5, 6);
    double a1 = bg::area(b1);
    double a2 = bg::area(b2);
    BOOST_CHECK_CLOSE(a1, a2, 0.001);

    // 3a: Check if Boost.Polygon's polygon fulfills Boost.Geometry's ring concept
    bg::concept::check<boost::polygon::polygon_data<double> >();

    // 3b: use a Boost.Polygon polygon (ring) in Boost.Geometry
    // Filling it is a two-step process using Boost.Polygon
    std::vector<boost::polygon::point_data<double> > point_vector;
    point_vector.push_back(boost::polygon::point_data<double>(0, 0));
    point_vector.push_back(boost::polygon::point_data<double>(0, 3));
    point_vector.push_back(boost::polygon::point_data<double>(4, 0));
    point_vector.push_back(boost::polygon::point_data<double>(0, 0));

    boost::polygon::polygon_data<double> r1;
    r1.set(point_vector.begin(), point_vector.end());

    bg::model::linear_ring<bg_point> r2;
    r2.push_back(bg_point(0, 0));
    r2.push_back(bg_point(0, 3));
    r2.push_back(bg_point(4, 0));
    r2.push_back(bg_point(0, 0));
    a1 = bg::area(r1);
    a2 = bg::area(r2);
    BOOST_CHECK_CLOSE(a1, a2, 0.001);


    // 4a: Boost.Polygon's polygon with holes
    point_vector.clear();
    point_vector.push_back(boost::polygon::point_data<double>(0, 0));
    point_vector.push_back(boost::polygon::point_data<double>(0, 10));
    point_vector.push_back(boost::polygon::point_data<double>(10, 10));
    point_vector.push_back(boost::polygon::point_data<double>(10, 0));
    point_vector.push_back(boost::polygon::point_data<double>(0, 0));

    boost::polygon::polygon_with_holes_data<double> poly1;
    poly1.set(point_vector.begin(), point_vector.end());

    // Fill the holes (we take two)
    std::vector<boost::polygon::polygon_data<double> > holes;
    holes.resize(2);

    point_vector.clear();
    point_vector.push_back(boost::polygon::point_data<double>(1, 1));
    point_vector.push_back(boost::polygon::point_data<double>(2, 1));
    point_vector.push_back(boost::polygon::point_data<double>(2, 2));
    point_vector.push_back(boost::polygon::point_data<double>(1, 2));
    point_vector.push_back(boost::polygon::point_data<double>(1, 1));
    holes[0].set(point_vector.begin(), point_vector.end());

    point_vector.clear();
    point_vector.push_back(boost::polygon::point_data<double>(3, 3));
    point_vector.push_back(boost::polygon::point_data<double>(4, 3));
    point_vector.push_back(boost::polygon::point_data<double>(4, 4));
    point_vector.push_back(boost::polygon::point_data<double>(3, 4));
    point_vector.push_back(boost::polygon::point_data<double>(3, 3));
    holes[1].set(point_vector.begin(), point_vector.end());

    poly1.set_holes(holes.begin(), holes.end());

    // Using Boost.Polygon
    a1 = bg::area(poly1);
    a2 = boost::polygon::area(poly1);
    BOOST_CHECK_CLOSE(a1, a2, 0.001);

    bg::model::polygon<bg_point> poly2;
    bg::read_wkt("POLYGON((0 0,0 10,10 10,10 0,0 0),(1 1,2 1,2 2,1 2,1 1),(3 3,4 3,4 4,3 4,3 3))", poly2);

    a2 = bg::area(poly2);
    BOOST_CHECK_CLOSE(a1, a2, 0.001);


    // Not finished:
    //bg::read_wkt("POLYGON((0 0,0 10,10 10,10 0,0 0),(1 1,2 1,2 2,1 2,1 1),(3 3,4 3,4 4,3 4,3 3))", poly1);


    return 0;
}

