// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree polymorphic nodes
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_DEFAULT_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_DEFAULT_HPP

#include <vector>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree {

// visitor forward declaration
template <typename Value, typename Parameters, typename Box, typename Allocators, typename Tag, bool IsVisitableConst>
struct visitor_poly;

// nodes types

template <typename Value, typename Parameters, typename Box, typename Allocators, typename Tag>
struct node_poly
{
    virtual ~node_poly() {}
    virtual void apply_visitor(visitor_poly<Value, Parameters, Box, Allocators, Tag, false> &) = 0;
    virtual void apply_visitor(visitor_poly<Value, Parameters, Box, Allocators, Tag, true> &) const = 0;
};

template <typename Value, typename Parameters, typename Box, typename Allocators, typename Tag>
struct internal_node_poly : public node_poly<Value, Parameters, Box, Allocators, Tag>
{
    typedef std::vector<
        std::pair<Box, node_poly<Value, Parameters, Box, Allocators, Tag> *>,
        typename Allocators::internal_node_elements_allocator_type
    > elements_type;

    inline internal_node_poly(typename Allocators::internal_node_elements_allocator_type & al)
        : elements(al)
    {}

    void apply_visitor(visitor_poly<Value, Parameters, Box, Allocators, Tag, false> & v) { v(*this); }
    void apply_visitor(visitor_poly<Value, Parameters, Box, Allocators, Tag, true> & v) const { v(*this); }

    elements_type elements;
};

template <typename Value, typename Parameters, typename Box, typename Allocators, typename Tag>
struct leaf_poly : public node_poly<Value, Parameters, Box, Allocators, Tag>
{
    typedef std::vector<
        Value,
        typename Allocators::leaf_elements_allocator_type
    > elements_type;

    inline leaf_poly(typename Allocators::leaf_elements_allocator_type & al)
        : elements(al)
    {}

    void apply_visitor(visitor_poly<Value, Parameters, Box, Allocators, Tag, false> & v) { v(*this); }
    void apply_visitor(visitor_poly<Value, Parameters, Box, Allocators, Tag, true> & v) const { v(*this); }

    elements_type elements;
};

// nodes traits

template <typename Value, typename Parameters, typename Box, typename Allocators, typename Tag>
struct node
{
    typedef node_poly<Value, Parameters, Box, Allocators, Tag> type;
};

template <typename Value, typename Parameters, typename Box, typename Allocators, typename Tag>
struct internal_node
{
    typedef internal_node_poly<Value, Parameters, Box, Allocators, Tag> type;
};

template <typename Value, typename Parameters, typename Box, typename Allocators, typename Tag>
struct leaf
{
    typedef leaf_poly<Value, Parameters, Box, Allocators, Tag> type;
};

// nodes conversion

template <typename Derived, typename Parameters, typename Value, typename Box, typename Allocators, typename Tag>
inline Derived & get(node_poly<Value, Parameters, Box, Allocators, Tag> & n)
{
    assert(dynamic_cast<Derived*>(&n));
    return static_cast<Derived&>(n);
}

template <typename Derived, typename Parameters, typename Value, typename Box, typename Allocators, typename Tag>
inline Derived * get(node_poly<Value, Parameters, Box, Allocators, Tag> * n)
{
    assert(dynamic_cast<Derived*>(n));
    return static_cast<Derived*>(n);
}

// visitor

template <typename Value, typename Parameters, typename Box, typename Allocators, typename Tag>
struct visitor_poly<Value, Parameters, Box, Allocators, Tag, true>
{
    typedef typename internal_node<Value, Parameters, Box, Allocators, Tag>::type internal_node;
    typedef typename leaf<Value, Parameters, Box, Allocators, Tag>::type leaf;

    virtual ~visitor_poly() {}

    virtual void operator()(internal_node const&) = 0;
    virtual void operator()(leaf const&) = 0;
};

template <typename Value, typename Parameters, typename Box, typename Allocators, typename Tag>
struct visitor_poly<Value, Parameters, Box, Allocators, Tag, false>
{
    typedef typename internal_node<Value, Parameters, Box, Allocators, Tag>::type internal_node;
    typedef typename leaf<Value, Parameters, Box, Allocators, Tag>::type leaf;

    virtual ~visitor_poly() {}

    virtual void operator()(internal_node &) = 0;
    virtual void operator()(leaf &) = 0;
};

// visitor traits

template <typename Value, typename Parameters, typename Box, typename Allocators, typename Tag, bool IsVisitableConst>
struct visitor
{
    typedef visitor_poly<Value, Parameters, Box, Allocators, Tag, IsVisitableConst> type;
};

template <typename Visitor, typename Visitable>
inline void apply_visitor(Visitor &v, Visitable & n)
{
    n.apply_visitor(v);
}

// element's indexable type

template <typename Value, typename Translator>
struct element_indexable_type
{
    typedef typename translator::indexable_type<Translator>::type type;
};

template <typename Value, typename Parameters, typename Box, typename Allocators, typename Tag, typename Translator>
struct element_indexable_type<
    std::pair<Box, node_poly<Value, Parameters, Box, Allocators, Tag> *>,
    Translator
>
{
    typedef Box type;
};

// element's indexable getter

template <typename Value, typename Translator>
inline typename Translator::result_type
element_indexable(Value const& el, Translator const& tr)
{
	return tr(el);
}

