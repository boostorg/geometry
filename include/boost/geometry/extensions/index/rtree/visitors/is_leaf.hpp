// Boost.Geometry Index
//
// R-tree leaf node checking visitor implementation
//
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_IS_LEAF_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_IS_LEAF_HPP

#include <boost/geometry/extensions/index/rtree/node/node.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

template <typename Value, typename NodeProxy>
struct is_leaf
    : public rtree::visitor<
          Value,
          typename NodeProxy::parameters_type,
          typename NodeProxy::box_type,
          typename NodeProxy::allocators_type,
          typename NodeProxy::node_tag,
          true
      >::type
{
    typedef typename NodeProxy::internal_node internal_node;
    typedef typename NodeProxy::leaf leaf;

    inline void operator()(internal_node const&)
    {
        result = false;
    }

    inline void operator()(leaf const&)
    {
        result = true;
    }

    bool result;
};

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_IS_LEAF_HPP
