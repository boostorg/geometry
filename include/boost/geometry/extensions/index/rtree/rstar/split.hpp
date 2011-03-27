// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R*-tree split algorithm implementation
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_SPLIT_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_SPLIT_HPP

#include <algorithm>

#include <boost/tuple/tuple.hpp>

#include <boost/geometry/extensions/index/algorithms/area.hpp>
#include <boost/geometry/extensions/index/algorithms/margin.hpp>
#include <boost/geometry/extensions/index/algorithms/overlap.hpp>
#include <boost/geometry/extensions/index/algorithms/union_area.hpp>

#include <boost/geometry/extensions/index/rtree/rtree_node.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/rtree_insert.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/rtree_is_leaf.hpp>

namespace boost { namespace geometry { namespace index {

namespace visitors {

// elements less

template <typename Elements, typename Translator, size_t AxisIndex, size_t Corner>
class rtree_rstar_elements_less
{
    typedef typename Elements::value_type element_type;
public:
    inline rtree_rstar_elements_less(Translator const& tr)
        : m_tr(tr)
    {}

    inline bool operator()(element_type const& e1, element_type const e2) const
    {
        return
            index::detail::get<Corner, AxisIndex>(
                index::detail::rtree_element_indexable(e1, m_tr)
            )
            <
            index::detail::get<Corner, AxisIndex>(
                index::detail::rtree_element_indexable(e2, m_tr)
            );
    }

private:
    Translator const& m_tr;
};

// rstar split axis data

template <typename Box>
struct rtree_rstar_split_axis_data
{
    typedef typename margin_result<Box>::type margin_type;
    typedef typename overlap_result<Box>::type overlap_type;
    typedef typename area_result<Box>::type area_type;

    inline rtree_rstar_split_axis_data()
        : margins_sum(0)
        , smallest_overlap(std::numeric_limits<overlap_type>::max())
        , smallest_area(std::numeric_limits<area_type>::max())
    {}

    inline void update(
        size_t corner,
        size_t median_index,
        Box const& left_box,
        Box const& right_box,
        margin_type const& margin,
        overlap_type const& overlap,
        area_type const& area)
    {
        margins_sum += margin;

        if ( overlap < smallest_overlap ||
            ( overlap == smallest_overlap && area < smallest_area ) )
        {
            choosen_corner = corner;
            choosen_median_index = median_index;
            choosen_left_box = left_box;
            choosen_right_box = right_box;
            smallest_overlap = overlap;
            smallest_area = area;
        }
    }

    size_t choosen_corner;
    size_t choosen_median_index;
    Box choosen_left_box;
    Box choosen_right_box;
    margin_type margins_sum;
    overlap_type smallest_overlap;
    area_type smallest_area;
};

// update axis data for given axis and corner

template <typename Value, typename Translator, typename Box, size_t Corner>
class rtree_rstar_split_update_axis_data_for_corner
{
    typedef typename rtree_rstar_split_axis_data<Box>::margin_type margin_type;
    typedef typename rtree_rstar_split_axis_data<Box>::overlap_type overlap_type;
    typedef typename rtree_rstar_split_axis_data<Box>::area_type area_type;

    BOOST_STATIC_ASSERT(Corner < 2);

public:
    template <typename Elements>
    static inline void apply(
        rtree_rstar_split_axis_data<Box> & split_axis_data,
        Elements const& sorted_elements,
        size_t min_elems,
        size_t max_elems,
        Translator const& tr)
    {
        size_t median_index_last = max_elems - min_elems + 2;
        for ( size_t median_index = min_elems ; median_index < median_index_last ; ++median_index )
        {
            Box left_box = index::detail::rtree_elements_box<Box>(sorted_elements.begin(), sorted_elements.begin() + median_index, tr);
            Box right_box = index::detail::rtree_elements_box<Box>(sorted_elements.begin() + median_index, sorted_elements.end(), tr);

            margin_type margin = index::margin(left_box) + index::margin(right_box);
            overlap_type overlap = index::overlap(left_box, right_box);
            area_type area = index::area(left_box) + index::area(right_box);

            split_axis_data.update(Corner, median_index, left_box, right_box, margin, overlap, area);
        }
    }
};

// split data

template <typename Elements, typename Box>
struct rtree_rstar_split_data
{
    typedef typename margin_result<Box>::type margin_type;
    typedef typename overlap_result<Box>::type overlap_type;
    typedef typename area_result<Box>::type area_type;

    inline rtree_rstar_split_data()
        : smallest_margins_sum(std::numeric_limits<margin_type>::max())
    {}

    inline void update(
        size_t axis,
        size_t corner,
        size_t median_index,
        Box const& left_box,
        Box const& right_box,
        margin_type const& margins_sum,
        Elements const& distribution)
    {
        if ( margins_sum < smallest_margins_sum )
        {
            choosen_axis = axis;
            choosen_corner = corner;
            choosen_median_index = median_index;
            choosen_left_box = left_box;
            choosen_right_box = right_box;
            smallest_margins_sum = margins_sum;
            choosen_distribution = distribution;
        }
    }

