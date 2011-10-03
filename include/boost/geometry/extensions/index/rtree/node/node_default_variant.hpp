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

template <typename Value, typename Parameters, typename Box, typename Tag>
struct internal_node_variant
{
    typedef std::vector<
        std::pair<
            Box,
            typename node<Value, Parameters, Box, Tag>::type *
        >
    > elements_type;

    elements_type elements;
};

template <typename Value, typename Parameters, typename Box, typename Tag>
struct leaf_variant
{
    typedef std::vector<Value> elements_type;
    elements_type elements;
};

// nodes traits

template <typename Value, typename Parameters, typename Box>
struct node<Value, Parameters, Box, node_default_variant_tag>
{
	typedef boost::variant<
		leaf_variant<Value, Parameters, Box, node_default_variant_tag>,
		internal_node_variant<Value, Parameters, Box, node_default_variant_tag>
	> type;
};

template <typename Value, typename Parameters, typename Box>
struct internal_node<Value, Parameters, Box, node_default_variant_tag>
{
    typedef internal_node_variant<Value, Parameters, Box, node_default_variant_tag> type;
};

template <typename Value, typename Parameters, typename Box>
struct leaf<Value, Parameters, Box, node_default_variant_tag>
{
    typedef leaf_variant<Value, Parameters, Box, node_default_variant_tag> type;
};

// nodes conversion

template <typename V, typename Value, typename Parameters, typename Box, typename Tag>
inline V & get(
	boost::variant<
		leaf_variant<Value, Parameters, Box, Tag>,
		internal_node_variant<Value, Parameters, Box, Tag>
	> &v
)
{
    return boost::get<V>(v);
}

template <typename V, typename Value, typename Parameters, typename Box, typename Tag>
inline V * get(
	boost::variant<
		leaf_variant<Value, Parameters, Box, Tag>,
		internal_node_variant<Value, Parameters, Box, Tag>
	> *v
)
{
    return boost::get<V>(v);
}

// visitor traits

template <typename Value, typename Parameters, typename Box, bool IsVisitableConst>
struct visitor<Value, Parameters, Box, node_default_variant_tag, IsVisitableConst>
{
    typedef static_visitor<> type;
};

template <typename Visitor, typename Value, typename Parameters, typename Box, typename Tag>
inline void apply_visitor(Visitor & v,
						  boost::variant<
							  leaf_variant<Value, Parameters, Box, Tag>,
							  internal_node_variant<Value, Parameters, Box, Tag>
						  > & n)
{
    boost::apply_visitor(v, n);
}

template <typename Visitor, typename Value, typename Parameters, typename Box, typename Tag>
inline void apply_visitor(Visitor & v,
						  boost::variant<
							  leaf_variant<Value, Parameters, Box, Tag>,
							  internal_node_variant<Value, Parameters, Box, Tag>
						  > const& n)
{
	boost::apply_visitor(v, n);
}

// element's indexable type

template <typename Value, typename Parameters, typename Box, typename Tag, typename Translator>
struct element_indexable_type<
    std::pair<
        Box,
        boost::variant<
            leaf_variant<Value, Parameters, Box, Tag>,
            internal_node_variant<Value, Parameters, Box, Tag>
        > *
    >,
    Translator
>
{
    typedef Box type;
};

// element's indexable getter

template <typename Value, typename Parameters, typename Box, typename Tag, typename Translator>
inline Box const&
element_indexable(std::pair<
					  Box,
					  boost::variant<
						  leaf_variant<Value, Parameters, Box, Tag>,
						  internal_node_variant<Value, Parameters, Box, Tag>
					  > *
				  > const& el,
				  Translator const&)
{
    return el.first;
}

// allocators

template <typename Value, typename Parameters, typename Box, typename Allocator>
struct allocators<Value, Parameters, Box, node_default_variant_tag, Allocator>
{
    typedef Allocator allocator_type;
    typedef typename allocator_type::size_type size_type;

    typedef typename allocator_type::template rebind<
        typename node<Value, Parameters, Box, node_default_variant_tag>::type
    >::other node_allocator_type;

    inline explicit allocators(Allocator alloc)
        : allocator(alloc)
        , node_allocator(allocator)
    {}

    allocator_type allocator;
    node_allocator_type node_allocator;
};

// create_node

template <typename Allocators, typename Value, typename Parameters, typename Box, typename Tag>
struct create_node<
    Allocators,
    internal_node_variant<Value, Parameters, Box, Tag>
>
{
    static inline typename node<Value, Parameters, Box, Tag>::type *
    apply(Allocators & allocators)
    {
        /*return new typename node<Value, Parameters, Box, Tag>::type(
            internal_node_variant<Value, Parameters, Box, Tag>() );*/

        typename node<Value, Parameters, Box, Tag>::type * p
            = allocators.node_allocator.allocate(1);

        allocators.node_allocator.construct(
            p,
            internal_node_variant<Value, Parameters, Box, Tag>());

        return p;
    }
};

template <typename Allocators, typename Value, typename Parameters, typename Box, typename Tag>
struct create_node<
    Allocators,
    leaf_variant<Value, Parameters, Box, Tag>
>
{
    static inline typename node<Value, Parameters, Box, Tag>::type * apply(Allocators & allocators)
    {
        /*return new typename node<Value, Parameters, Box, Tag>::type(
            leaf_variant<Value, Parameters, Box, Tag>() );*/

        typename node<Value, Parameters, Box, Tag>::type * p
            = allocators.node_allocator.allocate(1);

        allocators.node_allocator.construct(
            p,
            leaf_variant<Value, Parameters, Box, Tag>());

        return p;
    }
};

// destroy_node

template <typename Allocators, typename Value, typename Parameters, typename Box, typename Tag>
struct destroy_node<
    Allocators,
    internal_node_variant<Value, Parameters, Box, Tag>
>
{
    static inline void apply(Allocators & allocators, typename node<Value, Parameters, Box, Tag>::type * n)
    {
        //delete n;

        allocators.node_allocator.destroy(n);
        allocators.node_allocator.deallocate(n, 1);
    }
};

template <typename Allocators, typename Value, typename Parameters, typename Box, typename Tag>
struct destroy_node<
    Allocators,
    leaf_variant<Value, Parameters, Box, Tag>
>
{
    static inline void apply(Allocators & allocators, typename node<Value, Parameters, Box, Tag>::type * n)
    {
        //delete n;

        allocators.node_allocator.destroy(n);
        allocators.node_allocator.deallocate(n, 1);
    }
};

}} // namespace detail::rtree

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_DEFAULT_VARIANT_HPP
