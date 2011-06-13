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

#include <boost/geometry/extensions/index/algorithms/area.hpp>

#include <boost/geometry/extensions/index/rtree/node.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

namespace detail {

// Default choose_next_node
template <typename Value, typename Options, typename Box, typename ChooseNextNodeTag>
struct choose_next_node;

template <typename Value, typename Options, typename Box>
struct choose_next_node<Value, Options, Box, choose_by_area_diff_tag>
{
    typedef typename rtree::node<Value, Box, typename Options::node_tag>::type node;
    typedef typename rtree::internal_node<Value, Box, typename Options::node_tag>::type internal_node;
    typedef typename rtree::leaf<Value, Box, typename Options::node_tag>::type leaf;

    typedef typename rtree::elements_type<internal_node>::type children_type;

    typedef typename index::default_area_result<Box>::type area_type;

    template <typename Indexable>
    static inline size_t apply(internal_node & n, Indexable const& indexable, size_t /*node_relative_level*/)
    {
        children_type & children = rtree::elements(n);

        assert(!children.empty());

        size_t children_count = children.size();

        // choose index with smallest area change or smallest area
        size_t choosen_index = 0;
        area_type smallest_area_diff = std::numeric_limits<area_type>::max();
        area_type smallest_area = std::numeric_limits<area_type>::max();

        // caculate areas and areas of all nodes' boxes
        for ( size_t i = 0 ; i < children_count ; ++i )
        {
            typedef typename children_type::value_type child_type;
            child_type const& ch_i = children[i];

            // expanded child node's box
            Box box_exp(ch_i.first);
            geometry::expand(box_exp, indexable);

            // areas difference
            area_type area = index::area(box_exp);
            area_type area_diff = area - index::area(ch_i.first);

            // update the result
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
template <typename Value, typename Options, typename Translator, typename Box, typename RedistributeTag>
struct redistribute_elements;

// Default insert visitor
template <typename Element, typename Value, typename Options, typename Translator, typename Box>
class insert : public rtree::visitor<Value, Box, typename Options::node_tag, false>::type
{
protected:
    typedef typename rtree::node<Value, Box, typename Options::node_tag>::type node;
    typedef typename rtree::internal_node<Value, Box, typename Options::node_tag>::type internal_node;
    typedef typename rtree::leaf<Value, Box, typename Options::node_tag>::type leaf;

    inline insert(node* & root,
                  size_t & leafs_level,
                  Element const& element,
                  size_t min_elements,
                  size_t max_elements,
                  Translator const& t,
                  size_t relative_level = 0
    )
        : m_element(element)
        , m_tr(t)
        , m_min_elems_per_node(min_elements)
        , m_max_elems_per_node(max_elements)
		, m_relative_level(relative_level)
        , m_level(leafs_level - relative_level)
        , m_root_node(root)
        , m_leafs_level(leafs_level)
        , m_parent(0)
        , m_current_child_index(0)
        , m_current_level(0)
    {
		assert(m_relative_level <= leafs_level);
		assert(m_level <= m_leafs_level);
        // TODO
        // assert - check if Box is correct
    }

    template <typename Visitor>
    inline void traverse(Visitor & visitor, internal_node & n)
    {
        // choose next node
        size_t choosen_node_index = detail::choose_next_node<Value, Options, Box, typename Options::choose_next_node_tag>::
            apply(n, rtree::element_indexable(m_element, m_tr), m_leafs_level - m_current_level);

        // expand the node to contain value
        geometry::expand(
            rtree::elements(n)[choosen_node_index].first,
            rtree::element_indexable(m_element, m_tr));

        // next traversing step
        traverse_apply_visitor(visitor, n, choosen_node_index);
    }

    // TODO: awulkiew - change post_traverse name to handle_overflow or overflow_treatment?

    template <typename Node>
    inline void post_traverse(Node &n)
    {
        // if node isn't a root check if parent and current_child_index isn't corrupted
        assert(0 == m_parent || &n == rtree::get<Node>(rtree::elements(*m_parent)[m_current_child_index].second));

        // handle overflow
        if ( m_max_elems_per_node < rtree::elements(n).size() )
        {
			split(n);
        }
    }

    template <typename Visitor>
    inline void traverse_apply_visitor(Visitor & visitor, internal_node &n, size_t choosen_node_index)
    {
        // save previous traverse inputs and set new ones
        internal_node * parent_bckup = m_parent;
        size_t current_child_index_bckup = m_current_child_index;
        size_t current_level_bckup = m_current_level;

        // calculate new traverse inputs
        m_parent = &n;
        m_current_child_index = choosen_node_index;
        ++m_current_level;

        // next traversing step
        rtree::apply_visitor(visitor, *rtree::elements(n)[choosen_node_index].second);

        // restore previous traverse inputs
        m_parent = parent_bckup;
        m_current_child_index = current_child_index_bckup;
        m_current_level = current_level_bckup;
    }

	template <typename Node>
	inline void split(Node &n) const
	{
		// create additional node
		node * second_node = rtree::create_node(Node());
		Node & n2 = rtree::get<Node>(*second_node);

		// redistribute elements
		Box box1, box2;
		redistribute_elements<Value, Options, Translator, Box, typename Options::redistribute_tag>::
			apply(n, n2, box1, box2, m_min_elems_per_node, m_max_elems_per_node, m_tr);

		// check numbers of elements
		assert(m_min_elems_per_node <= rtree::elements(n).size() && rtree::elements(n).size() <= m_max_elems_per_node);
		assert(m_min_elems_per_node <= rtree::elements(n2).size() && rtree::elements(n2).size() <= m_max_elems_per_node);

		// node is not the root - just add the new node
		if ( m_parent != 0 )
		{
			// update old node's box
			rtree::elements(*m_parent)[m_current_child_index].first = box1;
			// add new node to the parent's children
			rtree::elements(*m_parent).push_back(std::make_pair(box2, second_node));
		}
		// node is the root - add level
		else
		{
			assert(&n == rtree::get<Node>(m_root_node));

			// create new root and add nodes
			node * new_root = rtree::create_node(internal_node());

			rtree::elements(rtree::get<internal_node>(*new_root)).push_back(std::make_pair(box1, m_root_node));
			rtree::elements(rtree::get<internal_node>(*new_root)).push_back(std::make_pair(box2, second_node));

			m_root_node = new_root;
			++m_leafs_level;
		}
	}

    Element const& m_element;
    Translator const& m_tr;
    const size_t m_min_elems_per_node;
    const size_t m_max_elems_per_node;
	const size_t m_relative_level;
    const size_t m_level;

    node* & m_root_node;
    size_t & m_leafs_level;

    // traversing input parameters
    internal_node *m_parent;
    size_t m_current_child_index;
    size_t m_current_level;
};

} // namespace detail

template <typename Element, typename Value, typename Options, typename Translator, typename Box, typename InsertTag>
struct insert;

// Default insert visitor
template <typename Element, typename Value, typename Options, typename Translator, typename Box>
struct insert<Element, Value, Options, Translator, Box, insert_tag>
	: public detail::insert<Element, Value, Options, Translator, Box>
{
    typedef detail::insert<Element, Value, Options, Translator, Box> base;
    typedef typename base::node node;
    typedef typename base::internal_node internal_node;
    typedef typename base::leaf leaf;

    inline insert(node* & root,
                  size_t & leafs_level,
                  Element const& element,
                  size_t min_elements,
                  size_t max_elements,
                  Translator const& tr,
                  size_t relative_level = 0
    )
        : base(root, leafs_level, element, min_elements, max_elements, tr, relative_level)
    {}

    inline void operator()(internal_node & n)
    {
        assert(base::m_current_level < base::m_leafs_level);

        if ( base::m_current_level < base::m_level )
        {
            // next traversing step
            base::traverse(*this, n);
        }
        else
        {
            assert( base::m_level == base::m_current_level );

            // push new child node
            rtree::elements(n).push_back(base::m_element);
        }

        base::post_traverse(n);
    }

    inline void operator()(leaf &)
    {
        assert(false);
    }
};

// Default insert visitor specialized for Values elements
template <typename Value, typename Options, typename Translator, typename Box>
struct insert<Value, Value, Options, Translator, Box, insert_tag>
	: public detail::insert<Value, Value, Options, Translator, Box>
{
    typedef detail::insert<Value, Value, Options, Translator, Box> base;
    typedef typename base::node node;
    typedef typename base::internal_node internal_node;
    typedef typename base::leaf leaf;

    inline insert(node* & root,
                  size_t & leafs_level,
                  Value const& v,
                  size_t min_elements,
                  size_t max_elements,
                  Translator const& t,
                  size_t relative_level = 0
    )
        : base(root, leafs_level, v, min_elements, max_elements, t, relative_level)
    {}

    inline void operator()(internal_node & n)
    {
        assert(base::m_current_level < base::m_leafs_level);
        assert(base::m_current_level < base::m_level);

        // next traversing step
        base::traverse(*this, n);

        base::post_traverse(n);
    }

    inline void operator()(leaf & n)
    {
        assert(base::m_current_level == base::m_leafs_level);
        assert( base::m_level == base::m_current_level ||
            base::m_level == std::numeric_limits<size_t>::max() );

        rtree::elements(n).push_back(base::m_element);

        base::post_traverse(n);
    }
};

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_VISITORS_INSERT_HPP
