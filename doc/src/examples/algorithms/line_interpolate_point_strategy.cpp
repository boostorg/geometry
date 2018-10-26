// Boost.Geometry
// QuickBook Example

// Copyright (c) 2018, Oracle and/or its affiliates
// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//[line_interpolate_point_strategy
//` Shows how to interpolate points on a linestring in geographic coordinate system

#include <iostream>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>

using namespace boost::geometry;

int main()
{
    typedef model::d2::point_xy<double,cs::geographic<degree> > point_type;
    using segment_type = model::segment<point_type>;
    using linestring_type = model::linestring<point_type>;
    using multipoint_type = model::multi_point<point_type>;

    segment_type const s { {0, 0}, {1, 1} };
    linestring_type const l { {0, 0}, {1, 0}, {1, 1}, {0, 1}, {0, 2} };
    point_type p;
    multipoint_type mp;
    double fraction = 0.1;

    srs::spheroid<double> spheroid(6378137.0, 6356752.3142451793);
    strategy::line_interpolate_point
            ::geographic<strategy::vincenty> str(spheroid);

    std::cout << "point interpolation" << std::endl;

    line_interpolate_point(s, fraction, p, str);
    std::cout << "on segment : " << wkt(p) << std::endl;

    line_interpolate_point(l, fraction, p, str);
    std::cout << "on linestring : " << wkt(p) << std::endl << std::endl;

    std::cout << "multipoint interpolation" << std::endl;

    line_interpolate_point(s, fraction, mp, str);
    std::cout << "on segment : " << wkt(mp) << std::endl;

    mp=multipoint_type();
    line_interpolate_point(l,fraction, mp, str);
    std::cout << "on linestring : " << wkt(mp) << std::endl;

    return 0;
}

//]

//[line_interpolate_point_strategy_output
/*`
Output:
[pre
point interpolation
on segment : POINT(0.09999 0.100005)
on linestring : POINT(0.398647 2.45744e-17)

multipoint interpolation
on segment : MULTIPOINT((0.09999 0.100005),(0.199981 0.20001),\
(0.299972 0.300014),(0.399966 0.400018),(0.499962 0.50002),(0.599961 0.60002),\
(0.699964 0.700019),(0.799971 0.800015),(0.899983 0.900009),(1 1))
on linestring : MULTIPOINT((0.398647 2.45744e-17),(0.797294 4.91476e-17),\
(1 0.197261),(1 0.598594),(1 0.999927),(0.601365 1.00004),(0.202658 1.00002),\
(0 1.19734),(0 1.59867),(0 2))
]
*/
//]
