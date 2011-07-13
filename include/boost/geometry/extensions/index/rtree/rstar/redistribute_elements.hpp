// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R*-tree split algorithm implementation
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_REDISTRIBUTE_ELEMENTS_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_REDISTRIBUTE_ELEMENTS_HPP

#include <algorithm>

#include <boost/geometry/extensions/index/algorithms/intersection_content.hpp>
#include <boost/geometry/extensions/index/algorithms/union_content.hpp>
#include <boost/geometry/extensions/index/algorithms/margin.hpp>

#include <boost/geometry/algorithms/intersection.hpp>

#include <boost/geometry/extensions/index/rtree/node/node.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/insert.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/is_leaf.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

namespace detail {

namespace rstar {

template <typename Element, typename Translator, size_t Corner, size_t AxisIndex>
class element_axis_corner_less
{	
public:
	element_axis_corner_less(Translator const& tr)
		: m_tr(tr)
	{}

	bool operator()(Element const& e1, Element const& e2) const
	{
		return index::get<Corner, AxisIndex>(rtree::element_indexable(e1, m_tr))
			< index::get<Corner, AxisIndex>(rtree::element_indexable(e2, m_tr));
	}

private:
	Translator const& m_tr;
};

template <typename Parameters, typename Box, size_t Corner, size_t AxisIndex>
struct choose_split_axis_and_index_for_corner
{
	typedef typename index::default_margin_result<Box>::type margin_type;
	typedef typename index::default_content_result<Box>::type content_type;

	template <typename Elements, typename Translator>
	static inline void apply(Elements const& elements,
						     size_t & choosen_index,
							 margin_type & sum_of_margins,
							 content_type & smallest_overlap,
							 content_type & smallest_content,
							 Translator const& tr)
	{
		typedef typename Elements::value_type element_type;

		BOOST_GEOMETRY_INDEX_ASSERT(elements.size() == Parameters::max_elements + 1, "wrong number of elements");

		// copy elements
		boost::array<element_type, Parameters::max_elements + 1> elements_copy;
		std::copy(elements.begin(), elements.end(), elements_copy.begin());
		
		// sort elements
		element_axis_corner_less<element_type, Translator, Corner, AxisIndex> elements_less(tr);
		std::sort(elements_copy.begin(), elements_copy.end(), elements_less);

		// init outputs
		choosen_index = Parameters::min_elements;
		sum_of_margins = 0;
		smallest_overlap = std::numeric_limits<content_type>::max();
		smallest_content = std::numeric_limits<content_type>::max();

		// calculate sum of margins for all distributions
		size_t index_last = Parameters::max_elements - Parameters::min_elements + 2;
		for ( size_t i = Parameters::min_elements ; i < index_last ; ++i )
		{
			// TODO - awulkiew: may be optimized - box of group 1 may be initialized with
			// box of min_elems number of elements and expanded for each iteration by another element

			Box box1 = rtree::elements_box<Box>(elements_copy.begin(), elements_copy.begin() + i, tr);
			Box box2 = rtree::elements_box<Box>(elements_copy.begin() + i, elements_copy.end(), tr);
			
			sum_of_margins += index::margin(box1) + index::margin(box2);

			content_type ovl = index::intersection_content(box1, box2);
			content_type con = index::content(box1) + index::content(box2);

			if ( ovl < smallest_overlap || (ovl == smallest_overlap && con <= smallest_content) )
			{
				choosen_index = i;
				smallest_overlap = ovl;
				smallest_content = con;
			}				
		}
	}
};

template <typename Parameters, typename Box, size_t AxisIndex, typename ElementIndexableTag>
struct choose_split_axis_and_index_for_axis
{
	//BOOST_STATIC_ASSERT(0);
};

template <typename Parameters, typename Box, size_t AxisIndex>
struct choose_split_axis_and_index_for_axis<Parameters, Box, AxisIndex, box_tag>
{
	typedef typename index::default_margin_result<Box>::type margin_type;
	typedef typename index::default_content_result<Box>::type content_type;

