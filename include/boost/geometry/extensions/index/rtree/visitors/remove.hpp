// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree remove
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_REMOVE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_REMOVE_HPP

#include <boost/geometry/extensions/index/rtree/node.hpp>

#include <boost/geometry/extensions/index/rtree/visitors/is_leaf.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

// Default remove algorithm
template <typename Value, typename Translator, typename Box, typename Tag>
class remove : public boost::static_visitor<>
{
    typedef typename rtree::node<Value, Box, Tag>::type node;
    typedef typename rtree::internal_node<Value, Box, Tag>::type internal_node;
    typedef typename rtree::leaf<Value, Box, Tag>::type leaf;

public:
    inline explicit remove(node* & root, Value const& v, size_t min_elements, size_t max_elements, Translator const& t)
        : m_value(v)
        , m_tr(t)
        , m_min_elems_per_node(min_elements)
        , m_max_elems_per_node(max_elements)
        , m_root_node(root)
        , m_is_value_removed(false)
        , m_parent(0)
        , m_current_child_index(0)
        , m_current_level(0)
        , m_is_underflow(false)
    {
        // TODO
        // assert - check if Value/Box is correct
    }

    inline void operator()(internal_node & n)
    {
        typedef typename rtree::elements_type<internal_node>::type children_type;
        children_type & children = rtree::elements_get(n);

        // traverse children which boxes intersects value's box
        size_t child_node_index = 0;
        for ( ; child_node_index < children.size() ; ++child_node_index )
        {
            // TODO: awulkiew - change intersects to within

            if ( geometry::intersects(children[child_node_index].first, m_tr(m_value)) )
            {
                // next traversing step
                traverse_apply_visitor(n, child_node_index);

                if ( m_is_value_removed )
                    break;
            }
        }

        // value was found and removed
        if ( m_is_value_removed )
        {
            typedef typename rtree::elements_type<internal_node>::type elements_type;
            typedef typename elements_type::iterator element_iterator;
            elements_type & elements = rtree::elements_get(n);

            // underflow occured - child node should be removed
            if ( m_is_underflow )
            {
                element_iterator underfl_el_it = elements.begin() + child_node_index;

                // move node to the container
                m_underflowed_nodes.push_back(std::make_pair(m_current_level + 1, underfl_el_it->second));
                elements.erase(underfl_el_it);

                // calc underflow
                m_is_underflow = elements.size() < m_min_elems_per_node;
            }

            // n is not root - adjust aabb
            if ( 0 != m_parent )
            {
                // test underflow state should be ok here
                // note that there may be less than min_elems elements in root
                assert((elements.size() < m_min_elems_per_node) == m_is_underflow);

                rtree::elements_get(*m_parent)[m_current_child_index].first
                    = rtree::elements_box<Box>(elements.begin(), elements.end(), m_tr);
            }
            // n is root node
            else
            {
                // current node must be a root
                assert(&n == boost::get<internal_node>(m_root_node));

                // value not found
                assert(m_is_value_removed);

                // reinsert elements from removed nodes
                // begin with levels closer to the root
                for ( typename std::vector< std::pair<size_t, node*> >::reverse_iterator it = m_underflowed_nodes.rbegin();
                        it != m_underflowed_nodes.rend() ; ++it )
                {
                    if ( boost::apply_visitor(is_leaf<Value, Box, Tag>(), *it->second) )
                        reinsert_elements(boost::get<leaf>(*it->second), it->first);
                    else
                        reinsert_elements(boost::get<internal_node>(*it->second), it->first);
                }

                // shorten the tree
                if ( rtree::elements_get(n).size() == 1 )
                {
                    m_root_node = rtree::elements_get(n)[0].second;
                }
            }
        }
    }

    inline void operator()(leaf & n)
    {
        typedef typename rtree::elements_type<leaf>::type elements_type;
        elements_type & elements = rtree::elements_get(n);

        // find value and remove it
        for ( typename elements_type::iterator it = elements.begin() ; it != elements.end() ; ++it )
        {
            if ( m_tr.equals(*it, m_value) )
            {
                elements.erase(it);
                m_is_value_removed = true;
                break;
            }
        }

        // if value was removed
        if ( m_is_value_removed )
        {
            // calc underflow
            m_is_underflow = elements.size() < m_min_elems_per_node;

            // n is not root - adjust aabb
            if ( 0 != m_parent )
            {
                rtree::elements_get(*m_parent)[m_current_child_index].first
                    = rtree::elements_box<Box>(elements.begin(), elements.end(), m_tr);
            }
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
        boost::apply_visitor(*this, *rtree::elements_get(n)[choosen_node_index].second);

        // restore previous traverse inputs
        m_parent = parent_bckup;
        m_current_child_index = current_child_index_bckup;
        m_current_level = current_level_bckup;
    }

    template <typename Node>
    void reinsert_elements(Node &n, size_t level)
    {
        typedef typename rtree::elements_type<Node>::type elements_type;
        elements_type & elements = rtree::elements_get(n);
        for ( typename elements_type::iterator it = elements.begin();
            it != elements.end() ; ++it )
        {
            visitors::insert<typename elements_type::value_type, Value, Translator, Box, Tag> insert_v(
                m_root_node,
                *it,
                m_min_elems_per_node,
                m_max_elems_per_node,
                m_tr,
                level);

            boost::apply_visitor(insert_v, *m_root_node);
        }
    }

    Value const& m_value;
    Translator const& m_tr;
    const size_t m_min_elems_per_node;
    const size_t m_max_elems_per_node;

    node* & m_root_node;
    bool m_is_value_removed;
    std::vector< std::pair<size_t, node*> > m_underflowed_nodes;

    // traversing input parameters
    internal_node *m_parent;
    size_t m_current_child_index;
    size_t m_current_level;

    // traversing output parameters
    bool m_is_underflow;
};

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_REMOVE_HPP
