// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree polymorphic nodes with static-size containers
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_DEFAULT_STATIC_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_DEFAULT_STATIC_HPP

#include <boost/geometry/extensions/index/pushable_array.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree {

template <typename Value, typename Parameters, typename Box>
struct internal_node_poly<Value, Parameters, Box, default_static_tag>
	: public node_poly<Value, Parameters, Box, default_static_tag>
{
    typedef index::pushable_array<
        std::pair<Box, node_poly<Value, Parameters, Box, default_static_tag> *>,
		Parameters::max_elements + 1
    > elements_type;

    void apply_visitor(visitor_poly<Value, Parameters, Box, default_static_tag, false> & v) { v(*this); }
    void apply_visitor(visitor_poly<Value, Parameters, Box, default_static_tag, true> & v) const { v(*this); }

    elements_type elements;
};

template <typename Value, typename Parameters, typename Box>
struct leaf_poly<Value, Parameters, Box, default_static_tag>
	: public node_poly<Value, Parameters, Box, default_static_tag>
{
    typedef index::pushable_array<Value, Parameters::max_elements + 1> elements_type;

    void apply_visitor(visitor_poly<Value, Parameters, Box, default_static_tag, false> & v) { v(*this); }
    void apply_visitor(visitor_poly<Value, Parameters, Box, default_static_tag, true> & v) const { v(*this); }

    elements_type elements;
};

}} // namespace detail::rtree

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_DEFAULT_STATIC_HPP
