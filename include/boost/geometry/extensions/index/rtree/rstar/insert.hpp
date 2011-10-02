// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R*-tree insert
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_INSERT_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_INSERT_HPP

#include <boost/geometry/extensions/index/algorithms/content.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

namespace detail {

namespace rstar {

template <typename Value, typename Options, typename Translator, typename Box>
class remove_elements_to_reinsert
{
public:
    typedef typename rtree::node<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type node;
    typedef typename rtree::internal_node<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type internal_node;
    typedef typename rtree::leaf<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type leaf;

	typedef typename Options::parameters_type parameters_type;

    template <typename Node>
    static inline void apply(typename rtree::elements_type<Node>::type & result_elements,
							 Node & n,
						 	 internal_node *parent,
							 size_t current_child_index,
							 Translator const& tr)
    {
        typedef typename rtree::elements_type<Node>::type elements_type;
        typedef typename elements_type::value_type element_type;
        typedef typename geometry::point_type<Box>::type point_type;
        // TODO: awulkiew - change second point_type to the point type of the Indexable?
        typedef typename geometry::default_distance_result<point_type>::type distance_type;

		elements_type & elements = rtree::elements(n);

		const size_t elements_count = parameters_type::max_elements + 1;
		const size_t reinserted_elements_count = parameters_type::reinserted_elements;

		BOOST_GEOMETRY_INDEX_ASSERT(parent, "node shouldn't be the root node");
		BOOST_GEOMETRY_INDEX_ASSERT(elements.size() == elements_count, "unexpected elements number");
        BOOST_GEOMETRY_INDEX_ASSERT(0 < reinserted_elements_count, "wrong value of elements to reinsert");

        // calculate current node's center
        point_type node_center;
        geometry::centroid(rtree::elements(*parent)[current_child_index].first, node_center);

        // fill the container of centers' distances of children from current node's center
		boost::array<std::pair<distance_type, element_type>, elements_count> sorted_elements;
		for ( size_t i = 0 ; i < elements_count ; ++i )
        {
            point_type element_center;
            geometry::centroid( rtree::element_indexable(elements[i], tr),
                element_center);
            sorted_elements[i].first = geometry::comparable_distance(node_center, element_center);
            sorted_elements[i].second = elements[i];
        }

        // sort elements by distances from center
        std::partial_sort(
            sorted_elements.begin(),
            sorted_elements.begin() + reinserted_elements_count,
            sorted_elements.end(),
            distances_dsc<distance_type, element_type>);

        // copy elements which will be reinserted
        result_elements.resize(reinserted_elements_count);
        for ( size_t i = 0 ; i < reinserted_elements_count ; ++i )
            result_elements[i] = sorted_elements[i].second;

        // copy remaining elements to the current node
        elements.resize(elements_count - reinserted_elements_count);
        for ( size_t i = reinserted_elements_count ; i < elements_count ; ++i )
            elements[i - reinserted_elements_count] = sorted_elements[i].second;
    }

private:
    template <typename Distance, typename El>
    static inline bool distances_asc(
        std::pair<Distance, El> const& d1,
        std::pair<Distance, El> const& d2)
    {
        return d1.first < d2.first;
    }
    
