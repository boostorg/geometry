// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - n-dimensional area/volume of boxes intersecion/overlap
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_OVERLAP_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_OVERLAP_HPP

#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/extensions/index/algorithms/area.hpp>

namespace boost { namespace geometry { namespace index {

template <typename Box>
struct overlap_result
{
    typedef typename area_result<Box>::type type;
};

template <typename Box>
typename overlap_result<Box>::type overlap(Box const& b1, Box const& b2)
{
    Box inters;
    geometry::assign_zero(inters);
    geometry::intersection(b1, b2, inters);
    return index::area(inters);
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_OVERLAP_HPP
