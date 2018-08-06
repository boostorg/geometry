// Boost.Geometry
// QuickBook Example
// Copyright (c) 2018, Oracle and/or its affiliates
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//[similarity_discrete_hausdorff_distance
//` Calculate Similarity between two geometries
#include <iostream>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/linestring.hpp>
int main()
{
    typedef boost::geometry::model::d2::point_xy<double> point_type;
    typedef boost::geometry::model::linestring<point_type> linestring_type;
    linestring_type linestring1;
    boost::geometry::read_wkt("LINESTRING(0 0,0 10,10 10,10 0,0 0)", linestring1);
    boost::geometry::read_wkt("LINESTRING(0 0,4 4,1 1,2 2,3 3)", linestring2);    
    typename distance_result<typename point_type<point_type>::type,typename point_type<linestring_type>::type>::type res;
    res = boost::geometry::densify(linestring1,linestrign2);
    std::cout << "Discrete Hausdorff Distance: " << res << std::endl;
    return 0;
}
//]
 //[similarity_discrete_hausdorff_distance_output
/*`
Output:
[pre
Discrete Hausdorff Distance: 5.0
]
*/
//]