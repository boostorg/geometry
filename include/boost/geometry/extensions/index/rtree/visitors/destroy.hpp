// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree destroy visitor
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_DELETE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_DELETE_HPP

#include <boost/geometry/extensions/index/rtree/node.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

template <typename Value, typename Translator, typename Box, typename Tag>
struct destroy : public boost::static_visitor<>
{
    typedef typename rtree::internal_node<Value, Box, Tag>::type internal_node;
    typedef typename rtree::leaf<Value, Box, Tag>::type leaf;

    inline void operator()(internal_node & n) const
    {
        typedef typename rtree::elements_type<internal_node>::type elements_type;
        elements_type & elements = rtree::elements_get(n);

        for (typename elements_type::iterator it = elements.begin();
            it != elements.end(); ++it)
        {
            boost::apply_visitor(*this, *it->second);

            rtree::delete_node(it->second);
        }
    }

    inline void operator()(leaf & n) const
    {
    }
};

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_DELETE_HPP
