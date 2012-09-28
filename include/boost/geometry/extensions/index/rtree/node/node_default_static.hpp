// Boost.Geometry Index
//
// R-tree nodes based on runtime-polymorphism, storing static-size containers
//
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_DEFAULT_STATIC_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_DEFAULT_STATIC_HPP

#include <boost/geometry/extensions/index/rtree/node/dynamic_visitor.hpp>
#include <boost/geometry/extensions/index/pushable_array.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree {

template <typename Value, typename Parameters, typename Box, typename Allocators>
struct dynamic_internal_node<Value, Parameters, Box, Allocators, node_default_static_tag>
	: public dynamic_node<Value, Parameters, Box, Allocators, node_default_static_tag>
{
    typedef index::pushable_array<
        std::pair<
            Box,
            dynamic_node<Value, Parameters, Box, Allocators, node_default_static_tag> *
        >,
		Parameters::max_elements + 1
    > elements_type;

    template <typename Dummy>
    inline dynamic_internal_node(Dummy) {}

    void apply_visitor(dynamic_visitor<Value, Parameters, Box, Allocators, node_default_static_tag, false> & v) { v(*this); }
    void apply_visitor(dynamic_visitor<Value, Parameters, Box, Allocators, node_default_static_tag, true> & v) const { v(*this); }

    elements_type elements;
};

template <typename Value, typename Parameters, typename Box, typename Allocators>
struct dynamic_leaf<Value, Parameters, Box, Allocators, node_default_static_tag>
	: public dynamic_node<Value, Parameters, Box, Allocators, node_default_static_tag>
{
    typedef index::pushable_array<Value, Parameters::max_elements + 1> elements_type;

    template <typename Dummy>
    inline dynamic_leaf(Dummy) {}

    void apply_visitor(dynamic_visitor<Value, Parameters, Box, Allocators, node_default_static_tag, false> & v) { v(*this); }
    void apply_visitor(dynamic_visitor<Value, Parameters, Box, Allocators, node_default_static_tag, true> & v) const { v(*this); }

    elements_type elements;
};

// elements derived type
template <typename OldValue, size_t N, typename NewValue>
struct container_from_elements_type<index::pushable_array<OldValue, N>, NewValue>
{
    typedef index::pushable_array<NewValue, N> type;
};

// allocators_poly

template <typename Allocator, typename Value, typename Parameters, typename Box>
struct allocators_poly<Allocator, Value, Parameters, Box, node_default_static_tag>
{
    typedef Allocator allocator_type;
    typedef typename allocator_type::size_type size_type;

    typedef typename allocator_type::template rebind<
        typename internal_node<Value, Parameters, Box, allocators_poly, node_default_static_tag>::type
    >::other internal_node_allocator_type;

    typedef typename allocator_type::template rebind<
        typename leaf<Value, Parameters, Box, allocators_poly, node_default_static_tag>::type
    >::other leaf_allocator_type;

    inline explicit allocators_poly(Allocator alloc)
        : allocator(alloc)
        , internal_node_allocator(allocator)
        , leaf_allocator(allocator)
    {}

    allocator_type allocator;
    internal_node_allocator_type internal_node_allocator;
    leaf_allocator_type leaf_allocator;
};

// create_node

template <typename Allocators, typename Value, typename Parameters, typename Box>
struct create_node<
    Allocators,
    dynamic_internal_node<Value, Parameters, Box, Allocators, node_default_static_tag>
>
{
    static inline typename node<Value, Parameters, Box, Allocators, node_default_static_tag>::type *
    apply(Allocators & allocators)
    {
        return create_node_poly<
            dynamic_internal_node<Value, Parameters, Box, Allocators, node_default_static_tag>
        >::template apply<
            typename node<Value, Parameters, Box, Allocators, node_default_static_tag>::type
        >(allocators.internal_node_allocator, allocators.internal_node_allocator);
    }
};

template <typename Allocators, typename Value, typename Parameters, typename Box>
struct create_node<
    Allocators,
    dynamic_leaf<Value, Parameters, Box, Allocators, node_default_static_tag>
>
{
    static inline typename node<Value, Parameters, Box, Allocators, node_default_static_tag>::type *
    apply(Allocators & allocators)
    {
        return create_node_poly<
            dynamic_leaf<Value, Parameters, Box, Allocators, node_default_static_tag>
        >::template apply<
            typename node<Value, Parameters, Box, Allocators, node_default_static_tag>::type
        >(allocators.leaf_allocator, allocators.leaf_allocator);
    }
};

}} // namespace detail::rtree

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_DEFAULT_STATIC_HPP
