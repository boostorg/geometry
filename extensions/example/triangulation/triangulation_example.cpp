// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2019 Tinko Bartels, Berlin, Germany.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Triangulation Example - computes a delaunay triangulation and draws the
// triangulation and the corresponding finite voronoi edges

#include <fstream>
#include <random>
#include <vector>

#include <boost/geometry.hpp>
#include <boost/geometry/extensions/triangulation/triangulation.hpp>
#include <boost/geometry/extensions/triangulation/geometries/voronoi_adaptor.hpp>

const int samples = 100;

int main()
{
    namespace bg = boost::geometry;
    typedef bg::model::point<double, 2, bg::cs::cartesian> point;
    typedef bg::model::triangulation<point> triangulation;

    std::default_random_engine gen(1);
    std::uniform_real_distribution<> dist(0.0, 1.0);
    std::vector<point> in;
    for (int i = 0 ; i < samples ; ++i)
    {
        in.push_back(point(dist(gen), dist(gen)));
    }
    triangulation out(samples);
    bg::delaunay_triangulation(in, out);

    std::ofstream svg("triangulation.svg");
    bg::svg_mapper<point> mapper(svg, 720, 720);
    mapper.add(point(0, 0));
    mapper.add(point(1, 1));

    for (auto const& f : bg::face_range(out))
    {
        mapper.map(f, "fill-opacity:0.3;fill:rgb(102,102,201);stroke:rgb(51,51,152);");
    }

    typedef bg::model::voronoi_face_view<triangulation> voronoi_face_view;
    for (auto it = out.vertices_begin() ; it != out.vertices_end() ; ++it)
    {
        voronoi_face_view vfv(out, it);
        mapper.map(voronoi_face_view(out, it), "opacity:1.0;fill:none;stroke:rgb(255,0,0);stroke-width:1");
    }

    return 0;
}
