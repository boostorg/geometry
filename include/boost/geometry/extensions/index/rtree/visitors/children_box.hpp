// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree node children box calculating visitor
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_CHILDREN_BOX_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_CHILDREN_BOX_HPP

#include <boost/geometry/extensions/index/rtree/node/node.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

template <typename Value, typename Options, typename Translator, typename Box, typename Allocators>
class children_box
    : public rtree::visitor<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag, true>::type
    , index::nonassignable
{
    typedef typename rtree::internal_node<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type internal_node;
    typedef typename rtree::leaf<Value, typename Options::parameters_type, Box, Allocators, typename Options::node_tag>::type leaf;

public:
    inline children_box(Translator const& tr)
        : m_tr(tr)
    {}

    inline void operator()(internal_node const& n)
    {
        typedef typename rtree::elements_type<internal_node>::type elements_type;
        elements_type const& elements = rtree::elements(n);

        result = rtree::elements_box<Box>(elements.begin(), elements.end(), m_tr);
    }

    inline void operator()(leaf const& n)
    {
        typedef typename rtree::elements_type<leaf>::type elements_type;
        elements_type const& elements = rtree::elements(n);

        result = rtree::elements_box<Box>(elements.begin(), elements.end(), m_tr);
    }

    Box result;

private:
    Translator const& m_tr;
};

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_CHILDREN_BOX_HPP
