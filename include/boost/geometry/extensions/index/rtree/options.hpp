// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - rtree variants options
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_OPTIONS_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_OPTIONS_HPP

namespace boost { namespace geometry { namespace index {

// InsertTag
struct insert_tag {};
struct reinsert_tag {};

// ChooseNextNodeTag
struct choose_by_area_diff_tag {};
struct choose_by_overlap_diff_tag {};

// RedistributeTag
struct linear_tag {};
struct quadratic_tag {};
struct rstar_tag {};

// NodeTag
struct default_tag {};
struct default_variant_tag {};
struct default_static_tag {};

// TODO: awulkiew - implement those:
//if ( m_min_elems_per_node < 1 )
//	m_min_elems_per_node = 1;
//if ( m_max_elems_per_node < 2 )
//	m_max_elems_per_node = 2;

template <size_t MaxElements, size_t MinElements>
struct linear
{
	static const size_t max_elements = MaxElements;
	static const size_t min_elements = MinElements;
};

template <size_t MaxElements, size_t MinElements>
struct quadratic
{
	static const size_t max_elements = MaxElements;
	static const size_t min_elements = MinElements;
};

namespace options { namespace detail { 

template <size_t MaxElements>
struct default_rstar_reinserted_elements
{
	static const size_t value = (MaxElements * 3) / 10;
};

}} // namespace options::detail

template <size_t MaxElements,
		  size_t MinElements,
		  bool UseNearlyMinimumCost = false,
		  size_t ReinsertedElements = options::detail::default_rstar_reinserted_elements<MaxElements>::value
		  >
struct rstar
{
	static const size_t max_elements = MaxElements;
	static const size_t min_elements = MinElements;
	static const bool use_nearly_minimum_cost = UseNearlyMinimumCost;
	static const size_t reinserted_elements = ReinsertedElements;
};

namespace options {

template <typename Parameters, typename InsertTag, typename ChooseNextNodeTag, typename RedistributeTag, typename NodeTag>
struct rtree
{
	typedef Parameters parameters_type;
	typedef InsertTag insert_tag;
	typedef ChooseNextNodeTag choose_next_node_tag;
	typedef RedistributeTag redistribute_tag;
	typedef NodeTag node_tag;
};

} // namespace options

namespace detail { namespace rtree {

template <typename Tag>
struct options_type
{
	// TODO: awulkiew - use static assert
};

template <typename Parameters, typename InsertTag, typename ChooseNextNodeTag, typename RedistributeTag, typename NodeTag>
struct options_type< options::rtree<Parameters, InsertTag, ChooseNextNodeTag, RedistributeTag, NodeTag> >
{
	typedef options::rtree<
		Parameters,
		InsertTag,
		ChooseNextNodeTag,
		RedistributeTag,
		NodeTag
	> type;
};

template <size_t MaxElements, size_t MinElements>
struct options_type< linear<MaxElements, MinElements> >
{
	typedef options::rtree<
		linear<MaxElements, MinElements>,
		insert_tag,
		choose_by_area_diff_tag,
		linear_tag,
		default_tag
	> type;
};

template <size_t MaxElements, size_t MinElements>
struct options_type< quadratic<MaxElements, MinElements> >
{
	typedef options::rtree<
		quadratic<MaxElements, MinElements>,
		insert_tag,
		choose_by_area_diff_tag,
		quadratic_tag,
		default_tag
	> type;
};

template <size_t MaxElements, size_t MinElements, bool UseNearlyMinimumCost, size_t ReinsertedElements>
struct options_type< rstar<MaxElements, MinElements, UseNearlyMinimumCost, ReinsertedElements> >
{
	typedef options::rtree<
		rstar<MaxElements, MinElements, UseNearlyMinimumCost, ReinsertedElements>,
		reinsert_tag,
		choose_by_overlap_diff_tag,
		rstar_tag,
		default_tag
	> type;
};

}} // namespace detail::rtree

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_OPTIONS_HPP
