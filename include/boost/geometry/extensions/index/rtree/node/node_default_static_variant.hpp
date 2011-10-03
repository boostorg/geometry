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

template <typename Value, typename Parameters, typename Box, typename Allocators>
struct internal_node_variant<Value, Parameters, Box, Allocators, node_default_static_variant_tag>
{
    typedef index::pushable_array<
        std::pair<
            Box,
            typename node<Value, Parameters, Box, Allocators, node_default_static_variant_tag>::type *
        >,
        Parameters::max_elements + 1
    > elements_type;

    template <typename Dummy>
    inline internal_node_variant(Dummy) {}

    elements_type elements;
};

template <typename Value, typename Parameters, typename Box, typename Allocators>
struct leaf_variant<Value, Parameters, Box, Allocators, node_default_static_variant_tag>
{
    typedef index::pushable_array<Value, Parameters::max_elements + 1> elements_type;

    template <typename Dummy>
    inline leaf_variant(Dummy) {}

    elements_type elements;
};

// nodes traits

template <typename Value, typename Parameters, typename Box, typename Allocators>
struct node<Value, Parameters, Box, Allocators, node_default_static_variant_tag>
{
	typedef boost::variant<
		leaf_variant<Value, Parameters, Box, Allocators, node_default_static_variant_tag>,
		internal_node_variant<Value, Parameters, Box, Allocators, node_default_static_variant_tag>
	> type;
};

template <typename Value, typename Parameters, typename Box, typename Allocators>
struct internal_node<Value, Parameters, Box, Allocators, node_default_static_variant_tag>
{
    typedef internal_node_variant<Value, Parameters, Box, Allocators, node_default_static_variant_tag> type;
};

template <typename Value, typename Parameters, typename Box, typename Allocators>
struct leaf<Value, Parameters, Box, Allocators, node_default_static_variant_tag>
{
    typedef leaf_variant<Value, Parameters, Box, Allocators, node_default_static_variant_tag> type;
};

// visitor traits

template <typename Value, typename Parameters, typename Box, typename Allocators, bool IsVisitableConst>
struct visitor<Value, Parameters, Box, Allocators, node_default_static_variant_tag, IsVisitableConst>
{
    typedef static_visitor<> type;
};

// allocators_variant

template <typename Allocator, typename Value, typename Parameters, typename Box>
struct allocators_variant<Allocator, Value, Parameters, Box, node_default_static_variant_tag>
{
    typedef Allocator allocator_type;
    typedef typename allocator_type::size_type size_type;

    typedef typename allocator_type::template rebind<
        typename node<Value, Parameters, Box, allocators_variant, node_default_static_variant_tag>::type
    >::other node_allocator_type;

    typedef typename allocator_type::template rebind<
        std::pair<Box, node_poly<Value, Parameters, Box, allocators_variant, node_default_static_variant_tag> *>
    >::other internal_node_elements_allocator_type;

    typedef typename allocator_type::template rebind<
        Value
    >::other leaf_elements_allocator_type;

    inline explicit allocators_variant(Allocator alloc)
        : allocator(alloc)
        , node_allocator(allocator)
    {}

    allocator_type allocator;
    node_allocator_type node_allocator;
};

// allocators

template <typename Allocator, typename Value, typename Parameters, typename Box>
struct allocators<Allocator, Value, Parameters, Box, node_default_static_variant_tag>
{
    typedef allocators_variant<Allocator, Value, Parameters, Box, node_default_static_variant_tag> type;
};

// create_node

template <typename Allocators, typename Value, typename Parameters, typename Box>
struct create_node<
    Allocators,
    internal_node_variant<Value, Parameters, Box, Allocators, node_default_static_variant_tag>
>
{
    static inline typename node<Value, Parameters, Box, Allocators, node_default_static_variant_tag>::type *
    apply(Allocators & allocators)
    {
        return create_node_variant<
            internal_node_variant<Value, Parameters, Box, Allocators, node_default_static_variant_tag>
        >::template apply<
            typename node<Value, Parameters, Box, Allocators, node_default_static_variant_tag>::type
        >(allocators.node_allocator, allocators.node_allocator);
    }
};

template <typename Allocators, typename Value, typename Parameters, typename Box>
struct create_node<
    Allocators,
    leaf_variant<Value, Parameters, Box, Allocators, node_default_static_variant_tag>
>
{
    static inline typename node<Value, Parameters, Box, Allocators, node_default_static_variant_tag>::type *
    apply(Allocators & allocators)
    {
        return create_node_variant<
            leaf_variant<Value, Parameters, Box, Allocators, node_default_static_variant_tag>
        >::template apply<
            typename node<Value, Parameters, Box, Allocators, node_default_static_variant_tag>::type
        >(allocators.node_allocator, allocators.node_allocator);
    }
};

}} // namespace detail::rtree

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_DEFAULT_STATIC_VARIANT_HPP
