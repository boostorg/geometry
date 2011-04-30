// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.Index - R-tree ChooseNextNode algorithm - per traverse level ChooseSubtree
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_LINEAR_LINEAR_CHOOSE_NEXT_NODE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_LINEAR_LINEAR_CHOOSE_NEXT_NODE_HPP

#include <algorithm>

#include <boost/geometry/algorithms/expand.hpp>

#include <boost/geometry/extensions/index/algorithms/area.hpp>
#include <boost/geometry/extensions/index/algorithms/overlap.hpp>
#include <boost/geometry/extensions/index/algorithms/union_area.hpp>

#include <boost/geometry/extensions/index/rtree/node.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/is_leaf.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace linear {

template <typename Value, typename Box, typename Tag>
struct choose_next_node
{
    typedef typename rtree::node<Value, Box, Tag>::type node;
    typedef typename rtree::internal_node<Value, Box, Tag>::type internal_node;
    typedef typename rtree::leaf<Value, Box, Tag>::type leaf;

    typedef typename internal_node::children_type children_type;

    typedef typename index::default_area_result<Box>::type area_type;

    template <typename Indexable>
    static inline size_t apply(internal_node & n, Indexable const& indexable)
    {
        assert(!n.children.empty());
        
        size_t children_count = n.children.size();

        // choose index with smallest area change or smallest area
        size_t choosen_index = 0;
        area_type smallest_area_diff = std::numeric_limits<area_type>::max();
        area_type smallest_area = std::numeric_limits<area_type>::max();

        // caculate areas and areas of all nodes' boxes
        for ( size_t i = 0 ; i < children_count ; ++i )
        {
            typedef typename children_type::value_type child_type;
            child_type const& ch_i = n.children[i];

            Box box_exp(ch_i.first);
            geometry::expand(box_exp, indexable);

            area_type area = index::area(box_exp);
            area_type area_diff = area - index::area(ch_i.first);

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

}}} // namespace detail::rtree:linear

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_LINEAR_LINEAR_CHOOSE_NEXT_NODE_HPP
