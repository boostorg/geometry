// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree insert algorithm implementation
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_LINEAR_INSERT_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_LINEAR_INSERT_HPP

#include <boost/geometry/extensions/index/algorithms/area.hpp>
#include <boost/geometry/extensions/index/algorithms/margin.hpp>
#include <boost/geometry/extensions/index/algorithms/overlap.hpp>
#include <boost/geometry/extensions/index/algorithms/union_area.hpp>

#include <boost/geometry/extensions/index/rtree/node.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/insert.hpp>

#include <boost/geometry/extensions/index/rtree/linear/choose_next_node.hpp>
#include <boost/geometry/extensions/index/rtree/linear/split.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

template <typename Value, typename Translator, typename Box>
class insert<Value, Translator, Box, linear_tag> : public boost::static_visitor<>
{
    typedef typename rtree::node<Value, Box, linear_tag>::type node;
    typedef typename rtree::internal_node<Value, Box, linear_tag>::type internal_node;
    typedef typename rtree::leaf<Value, Box, linear_tag>::type leaf;

public:
    inline explicit insert(node* & root, Value const& v, size_t min_elements, size_t max_elements, Translator const& t)
        : m_value(v), m_tr(t), m_min_elems_per_node(min_elements), m_max_elems_per_node(max_elements)
        , m_root_node(root)
        , m_parent(0), m_current_child_index(0), m_current_level(0)
    {
        // TODO
        // assert - check if Box is correct
    }

    inline void operator()(internal_node & n)
    {
        // choose next node
        size_t choosen_node_index = linear::choose_next_node<Value, Box, linear_tag>::
            apply(n, rtree::element_indexable(m_value, m_tr));

        // expand the node to contain value
        geometry::expand(n.children[choosen_node_index].first, m_tr(m_value));

        // next traversing step
        traverse_apply_visitor(n, choosen_node_index);

        // handle overflow
        if ( m_max_elems_per_node < n.children.size() )
            linear::split<Value, Translator, Box>::
                apply(n, m_parent, m_current_child_index, m_root_node, m_min_elems_per_node, m_max_elems_per_node, m_tr);
    }

    inline void operator()(leaf & n)
    {
        // push value
        n.values.push_back(m_value);

        // handle overflow
        if ( m_max_elems_per_node < n.values.size() )
            linear::split<Value, Translator, Box>::
                apply(n, m_parent, m_current_child_index, m_root_node, m_min_elems_per_node, m_max_elems_per_node, m_tr);
    }

private:
    inline void traverse_apply_visitor(internal_node &n, size_t choosen_node_index)
    {
        // save previous traverse inputs and set new ones
        internal_node * parent_bckup = m_parent;
        size_t current_child_index_bckup = m_current_child_index;
        size_t current_level_bckup = m_current_level;

        m_parent = &n;
        m_current_child_index = choosen_node_index;
        ++m_current_level;

        // next traversing step
        boost::apply_visitor(*this, *n.children[choosen_node_index].second);

        // restore previous traverse inputs
        m_parent = parent_bckup;
        m_current_child_index = current_child_index_bckup;
        m_current_level = current_level_bckup;
    }

    Value const& m_value;
    Translator const& m_tr;
    const size_t m_min_elems_per_node;
    const size_t m_max_elems_per_node;

    node* & m_root_node;
    
    // traversing input parameters
    internal_node *m_parent;
    size_t m_current_child_index;
    size_t m_current_level;
};

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_LINEAR_INSERT_HPP
