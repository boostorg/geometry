// Boost.Geometry Index
//
// R-tree destroying visitor implementation
//
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_DELETE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_DELETE_HPP

#include <boost/geometry/extensions/index/rtree/node/node.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

template <typename Value, typename NodeProxy>
class destroy
    : public rtree::visitor<
          Value,
          typename NodeProxy::parameters_type,
          typename NodeProxy::box_type,
          typename NodeProxy::allocators_type,
          typename NodeProxy::node_tag,
          false
      >::type
    , boost::noncopyable
{
public:
    typedef typename NodeProxy::node node;
    typedef typename NodeProxy::internal_node internal_node;
    typedef typename NodeProxy::leaf leaf;

    inline destroy(node * root_node, NodeProxy & node_proxy)
        : m_current_node(root_node)
        , m_node_proxy(node_proxy)
    {}

    inline void operator()(internal_node & n)
    {
        BOOST_GEOMETRY_INDEX_ASSERT(&n == rtree::get<internal_node>(m_current_node), "invalid pointers");

        node * node_to_destroy = m_current_node;

        typedef typename rtree::elements_type<internal_node>::type elements_type;
        elements_type & elements = rtree::elements(n);

        for (typename elements_type::iterator it = elements.begin();
            it != elements.end(); ++it)
        {
            m_current_node = it->second;
            rtree::apply_visitor(*this, *m_current_node);
        }

        m_node_proxy.template destroy<internal_node>(node_to_destroy);
    }

    inline void operator()(leaf & l)
    {
        BOOST_GEOMETRY_INDEX_ASSERT(&l == rtree::get<leaf>(m_current_node), "invalid pointers");

        m_node_proxy.template destroy<leaf>(m_current_node);
    }

private:
    node * m_current_node;
    NodeProxy & m_node_proxy;
};

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_DELETE_HPP
