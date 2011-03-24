// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree delete visitor
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_RTREE_DELETE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_RTREE_DELETE_HPP

#include <boost/geometry/extensions/index/rtree/rtree_node.hpp>

namespace boost { namespace geometry { namespace index {

namespace visitors {

template <typename Value, typename Translator, typename Box, typename Tag>
struct rtree_delete : public boost::static_visitor<>
{
    typedef typename index::detail::rtree_internal_node<Value, Box, Tag>::type internal_node;
    typedef typename index::detail::rtree_leaf<Value, Box, Tag>::type leaf;

    inline void operator()(internal_node & n) const
    {
        typedef typename internal_node::children_type children_type;

        for (typename children_type::iterator it = n.children.begin();
            it != n.children.end(); ++it)
        {
            boost::apply_visitor(*this, *it->second);

            index::detail::rtree_delete_node(it->second);
        }
    }

    inline void operator()(leaf & n) const
    {
    }
};

} // namespace visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_RTREE_DELETE_HPP
