// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree insert details
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_INSERT_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_INSERT_HPP

#include <boost/geometry/extensions/index/rtree/node.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

namespace detail {

// Default choose_next_node
template <typename Value, typename Box, typename Tag>
struct choose_next_node
{
    typedef typename rtree::node<Value, Box, Tag>::type node;
    typedef typename rtree::internal_node<Value, Box, Tag>::type internal_node;
    typedef typename rtree::leaf<Value, Box, Tag>::type leaf;

    typedef typename internal_node::children_type children_type;

    typedef typename index::default_area_result<Box>::type area_type;

    template <typename Indexable>
    static inline size_t apply(internal_node & n, Indexable const& indexable)
    {
        assert(!n.children.empty());

        size_t children_count = n.children.size();

        // choose index with smallest area change or smallest area
        size_t choosen_index = 0;
        area_type smallest_area_diff = std::numeric_limits<area_type>::max();
        area_type smallest_area = std::numeric_limits<area_type>::max();

        // caculate areas and areas of all nodes' boxes
        for ( size_t i = 0 ; i < children_count ; ++i )
        {
            typedef typename children_type::value_type child_type;
            child_type const& ch_i = n.children[i];

            Box box_exp(ch_i.first);
            geometry::expand(box_exp, indexable);

            area_type area = index::area(box_exp);
            area_type area_diff = area - index::area(ch_i.first);

            if ( area_diff < smallest_area_diff ||
                ( area_diff == smallest_area_diff && area < smallest_area ) )
            {
                smallest_area_diff = area_diff;
                smallest_area = area;
                choosen_index = i;
            }
        }

        return choosen_index;
    }
};

// Not implemented here
template <typename Value, typename Translator, typename Box, typename Tag>
struct redistribute_elements;

// Default split algorithm
template <typename Value, typename Translator, typename Box, typename Tag>
class split
{
    typedef typename rtree::node<Value, Box, Tag>::type node;
    typedef typename rtree::internal_node<Value, Box, Tag>::type internal_node;
    typedef typename rtree::leaf<Value, Box, Tag>::type leaf;

    static const size_t dimension = index::traits::dimension<Box>::value;

public:
    template <typename Node>
    static inline void apply(
        Node & n,
        internal_node *parent,
        size_t current_child_index,
        node *& root,
        size_t min_elems,
        size_t max_elems,
        Translator const& tr)
    {
        node * second_node = rtree::create_node(Node());

        // redistribute elements
        Box box1, box2;
        redistribute_elements<Value, Translator, Box, Tag>::
            apply(n, boost::get<Node>(*second_node), box1, box2, min_elems, max_elems, tr);

        // node is not the root
        if ( parent != 0 )
        {
            // update old node's box
            parent->children[current_child_index].first = box1;
            // add new node to the parent's children
            parent->children.push_back(std::make_pair(box2, second_node));
        }
        // node is the root
        else
        {
            assert(&n == boost::get<Node>(root));

            // create new root and add nodes
            node * new_root = rtree::create_node(internal_node());

            boost::get<internal_node>(*new_root).children.push_back(std::make_pair(box1, root));
            boost::get<internal_node>(*new_root).children.push_back(std::make_pair(box2, second_node));

            root = new_root;
        }
    }
};

// Default overflow treatment algorithm
template <typename Value, typename Translator, typename Box, typename Tag>
struct overflow_treatment
{
    typedef typename rtree::node<Value, Box, Tag>::type node;
    typedef typename rtree::internal_node<Value, Box, Tag>::type internal_node;
    typedef typename rtree::leaf<Value, Box, Tag>::type leaf;

public:
    template <typename Node>
    static inline void apply(
        Node & n,
        internal_node *parent,
        size_t current_child_index,
        node *& root,
        size_t min_elems,
        size_t max_elems,
        Translator const& tr)
    {
        split<Value, Translator, Box, Tag>::apply(n, parent, current_child_index, root, min_elems, max_elems, tr);
    }
};

} // namespace detail

// Default insert algorithm
template <typename Value, typename Translator, typename Box, typename Tag>
class insert : public boost::static_visitor<>
{
    typedef typename rtree::node<Value, Box, Tag>::type node;
    typedef typename rtree::internal_node<Value, Box, Tag>::type internal_node;
    typedef typename rtree::leaf<Value, Box, Tag>::type leaf;

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
        size_t choosen_node_index = detail::choose_next_node<Value, Box, Tag>::
            apply(n, rtree::element_indexable(m_value, m_tr));

        // expand the node to contain value
        geometry::expand(n.children[choosen_node_index].first, m_tr(m_value));

        // next traversing step
        traverse_apply_visitor(n, choosen_node_index);

        // handle overflow
        if ( m_max_elems_per_node < n.children.size() )
        {
            detail::overflow_treatment<Value, Translator, Box, Tag>::
                apply(n, m_parent, m_current_child_index, m_root_node, m_min_elems_per_node, m_max_elems_per_node, m_tr);
        }
    }

    inline void operator()(leaf & n)
    {
        // push value
        n.values.push_back(m_value);

        // handle overflow
        if ( m_max_elems_per_node < n.values.size() )
        {
            detail::overflow_treatment<Value, Translator, Box, Tag>::
                apply(n, m_parent, m_current_child_index, m_root_node, m_min_elems_per_node, m_max_elems_per_node, m_tr);
        }
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

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_INSERT_HPP
