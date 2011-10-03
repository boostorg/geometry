// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree variant nodes
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_DEFAULT_VARIANT_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_DEFAULT_VARIANT_HPP

#include <vector>
#include <boost/variant.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree {

// nodes default types

template <typename Value, typename Parameters, typename Box, typename Allocators, typename Tag>
struct internal_node_variant
{
    typedef std::vector<
        std::pair<
            Box,
            typename node<Value, Parameters, Box, Allocators, Tag>::type *
        >,
        typename Allocators::internal_node_elements_allocator_type
    > elements_type;

    inline internal_node_variant(typename Allocators::internal_node_elements_allocator_type & al)
        : elements(al)
    {}

    elements_type elements;
};

template <typename Value, typename Parameters, typename Box, typename Allocators, typename Tag>
struct leaf_variant
{
    typedef std::vector<
        Value,
        typename Allocators::leaf_elements_allocator_type
    > elements_type;

    inline leaf_variant(typename Allocators::leaf_elements_allocator_type & al)
        : elements(al)
    {}

    elements_type elements;
};

// nodes traits

template <typename Value, typename Parameters, typename Box, typename Allocators>
struct node<Value, Parameters, Box, Allocators, node_default_variant_tag>
{
	typedef boost::variant<
		leaf_variant<Value, Parameters, Box, Allocators, node_default_variant_tag>,
		internal_node_variant<Value, Parameters, Box, Allocators, node_default_variant_tag>
	> type;
};

template <typename Value, typename Parameters, typename Box, typename Allocators>
struct internal_node<Value, Parameters, Box, Allocators, node_default_variant_tag>
{
    typedef internal_node_variant<Value, Parameters, Box, Allocators, node_default_variant_tag> type;
};

template <typename Value, typename Parameters, typename Box, typename Allocators>
struct leaf<Value, Parameters, Box, Allocators, node_default_variant_tag>
{
    typedef leaf_variant<Value, Parameters, Box, Allocators, node_default_variant_tag> type;
};

// nodes conversion

template <typename V, typename Value, typename Parameters, typename Box, typename Allocators, typename Tag>
inline V & get(
	boost::variant<
		leaf_variant<Value, Parameters, Box, Allocators, Tag>,
		internal_node_variant<Value, Parameters, Box, Allocators, Tag>
	> &v
)
{
    return boost::get<V>(v);
}

template <typename V, typename Value, typename Parameters, typename Box, typename Allocators, typename Tag>
inline V * get(
	boost::variant<
		leaf_variant<Value, Parameters, Box, Allocators, Tag>,
		internal_node_variant<Value, Parameters, Box, Allocators, Tag>
	> *v
)
{
    return boost::get<V>(v);
}

// visitor traits

template <typename Value, typename Parameters, typename Box, typename Allocators, bool IsVisitableConst>
struct visitor<Value, Parameters, Box, Allocators, node_default_variant_tag, IsVisitableConst>
{
    typedef static_visitor<> type;
};

template <typename Visitor, typename Value, typename Parameters, typename Box, typename Allocators, typename Tag>
inline void apply_visitor(Visitor & v,
						  boost::variant<
							  leaf_variant<Value, Parameters, Box, Allocators, Tag>,
							  internal_node_variant<Value, Parameters, Box, Allocators, Tag>
						  > & n)
{
    boost::apply_visitor(v, n);
}

template <typename Visitor, typename Value, typename Parameters, typename Box, typename Allocators, typename Tag>
inline void apply_visitor(Visitor & v,
						  boost::variant<
							  leaf_variant<Value, Parameters, Box, Allocators, Tag>,
							  internal_node_variant<Value, Parameters, Box, Allocators, Tag>
						  > const& n)
{
	boost::apply_visitor(v, n);
}

// element's indexable type

template <typename Value, typename Parameters, typename Box, typename Allocators, typename Tag, typename Translator>
struct element_indexable_type<
    std::pair<
        Box,
        boost::variant<
            leaf_variant<Value, Parameters, Box, Allocators, Tag>,
            internal_node_variant<Value, Parameters, Box, Allocators, Tag>
        > *
    >,
    Translator
>
{
    typedef Box type;
};

// element's indexable getter

template <typename Value, typename Parameters, typename Box, typename Allocators, typename Tag, typename Translator>
inline Box const&
element_indexable(std::pair<
					  Box,
					  boost::variant<
						  leaf_variant<Value, Parameters, Box, Allocators, Tag>,
						  internal_node_variant<Value, Parameters, Box, Allocators, Tag>
					  > *
				  > const& el,
				  Translator const&)
{
    return el.first;
}

