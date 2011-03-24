// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree is leaf visitor
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_RTREE_IS_LEAF_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_RTREE_IS_LEAF_HPP

#include <boost/geometry/extensions/index/rtree/rtree_node.hpp>

namespace boost { namespace geometry { namespace index {

namespace visitors {

template <typename Value, typename Box, typename Tag>
struct rtree_is_leaf : public boost::static_visitor<bool>
{
    typedef typename index::detail::rtree_internal_node<Value, Box, Tag>::type internal_node;
    typedef typename index::detail::rtree_leaf<Value, Box, Tag>::type leaf;

    inline bool operator()(internal_node const&) const
    {
        return false;
    }

    inline bool operator()(leaf const&) const
    {
        return true;
    }
};

} // namespace visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_RTREE_IS_LEAF_HPP
