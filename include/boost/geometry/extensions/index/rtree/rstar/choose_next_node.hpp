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

    typedef typename internal_node::children_type children_type;

    typedef typename index::default_area_result<Box>::type area_type;
    typedef typename index::default_overlap_result<Box>::type overlap_type;

public:
    template <typename Indexable>
    static inline size_t apply(internal_node & n, Indexable const& indexable)
    {
        assert(!n.children.empty());
        
        bool has_leaves = boost::apply_visitor(
            visitors::is_leaf<Value, Box, rstar_tag>(),
            *n.children.front().second);

        if ( has_leaves )
            return branch_impl(n, indexable);
            //return impl<branch_areas>(n, indexable);
        else
            return internal_node_impl(n, indexable);
            //return impl<internal_node_areas>(n, indexable);
    }

private:
    template <typename Indexable>
    static inline size_t branch_impl(internal_node & n, Indexable const& indexable)
    {
        size_t children_count = n.children.size();
        // overlaps values of all nodes' boxes,
        // overlaps and areas of extended boxes are stored at indexes i + children_count
        std::vector<overlap_type> overlaps(children_count * 2, overlap_type(0));
        std::vector<overlap_type> areas(children_count * 2);
        // caculate overlaps and areas of all nodes' boxes
        for (size_t i = 0 ; i < children_count ; ++i )
        {
            typedef typename children_type::value_type child_type;
            child_type const& ch_i = n.children[i];

            Box ch_ext;
            // calculate expanded box fo node ch_i
            geometry::convert(ch_i.first, ch_ext);
            geometry::expand(ch_ext, indexable);

            areas[i] = index::area(ch_i.first);
            areas[i + children_count] = index::area(ch_ext);

            for (size_t j = i + 1 ; j < children_count ; ++j )
            {
                child_type const& ch_j = n.children[j];
                
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
        size_t children_count = n.children.size();

        // choose index with smallest area change or smallest area
        size_t choosen_index = 0;
        area_type smallest_area_change = std::numeric_limits<area_type>::max();
        area_type smallest_area = std::numeric_limits<area_type>::max();

        // caculate areas and areas of all nodes' boxes
        for ( size_t i = 0 ; i < children_count ; ++i )
        {
            typedef typename children_type::value_type child_type;
            child_type const& ch_i = n.children[i];

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

    //template <typename Areas, typename Indexable>
    //static inline size_t impl(internal_node & n, Indexable const& indexable)
    //{
    //    typedef typename children_type::iterator children_iterator;

    //    //assert(!n.children.empty());

    //    children_iterator temp_it = n.children.begin();
    //    children_iterator child_it = temp_it;
    //    Areas min_areas(n.children, child_it, indexable);

    //    for (children_iterator it = ++temp_it;
    //        it != n.children.end(); ++it)
    //    {
    //        Areas areas(n.children, it, indexable);

    //        if ( areas < min_areas )
    //        {
    //            child_it = it;
    //            min_areas = areas;
    //        }
    //    }

    //    return child_it - n.children.begin();
    //}

    //struct branch_areas
    //{
    //    typedef typename index::area_result<Box>::type area_type;

    //    template <typename Indexable>
    //    inline branch_areas(children_type const& ch, typename children_type::iterator const& k_it, Indexable const& indexable)
    //    {
    //        overlap_area = 0;
    //        for (typename children_type::const_iterator it = ch.begin(); it != ch.end(); ++it)
    //            if ( it != k_it )
    //                overlap_area += index::overlap(k_it->first, it->first);

    //        area = index::area(k_it->first);

    //        diff_area = index::union_area(k_it->first, indexable) - area;
    //    }

    //    inline bool operator<(branch_areas &a) const
    //    {
    //        return overlap_area < a.overlap_area ||
    //            ( overlap_area == a.overlap_area && diff_area < a.diff_area ) ||
    //            ( diff_area == a.diff_area && area < a.area );
    //    }

    //    area_type overlap_area;
    //    area_type diff_area;
    //    area_type area;
    //};

    //struct internal_node_areas
    //{
    //    typedef typename area_result<Box>::type area_type;

    //    template <typename Indexable>
    //    inline internal_node_areas(children_type const&, typename children_type::iterator const& k_it, Indexable const& indexable)
    //    {
    //        area = index::area(k_it->first);
    //        diff_area = index::union_area(k_it->first, indexable) - area;
    //    }

    //    inline bool operator<(internal_node_areas &a) const
    //    {
    //        return diff_area < a.diff_area ||
    //            ( diff_area == a.diff_area && area < a.area );
    //    }

    //    area_type diff_area;
    //    area_type area;
    //};
};

} // namespace detail

}}} // namespace detail::rtree::visitors

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_CHOOSE_NEXT_NODE_HPP
