// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R*-tree insert algorithm implementation
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_INSERT_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_INSERT_HPP

#include <boost/geometry/extensions/index/algorithms/area.hpp>
#include <boost/geometry/extensions/index/algorithms/margin.hpp>
#include <boost/geometry/extensions/index/algorithms/overlap.hpp>
#include <boost/geometry/extensions/index/algorithms/union_area.hpp>

#include <boost/geometry/extensions/index/rtree/node.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/insert.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/is_leaf.hpp>

#include <boost/geometry/extensions/index/rtree/rstar/insert_impl.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

template <typename Value, typename Translator, typename Box>
class insert<Value, Translator, Box, rstar_tag> : public boost::static_visitor<>
{
    typedef typename rtree::node<Value, Box, rstar_tag>::type node;
    typedef typename rtree::internal_node<Value, Box, rstar_tag>::type internal_node;
    typedef typename rtree::leaf<Value, Box, rstar_tag>::type leaf;

public:
    inline explicit insert(node* & root, Value const& v, size_t min_elements, size_t max_elements, Translator const& t)
        : m_root(root)
        , m_impl(root, v, min_elements, max_elements, t)
    {
        // TODO
        // assert - check if Box is correct
    }

    inline void operator()(internal_node & n)
    {
        assert(&n == &boost::get<internal_node>(*m_root));
        boost::apply_visitor(m_impl, *m_root);
    }

    inline void operator()(leaf & n)
    {
        assert(&n == &boost::get<leaf>(*m_root));
        boost::apply_visitor(m_impl, *m_root);
    }

private:
    node* & m_root;
    index::detail::rtree::rstar::insert_impl<Value, Translator, Box, Value> m_impl;
};

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_INSERT_HPP