	template <typename Elements, typename Translator>
	static inline void apply(Elements const& elements,
							 size_t & choosen_corner,
							 size_t & choosen_index,
							 margin_type & sum_of_margins,
							 content_type & smallest_overlap,
							 content_type & smallest_content,
							 Translator const& tr)
	{
		size_t index1 = 0;
		margin_type som1 = 0;
		content_type ovl1 = std::numeric_limits<content_type>::max();
		content_type con1 = std::numeric_limits<content_type>::max();

		choose_split_axis_and_index_for_corner<Parameters, Box, min_corner, AxisIndex>::
			apply(elements, index1,
				  som1, ovl1, con1,
				  tr);

		size_t index2 = 0;
		margin_type som2 = 0;
		content_type ovl2 = std::numeric_limits<content_type>::max();
		content_type con2 = std::numeric_limits<content_type>::max();

		choose_split_axis_and_index_for_corner<Parameters, Box, max_corner, AxisIndex>::
			apply(elements, index2,
				  som2, ovl2, con2,
				  tr);

		sum_of_margins = som1 + som2;

		if ( ovl1 < ovl2 || (ovl1 == ovl2 && con1 <= con2) )
		{
			choosen_corner = min_corner;
			choosen_index = index1;
			smallest_overlap = ovl1;
			smallest_content = con1;
		}
		else
		{
			choosen_corner = max_corner;
			choosen_index = index2;
			smallest_overlap = ovl2;
			smallest_content = con2;
		}
	}
};

template <typename Parameters, typename Box, size_t AxisIndex>
struct choose_split_axis_and_index_for_axis<Parameters, Box, AxisIndex, point_tag>
{
	typedef typename index::default_margin_result<Box>::type margin_type;
	typedef typename index::default_content_result<Box>::type content_type;

	template <typename Elements, typename Translator>
	static inline void apply(Elements const& elements,
							 size_t & choosen_corner,
							 size_t & choosen_index,
							 margin_type & sum_of_margins,
							 content_type & smallest_overlap,
							 content_type & smallest_content,
							 Translator const& tr)
	{
		choose_split_axis_and_index_for_corner<Parameters, Box, min_corner, AxisIndex>::
			apply(elements, choosen_index,
				  sum_of_margins, smallest_overlap, smallest_content,
				  tr);

		choosen_corner = min_corner;
	}
};

template <typename Parameters, typename Box, size_t Dimension>
struct choose_split_axis_and_index
{
	BOOST_STATIC_ASSERT(0 < Dimension);

	typedef typename index::default_margin_result<Box>::type margin_type;
	typedef typename index::default_content_result<Box>::type content_type;

	template <typename Elements, typename Translator>
	static inline void apply(Elements const& elements,
							 size_t & choosen_axis,
							 size_t & choosen_corner,
							 size_t & choosen_index,
							 margin_type & smallest_sum_of_margins,
							 content_type & smallest_overlap,
							 content_type & smallest_content,
							 Translator const& tr)
	{
		typedef typename rtree::element_indexable_type<typename Elements::value_type, Translator>::type element_indexable_type;

		choose_split_axis_and_index<Parameters, Box, Dimension - 1>::
			apply(elements, choosen_axis, choosen_corner, choosen_index,
				  smallest_sum_of_margins, smallest_overlap, smallest_content,
				  tr);

		margin_type sum_of_margins = 0;

		size_t corner = min_corner;
		size_t index = 0;

		content_type overlap_val = std::numeric_limits<content_type>::max();
		content_type content_val = std::numeric_limits<content_type>::max();

		choose_split_axis_and_index_for_axis<
			Parameters,
			Box,
			Dimension - 1,
			typename index::traits::tag<element_indexable_type>::type
		>::apply(elements, corner, index, sum_of_margins, overlap_val, content_val, tr);

		if ( sum_of_margins < smallest_sum_of_margins )
		{
			choosen_axis = Dimension - 1;
			choosen_corner = corner;
			choosen_index = index;
			smallest_sum_of_margins = sum_of_margins;
			smallest_overlap = overlap_val;
			smallest_content = content_val;
		}
	}
};

template <typename Parameters, typename Box>
struct choose_split_axis_and_index<Parameters, Box, 1>
{
	typedef typename index::default_margin_result<Box>::type margin_type;
	typedef typename index::default_content_result<Box>::type content_type;

	template <typename Elements, typename Translator>
	static inline void apply(Elements const& elements,
							 size_t & choosen_axis,
							 size_t & choosen_corner,
							 size_t & choosen_index,
							 margin_type & smallest_sum_of_margins,
							 content_type & smallest_overlap,
							 content_type & smallest_content,
							 Translator const& tr)
	{
		typedef typename rtree::element_indexable_type<typename Elements::value_type, Translator>::type element_indexable_type;

		choosen_axis = 0;

		choose_split_axis_and_index_for_axis<
			Parameters,
			Box,
			0,
			typename index::traits::tag<element_indexable_type>::type
		>::apply(elements, choosen_corner, choosen_index, smallest_sum_of_margins, smallest_overlap, smallest_content, tr);
	}
};

template <size_t Corner, size_t Dimension>
struct partial_sort
{
	BOOST_STATIC_ASSERT(0 < Dimension);

