// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree variant nodes
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_VARIANT_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_VARIANT_HPP

#include <vector>
#include <boost/variant.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree {

template <typename Value, typename Box, typename Tag>
struct node;

// nodes default types

template <typename Value, typename Box, typename Tag>
struct internal_node_variant
{
    typedef std::vector<
        std::pair<
            Box,
            typename node<Value, Box, Tag>::type *
        >
    > elements_type;

    elements_type elements;
};

template <typename Value, typename Box, typename Tag>
struct leaf_variant
{
    typedef std::vector<Value> elements_type;
    elements_type elements;
};

// nodes traits

template <typename Value, typename Box, typename Tag>
struct node
{
    typedef boost::variant<
        leaf_variant<Value, Box, Tag>,
        internal_node_variant<Value, Box, Tag>
    > type;
};

template <typename Value, typename Box, typename Tag>
struct internal_node
{
    typedef internal_node_variant<Value, Box, Tag> type;
};

template <typename Value, typename Box, typename Tag>
struct leaf
{
    typedef leaf_variant<Value, Box, Tag> type;
};

// nodes conversion

template <typename V, typename Variant>
inline V & get(Variant &v)
{
    return boost::get<V>(v);
}

template <typename V, typename Variant>
inline V * get(Variant *v)
{
    return boost::get<V>(v);
}

// visitor traits

template <typename Value, typename Box, typename Tag, bool IsVisitableConst>
struct visitor
{
    typedef static_visitor<> type;
};

template <typename Visitor, typename Visitable>
inline void apply_visitor(Visitor &v, Visitable &n)
{
    boost::apply_visitor(v, n);
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
    std::pair<
        Box,
        boost::variant<
            leaf_variant<Value, Box, Tag>,
            internal_node_variant<Value, Box, Tag>
        > *
    >,
    Translator
>
{
    typedef Box type;
};

template <typename Value, typename Box, typename Tag, typename Translator>
inline Box const&
element_indexable(
    std::pair<
        Box,
        boost::variant<
            leaf_variant<Value, Box, Tag>,
            internal_node_variant<Value, Box, Tag>
        > *
    > const& el,
    Translator const&)
{
    return el.first;
}

// create leaf node

template <typename Value, typename Box, typename Tag>
inline typename node<Value, Box, Tag>::type *
create_node(leaf_variant<Value, Box, Tag> const& l)
{
    typedef typename node<Value, Box, Tag>::type node;
    node * n = new node(l);
    return n;
}

// create internal node

template <typename Value, typename Box, typename Tag>
inline typename node<Value, Box, Tag>::type *
create_node(internal_node_variant<Value, Box, Tag> const& in)
{
    typedef typename node<Value, Box, Tag>::type node;
    node * n = new node(in);
    return n;
}

// default node

template <typename Value, typename Box, typename Tag>
inline void delete_node(
    boost::variant<
        leaf_variant<Value, Box, Tag>,
        internal_node_variant<Value, Box, Tag>
    > * n)
{
    delete n;
}

}} // namespace detail::rtree

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_VARIANT_HPP
