// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree nodes
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_HPP

#include <boost/geometry/extensions/index/rtree/node/node_default.hpp>
#include <boost/geometry/extensions/index/rtree/node/node_default_variant.hpp>

#include <boost/geometry/extensions/index/rtree/node/node_default_static.hpp>
#include <boost/geometry/extensions/index/rtree/node/node_default_static_variant.hpp>

#include <boost/geometry/algorithms/expand.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree {

// elements box

template <typename Box, typename FwdIter, typename Translator>
inline Box elements_box(FwdIter first, FwdIter last, Translator const& tr)
{
    BOOST_GEOMETRY_INDEX_ASSERT(first != last, "Can't calculate element's box");

    Box result;

    geometry::convert(element_indexable(*first, tr), result);
    ++first;

    for ( ; first != last ; ++first )
        geometry::expand(result, element_indexable(*first, tr));

    return result;
}

}} // namespace detail::rtree

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_HPP
