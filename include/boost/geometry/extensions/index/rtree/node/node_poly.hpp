// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree polymorphic nodes
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_POLY_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_POLY_HPP

#include <vector>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree {

// visitor forward declaration
template <typename Value, typename Box, typename Tag, bool IsVisitableConst>
struct visitor_poly;

// nodes types

template <typename Value, typename Box, typename Tag>
struct node_poly
{
    virtual ~node_poly() {}
    virtual void apply_visitor(visitor_poly<Value, Box, Tag, false> &) = 0;
    virtual void apply_visitor(visitor_poly<Value, Box, Tag, true> &) const = 0;
};

template <typename Value, typename Box, typename Tag>
struct internal_node_poly : public node_poly<Value, Box, Tag>
{
    typedef std::vector<
        std::pair<Box, node_poly<Value, Box, Tag> *>
    > elements_type;

    void apply_visitor(visitor_poly<Value, Box, Tag, false> & v) { v(*this); }
    void apply_visitor(visitor_poly<Value, Box, Tag, true> & v) const { v(*this); }

    elements_type elements;
};

template <typename Value, typename Box, typename Tag>
struct leaf_poly : public node_poly<Value, Box, Tag>
{
    typedef std::vector<Value> elements_type;

    void apply_visitor(visitor_poly<Value, Box, Tag, false> & v) { v(*this); }
    void apply_visitor(visitor_poly<Value, Box, Tag, true> & v) const { v(*this); }

    elements_type elements;
};

// nodes traits

template <typename Value, typename Box, typename Tag>
struct node
{
    typedef node_poly<Value, Box, Tag> type;
};

template <typename Value, typename Box, typename Tag>
struct internal_node
{
    typedef internal_node_poly<Value, Box, Tag> type;
};

template <typename Value, typename Box, typename Tag>
struct leaf
{
    typedef leaf_poly<Value, Box, Tag> type;
};

// nodes conversion

template <typename Derived, typename Value, typename Box, typename Tag>
inline Derived & get(node_poly<Value, Box, Tag> & n)
{
    assert(dynamic_cast<Derived*>(&n));
    return dynamic_cast<Derived&>(n);
}

template <typename Derived, typename Value, typename Box, typename Tag>
inline Derived * get(node_poly<Value, Box, Tag> * n)
{
    assert(dynamic_cast<Derived*>(n));
    return dynamic_cast<Derived*>(n);
}

// visitor

template <typename Value, typename Box, typename Tag>
struct visitor_poly<Value, Box, Tag, true>
{
    typedef typename internal_node<Value, Box, Tag>::type internal_node;
    typedef typename leaf<Value, Box, Tag>::type leaf;

    virtual ~visitor_poly() {}

    virtual void operator()(internal_node const&) = 0;
    virtual void operator()(leaf const&) = 0;
};

template <typename Value, typename Box, typename Tag>
struct visitor_poly<Value, Box, Tag, false>
{
    typedef typename internal_node<Value, Box, Tag>::type internal_node;
    typedef typename leaf<Value, Box, Tag>::type leaf;

    virtual ~visitor_poly() {}

    virtual void operator()(internal_node &) = 0;
    virtual void operator()(leaf &) = 0;
};

// visitor traits

template <typename Value, typename Box, typename Tag, bool IsVisitableConst>
struct visitor
{
    typedef visitor_poly<Value, Box, Tag, IsVisitableConst> type;
};

template <typename Visitor, typename Visitable>
inline void apply_visitor(Visitor &v, Visitable & n)
{
    n.apply_visitor(v);
}

// uniform indexable for child node element's box and value's indexable

// value's indexable version

template <typename Value, typename Translator>
struct element_indexable_type
{
    typedef typename Translator::indexable_type type;
};

// node element's indexable specialization

template <typename Value, typename Box, typename Tag, typename Translator>
struct element_indexable_type<
    std::pair<Box, node_poly<Value, Box, Tag> *>,
    Translator
>
{
    typedef Box type;
};

template <typename Value, typename Box, typename Tag, typename Translator>
inline Box const&
element_indexable(
    std::pair< Box, node_poly<Value, Box, Tag> *> const& el,
    Translator const&)
{
    return el.first;
}

// create leaf node

template <typename Value, typename Box, typename Tag>
inline typename node<Value, Box, Tag>::type *
    create_node(leaf_poly<Value, Box, Tag> const& l)
{
    typedef typename node<Value, Box, Tag>::type node;
    node * n = new leaf_poly<Value, Box, Tag>(l);
    return n;
}

// create internal node

template <typename Value, typename Box, typename Tag>
inline typename node<Value, Box, Tag>::type *
    create_node(internal_node_poly<Value, Box, Tag> const& in)
{
    typedef typename node<Value, Box, Tag>::type node;
    node * n = new internal_node_poly<Value, Box, Tag>(in);
    return n;
}

// default node

template <typename Value, typename Box, typename Tag>
inline void delete_node(node_poly<Value, Box, Tag> * n)
{
    delete n;
}

}} // namespace detail::rtree

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_POLY_HPP