    template <typename Distance, typename El>
    static inline bool distances_dsc(
        std::pair<Distance, El> const& d1,
        std::pair<Distance, El> const& d2)
    {
        return d1.first > d2.first;
    }
};

template <size_t InsertIndex, typename Element, typename Value, typename Options, typename Box>
struct level_insert_elements_type
{
	typedef typename rtree::elements_type<
		typename rtree::internal_node<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type
	>::type type;
};

template <typename Value, typename Options, typename Box>
struct level_insert_elements_type<0, Value, Value, Options, Box>
{
	typedef typename rtree::elements_type<
		typename rtree::leaf<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type
	>::type type;
};

template <size_t InsertIndex, typename Element, typename Value, typename Options, typename Translator, typename Box, typename Allocators>
struct level_insert_base
	: public detail::insert<Element, Value, Options, Translator, Box, Allocators>
{
	typedef detail::insert<Element, Value, Options, Translator, Box, Allocators> base;
	typedef typename base::node node;
	typedef typename base::internal_node internal_node;
	typedef typename base::leaf leaf;

	typedef typename level_insert_elements_type<InsertIndex, Element, Value, Options, Box>::type elements_type;
	typedef typename Options::parameters_type parameters_type;

	inline level_insert_base(node* & root,
		 					 size_t & leafs_level,
		 					 Element const& element,
							 Translator const& tr,
                             Allocators & allocators,
							 size_t relative_level)
		: base(root, leafs_level, element, tr, allocators, relative_level)
		, result_relative_level(0)
	{}

	template <typename Node>
	inline void handle_possible_reinsert_or_split_of_root(Node &n)
	{
		BOOST_GEOMETRY_INDEX_ASSERT(result_elements.empty(), "reinsert should be handled only once for level");
		
		result_relative_level = base::m_leafs_level - base::m_current_level;

		// overflow
		if ( parameters_type::max_elements < rtree::elements(n).size() )
		{
			// node isn't root node
			if ( base::m_parent )
			{
				rstar::remove_elements_to_reinsert<Value, Options, Translator, Box>::apply(
					result_elements, n,
					base::m_parent, base::m_current_child_index,
					base::m_tr);
			}
			// node is root node
			else
			{
				BOOST_GEOMETRY_INDEX_ASSERT(&n == rtree::get<Node>(base::m_root_node), "node should be the root node");
				base::split(n);
			}
		}
	}

	template <typename Node>
	inline void handle_possible_split(Node &n) const
	{
		// overflow
		if ( parameters_type::max_elements < rtree::elements(n).size() )
		{
			base::split(n);
		}
	}

	template <typename Node>
	inline void recalculate_aabb_if_necessary(Node &n) const
	{
		if ( !result_elements.empty() && base::m_parent )
		{
			// calulate node's new box
			rtree::elements(*base::m_parent)[base::m_current_child_index].first =
				elements_box<Box>(rtree::elements(n).begin(), rtree::elements(n).end(), base::m_tr);
		}
	}

	size_t result_relative_level;
	elements_type result_elements;
};

template <size_t InsertIndex, typename Element, typename Value, typename Options, typename Translator, typename Box, typename Allocators>
struct level_insert
    : public level_insert_base<InsertIndex, Element, Value, Options, Translator, Box, Allocators>
{
	typedef level_insert_base<InsertIndex, Element, Value, Options, Translator, Box, Allocators> base;
    typedef typename base::node node;
    typedef typename base::internal_node internal_node;
    typedef typename base::leaf leaf;

    inline level_insert(node* & root,
                        size_t & leafs_level,
                        Element const& element,
                        Translator const& tr,
                        Allocators & allocators,
                        size_t relative_level)
        : base(root, leafs_level, element, tr, allocators, relative_level)
    {}

    inline void operator()(internal_node & n)
    {
		BOOST_GEOMETRY_INDEX_ASSERT(base::m_current_level < base::m_leafs_level, "unexpected level");

        if ( base::m_current_level < base::m_level )
        {
            // next traversing step
            base::traverse(*this, n);

            // further insert
            if ( 0 < InsertIndex )
            {
                BOOST_GEOMETRY_INDEX_ASSERT(0 < base::m_level, "illegal level value, level shouldn't be the root level for 0 < InsertIndex");

                if ( base::m_current_level == base::m_level - 1 )
                {
                    base::handle_possible_reinsert_or_split_of_root(n);
                }
            }
        }
        else
        {
			BOOST_GEOMETRY_INDEX_ASSERT(base::m_level == base::m_current_level, "unexpected level");

            // push new child node
            rtree::elements(n).push_back(base::m_element);

            // first insert
            if ( 0 == InsertIndex )
            {
                base::handle_possible_reinsert_or_split_of_root(n);
            }
            // not the first insert
            else
            {
                base::handle_possible_split(n);
            }
        }

        base::recalculate_aabb_if_necessary(n);
    }

    inline void operator()(leaf &)
    {
        assert(false);
    }
};

template <size_t InsertIndex, typename Value, typename Options, typename Translator, typename Box, typename Allocators>
struct level_insert<InsertIndex, Value, Value, Options, Translator, Box, Allocators>
    : public level_insert_base<InsertIndex, Value, Value, Options, Translator, Box, Allocators>
{
    typedef level_insert_base<InsertIndex, Value, Value, Options, Translator, Box, Allocators> base;
    typedef typename base::node node;
    typedef typename base::internal_node internal_node;
    typedef typename base::leaf leaf;

    inline level_insert(node* & root,
                        size_t & leafs_level,
                        Value const& v,
                        Translator const& t,
                        Allocators & allocators,
                        size_t relative_level)
        : base(root, leafs_level, v, t, allocators, relative_level)
    {}

    inline void operator()(internal_node & n)
    {
		BOOST_GEOMETRY_INDEX_ASSERT(base::m_current_level < base::m_leafs_level, "unexpected level");
		BOOST_GEOMETRY_INDEX_ASSERT(base::m_current_level < base::m_level, "unexpected level");

        // next traversing step
        base::traverse(*this, n);

		BOOST_GEOMETRY_INDEX_ASSERT(0 < base::m_level, "illegal level value, level shouldn't be the root level for 0 < InsertIndex");
        
        if ( base::m_current_level == base::m_level - 1 )
        {
            base::handle_possible_reinsert_or_split_of_root(n);
        }

        base::recalculate_aabb_if_necessary(n);
    }

    inline void operator()(leaf & n)
    {
		BOOST_GEOMETRY_INDEX_ASSERT(base::m_current_level == base::m_leafs_level, "unexpected level");
		BOOST_GEOMETRY_INDEX_ASSERT(base::m_level == base::m_current_level || base::m_level == std::numeric_limits<size_t>::max(), "unexpected level");
        
        rtree::elements(n).push_back(base::m_element);

        base::handle_possible_split(n);
    }
};

template <typename Value, typename Options, typename Translator, typename Box, typename Allocators>
struct level_insert<0, Value, Value, Options, Translator, Box, Allocators>
    : public level_insert_base<0, Value, Value, Options, Translator, Box, Allocators>
{
    typedef level_insert_base<0, Value, Value, Options, Translator, Box, Allocators> base;
    typedef typename base::node node;
    typedef typename base::internal_node internal_node;
    typedef typename base::leaf leaf;

    inline level_insert(node* & root,
                        size_t & leafs_level,
                        Value const& v,
                        Translator const& t,
                        Allocators & allocators,
                        size_t relative_level)
        : base(root, leafs_level, v, t, allocators, relative_level)
    {}

    inline void operator()(internal_node & n)
    {
		BOOST_GEOMETRY_INDEX_ASSERT(base::m_current_level < base::m_leafs_level, "unexpected level");
		BOOST_GEOMETRY_INDEX_ASSERT(base::m_current_level < base::m_level, "unexpected level");
		
        // next traversing step
        base::traverse(*this, n);

        base::recalculate_aabb_if_necessary(n);
    }

    inline void operator()(leaf & n)
    {
		BOOST_GEOMETRY_INDEX_ASSERT(base::m_current_level == base::m_leafs_level, "unexpected level");
		BOOST_GEOMETRY_INDEX_ASSERT(base::m_level == base::m_current_level || base::m_level == std::numeric_limits<size_t>::max(), "unexpected level");

        rtree::elements(n).push_back(base::m_element);

        base::handle_possible_reinsert_or_split_of_root(n);

		base::recalculate_aabb_if_necessary(n);
    }
};

} // namespace rstar

} // namespace detail

