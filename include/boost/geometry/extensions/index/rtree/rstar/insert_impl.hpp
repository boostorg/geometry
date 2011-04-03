// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R*-tree reinsert algorithm implementation
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_INSERT_IMPL_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_INSERT_IMPL_HPP

#include <algorithm>

#include <boost/geometry/strategies/strategies.hpp>
#include <boost/geometry/algorithms/centroid.hpp>
#include <boost/geometry/algorithms/distance.hpp>

#include <boost/geometry/extensions/index/algorithms/area.hpp>
#include <boost/geometry/extensions/index/algorithms/margin.hpp>
#include <boost/geometry/extensions/index/algorithms/overlap.hpp>
#include <boost/geometry/extensions/index/algorithms/union_area.hpp>

#include <boost/geometry/extensions/index/rtree/node.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/insert.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/is_leaf.hpp>

#include <boost/geometry/extensions/index/rtree/rstar/choose_next_node.hpp>
#include <boost/geometry/extensions/index/rtree/rstar/split.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace rstar {

template <typename Value, typename Translator, typename Box, typename Element>
class insert_base;

template <typename Value, typename Translator, typename Box, typename Element>
class insert_impl : public insert_base<Value, Translator, Box, Element>
{
    typedef insert_base<Value, Translator, Box, Element> base;

public:
    inline insert_impl(
        node* & root,
        Element const& el,
        size_t min_elements,
        size_t max_elements,
        Translator const& t,
        size_t level = std::numeric_limits<size_t>::max()
    )
        : base(root, el, min_elements, max_elements, t, level)
    {}

    inline void operator()(internal_node & n)
    {
        if ( m_current_level < m_level )
        {
            // next traversing step
            base::traverse(*this, n);
        }
        else
        {
            assert( m_level == m_current_level );

            // push new child node
            n.children.push_back(m_element);
        }

        if ( m_max_elems_per_node < n.children.size() )
            base::overflow_treatment(n);
    }

    inline void operator()(leaf & n)
    {
        assert(false);
    }
};

template <typename Value, typename Translator, typename Box>
class insert_impl<Value, Translator, Box, Value> : public insert_base<Value, Translator, Box, Value>
{
    typedef insert_base<Value, Translator, Box, Value> base;

public:
    inline insert_impl(
        node* & root,
        Value const& v,
        size_t min_elements,
        size_t max_elements,
        Translator const& t,
        size_t level = std::numeric_limits<size_t>::max()
    )
        : base(root, v, min_elements, max_elements, t, level)
    {}

    inline void operator()(internal_node & n)
    {
        assert(m_current_level < m_level);

        // next traversing step
        base::traverse(*this, n);
        
        if ( m_max_elems_per_node < n.children.size() )
            base::overflow_treatment(n);
    }

    inline void operator()(leaf & n)
    {
        assert( m_level == m_current_level ||
            m_level == std::numeric_limits<size_t>::max() );

        n.values.push_back(m_element);

        if ( m_max_elems_per_node < n.values.size() )
            base::overflow_treatment(n);
    }
};

template <typename Value, typename Translator, typename Box, typename Element>
class insert_base : public boost::static_visitor<>
{
protected:
    typedef typename rtree::node<Value, Box, rstar_tag>::type node;
    typedef typename rtree::internal_node<Value, Box, rstar_tag>::type internal_node;
    typedef typename rtree::leaf<Value, Box, rstar_tag>::type leaf;

    inline insert_base(
        node* & root,
        Element const& el,
        size_t min_elements,
        size_t max_elements,
        Translator const& t,
        size_t level = std::numeric_limits<size_t>::max()
    )
        : m_element(el)
        , m_tr(t)
        , m_min_elems_per_node(min_elements)
        , m_max_elems_per_node(max_elements)
        , m_reinserted_elements_count(size_t(max_elements * 0.3f))
        , m_level(level)
        , m_root_node(root)
        , m_parent(0), m_current_child_index(0), m_current_level(0)
    {}

    template <typename Derived>
    inline void traverse(Derived & d, internal_node & n)
    {
        // choose next node, where value insert traversing should go
        size_t choosen_node_index = rstar::choose_next_node<Value, Box>::
            apply(n, rtree::element_indexable(m_element, m_tr));

        // expand the node to contain value
        geometry::expand(
            n.children[choosen_node_index].first,
            rtree::element_indexable(m_element, m_tr));

        // apply traversing visitor
        traverse_apply_visitor(d, n, choosen_node_index);
    }

