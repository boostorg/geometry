// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Random Example - showing random sampling in various geometries

#include <fstream>
#include <random>
#include <cmath>

#include <boost/geometry.hpp>
#include <boost/geometry/extensions/random/uniform_point_distribution.hpp>

const int samples = 100;

template<typename Distribution, typename Generator, typename Mapper>
void draw_samples(Distribution& d, Generator& g, Mapper& m) {
    for (int i = 0 ; i < samples ; ++i) {
        m.map(d(g), "fill-opacity:1;fill:rgb(255,0,0);stroke:rgb(255,0,0);stroke-width:1", 1);
    }
}

int main()
{
    namespace bg = boost::geometry;
    typedef bg::model::point<double, 2, bg::cs::cartesian> point;
    typedef bg::model::segment<point> segment;
    typedef bg::model::box<point> box;
    typedef bg::model::multi_point<point> multi_point;
    typedef bg::model::polygon<point> polygon;

    std::default_random_engine generator(1);
    polygon poly;
    boost::geometry::read_wkt("POLYGON((16 21,17.1226 17.5451,20.7553 17.5451,17.8164 15.4098,18.9389 11.9549,16 14.0902,13.0611 11.9549,14.1836 15.4098,11.2447 17.5451,14.8774 17.5451,16 21))", poly);
    box b(point(0, 0), point(10, 10));
    segment s(point(11, 0), point(21, 10));
    multi_point mp;
    for (double y = 11.0 ; y <= 21.0 ; y += 0.5) {
        for(double x = 0.0 ; x <= 10.0 ; x += 0.5) {
            bg::append(mp, point(x, y));
        }
    }
    auto box_dist = bg::random::uniform_point_distribution(b);
    auto mp_dist = bg::random::uniform_point_distribution(mp);
    auto seg_dist = bg::random::uniform_point_distribution(s);
    auto poly_dist = bg::random::uniform_point_distribution(poly);
    std::ofstream svg("random.svg");
    bg::svg_mapper<point> mapper(svg, 720, 720);
    mapper.add(poly);
    mapper.add(b);
    mapper.add(mp);
    mapper.add(s);
    mapper.add(box(point(0, 22), point(22, 35)));
    mapper.map(b, "fill-opacity:0.3;fill:rgb(51,51,153);stroke:rgb(51,51,153);stroke-width:2");
    mapper.map(s, "fill-opacity:0.3;fill:rgb(51,51,153);stroke:rgb(51,51,153);stroke-width:2");
    mapper.map(mp, "fill-opacity:0.3;fill:rgb(51,51,153);stroke:rgb(51,51,153);stroke-width:2", 2);
    mapper.map(poly, "fill-opacity:0.3;fill:rgb(51,51,153);stroke:rgb(51,51,153);stroke-width:2");
    draw_samples(box_dist, generator, mapper);
    draw_samples(mp_dist, generator, mapper);
    draw_samples(seg_dist, generator, mapper);
    draw_samples(poly_dist, generator, mapper);

    typedef bg::model::point<double, 2, bg::cs::spherical_equatorial<bg::degree>> point_spherical;
    typedef bg::model::box<point_spherical> box_spherical;
    box_spherical sb(point_spherical(0, 0), point_spherical(90, 90));
    auto sb_dist = bg::random::uniform_point_distribution(sb);
    for (int i = 0 ; i < 10*samples ; ++i) {
        point_spherical p = sb_dist(generator);
        double x = bg::get<0>(p) / 90 * 22;
        double y = 32 - bg::get<1>(p) / 90 * 10;
        mapper.map(point(x,y), "fill-opacity:1;fill:rgb(255,0,0);stroke:rgb(255,0,0);stroke-width:1", 1);
    }
    return 0;
}
