// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree copying visitor
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_COPY_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_COPY_HPP

#include <boost/geometry/extensions/index/rtree/node/node.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

template <typename Value, typename Options, typename Translator, typename Box>
struct copy
    : public rtree::visitor<Value, typename Options::parameters_type, Box, typename Options::node_tag, false>::type
{
    typedef typename rtree::node<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type node;
    typedef typename rtree::internal_node<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type internal_node;
    typedef typename rtree::leaf<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type leaf;

    explicit inline copy()
        : result(0)
    {}

    inline void operator()(internal_node & n)
    {
        node * new_node = rtree::create_node(internal_node());

        typedef typename rtree::elements_type<internal_node>::type elements_type;
        elements_type & elements = rtree::elements(n);

        elements_type & elements_dst = rtree::elements(rtree::get<internal_node>(*new_node));

        for (typename elements_type::iterator it = elements.begin();
            it != elements.end(); ++it)
        {
            rtree::apply_visitor(*this, *it->second);

            elements_dst.push_back( std::make_pair(it->first, result) );
        }

        result = new_node;
    }

    inline void operator()(leaf & l)
    {
        node * new_node = rtree::create_node(leaf());
        
        typedef typename rtree::elements_type<leaf>::type elements_type;
        elements_type & elements = rtree::elements(l);

        elements_type & elements_dst = rtree::elements(rtree::get<leaf>(*new_node));

        for (typename elements_type::iterator it = elements.begin();
            it != elements.end(); ++it)
        {
            elements_dst.push_back(*it);
        }

        result = new_node;
    }

    node * result;
};

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_COPY_HPP
