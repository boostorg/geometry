// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree is leaf visitor
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_IS_LEAF_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_IS_LEAF_HPP

#include <boost/geometry/extensions/index/rtree/node/node.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

template <typename Value, typename Options, typename Box>
struct is_leaf : public rtree::visitor<Value, typename Options::parameters_type, Box, typename Options::node_tag, true>::type
{
    typedef typename rtree::internal_node<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type internal_node;
    typedef typename rtree::leaf<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type leaf;

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
