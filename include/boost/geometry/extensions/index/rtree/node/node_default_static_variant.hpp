// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree variant nodes with static-size containers
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_DEFAULT_STATIC_VARIANT_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_DEFAULT_STATIC_VARIANT_HPP

#include <vector>
#include <boost/variant.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree {

// nodes default types

template <typename Value, typename Parameters, typename Box>
struct internal_node_variant<Value, Parameters, Box, node_default_static_variant_tag>
{
    typedef index::pushable_array<
        std::pair<
            Box,
            typename node<Value, Parameters, Box, node_default_static_variant_tag>::type *
        >,
        Parameters::max_elements + 1
    > elements_type;

    elements_type elements;
};

template <typename Value, typename Parameters, typename Box>
struct leaf_variant<Value, Parameters, Box, node_default_static_variant_tag>
{
    typedef index::pushable_array<Value, Parameters::max_elements + 1> elements_type;
    elements_type elements;
};

// nodes traits

template <typename Value, typename Parameters, typename Box>
struct node<Value, Parameters, Box, node_default_static_variant_tag>
{
	typedef boost::variant<
		leaf_variant<Value, Parameters, Box, node_default_static_variant_tag>,
		internal_node_variant<Value, Parameters, Box, node_default_static_variant_tag>
	> type;
};

template <typename Value, typename Parameters, typename Box>
struct internal_node<Value, Parameters, Box, node_default_static_variant_tag>
{
    typedef internal_node_variant<Value, Parameters, Box, node_default_static_variant_tag> type;
};

template <typename Value, typename Parameters, typename Box>
struct leaf<Value, Parameters, Box, node_default_static_variant_tag>
{
    typedef leaf_variant<Value, Parameters, Box, node_default_static_variant_tag> type;
};

// visitor traits

template <typename Value, typename Parameters, typename Box, bool IsVisitableConst>
struct visitor<Value, Parameters, Box, node_default_static_variant_tag, IsVisitableConst>
{
    typedef static_visitor<> type;
};

}} // namespace detail::rtree

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_DEFAULT_STATIC_VARIANT_HPP
