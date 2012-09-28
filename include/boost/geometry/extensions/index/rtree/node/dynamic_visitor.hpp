// Boost.Geometry Index
//
// R-tree nodes dynamic visitor and nodes base type
//
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_DYNAMIC_VISITOR_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_DYNAMIC_VISITOR_HPP

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree {

// visitor forward declaration
template <typename Value, typename Parameters, typename Box, typename Allocators, typename Tag, bool IsVisitableConst>
struct dynamic_visitor;

// node

template <typename Value, typename Parameters, typename Box, typename Allocators, typename Tag>
struct dynamic_node
{
    virtual ~dynamic_node() {}
    virtual void apply_visitor(dynamic_visitor<Value, Parameters, Box, Allocators, Tag, false> &) = 0;
    virtual void apply_visitor(dynamic_visitor<Value, Parameters, Box, Allocators, Tag, true> &) const = 0;
};

// nodes variants forward declarations

template <typename Value, typename Parameters, typename Box, typename Allocators, typename Tag>
struct dynamic_internal_node;

template <typename Value, typename Parameters, typename Box, typename Allocators, typename Tag>
struct dynamic_leaf;

// visitor

template <typename Value, typename Parameters, typename Box, typename Allocators, typename Tag>
struct dynamic_visitor<Value, Parameters, Box, Allocators, Tag, true>
{
    typedef dynamic_internal_node<Value, Parameters, Box, Allocators, Tag> internal_node;
    typedef dynamic_leaf<Value, Parameters, Box, Allocators, Tag> leaf;

    virtual ~dynamic_visitor() {}

    virtual void operator()(internal_node const&) = 0;
    virtual void operator()(leaf const&) = 0;
};

template <typename Value, typename Parameters, typename Box, typename Allocators, typename Tag>
struct dynamic_visitor<Value, Parameters, Box, Allocators, Tag, false>
{
    typedef dynamic_internal_node<Value, Parameters, Box, Allocators, Tag> internal_node;
    typedef dynamic_leaf<Value, Parameters, Box, Allocators, Tag> leaf;

    virtual ~dynamic_visitor() {}

    virtual void operator()(internal_node &) = 0;
    virtual void operator()(leaf &) = 0;
};

}} // namespace detail::rtree

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_DYNAMIC_VISITOR_HPP