    size_t choosen_axis;
    size_t choosen_corner;
    size_t choosen_median_index;
    Box choosen_left_box;
    Box choosen_right_box;
    margin_type smallest_margins_sum;
    Elements choosen_distribution;
};

// update split data for given axis and corner

template <typename Value, typename Translator, typename Box, size_t AxisIndex, size_t Corner>
class rtree_rstar_split_update_data_for_axis_and_corner
{
    typedef typename rtree_rstar_split_axis_data<Box>::margin_type margin_type;
    typedef typename rtree_rstar_split_axis_data<Box>::overlap_type overlap_type;
    typedef typename rtree_rstar_split_axis_data<Box>::area_type area_type;

public:
    template <typename Elements>
    static inline void apply(
        rtree_rstar_split_data<Elements, Box> & split_data,
        Elements & elements,
        size_t min_elems,
        size_t max_elems,
        Translator const& tr)
    {
        rtree_rstar_split_axis_data<Box> split_axis_data;

        rtree_rstar_elements_less<Elements, Translator, AxisIndex, Corner> less_min(tr);
        std::sort(elements.begin(), elements.end(), less_min);

        rtree_rstar_split_update_axis_data_for_corner<Value, Translator, Box, Corner>::
            apply(split_axis_data, elements, min_elems, max_elems, tr);

        split_data.update(
            AxisIndex,
            split_axis_data.choosen_corner,
            split_axis_data.choosen_median_index,
            split_axis_data.choosen_left_box,
            split_axis_data.choosen_right_box,
            split_axis_data.margins_sum,
            elements);
    }
};

// for each dimension and corner update split data

template <typename Value, typename Translator, typename Box, size_t Dimension>
struct rtree_rstar_split_update_data
{
    BOOST_STATIC_ASSERT(0 < Dimension);

    template <typename Elements>
    static inline void apply(
        rtree_rstar_split_data<Elements, Box> & split_data,
        Elements & elements,
        size_t min_elems,
        size_t max_elems,
        Translator const& tr)
    {
        rtree_rstar_split_update_data<Value, Translator, Box, Dimension - 1>::
            apply(split_data, elements, min_elems, max_elems, tr);

        rtree_rstar_split_update_data_for_axis_and_corner<Value, Translator, Box, Dimension - 1, min_corner>::
            apply(split_data, elements, min_elems, max_elems, tr);
        rtree_rstar_split_update_data_for_axis_and_corner<Value, Translator, Box, Dimension - 1, max_corner>::
            apply(split_data, elements, min_elems, max_elems, tr);
    }
};

template <typename Value, typename Translator, typename Box>
struct rtree_rstar_split_update_data<Value, Translator, Box, 1>
{
    template <typename Elements>
    static inline void apply(
        rtree_rstar_split_data<Elements, Box> & split_data,
        Elements & elements,
        size_t min_elems,
        size_t max_elems,
        Translator const& tr)
    {
        rtree_rstar_split_update_data_for_axis_and_corner<Value, Translator, Box, 0, min_corner>::
            apply(split_data, elements, min_elems, max_elems, tr);
        rtree_rstar_split_update_data_for_axis_and_corner<Value, Translator, Box, 0, max_corner>::
            apply(split_data, elements, min_elems, max_elems, tr);
    }
};

// split

template <typename Value, typename Translator, typename Box>
class rtree_rstar_split
{
    typedef typename index::detail::rtree_node<Value, Box, rtree_rstar_tag>::type node;
    typedef typename index::detail::rtree_internal_node<Value, Box, rtree_rstar_tag>::type internal_node;
    typedef typename index::detail::rtree_leaf<Value, Box, rtree_rstar_tag>::type leaf;

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
        typedef typename index::detail::rtree_elements_type<Node>::type elements_type;
        typedef typename elements_type::value_type element_type;

        elements_type & elements = index::detail::rtree_elements_get(n);

        assert(elements.size() == max_elems + 1);

        // get split data
        rtree_rstar_split_data<elements_type, Box> split_data;
        rtree_rstar_split_update_data<Value, Translator, Box, dimension>::
            apply(split_data, elements, min_elems, max_elems, tr);
       
        assert(min_elems <= split_data.choosen_median_index);
        assert(split_data.choosen_median_index <= max_elems + 1 - min_elems);

        // create new node
        node * right_node = rtree_create_node(Node());
        elements_type & new_elems = index::detail::rtree_elements_get(boost::get<Node>(*right_node));

        // update new node's elements
        new_elems.resize(max_elems + 1 - split_data.choosen_median_index);
        std::copy(
            split_data.choosen_distribution.begin() + split_data.choosen_median_index,
            split_data.choosen_distribution.end(),
            new_elems.begin());

        // update elements of the current node
        elements.resize(split_data.choosen_median_index);
        std::copy(
            split_data.choosen_distribution.begin(),
            split_data.choosen_distribution.begin() + split_data.choosen_median_index,
            elements.begin());
        
        if ( parent != 0 )
        {
            // update old node's box
            parent->children[current_child_index].first = split_data.choosen_left_box;
            // add new node to the parent's children
            parent->children.push_back(std::make_pair(split_data.choosen_right_box, right_node));
        }
        else
        {
            assert(&n == boost::get<Node>(root));

            // create new root and add nodes
            node * new_root = rtree_create_node(internal_node());

            boost::get<internal_node>(*new_root).children.push_back(std::make_pair(split_data.choosen_left_box, root));
            boost::get<internal_node>(*new_root).children.push_back(std::make_pair(split_data.choosen_right_box, right_node));

            root = new_root;
        }
    }
};

} // namespace visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_SPLIT_HPP
