// Boost.Geometry

// Copyright (c) 2025 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Polyhedral surface example

#include <iostream>
#include <boost/geometry/geometry.hpp>

namespace bg = boost::geometry;

int main()
{
    using point_t = bg::model::point<double, 3, bg::cs::cartesian>;
    using polygon_t = bg::model::polygon<point_t>;
    using polyhedral_t = bg::model::polyhedral_surface<polygon_t>;

    // intializing an empty polyhedral surface (default constructor)
    polyhedral_t polyhedral0;

    // creating a polyhderal surface (cube) using standard initiallized list
    polyhedral_t polyhedral1 = {{{{0, 0, 0}, {0, 1, 0}, {1, 1, 0}, {1, 0, 0}, {0, 0, 0}}},\
                                {{{0, 0, 0}, {0, 1, 0}, {0, 1, 1}, {0, 0, 1}, {0, 0, 0}}},\
                                {{{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}, {0, 0, 0}}},\
                                {{{1, 1, 1}, {1, 0, 1}, {0, 0, 1}, {0, 1, 1}, {1, 1, 1}}},\
                                {{{1, 1, 1}, {1, 0, 1}, {1, 0, 0}, {1, 1, 0}, {1, 1, 1}}},\
                                {{{1, 1, 1}, {1, 1, 0}, {0, 1, 0}, {0, 1, 1}, {1, 1, 1}}}};

    // creating a polyhderal surface (triangular pyramid) using append
    // and resize
    polyhedral0.resize(4);
    bg::append(polyhedral0[0].outer(), point_t(0.0, 0.0, 0.0));
    bg::append(polyhedral0[0].outer(), point_t(5.0, 0.0, 0.0));
    bg::append(polyhedral0[0].outer(), point_t(0.0, 5.0, 0.0));
    bg::append(polyhedral0[1].outer(), point_t(0.0, 0.0, 0.0));
    bg::append(polyhedral0[1].outer(), point_t(0.0, 5.0, 0.0));
    bg::append(polyhedral0[1].outer(), point_t(0.0, 0.0, 5.0));
    bg::append(polyhedral0[2].outer(), point_t(0.0, 0.0, 0.0));
    bg::append(polyhedral0[2].outer(), point_t(5.0, 0.0, 0.0));
    bg::append(polyhedral0[2].outer(), point_t(0.0, 0.0, 5.0));
    bg::append(polyhedral0[3].outer(), point_t(5.0, 0.0, 0.0));
    bg::append(polyhedral0[3].outer(), point_t(0.0, 5.0, 0.0));
    bg::append(polyhedral0[3].outer(), point_t(0.0, 0.0, 5.0));

    // Initialize polyhedral surface (cube) with wkt
    polyhedral_t polyhedral2;
    bg::read_wkt("POLYHEDRALSURFACE(((0 0 0, 0 1 0, 1 1 0, 1 0 0, 0 0 0)),\
                                    ((0 0 0, 0 1 0, 0 1 1, 0 0 1, 0 0 0)),\
                                    ((0 0 0, 1 0 0, 1 0 1, 0 0 1, 0 0 0)),\
                                    ((1 1 1, 1 0 1, 0 0 1, 0 1 1, 1 1 1)),\
                                    ((1 1 1, 1 0 1, 1 0 0, 1 1 0, 1 1 1)),\
                                    ((1 1 1, 1 1 0, 0 1 0, 0 1 1, 1 1 1)))", polyhedral2);

    // Write polyhedral surface wkt
    std::cout << bg::wkt(polyhedral0) << std::endl;
    std::cout << bg::wkt(polyhedral1) << std::endl;
    std::cout << bg::wkt(polyhedral2) << std::endl;

    return 0;
}
