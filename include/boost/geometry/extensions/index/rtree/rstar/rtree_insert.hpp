// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R*-tree insert algorithm implementation
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_RTREE_INSERT_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_RTREE_INSERT_HPP

#include <algorithm>

#include <boost/tuple/tuple.hpp>

#include <boost/geometry/extensions/index/algorithms/area.hpp>
#include <boost/geometry/extensions/index/algorithms/margin.hpp>
#include <boost/geometry/extensions/index/algorithms/overlap.hpp>
#include <boost/geometry/extensions/index/algorithms/union_area.hpp>

#include <boost/geometry/extensions/index/rtree/rtree_node.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/rtree_insert.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/rtree_is_leaf.hpp>

#include <boost/geometry/extensions/index/rtree/rstar/choose_next_node.hpp>
#include <boost/geometry/extensions/index/rtree/rstar/split.hpp>

namespace boost { namespace geometry { namespace index {

namespace visitors {

template <typename Value, typename Translator, typename Box>
class rtree_insert<Value, Translator, Box, rtree_rstar_tag> : public boost::static_visitor<>
{
    typedef typename index::detail::rtree_node<Value, Box, rtree_rstar_tag>::type node;
    typedef typename index::detail::rtree_internal_node<Value, Box, rtree_rstar_tag>::type internal_node;
    typedef typename index::detail::rtree_leaf<Value, Box, rtree_rstar_tag>::type leaf;

public:
    inline explicit rtree_insert(node* & root, Value const& v, size_t min_elements, size_t max_elements, Translator const& t)
        : m_value(v), m_tr(t), m_min_elems_per_node(min_elements), m_max_elems_per_node(max_elements)
        , m_root_node(root)
        , m_parent(0), m_current_child_index(0)
    {}

    inline void operator()(internal_node & n)
    {
        // save previous traverse inputs and set new ones
        internal_node * parent_bckup = m_parent;
        m_parent = &n;
        size_t current_child_index_bckup = m_current_child_index;

        // choose next node, where value insert traversing should go
        m_current_child_index =
            rtree_rstar_choose_next_node<Value, Box, rtree_rstar_chnn_min_overlap_cost>::
            apply(n, m_tr(m_value));

        // TODO: awulkiew - if reinsert is implemented this must be changed
        geometry::expand(n.children[m_current_child_index].first, m_tr(m_value));

        // next traversing step
        boost::apply_visitor(*this, *n.children[m_current_child_index].second);

        // restore previous traverse inputs
        m_parent = parent_bckup;
        m_current_child_index = current_child_index_bckup;

        if ( m_max_elems_per_node < n.children.size() )
        {
            rtree_rstar_split<Value, Translator, Box>::
                apply(n, m_parent, m_current_child_index, m_root_node, m_min_elems_per_node, m_max_elems_per_node, m_tr);
        }
    }

    inline void operator()(leaf & n)
    {
        n.values.push_back(m_value);

        if ( m_max_elems_per_node < n.values.size() )
        {
            rtree_rstar_split<Value, Translator, Box>::
                apply(n, m_parent, m_current_child_index, m_root_node, m_min_elems_per_node, m_max_elems_per_node, m_tr);
        }
    }

private:
    Value const& m_value;
    Translator const& m_tr;
    size_t m_min_elems_per_node;
    size_t m_max_elems_per_node;

    node* & m_root_node;

    // traversing input parameters
    internal_node *m_parent;
    size_t m_current_child_index;
};

} // namespace visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_RTREE_INSERT_HPP