template <typename Value, typename Parameters, typename Box, typename Allocators, typename Tag, typename Translator>
inline Box const&
element_indexable(
    std::pair< Box, node_poly<Value, Parameters, Box, Allocators, Tag> *> const& el,
    Translator const&)
{
    return el.first;
}

// nodes elements

template <typename Node>
struct elements_type
{
	typedef typename Node::elements_type type;
};

template <typename Node>
inline typename elements_type<Node>::type &
elements(Node & n)
{
	return n.elements;
}

template <typename Node>
inline typename elements_type<Node>::type const&
elements(Node const& n)
{
	return n.elements;
}

// allocators

template <typename Allocator, typename Value, typename Parameters, typename Box, typename Tag>
struct allocators_poly
{
    typedef Allocator allocator_type;
    typedef typename allocator_type::size_type size_type;

    typedef typename allocator_type::template rebind<
        typename internal_node<Value, Parameters, Box, allocators_poly, Tag>::type
    >::other internal_node_allocator_type;

    typedef typename allocator_type::template rebind<
        typename leaf<Value, Parameters, Box, allocators_poly, Tag>::type
    >::other leaf_allocator_type;

    typedef typename allocator_type::template rebind<
        std::pair<Box, node_poly<Value, Parameters, Box, allocators_poly, Tag> *>
    >::other internal_node_elements_allocator_type;

    typedef typename allocator_type::template rebind<
        Value
    >::other leaf_elements_allocator_type;

    inline explicit allocators_poly(Allocator alloc)
        : allocator(alloc)
        , internal_node_allocator(allocator)
        , leaf_allocator(allocator)
        , internal_node_elements_allocator(allocator)
        , leaf_elements_allocator(allocator)
    {}

    allocator_type allocator;
    internal_node_allocator_type internal_node_allocator;
    leaf_allocator_type leaf_allocator;
    internal_node_elements_allocator_type internal_node_elements_allocator;
    leaf_elements_allocator_type leaf_elements_allocator;
};

// allocators

template <typename Allocator, typename Value, typename Parameters, typename Box, typename Tag>
struct allocators
{
    typedef allocators_poly<Allocator, Value, Parameters, Box, Tag> type;
};

// create_node_impl

template <typename Node>
struct create_node_poly
{
    template <typename RetNode, typename AllocNode, typename AllocElems>
    static inline RetNode * apply(AllocNode & alloc_node, AllocElems & alloc_elems)
    {
        Node * p = alloc_node.allocate(1);

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

// destroy_node_impl

template <typename Node>
struct destroy_node_poly
{
    template <typename AllocNode, typename BaseNode>
    static inline void apply(AllocNode & alloc_node, BaseNode * n)
    {
        Node * p = rtree::get<Node>(n);
        alloc_node.destroy(p);
        alloc_node.deallocate(p, 1);
    }
};

// create_node

template <typename Allocators, typename Node>
struct create_node
{
    BOOST_MPL_ASSERT_MSG(
        (false),
        NOT_IMPLEMENTED_FOR_THIS_NODE_TYPE,
        (create_node));
};

template <typename Allocators, typename Value, typename Parameters, typename Box, typename Tag>
struct create_node<
    Allocators,
    internal_node_poly<Value, Parameters, Box, Allocators, Tag>
>
{
    static inline typename node<Value, Parameters, Box, Allocators, Tag>::type *
    apply(Allocators & allocators)
    {
        return create_node_poly<
            internal_node_poly<Value, Parameters, Box, Allocators, Tag>
        >::template apply<
            typename node<Value, Parameters, Box, Allocators, Tag>::type
        >(allocators.internal_node_allocator, allocators.internal_node_elements_allocator);
    }
};

template <typename Allocators, typename Value, typename Parameters, typename Box, typename Tag>
struct create_node<
    Allocators,
    leaf_poly<Value, Parameters, Box, Allocators, Tag>
>
{
    static inline typename node<Value, Parameters, Box, Allocators, Tag>::type *
    apply(Allocators & allocators)
    {
        return create_node_poly<
            leaf_poly<Value, Parameters, Box, Allocators, Tag>
        >::template apply<
            typename node<Value, Parameters, Box, Allocators, Tag>::type
        >(allocators.leaf_allocator, allocators.leaf_elements_allocator);
    }
};

// destroy_node

template <typename Allocators, typename Node>
struct destroy_node
{
    BOOST_MPL_ASSERT_MSG(
        (false),
        NOT_IMPLEMENTED_FOR_THIS_NODE_TYPE,
        (destroy_node));
};

template <typename Allocators, typename Value, typename Parameters, typename Box, typename Tag>
struct destroy_node<
    Allocators,
    internal_node_poly<Value, Parameters, Box, Allocators, Tag>
>
{
    static inline void apply(Allocators & allocators, typename node<Value, Parameters, Box, Allocators, Tag>::type * n)
    {
        destroy_node_poly<
            internal_node_poly<Value, Parameters, Box, Allocators, Tag>
        >::apply(allocators.internal_node_allocator, n);
    }
};

template <typename Allocators, typename Value, typename Parameters, typename Box, typename Tag>
struct destroy_node<
    Allocators,
    leaf_poly<Value, Parameters, Box, Allocators, Tag>
>
{
    static inline void apply(Allocators & allocators, typename node<Value, Parameters, Box, Allocators, Tag>::type * n)
    {
        destroy_node_poly<
            leaf_poly<Value, Parameters, Box, Allocators, Tag>
        >::apply(allocators.leaf_allocator, n);
    }
};

}} // namespace detail::rtree

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_DEFAULT_HPP
