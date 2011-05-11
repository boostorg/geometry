// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R*-tree ChooseNextNode algorithm
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_CHOOSE_NEXT_NODE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_CHOOSE_NEXT_NODE_HPP

#include <algorithm>

#include <boost/geometry/algorithms/expand.hpp>

#include <boost/geometry/extensions/index/algorithms/area.hpp>
#include <boost/geometry/extensions/index/algorithms/overlap.hpp>
#include <boost/geometry/extensions/index/algorithms/union_area.hpp>

#include <boost/geometry/extensions/index/rtree/node.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/is_leaf.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace visitors {

namespace detail {

template <typename Value, typename Box>
class choose_next_node<Value, Box, rstar_tag>
{
    typedef typename rtree::node<Value, Box, rstar_tag>::type node;
    typedef typename rtree::internal_node<Value, Box, rstar_tag>::type internal_node;
    typedef typename rtree::leaf<Value, Box, rstar_tag>::type leaf;

    typedef typename rtree::elements_type<internal_node>::type children_type;

    typedef typename index::default_area_result<Box>::type area_type;
    typedef typename index::default_overlap_result<Box>::type overlap_type;

public:
    template <typename Indexable>
    static inline size_t apply(internal_node & n, Indexable const& indexable)
    {
        children_type & children = rtree::elements_get(n);
        assert(!children.empty());
        
        bool has_leaves = boost::apply_visitor(
            visitors::is_leaf<Value, Box, rstar_tag>(),
            *children.front().second);

        if ( has_leaves )
            return branch_impl(n, indexable);
        else
            return internal_node_impl(n, indexable);
    }

private:
    template <typename Indexable>
    static inline size_t branch_impl(internal_node & n, Indexable const& indexable)
    {
        children_type & children = rtree::elements_get(n);
        size_t children_count = children.size();

        // overlaps values of all nodes' boxes,
        // overlaps and areas of extended boxes are stored at indexes i + children_count
        std::vector<overlap_type> overlaps(children_count * 2, overlap_type(0));
        std::vector<overlap_type> areas(children_count * 2);
        // caculate overlaps and areas of all nodes' boxes
        for (size_t i = 0 ; i < children_count ; ++i )
        {
            typedef typename children_type::value_type child_type;
            child_type const& ch_i = children[i];

            Box ch_ext;
            // calculate expanded box fo node ch_i
            geometry::convert(ch_i.first, ch_ext);
            geometry::expand(ch_ext, indexable);

            areas[i] = index::area(ch_i.first);
            areas[i + children_count] = index::area(ch_ext);

            for (size_t j = i + 1 ; j < children_count ; ++j )
            {
                child_type const& ch_j = children[j];
                
                // add overlap of both boxes
                overlap_type ovl = index::overlap(ch_i.first, ch_j.first);
                overlaps[i] += ovl;
                overlaps[j] += ovl;

                // add overlap of expanded box i and box j
                overlaps[i + children_count] = index::overlap(ch_ext, ch_j.first);
                
                // add overlap of expanded box j and box i
                geometry::convert(ch_j.first, ch_ext);
                geometry::expand(ch_ext, indexable);
                overlaps[j + children_count] = index::overlap(ch_ext, ch_i.first);
            }
        }

        // choose index with smallest overlap change value, or area change or smallest area
        size_t choosen_index = 0;
        overlap_type smallest_overlap_change = std::numeric_limits<overlap_type>::max();
        area_type smallest_area_change = std::numeric_limits<area_type>::max();
        area_type smallest_area = std::numeric_limits<area_type>::max();

        for ( size_t i = 0 ; i < children_count ; ++i )
        {
            overlap_type overlap_change = overlaps[i + children_count] - overlaps[i];
            area_type area_change = areas[i + children_count] - areas[i];
            area_type area = areas[i + children_count];

            if ( overlap_change < smallest_overlap_change ||
                 ( overlap_change == smallest_overlap_change && area_change < smallest_area_change ) ||
                 ( area_change == smallest_area_change && smallest_area < area ) )
            {
                smallest_overlap_change = overlap_change;
                smallest_area_change = area_change;
                smallest_area = area;
                choosen_index = i;
            }
        }

        return choosen_index;
    }

    template <typename Indexable>
    static inline size_t internal_node_impl(internal_node & n, Indexable const& indexable)
    {
        children_type & children = rtree::elements_get(n);
        size_t children_count = children.size();

        // choose index with smallest area change or smallest area
        size_t choosen_index = 0;
        area_type smallest_area_change = std::numeric_limits<area_type>::max();
        area_type smallest_area = std::numeric_limits<area_type>::max();

        // caculate areas and areas of all nodes' boxes
        for ( size_t i = 0 ; i < children_count ; ++i )
        {
            typedef typename children_type::value_type child_type;
            child_type const& ch_i = children[i];

            Box ch_exp;
            geometry::convert(ch_i.first, ch_exp);
            geometry::expand(ch_exp, indexable);

            area_type area = index::area(ch_exp);
            area_type area_change = area - index::area(ch_i.first);
            
            if ( area_change < smallest_area_change ||
                ( area_change == smallest_area_change && smallest_area < area ) )
            {
                smallest_area_change = area_change;
                smallest_area = area;
                choosen_index = i;
            }
        }

        return choosen_index;
    }
};

} // namespace detail

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_CHOOSE_NEXT_NODE_HPP
