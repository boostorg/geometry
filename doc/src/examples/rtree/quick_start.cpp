// Boost.Geometry Index
//
// Quickbook Examples
//
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//[rtree_quickstart_include

#include <vector>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/box.hpp>

#include <boost/geometry/extensions/index/rtree/rtree.hpp>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;
//]

int main(void)
{
    //[rtree_quickstart_valuetype
    typedef bg::model::point<float, 2, bg::cs::cartesian> point;
    typedef bg::model::box<point> box;
    typedef std::pair<box, unsigned> value;
    //]

    //[rtree_quickstart_create
    bgi::rtree< value, bgi::quadratic<32, 8> > rtree;
    //]

    //[rtree_quickstart_insert
    // create some box
    // this typically will be an envelope of some geometry
    box b(point(0, 0), point(10, 10));
    // insert new value
    rtree.insert(std::make_pair(b, 0));
    //]
    
    //[rtree_quickstart_query
    // find values intersecting a box
    std::vector<value> result;
    rtree.query(b, std::back_inserter(result));
    //]

    //[rtree_quickstart_nearest
    // find 5 nearest values to a point
    rtree.nearest(point(0, 0), 5, std::back_inserter(result));
    //]

    return 0;
}
