// Boost.Geometry
// QuickBook Example

// Copyright (c) 2018, Oracle and/or its affiliates
// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//[line_interpolate_point
//` Shows how to interpolate points on a linestring

#include <iostream>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>

using namespace boost::geometry;

int main()
{
    typedef boost::geometry::model::d2::point_xy<double> point_type;
    using segment_type = model::segment<point_type>;
    using linestring_type = model::linestring<point_type>;
    using multipoint_type = model::multi_point<point_type>;

    segment_type const s { {0, 0}, {1, 1} };
    linestring_type const l { {0, 0}, {1, 0}, {1, 1}, {0, 1}, {0, 2} };
    point_type p;
    multipoint_type mp;
    double fraction = 0.1;

    std::cout << "point interpolation" << std::endl;

    line_interpolate_point(s, fraction, p);
    std::cout << "on segment : " << wkt(p) << std::endl;

    line_interpolate_point(l, fraction, p);
    std::cout << "on linestring : " << wkt(p) << std::endl << std::endl;

    std::cout << "multipoint interpolation" << std::endl;

    line_interpolate_point(s, fraction, mp);
    std::cout << "on segment : " << wkt(mp) << std::endl;

    mp=multipoint_type();
    line_interpolate_point(l,fraction, mp);
    std::cout << "on linestring : " << wkt(mp) << std::endl;

    return 0;
}

//]

//[line_interpolate_point_output
/*`
Output:
[pre
point interpolation
on segment : POINT(0.1 0.1)
on linestring : POINT(0.4 0)

multipoint interpolation
on segment : MULTIPOINT((0.1 0.1),(0.2 0.2),(0.3 0.3),(0.4 0.4),(0.5 0.5),(0.6 0.6),(0.7 0.7),(0.8 0.8),(0.9 0.9),(1 1))
on linestring : MULTIPOINT((0.4 0),(0.8 0),(1 0.2),(1 0.6),(1 1),(0.6 1),(0.2 1),(0 1.2),(0 1.6),(0 2))
]
*/
//]
