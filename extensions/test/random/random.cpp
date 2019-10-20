// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2019 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <random>
#include <vector>
#include <sstream>

#include <geometry_test_common.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/extensions/random/uniform_point_distribution.hpp>

typedef bg::model::point<double, 2, bg::cs::cartesian> point2d_cart;
typedef bg::model::point<double, 3, bg::cs::cartesian> point3d_cart;
typedef bg::model::point<double, 2, bg::cs::geographic<bg::degree>> point2d_geog;

void test_geographic()
{
    //We check whether the generated points lie roughly along
    //the great circle segment (<2 km distance on a sphere with
    //the radius of earth) and are distributed uniformly with respect
    //to great circle arc length.
    typedef bg::model::linestring<point2d_geog> linestring;
    linestring ls {{ 0.0, 0.0 }, { 45.0, 45.0 }, { 60.0, 60.0 }};
    bg::random::uniform_point_distribution<linestring> l_dist(ls);
    std::mt19937 generator(0);
    int sample_count = 2000;
    int count_below_45 = 0;
    for (int i = 0 ; i < sample_count ; ++i)
    {
        point2d_geog sample = l_dist(generator);
        BOOST_CHECK( bg::distance(sample, ls) < 2000 );
        if(bg::get<0>(sample) < 45.0) count_below_45++;
    }
    double length_ratio = bg::distance(ls[0], ls[1]) / 
        ( bg::distance(ls[0], ls[1]) + bg::distance(ls[1], ls[2]) );
    double sample_ratio = ((double) count_below_45) / sample_count;
    bool in_range = sample_ratio * 0.95 < length_ratio 
        && sample_ratio * 1.05 > length_ratio;
    BOOST_CHECK( in_range );

    //We check whether the generated points lie in the spherical box
    //(which is actually a triangle in this case) and whether the latitude
    //is distributed as expected for uniform spherical distribution, using
    //known area ratios of spherical caps.
    typedef bg::model::box<point2d_geog> box;
    box b {{ 0.0, 0.0 }, { 90.0, 90.0 }};
    bg::random::uniform_point_distribution<box> b_dist(b);
    int under_60 = 0;
    for (int i = 0 ; i < sample_count ; ++i)
    {
        point2d_geog sample = b_dist(generator);
        BOOST_CHECK( bg::within(sample, b) );
        if(bg::get<1>(sample) < 60.0) ++under_60;
    }
    BOOST_CHECK_GT(under_60, 0.5 * 0.95 * sample_count);
    BOOST_CHECK_LT(under_60, 0.5 * 1.05 * sample_count);
}

void test_polygon()
{
    //This test will test uniform sampling in polygon, which also checks
    //uniform sampling in boxes. We check whether two equal distributions
    //(copied using operator<< and operator>>) generate the same sequence
    //of points and whether those points are uniformly distributed with
    //respect to cartesian area.
    typedef bg::model::polygon<point2d_cart> polygon;
    polygon poly;
    bg::read_wkt(
        "POLYGON((16 21,17.1226 17.5451,20.7553 17.5451, 17.8164 15.4098,"
        "18.9389 11.9549,16 14.0902,13.0611 11.9549, 14.1836 15.4098,"
        "11.2447 17.5451,14.8774 17.5451,16 21))",
        poly);
    bg::random::uniform_point_distribution<polygon> poly_dist(poly);
    bg::random::uniform_point_distribution<polygon> poly_dist2;
    BOOST_CHECK( !(poly_dist == poly_dist2) );
    std::stringstream ss;
    ss << poly_dist;
    ss >> poly_dist2;
    BOOST_CHECK( poly_dist == poly_dist2 );
    std::mt19937 generator(0), generator2(0);
    for (int i = 0 ; i < 100 ; ++i)
    {
        point2d_cart sample1 = poly_dist(generator);
        BOOST_CHECK( bg::equals(sample1, poly_dist2(generator2)) );
        BOOST_CHECK( bg::within(sample1, poly) );
    }
    std::vector<point2d_cart> randoms;
    const int uniformity_test_samples = 2000;
    for (int i = 0 ; i < uniformity_test_samples ; ++i)
    {
        randoms.push_back(poly_dist(generator));
    }
    typedef bg::model::box<point2d_cart> box;
    box env, lhalf;
    bg::envelope(poly, env);
    bg::set<bg::min_corner, 0>(lhalf, bg::get<bg::min_corner, 0>(env));
    bg::set<bg::min_corner, 1>(lhalf, bg::get<bg::min_corner, 1>(env));
    bg::set<bg::max_corner, 0>(lhalf, bg::get<bg::max_corner, 0>(env));
    bg::set<bg::max_corner, 1>(lhalf,
        (bg::get<bg::max_corner, 1>(env) + bg::get<bg::min_corner, 1>(env)) / 2);
    std::vector<polygon> lower;
    bg::intersection(lhalf, poly, lower);
    double area_ratio = bg::area(lower[0])/bg::area(poly);
    int in_lower = 0;
    for (int i = 0 ; i < uniformity_test_samples ; ++i)
    {
        if(bg::within(randoms[i], lhalf))
            ++in_lower;
    }
    double sample_ratio = ((double) in_lower ) / uniformity_test_samples;
    BOOST_CHECK_GT( sample_ratio * 1.05, area_ratio );
    BOOST_CHECK_LT( sample_ratio * 0.95, area_ratio );
}

void test_multipoint()
{
    typedef bg::model::multi_point<point3d_cart> multipoint;
    multipoint mp {{0.0, 0.0, 0.0}, {1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}};
    int first = 0;
    bg::random::uniform_point_distribution<multipoint> mp_dist(mp);
    std::mt19937 generator(0);
    int sample_count = 1000;
    for (int i = 0 ; i < sample_count ; ++i)
    {
        point3d_cart sample = mp_dist(generator);
        BOOST_CHECK( bg::within(sample, mp) );
        if(bg::equals(sample, mp[0])) ++first;
    }
    BOOST_CHECK_GT(first * 1.05, sample_count / 3);
    BOOST_CHECK_LT(first * 0.95, sample_count / 3);
}

int test_main(int, char* [])
{
    test_polygon();
    test_geographic();
    test_multipoint();
    return 0;
}
