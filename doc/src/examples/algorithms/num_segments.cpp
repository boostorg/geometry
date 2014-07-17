// Boost.Geometry (aka GGL, Generic Geometry Library)
// QuickBook Example

// Copyright (c) 2014, Oracle and/or its affiliates

// Contributed and/or modified by Menelaos Karavelas, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

//[num_segments
//` Get the number of segments in a geometry

#include <iostream>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/multi_polygon.hpp>


int main()
{
    boost::geometry::model::multi_polygon
        <
            boost::geometry::model::polygon
                <
                    boost::geometry::model::d2::point_xy<double>
                >
        > mp;
    boost::geometry::read_wkt("MULTIPOLYGON(((0 0,0 10,10 0,0 0),(1 1,1 9,9 1,1 1)),((10 10,10 7,7 10,10 10)))", mp);
    std::cout << "Number of segments: " << boost::geometry::num_segments(mp) << std::endl;

    return 0;
}

//]


//[num_segments_output
/*`
Output:
[pre
 Number of points: 9
]
*/
//]
