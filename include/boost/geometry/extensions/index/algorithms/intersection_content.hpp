// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - boxes union/intersection area/volume
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_INTERSECTION_CONTENT_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_INTERSECTION_CONTENT_HPP

#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/extensions/index/algorithms/content.hpp>

namespace boost { namespace geometry { namespace index {

/**
 * \brief Compute the area of the intersection of b1 and b2
 */
template <typename Box>
inline typename default_content_result<Box>::type intersection_content(Box const& box1, Box const& box2)
{
    typename default_content_result<Box>::type result = 0;
    if ( geometry::intersects(box1, box2) )
    {
        Box box_intersection;
        geometry::intersection(box1, box2, box_intersection);
        result = index::content(box_intersection);
    }
    return result;
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_INTERSECTION_CONTENT_HPP
