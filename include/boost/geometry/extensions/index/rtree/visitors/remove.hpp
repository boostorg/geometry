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

#include <boost/geometry/extensions/index/rtree/node/node.hpp>

#include <boost/geometry/extensions/index/rtree/visitors/is_leaf.hpp>

#include <boost/geometry/algorithms/covered_by.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

// Default remove algorithm
template <typename Value, typename Options, typename Translator, typename Box, typename Allocators>
class remove
    : public rtree::visitor<Value, typename Options::parameters_type, Box, typename Options::node_tag, false>::type
    , index::nonassignable
{
    typedef typename rtree::node<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type node;
    typedef typename rtree::internal_node<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type internal_node;
    typedef typename rtree::leaf<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type leaf;

    typedef typename Options::parameters_type parameters_type;

public:
    inline remove(node* & root,
                  size_t & leafs_level,
                  Value const& v,
                  Translator const& t,
                  Allocators & allocators)
        : m_value(v)
        , m_tr(t)
        , m_allocators(allocators)
        , m_root_node(root)
        , m_leafs_level(leafs_level)
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
        children_type & children = rtree::elements(n);

        // traverse children which boxes intersects value's box
        size_t child_node_index = 0;
        for ( ; child_node_index < children.size() ; ++child_node_index )
        {
            if ( geometry::covered_by(m_tr(m_value), children[child_node_index].first) )
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
            elements_type & elements = rtree::elements(n);

            // underflow occured - child node should be removed
            if ( m_is_underflow )
            {
                element_iterator underfl_el_it = elements.begin() + child_node_index;

                // move node to the container - store node's relative level as well
                m_underflowed_nodes.push_back(std::make_pair(m_leafs_level - m_current_level, underfl_el_it->second));
                elements.erase(underfl_el_it);

                // calc underflow
                m_is_underflow = elements.size() < parameters_type::min_elements;
            }

            // n is not root - adjust aabb
            if ( 0 != m_parent )
            {
                // underflow state should be ok here
                // note that there may be less than min_elems elements in root
                // so this condition should be checked only here
                assert((elements.size() < parameters_type::min_elements) == m_is_underflow);

                rtree::elements(*m_parent)[m_current_child_index].first
                    = rtree::elements_box<Box>(elements.begin(), elements.end(), m_tr);
            }
            // n is root node
            else
            {
                BOOST_GEOMETRY_INDEX_ASSERT(&n == rtree::get<internal_node>(m_root_node), "node must be the root");
                BOOST_GEOMETRY_INDEX_ASSERT(m_is_value_removed, "value not found");

                // reinsert elements from removed nodes
                // begin with levels closer to the root
                for ( typename std::vector< std::pair<size_t, node*> >::reverse_iterator it = m_underflowed_nodes.rbegin();
                        it != m_underflowed_nodes.rend() ; ++it )
                {
                    is_leaf<Value, Options, Box> ilv;
                    rtree::apply_visitor(ilv, *it->second);
                    if ( ilv.result )
                    {
                        reinsert_elements(rtree::get<leaf>(*it->second), it->first);

                        rtree::destroy_node<Allocators, leaf>::apply(m_allocators, it->second);
                    }
                    else
                    {
                        reinsert_elements(rtree::get<internal_node>(*it->second), it->first);

                        rtree::destroy_node<Allocators, internal_node>::apply(m_allocators, it->second);
                    }
                }

                // shorten the tree
                if ( rtree::elements(n).size() == 1 )
                {
                    node * root_to_destroy = m_root_node;
                    m_root_node = rtree::elements(n)[0].second;
                    --m_leafs_level;

                    rtree::destroy_node<Allocators, internal_node>::apply(m_allocators, root_to_destroy);
                }
            }
        }
    }

    inline void operator()(leaf & n)
    {
        typedef typename rtree::elements_type<leaf>::type elements_type;
        elements_type & elements = rtree::elements(n);

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
            m_is_underflow = elements.size() < parameters_type::min_elements;

            // n is not root - adjust aabb
            if ( 0 != m_parent )
            {
                rtree::elements(*m_parent)[m_current_child_index].first
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
        rtree::apply_visitor(*this, *rtree::elements(n)[choosen_node_index].second);

        // restore previous traverse inputs
        m_parent = parent_bckup;
        m_current_child_index = current_child_index_bckup;
        m_current_level = current_level_bckup;
    }

    template <typename Node>
    void reinsert_elements(Node &n, size_t node_relative_level)
    {
        typedef typename rtree::elements_type<Node>::type elements_type;
        elements_type & elements = rtree::elements(n);
        for ( typename elements_type::iterator it = elements.begin();
            it != elements.end() ; ++it )
        {
            visitors::insert<
                typename elements_type::value_type,
                Value,
                Options,
                Translator,
                Box,
                Allocators,
                typename Options::insert_tag
            > insert_v(
                m_root_node,
                m_leafs_level,
                *it,
                m_tr,
                m_allocators,
                node_relative_level - 1);

            rtree::apply_visitor(insert_v, *m_root_node);
        }
    }

    Value const& m_value;
    Translator const& m_tr;
    Allocators & m_allocators;

    node* & m_root_node;
    size_t & m_leafs_level;
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
