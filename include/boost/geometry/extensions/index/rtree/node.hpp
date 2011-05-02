// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree default nodes
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_HPP

#include <vector>
#include <boost/variant.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree {

template <typename Value, typename Box, typename Tag>
struct node;

// nodes default types

template <typename Value, typename Box, typename Tag>
struct internal_node_def
{
    typedef std::vector<
        std::pair<
            Box,
            typename node<Value, Box, Tag>::type *
        >
    > children_type;

    children_type children;
};

template <typename Value, typename Box, typename Tag>
struct leaf_def
{
    typedef std::vector<Value> values_type;
    values_type values;
};

// nodes traits

template <typename Value, typename Box, typename Tag>
struct node
{
    typedef boost::variant<
        leaf_def<Value, Box, Tag>,
        internal_node_def<Value, Box, Tag>
    > type;
};

template <typename Value, typename Box, typename Tag>
struct internal_node
{
    typedef internal_node_def<Value, Box, Tag> type;
};

template <typename Value, typename Box, typename Tag>
struct leaf
{
    typedef leaf_def<Value, Box, Tag> type;
};

// nodes elements extractor

template <typename Node>
struct elements_type
{
    typedef typename Node::elements_type type;
};

template <typename Value, typename Box, typename Tag>
struct elements_type< internal_node_def<Value, Box, Tag> >
{
    typedef typename internal_node_def<Value, Box, Tag>::children_type type;
};

template <typename Value, typename Box, typename Tag>
struct elements_type< leaf_def<Value, Box, Tag> >
{
    typedef typename leaf_def<Value, Box, Tag>::values_type type;
};

template <typename Node>
typename elements_type<Node>::type &
elements_get(Node & n)
{
    return n.elements;
}

template <typename Value, typename Box, typename Tag>
typename internal_node_def<Value, Box, Tag>::children_type &
elements_get(internal_node_def<Value, Box, Tag> & n)
{
    return n.children;
}

template <typename Value, typename Box, typename Tag>
typename leaf_def<Value, Box, Tag>::values_type &
elements_get(leaf_def<Value, Box, Tag> & n)
{
    return n.values;
}

//template <typename Node>
//struct element_type
//{
//    typedef typename elements_type<Node>::type::value_type type;
//};

// uniform indexable type for child node element's box and value's indexable

template <typename Value, typename Translator>
struct element_indexable_type
{
    typedef typename Translator::indexable_type type;
};

template <typename Value, typename Box, typename Tag, typename Translator>
struct element_indexable_type<
    std::pair<
        Box,
        boost::variant<
            leaf_def<Value, Box, Tag>,
            internal_node_def<Value, Box, Tag>
        > *
    >,
    Translator
>
{
    typedef Box type;
};

// uniform indexable getter for child node element's box and value's indexable

template <typename Value, typename Box, typename Tag, typename Translator>
Box const&
element_indexable(
    std::pair<
        Box,
        boost::variant<
            leaf_def<Value, Box, Tag>,
            internal_node_def<Value, Box, Tag>
        > *
    > const& el,
    Translator const&)
{
    return el.first;
}

template <typename Value, typename Translator>
typename Translator::indexable_type const&
element_indexable(Value const& el, Translator const& tr)
{
    return tr(el);
};

// elements box

template <typename Box, typename FwdIter, typename Translator>
inline Box elements_box(FwdIter first, FwdIter last, Translator const& tr)
{
    Box result;

    if (first == last)
    {    
        geometry::assign_zero(result);
        return result;
    }

    geometry::convert(element_indexable(*first, tr), result);
    ++first;

    for ( ; first != last ; ++first )
        geometry::expand(result, element_indexable(*first, tr));

    return result;
}

// create leaf node

template <typename Value, typename Box, typename Tag>
typename node<Value, Box, Tag>::type *
create_node(leaf_def<Value, Box, Tag> const& l)
{
    typedef typename node<Value, Box, Tag>::type node;
    node * n = new node(l);
    return n;
}

// create internal node

template <typename Value, typename Box, typename Tag>
typename node<Value, Box, Tag>::type *
create_node(internal_node_def<Value, Box, Tag> const& in)
{
    typedef typename node<Value, Box, Tag>::type node;
    node * n = new node(in);
    return n;
}

// default node

template <typename Value, typename Box, typename Tag>
void delete_node(
    boost::variant<
        leaf_def<Value, Box, Tag>,
        internal_node_def<Value, Box, Tag>
    > * n)
{
    delete n;
}

}} // namespace detail::rtree

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_HPP
