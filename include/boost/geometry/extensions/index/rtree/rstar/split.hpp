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

#include <boost/geometry/extensions/index/rtree/node.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/insert.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/is_leaf.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace rstar {

// elements less

template <typename Elements, typename Translator, size_t AxisIndex, size_t Corner>
class elements_less
{
    typedef typename Elements::value_type element_type;
public:
    inline elements_less(Translator const& tr)
        : m_tr(tr)
    {}

    inline bool operator()(element_type const& e1, element_type const e2) const
    {
        return
            index::get<Corner, AxisIndex>(
                rtree::element_indexable(e1, m_tr)
            )
            <
            index::get<Corner, AxisIndex>(
                rtree::element_indexable(e2, m_tr)
            );
    }

private:
    Translator const& m_tr;
};

// rstar split axis data

template <typename Box>
struct split_axis_data
{
    typedef typename default_margin_result<Box>::type margin_type;
    typedef typename default_overlap_result<Box>::type overlap_type;
    typedef typename default_area_result<Box>::type area_type;

    inline split_axis_data()
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
class split_update_axis_data_for_corner
{
    typedef typename split_axis_data<Box>::margin_type margin_type;
    typedef typename split_axis_data<Box>::overlap_type overlap_type;
    typedef typename split_axis_data<Box>::area_type area_type;

    BOOST_STATIC_ASSERT(Corner < 2);

public:
    template <typename Elements>
    static inline void apply(
        split_axis_data<Box> & sad,
        Elements const& sorted_elements,
        size_t min_elems,
        size_t max_elems,
        Translator const& tr)
    {
        size_t median_index_last = max_elems - min_elems + 2;
        for ( size_t median_index = min_elems ; median_index < median_index_last ; ++median_index )
        {
            Box left_box = rtree::elements_box<Box>(sorted_elements.begin(), sorted_elements.begin() + median_index, tr);
            Box right_box = rtree::elements_box<Box>(sorted_elements.begin() + median_index, sorted_elements.end(), tr);

            margin_type margin = index::margin(left_box) + index::margin(right_box);
            overlap_type overlap = index::overlap(left_box, right_box);
            area_type area = index::area(left_box) + index::area(right_box);

            sad.update(Corner, median_index, left_box, right_box, margin, overlap, area);
        }
    }
};

// split data

template <typename Elements, typename Box>
struct split_data
{
    typedef typename default_margin_result<Box>::type margin_type;
    typedef typename default_overlap_result<Box>::type overlap_type;
    typedef typename default_area_result<Box>::type area_type;

    inline split_data()
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
class split_update_data_for_axis_and_corner
{
    typedef typename split_axis_data<Box>::margin_type margin_type;
    typedef typename split_axis_data<Box>::overlap_type overlap_type;
    typedef typename split_axis_data<Box>::area_type area_type;

public:
    template <typename Elements>
    static inline void apply(
        split_data<Elements, Box> & split_data,
        Elements & elements,
        size_t min_elems,
        size_t max_elems,
        Translator const& tr)
    {
        split_axis_data<Box> sad;

        elements_less<Elements, Translator, AxisIndex, Corner> less_min(tr);
        std::sort(elements.begin(), elements.end(), less_min);

        split_update_axis_data_for_corner<Value, Translator, Box, Corner>::
            apply(sad, elements, min_elems, max_elems, tr);

        split_data.update(
            AxisIndex,
            sad.choosen_corner,
            sad.choosen_median_index,
            sad.choosen_left_box,
            sad.choosen_right_box,
            sad.margins_sum,
            elements);
    }
};

// for each dimension and corner update split data

template <typename Value, typename Translator, typename Box, size_t Dimension>
struct split_update_data
{
    BOOST_STATIC_ASSERT(0 < Dimension);

    template <typename Elements>
    static inline void apply(
        split_data<Elements, Box> & sd,
        Elements & elements,
        size_t min_elems,
        size_t max_elems,
        Translator const& tr)
    {
        split_update_data<Value, Translator, Box, Dimension - 1>::
            apply(sd, elements, min_elems, max_elems, tr);

        split_update_data_for_axis_and_corner<Value, Translator, Box, Dimension - 1, min_corner>::
            apply(sd, elements, min_elems, max_elems, tr);
        split_update_data_for_axis_and_corner<Value, Translator, Box, Dimension - 1, max_corner>::
            apply(sd, elements, min_elems, max_elems, tr);
    }
};

template <typename Value, typename Translator, typename Box>
struct split_update_data<Value, Translator, Box, 1>
{
    template <typename Elements>
    static inline void apply(
        split_data<Elements, Box> & sd,
        Elements & elements,
        size_t min_elems,
        size_t max_elems,
        Translator const& tr)
    {
        split_update_data_for_axis_and_corner<Value, Translator, Box, 0, min_corner>::
            apply(sd, elements, min_elems, max_elems, tr);
        split_update_data_for_axis_and_corner<Value, Translator, Box, 0, max_corner>::
            apply(sd, elements, min_elems, max_elems, tr);
    }
};

// split

template <typename Value, typename Translator, typename Box>
class split
{
    typedef typename rtree::node<Value, Box, rstar_tag>::type node;
    typedef typename rtree::internal_node<Value, Box, rstar_tag>::type internal_node;
    typedef typename rtree::leaf<Value, Box, rstar_tag>::type leaf;

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
        typedef typename rtree::elements_type<Node>::type elements_type;
        typedef typename elements_type::value_type element_type;

        elements_type & elements = rtree::elements_get(n);

        assert(elements.size() == max_elems + 1);

        // get split data
        split_data<elements_type, Box> sd;
        split_update_data<Value, Translator, Box, dimension>::
            apply(sd, elements, min_elems, max_elems, tr);
       
        assert(min_elems <= sd.choosen_median_index);
        assert(sd.choosen_median_index <= max_elems + 1 - min_elems);

        // create new node
        node * right_node = rtree::create_node(Node());
        elements_type & new_elems = rtree::elements_get(boost::get<Node>(*right_node));

        // update new node's elements
        new_elems.resize(max_elems + 1 - sd.choosen_median_index);
        std::copy(
            sd.choosen_distribution.begin() + sd.choosen_median_index,
            sd.choosen_distribution.end(),
            new_elems.begin());

        // update elements of the current node
        elements.resize(sd.choosen_median_index);
        std::copy(
            sd.choosen_distribution.begin(),
            sd.choosen_distribution.begin() + sd.choosen_median_index,
            elements.begin());
        
        // node is not the root
        if ( parent != 0 )
        {
            // update old node's box
            parent->children[current_child_index].first = sd.choosen_left_box;
            // add new node to the parent's children
            parent->children.push_back(std::make_pair(sd.choosen_right_box, right_node));
        }
        // node is the root
        else
        {
            assert(&n == boost::get<Node>(root));

            // create new root and add nodes
            node * new_root = rtree::create_node(internal_node());

            boost::get<internal_node>(*new_root).children.push_back(std::make_pair(sd.choosen_left_box, root));
            boost::get<internal_node>(*new_root).children.push_back(std::make_pair(sd.choosen_right_box, right_node));

            root = new_root;
        }
    }
};

}}} // namespace detail::rtree:rstar

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_SPLIT_HPP
