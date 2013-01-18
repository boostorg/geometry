// Boost.Geometry Index
//
// R-tree node auto ptr
//
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_AUTO_PTR_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_AUTO_PTR_HPP

#include <boost/noncopyable.hpp>

#include <boost/geometry/extensions/index/detail/rtree/visitors/destroy.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree {

template <typename Value, typename Options, typename Translator, typename Box, typename Allocators>
class node_auto_ptr
    : boost::noncopyable
{
    typedef typename rtree::node<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type node;

public:
    node_auto_ptr(node * ptr, Allocators & allocators)
        : m_ptr(ptr)
        , m_allocators(allocators)
    {}

    ~node_auto_ptr()
    {
        reset();
    }

    void reset(node * ptr = 0)
    {
        if ( m_ptr )
        {
            detail::rtree::visitors::destroy<Value, Options, Translator, Box, Allocators> del_v(m_ptr, m_allocators);
            detail::rtree::apply_visitor(del_v, *m_ptr);
        }
        m_ptr = ptr;
    }

    void release()
    {
        m_ptr = 0;
    }

    node * get() const
    {
        return m_ptr;
    }

    node & operator*() const
    {
        return *m_ptr;
    }

    node * operator->() const
    {
        return m_ptr;
    }

private:
    node * m_ptr;
    Allocators & m_allocators;
};

}} // namespace detail::rtree

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_NODE_NODE_AUTO_PTR_HPP