// allocators

template <typename Allocator, typename Value, typename Parameters, typename Box, typename Tag>
struct allocators_variant
{
    typedef Allocator allocator_type;
    typedef typename allocator_type::size_type size_type;

    typedef typename allocator_type::template rebind<
        typename node<Value, Parameters, Box, allocators_variant, node_default_variant_tag>::type
    >::other node_allocator_type;

    typedef typename allocator_type::template rebind<
        std::pair<Box, node_poly<Value, Parameters, Box, allocators_variant, node_default_variant_tag> *>
    >::other internal_node_elements_allocator_type;

    typedef typename allocator_type::template rebind<
        Value
    >::other leaf_elements_allocator_type;

    inline explicit allocators_variant(Allocator alloc)
        : allocator(alloc)
        , node_allocator(allocator)
        , internal_node_elements_allocator(allocator)
        , leaf_elements_allocator(allocator)
    {}

    allocator_type allocator;
    node_allocator_type node_allocator;
    internal_node_elements_allocator_type internal_node_elements_allocator;
    leaf_elements_allocator_type leaf_elements_allocator;
};

// allocators

template <typename Allocator, typename Value, typename Parameters, typename Box>
struct allocators<Allocator, Value, Parameters, Box, node_default_variant_tag>
{
    typedef allocators_variant<Allocator, Value, Parameters, Box, node_default_variant_tag> type;
};

// create_node_variant

template <typename Node>
struct create_node_variant
{
    template <typename RetNode, typename AllocNode, typename AllocElems>
    static inline RetNode * apply(AllocNode & alloc_node, AllocElems & alloc_elems)
    {
        RetNode * p = alloc_node.allocate(1);

        if ( 0 == p )
            throw std::bad_alloc();

        try
        {
            alloc_node.construct(p, Node(alloc_elems));
        }
        catch(...)
        {
            alloc_node.deallocate(p, 1);
            throw;
        }

        return p;
    }
};

// destroy_node_variant

template <typename Node>
struct destroy_node_variant
{
    template <typename AllocNode, typename BaseNode>
    static inline void apply(AllocNode & alloc_node, BaseNode * n)
    {
        alloc_node.destroy(n);
        alloc_node.deallocate(n, 1);
    }
};

// create_node

template <typename Allocators, typename Value, typename Parameters, typename Box, typename Tag>
struct create_node<
    Allocators,
    internal_node_variant<Value, Parameters, Box, Allocators, Tag>
>
{
    static inline typename node<Value, Parameters, Box, Allocators, Tag>::type *
    apply(Allocators & allocators)
    {
        return create_node_variant<
            internal_node_variant<Value, Parameters, Box, Allocators, Tag>
        >::template apply<
            typename node<Value, Parameters, Box, Allocators, Tag>::type
        >(allocators.node_allocator, allocators.internal_node_elements_allocator);
    }
};

template <typename Allocators, typename Value, typename Parameters, typename Box, typename Tag>
struct create_node<
    Allocators,
    leaf_variant<Value, Parameters, Box, Allocators, Tag>
>
{
    static inline typename node<Value, Parameters, Box, Allocators, Tag>::type *
    apply(Allocators & allocators)
    {
        return create_node_variant<
            leaf_variant<Value, Parameters, Box, Allocators, Tag>
        >::template apply<
            typename node<Value, Parameters, Box, Allocators, Tag>::type
        >(allocators.node_allocator, allocators.leaf_elements_allocator);
    }
};

// destroy_node

template <typename Allocators, typename Value, typename Parameters, typename Box, typename Tag>
struct destroy_node<
    Allocators,
    internal_node_variant<Value, Parameters, Box, Allocators, Tag>
>
{
    static inline void apply(Allocators & allocators, typename node<Value, Parameters, Box, Allocators, Tag>::type * n)
    {
        destroy_node_variant<
            internal_node_variant<Value, Parameters, Box, Allocators, Tag>
        >::apply(allocators.node_allocator, n);
    }
};

template <typename Allocators, typename Value, typename Parameters, typename Box, typename Tag>
struct destroy_node<
    Allocators,
    leaf_variant<Value, Parameters, Box, Allocators, Tag>
>
{
    static inline void apply(Allocators & allocators, typename node<Value, Parameters, Box, Allocators, Tag>::type * n)
    {
        destroy_node_variant<
            leaf_variant<Value, Parameters, Box, Allocators, Tag>
        >::apply(allocators.node_allocator, n);
    }
};

}} // namespace detail::rtree

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_DEFAULT_VARIANT_HPP
