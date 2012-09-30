// Boost.Geometry Index
//
// R-tree deep copying visitor implementation
//
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_COPY_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_COPY_HPP

#include <boost/geometry/extensions/index/rtree/node/node.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

template <typename Value, typename NodeProxy>
class copy
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

    explicit inline copy(NodeProxy & node_proxy)
        : result(0)
        , m_node_proxy(node_proxy)
    {}

    inline void operator()(internal_node & n)
    {
        node * new_node = m_node_proxy.template create<internal_node>();

        typedef typename rtree::elements_type<internal_node>::type elements_type;
        elements_type & elements = rtree::elements(n);

        elements_type & elements_dst = rtree::elements(rtree::get<internal_node>(*new_node));

        for (typename elements_type::iterator it = elements.begin();
            it != elements.end(); ++it)
        {
            rtree::apply_visitor(*this, *it->second);

            elements_dst.push_back( std::make_pair(it->first, result) );
        }

        result = new_node;
    }

    inline void operator()(leaf & l)
    {
        node * new_node = m_node_proxy.template create<leaf>();
        
        typedef typename rtree::elements_type<leaf>::type elements_type;
        elements_type & elements = rtree::elements(l);

        elements_type & elements_dst = rtree::elements(rtree::get<leaf>(*new_node));

        for (typename elements_type::iterator it = elements.begin();
            it != elements.end(); ++it)
        {
            elements_dst.push_back(*it);
        }

        result = new_node;
    }

    node * result;

private:
    NodeProxy & m_node_proxy;
};

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_COPY_HPP