	template <typename Elements, typename Translator>
	static inline void apply(Elements & elements, const size_t axis, const size_t index, Translator const& tr)
	{
		if ( axis < Dimension - 1 )
		{
			partial_sort<Corner, Dimension - 1>::apply(elements, axis, index, tr);
		}
		else
		{
			BOOST_GEOMETRY_INDEX_ASSERT(axis == Dimension - 1, "unexpected axis value");

			typedef typename Elements::value_type element_type;
			element_axis_corner_less<element_type, Translator, Corner, Dimension - 1> less(tr);
			std::partial_sort(elements.begin(), elements.begin() + index, elements.end(), less);
		}
	}
};

template <size_t Corner>
struct partial_sort<Corner, 1>
{
	template <typename Elements, typename Translator>
	static inline void apply(Elements & elements, const size_t axis, const size_t index, Translator const& tr)
	{
		BOOST_GEOMETRY_INDEX_ASSERT(axis == 0, "unexpected axis value");

		typedef typename Elements::value_type element_type;
		element_axis_corner_less<element_type, Translator, Corner, 0> less(tr);
		std::partial_sort(elements.begin(), elements.begin() + index, elements.end(), less);
	}
};

} // namespace rstar

template <typename Value, typename Options, typename Translator, typename Box>
struct redistribute_elements<Value, Options, Translator, Box, rstar_tag>
{
    typedef typename rtree::node<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type node;
    typedef typename rtree::internal_node<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type internal_node;
    typedef typename rtree::leaf<Value, typename Options::parameters_type, Box, typename Options::node_tag>::type leaf;

	typedef typename Options::parameters_type parameters_type;

	static const size_t dimension = index::traits::dimension<Box>::value;

	typedef typename index::default_margin_result<Box>::type margin_type;
    typedef typename index::default_content_result<Box>::type content_type;

    template <typename Node>
    static inline void apply(
        Node & n,
        Node & second_node,
        Box & box1,
        Box & box2,
        Translator const& tr)
    {
		typedef typename rtree::elements_type<Node>::type elements_type;
		
        elements_type & elements1 = rtree::elements(n);
		elements_type & elements2 = rtree::elements(second_node);

		size_t split_axis = 0;
		size_t split_corner = 0;
		size_t split_index = parameters_type::min_elements;
		margin_type smallest_sum_of_margins = std::numeric_limits<margin_type>::max();
		content_type smallest_overlap = std::numeric_limits<content_type>::max();
		content_type smallest_content = std::numeric_limits<content_type>::max();

		rstar::choose_split_axis_and_index<typename Options::parameters_type, Box, index::traits::dimension<Box>::value>::
			apply(elements1,
				  split_axis, split_corner, split_index,
				  smallest_sum_of_margins, smallest_overlap, smallest_content,
				  tr);

		// TODO: awulkiew - get rid of following static_casts?

		BOOST_GEOMETRY_INDEX_ASSERT(split_axis < index::traits::dimension<Box>::value, "unexpected value");
		BOOST_GEOMETRY_INDEX_ASSERT(split_corner == static_cast<size_t>(min_corner) || split_corner == static_cast<size_t>(max_corner), "unexpected value");
		BOOST_GEOMETRY_INDEX_ASSERT(parameters_type::min_elements <= split_index && split_index <= parameters_type::max_elements - parameters_type::min_elements + 1, "unexpected value");
		
		// TODO: awulkiew - check if std::partial_sort produces the same result as std::sort
		if ( split_corner == static_cast<size_t>(min_corner) )
			rstar::partial_sort<min_corner, dimension>::apply(elements1, split_axis, split_index, tr);
		else
			rstar::partial_sort<max_corner, dimension>::apply(elements1, split_axis, split_index, tr);

		// copy elements to node 2 and remove from node 1
		elements2.resize(parameters_type::max_elements + 1 - split_index);
		std::copy(elements1.begin() + split_index, elements1.end(), elements2.begin());
		elements1.resize(split_index);

		// calculate boxes
		box1 = rtree::elements_box<Box>(elements1.begin(), elements1.end(), tr);
		box2 = rtree::elements_box<Box>(elements2.begin(), elements2.end(), tr);
    }
};

} // namespace detail

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_REDISTRIBUTE_ELEMENTS_HPP
