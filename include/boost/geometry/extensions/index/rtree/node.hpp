// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree nodes
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_HPP

#ifndef BOOST_GEOMETRY_INDEX_USE_VARIANT_NODES

#include <boost/geometry/extensions/index/rtree/node/node_poly.hpp>

#else

#include <boost/geometry/extensions/index/rtree/node/node_variant.hpp>

#endif // BOOST_GEOMETRY_INDEX_USE_VARIANT_NODES

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree {

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

// uniform indexable getter for child node element's box and value's indexable
// value's indexable version

template <typename Value, typename Translator>
inline typename Translator::indexable_type const&
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

}} // namespace detail::rtree

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_HPP
