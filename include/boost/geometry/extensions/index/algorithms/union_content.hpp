// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - boxes union/sum area/volume
//
// Copyright 2008 Federico J. Fernandez.
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_UNION_CONTENT_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_UNION_CONTENT_HPP

#include <boost/geometry/algorithms/expand.hpp>
#include <boost/geometry/extensions/index/algorithms/content.hpp>

namespace boost { namespace geometry { namespace index {

/**
 * \brief Compute the area of the union of b1 and b2
 */
template <typename Box, typename Geometry>
inline typename default_content_result<Box>::type union_content(Box const& b, Geometry const& g)
{
    Box expanded_box(b);
    geometry::expand(expanded_box, g);
    return index::content(expanded_box);
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_ALGORITHMS_UNION_CONTENT_HPP
