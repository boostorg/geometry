// Boost.Geometry
// QuickBook Example

// Copyright (c) 2025 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//[polyhedral_surface
//` Declaration and use of the Boost.Geometry model::polyhedral_surface, modelling the PolyhedralSurface Concept

#include <iostream>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>

namespace bg = boost::geometry;

int main()
{
    using point_t = bg::model::point<double, 3, bg::cs::cartesian>;
    using polygon_t = bg::model::polygon<point_t>;
    using polyhedral_t = bg::model::polyhedral_surface<polygon_t>;

    polyhedral_t polyhedral0; /*< Initializing an empty polyhedral surface (default constructor) >*/

    polyhedral_t polyhedral1 = {{{{0, 0, 0}, {0, 1, 0}, {1, 1, 0}, {1, 0, 0}, {0, 0, 0}}},
                                {{{0, 0, 0}, {0, 1, 0}, {0, 1, 1}, {0, 0, 1}, {0, 0, 0}}},
                                {{{0, 0, 0}, {1, 0, 0}, {1, 0, 1}, {0, 0, 1}, {0, 0, 0}}},
                                {{{1, 1, 1}, {1, 0, 1}, {0, 0, 1}, {0, 1, 1}, {1, 1, 1}}},
                                {{{1, 1, 1}, {1, 0, 1}, {1, 0, 0}, {1, 1, 0}, {1, 1, 1}}},
                                {{{1, 1, 1}, {1, 1, 0}, {0, 1, 0}, {0, 1, 1}, {1, 1, 1}}}}; /*< Creating a polyhedral surface (cube) using standard initialized list >*/


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
    bg::append(polyhedral0[3].outer(), point_t(0.0, 0.0, 5.0)); /*< Creating a polyhedral surface (triangular pyramid) using append and resize >*/

    polyhedral_t polyhedral2;
    bg::read_wkt("POLYHEDRALSURFACE(((0 0 0, 0 1 0, 1 1 0, 1 0 0, 0 0 0)),\
                                    ((0 0 0, 0 1 0, 0 1 1, 0 0 1, 0 0 0)),\
                                    ((0 0 0, 1 0 0, 1 0 1, 0 0 1, 0 0 0)),\
                                    ((1 1 1, 1 0 1, 0 0 1, 0 1 1, 1 1 1)),\
                                    ((1 1 1, 1 0 1, 1 0 0, 1 1 0, 1 1 1)),\
                                    ((1 1 1, 1 1 0, 0 1 0, 0 1 1, 1 1 1)))", polyhedral2); /*< Initialize polyhedral surface (cube) with wkt >*/

    std::cout << bg::wkt(polyhedral0) << std::endl; /*< Write polyhedral surface wkt >*/
    std::cout << bg::wkt(polyhedral1) << std::endl;
    std::cout << bg::wkt(polyhedral2) << std::endl;

    return 0;
}

//]


//[polyhedral_surface_output
/*`
Output:
[pre
POLYHEDRALSURFACE(((0 0 0,5 0 0,0 5 0,0 0 0)),((0 0 0,0 5 0,0 0 5,0 0 0)),((0 0 0,5 0 0,0 0 5,0 0 0)),((5 0 0,0 5 0,0 0 5,5 0 0)))
POLYHEDRALSURFACE(((0 0 0,0 1 0,1 1 0,1 0 0,0 0 0)),((0 0 0,0 1 0,0 1 1,0 0 1,0 0 0)),((0 0 0,1 0 0,1 0 1,0 0 1,0 0 0)),((1 1 1,1 0 1,0 0 1,0 1 1,1 1 1)),((1 1 1,1 0 1,1 0 0,1 1 0,1 1 1)),((1 1 1,1 1 0,0 1 0,0 1 1,1 1 1)))
POLYHEDRALSURFACE(((0 0 0,0 1 0,1 1 0,1 0 0,0 0 0)),((0 0 0,0 1 0,0 1 1,0 0 1,0 0 0)),((0 0 0,1 0 0,1 0 1,0 0 1,0 0 0)),((1 1 1,1 0 1,0 0 1,0 1 1,1 1 1)),((1 1 1,1 0 1,1 0 0,1 1 0,1 1 1)),((1 1 1,1 1 0,0 1 0,0 1 1,1 1 1)))
]
*/
//]