// R*-tree insert visitor
template <typename Element, typename Value, typename Options, typename Translator, typename Box, typename Allocators>
class insert<Element, Value, Options, Translator, Box, Allocators, insert_reinsert_tag>
	: public rtree::visitor<Value, typename Options::parameters_type, Box, typename Options::node_tag, false>::type
	, index::nonassignable
{
private:
	typedef typename rtree::node<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type node;
	typedef typename rtree::internal_node<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type internal_node;
	typedef typename rtree::leaf<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type leaf;

public:
	inline insert(node* & root,
				  size_t & leafs_level,
				  Element const& element,
				  Translator const& tr,
                  Allocators & allocators,
				  size_t relative_level = 0)
		: m_root(root), m_leafs_level(leafs_level), m_element(element)
		, m_tr(tr), m_relative_level(relative_level), m_allocators(allocators)
	{}

	inline void operator()(internal_node & BOOST_GEOMETRY_INDEX_ASSERT_UNUSED_PARAM(n))
	{
		BOOST_GEOMETRY_INDEX_ASSERT(&n == rtree::get<internal_node>(m_root), "current node should be the root");
		
		detail::rstar::level_insert<0, Element, Value, Options, Translator, Box, Allocators> lins_v(
			m_root, m_leafs_level, m_element, m_tr, m_allocators, m_relative_level);

		rtree::apply_visitor(lins_v, *m_root);

		if ( !lins_v.result_elements.empty() )
		{
			recursive_reinsert(lins_v.result_elements, lins_v.result_relative_level);
		}
	}

	inline void operator()(leaf & BOOST_GEOMETRY_INDEX_ASSERT_UNUSED_PARAM(n))
	{
		BOOST_GEOMETRY_INDEX_ASSERT(&n == rtree::get<leaf>(m_root), "current node should be the root");

		detail::rstar::level_insert<0, Element, Value, Options, Translator, Box, Allocators> lins_v(
			m_root, m_leafs_level, m_element, m_tr, m_allocators, m_relative_level);

		rtree::apply_visitor(lins_v, *m_root);

		// we're in the root, so root should be split and there should be no elements to reinsert
		assert(lins_v.result_elements.empty());
	}

private:
	template <typename Elements>
	inline void recursive_reinsert(Elements const& elements, size_t relative_level)
	{
		typedef typename Elements::value_type element_type;

		// reinsert children starting from the minimum distance
		for ( typename Elements::const_reverse_iterator it = elements.rbegin();
			it != elements.rend(); ++it)
		{
			detail::rstar::level_insert<1, element_type, Value, Options, Translator, Box, Allocators> lins_v(
				m_root, m_leafs_level, *it, m_tr, m_allocators, relative_level);

			rtree::apply_visitor(lins_v, *m_root);

			assert(relative_level + 1 == lins_v.result_relative_level);

			// non-root relative level
			if ( lins_v.result_relative_level < m_leafs_level && !lins_v.result_elements.empty())
			{
				recursive_reinsert(lins_v.result_elements, lins_v.result_relative_level);
			}
		}
	}

	node* & m_root;
	size_t & m_leafs_level;
	Element const& m_element;
	Translator const& m_tr;
	size_t m_relative_level;

    Allocators m_allocators;
};

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_INSERT_HPP
