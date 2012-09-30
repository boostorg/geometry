// Boost.Geometry Index
//
// R-tree querying visitor implementation
//
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_QUERY_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_QUERY_HPP

#include <boost/geometry/extensions/index/rtree/node/node.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

template <typename Value, typename NodeProxy, typename Predicates, typename OutIter>
class query
    : public rtree::visitor<
          Value,
          typename NodeProxy::parameters_type,
          typename NodeProxy::box_type,
          typename NodeProxy::allocators_type,
          typename NodeProxy::node_tag,
          true
      >::type
    , index::nonassignable
{
    typedef typename NodeProxy::node node;
    typedef typename NodeProxy::internal_node internal_node;
    typedef typename NodeProxy::leaf leaf;

public:
    inline query(NodeProxy const& node_proxy, Predicates const& predicates, OutIter out_it)
        : out_iter(out_it)
        , found_count(0)
        , m_node_proxy(node_proxy)
        , m_predicates(predicates)
    {}

    inline void operator()(internal_node const& n)
    {
        typedef typename rtree::elements_type<internal_node>::type elements_type;
        elements_type const& elements = rtree::elements(n);

        // traverse nodes meeting predicates
        for (typename elements_type::const_iterator it = elements.begin();
            it != elements.end(); ++it)
        {
            // if node meets predicates
            // 0 - dummy value
            if ( index::predicates_check<rtree::node_tag>(m_predicates, 0, m_node_proxy.indexable(*it)) )
                rtree::apply_visitor(*this, *it->second);
        }
    }

    inline void operator()(leaf const& n)
    {
        typedef typename rtree::elements_type<leaf>::type elements_type;
        elements_type const& elements = rtree::elements(n);

        // get all values meeting predicates
        for (typename elements_type::const_iterator it = elements.begin();
            it != elements.end(); ++it)
        {
            // if value meets predicates
            if ( index::predicates_check<rtree::value_tag>(m_predicates, *it, m_node_proxy.indexable(*it)) )
            {
                out_iter = *it;
                ++out_iter;

                ++found_count;
            }
        }
    }

    OutIter out_iter;
    size_t found_count;

private:
    NodeProxy const& m_node_proxy;
    Predicates const& m_predicates;
};

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_QUERY_HPP
