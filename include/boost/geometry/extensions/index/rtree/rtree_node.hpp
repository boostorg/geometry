// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree details
//
// Copyright 2008 Federico J. Fernandez.
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_DEFAULT_NODE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_DEFAULT_NODE_HPP

#include <vector>
#include <boost/variant.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail {

template <typename Value, typename Box, typename Tag>
struct rtree_node;

// nodes default types

template <typename Value, typename Box, typename Tag>
struct rtree_internal_node_def
{
    typedef std::vector<
        std::pair<
            Box,
            typename rtree_node<Value, Box, Tag>::type *
        >
    > children_type;

    children_type children;
};

template <typename Value, typename Box, typename Tag>
struct rtree_leaf_def
{
    typedef std::vector<Value> values_type;
    values_type values;
};

// nodes traits

template <typename Value, typename Box, typename Tag>
struct rtree_node
{
    typedef boost::variant<
        rtree_leaf_def<Value, Box, Tag>,
        rtree_internal_node_def<Value, Box, Tag>
    > type;
};

template <typename Value, typename Box, typename Tag>
struct rtree_internal_node
{
    typedef rtree_internal_node_def<Value, Box, Tag> type;
};

template <typename Value, typename Box, typename Tag>
struct rtree_leaf
{
    typedef rtree_leaf_def<Value, Box, Tag> type;
};

// nodes elements extractor

template <typename Node>
struct rtree_elements_type
{
    typedef typename Node::elements_type type;
};

template <typename Value, typename Box, typename Tag>
struct rtree_elements_type< rtree_internal_node_def<Value, Box, Tag> >
{
    typedef typename rtree_internal_node_def<Value, Box, Tag>::children_type type;
};

template <typename Value, typename Box, typename Tag>
struct rtree_elements_type< rtree_leaf_def<Value, Box, Tag> >
{
    typedef typename rtree_leaf_def<Value, Box, Tag>::values_type type;
};

template <typename Node>
typename rtree_elements_type<Node>::type & get_elements(Node & n)
{
    return n.elements;
}

template <typename Value, typename Box, typename Tag>
typename rtree_internal_node_def<Value, Box, Tag>::children_type &
get_elements(rtree_internal_node_def<Value, Box, Tag> & n)
{
    return n.children;
}

template <typename Value, typename Box, typename Tag>
typename rtree_leaf_def<Value, Box, Tag>::values_type &
get_elements(rtree_leaf_def<Value, Box, Tag> & n)
{
    return n.values;
}

// uniform indexable type for child node element's box and value's indexable

template <typename Value, typename Translator>
struct rtree_element_indexable_type
{
    typedef typename Translator::indexable_type type;
};

template <typename Value, typename Box, typename Tag, typename Translator>
struct rtree_element_indexable_type<
    std::pair<
        Box,
        boost::variant<
            rtree_leaf_def<Value, Box, Tag>,
            rtree_internal_node_def<Value, Box, Tag>
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
rtree_element_indexable(
    std::pair<
        Box,
        boost::variant<
            rtree_leaf_def<Value, Box, Tag>,
            rtree_internal_node_def<Value, Box, Tag>
        > *
    > const& el,
    Translator const&)
{
    return el.first;
}

template <typename Value, typename Translator>
typename Translator::indexable_type const&
rtree_element_indexable(Value const& el, Translator const& tr)
{
    return tr(el);
};

// elements box

template <typename Box, typename FwdIter, typename Translator>
inline Box elements_box(FwdIter first, FwdIter last, Translator const& tr)
{
    assert(first != last);

    Box result;

    geometry::convert(index::detail::rtree_element_indexable(*first, tr), result);
    ++first;

    for ( ; first != last ; ++first )
        geometry::expand(result, index::detail::rtree_element_indexable(*first, tr));

    return result;
}

// create leaf node

template <typename Value, typename Box, typename Tag>
typename rtree_node<Value, Box, Tag>::type *
rtree_create_node(rtree_leaf_def<Value, Box, Tag> const& l)
{
    typedef typename rtree_node<Value, Box, Tag>::type node;
    node * n = new node();
    *n = l;
    return n;
}

// create internal node

template <typename Value, typename Box, typename Tag>
typename rtree_node<Value, Box, Tag>::type *
rtree_create_node(rtree_internal_node_def<Value, Box, Tag> const& in)
{
    typedef typename rtree_node<Value, Box, Tag>::type node;
    node * n = new node();
    *n = in;
    return n;
}

// default node

template <typename Value, typename Box, typename Tag>
void rtree_delete_node(
    boost::variant<
        rtree_leaf_def<Value, Box, Tag>,
        rtree_internal_node_def<Value, Box, Tag>
    > * n)
{
    delete n;
}

} // namespace detail

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_DEFAULT_NODE_HPP