    template <typename Derived>
    inline void traverse_apply_visitor(Derived & d, internal_node &n, size_t choosen_node_index)
    {
        // save previous traverse inputs and set new ones
        internal_node * parent_bckup = m_parent;
        size_t current_child_index_bckup = m_current_child_index;
        size_t current_level_bckup = m_current_level;

        m_parent = &n;
        m_current_child_index = choosen_node_index;
        ++m_current_level;

        // next traversing step
        boost::apply_visitor(d, *n.children[choosen_node_index].second);

        // restore previous traverse inputs
        m_parent = parent_bckup;
        m_current_child_index = current_child_index_bckup;
        m_current_level = current_level_bckup;
    }

    // before calling overflow_treatment all nodes have aabbs expanded
    // and the number of elements in the current node is max + 1
    template <typename Node>
    inline void overflow_treatment(Node & n)
    {
        // TODO: awulkiew - replace this condition with tag dispatched template

        // first time insert
        if ( m_parent != 0 &&
             m_level == std::numeric_limits<size_t>::max() &&
             0 < m_reinserted_elements_count )
        {
            reinsert(n);
        }
        // second time insert
        else
        {
            rstar::split<Value, Translator, Box>::
                apply(n, m_parent, m_current_child_index, m_root_node, m_min_elems_per_node, m_max_elems_per_node, m_tr);
        }
    }

    template <typename Distance, typename Element>
    static inline bool distances_asc(
        std::pair<Distance, Element> const& d1,
        std::pair<Distance, Element> const& d2)
    {
        return d1.first < d2.first;
    }

    template <typename Distance, typename Element>
    static inline bool distances_dsc(
        std::pair<Distance, Element> const& d1,
        std::pair<Distance, Element> const& d2)
    {
        return d1.first > d2.first;
    }

    template <typename Node>
    inline void reinsert(Node & n)
    {
        typedef typename index::detail::rtree::elements_type<Node>::type elements_type;
        typedef typename index::detail::rtree::element_type<Node>::type element_type;
        typedef typename geometry::point_type<Box>::type point_type;
        // TODO: awulkiew - use distance_result
        typedef typename index::traits::coordinate_type<point_type>::type distance_type;
        
        assert(m_parent != 0);
        assert(0 < m_reinserted_elements_count);

        point_type node_center;
        geometry::centroid(m_parent->children[m_current_child_index].first, node_center);

        elements_type & elements = index::detail::rtree::elements_get(n);

        size_t elements_count = elements.size();
        std::vector< std::pair<distance_type, element_type> > distances(elements_count);
        for ( size_t i = 0 ; i < elements_count ; ++i )
        {
            // TODO: awulkiew - use distance_sqr
            // (use select_calculation_type if distance_sqr must be implemented in geometry::index)
            // change point type for this geometry
            point_type element_center;
            geometry::centroid( index::detail::rtree::element_indexable(
                elements[i],
                m_tr
            ), element_center);

            distances[i].first = geometry::distance(node_center, element_center);
            distances[i].second = elements[i];
        }

        // sort elements by distances from center
        std::partial_sort(
            distances.begin(),
            distances.begin() + m_reinserted_elements_count,
            distances.end(),
            distances_dsc<distance_type, element_type>);

        // copy elements which will be reinserted
        elements_type elements_to_reinsert(m_reinserted_elements_count);
        for ( size_t i = 0 ; i < m_reinserted_elements_count ; ++i )
            elements_to_reinsert[i] = distances[i].second;

        // copy elements to the current node
        elements.resize(elements_count - m_reinserted_elements_count);
        for ( size_t i = m_reinserted_elements_count ; i < elements_count ; ++i )
            elements[i - m_reinserted_elements_count] = distances[i].second;

        // calulate node's new box
        m_parent->children[m_current_child_index].first =
            elements_box<Box>(elements.begin(), elements.end(), m_tr);
        
        // reinsert children starting from the minimum distance
        for ( size_t i = m_reinserted_elements_count ; 0 < i ; --i )
        {
            insert_impl<Value, Translator, Box, element_type> insert_v(
                m_root_node, elements_to_reinsert[i - 1],
                m_min_elems_per_node, m_max_elems_per_node,
                m_tr, m_current_level);
            boost::apply_visitor(insert_v, *m_root_node);
        }
    }

    Element const& m_element;
    Translator const& m_tr;
    const size_t m_min_elems_per_node;
    const size_t m_max_elems_per_node;
    const size_t m_reinserted_elements_count;

    const size_t m_level;
    
    node* & m_root_node;

    // traversing input parameters
    internal_node *m_parent;
    size_t m_current_child_index;
    size_t m_current_level;
};

}}} // namespace detail::rtree::rstar

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_INSERT_IMPL_HPP
