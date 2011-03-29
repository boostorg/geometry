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

#include <boost/geometry/extensions/index/algorithms/area.hpp>
#include <boost/geometry/extensions/index/algorithms/overlap.hpp>
#include <boost/geometry/extensions/index/algorithms/union_area.hpp>

#include <boost/geometry/extensions/index/rtree/node.hpp>
#include <boost/geometry/extensions/index/rtree/visitors/is_leaf.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail { namespace rtree { namespace rstar {

template <typename Value, typename Box>
class choose_next_node
{
    typedef typename rtree::node<Value, Box, rstar_tag>::type node;
    typedef typename rtree::internal_node<Value, Box, rstar_tag>::type internal_node;
    typedef typename rtree::leaf<Value, Box, rstar_tag>::type leaf;

    typedef typename internal_node::children_type children_type;

public:
    template <typename Indexable>
    static inline size_t apply(internal_node & n, Indexable const& indexable)
    {
        assert(!n.children.empty());
        
        bool has_leaves = boost::apply_visitor(
            visitors::is_leaf<Value, Box, rstar_tag>(),
            *n.children.front().second);

        if ( !has_leaves )
            return impl<internal_node_areas>(n, indexable);
        else
            return impl<branch_areas>(n, indexable);
    }

private:
    template <typename Areas, typename Indexable>
    static inline size_t impl(internal_node & n, Indexable const& indexable)
    {
        typedef typename children_type::iterator children_iterator;

        //assert(!n.children.empty());

        children_iterator temp_it = n.children.begin();
        children_iterator child_it = temp_it;
        Areas min_areas(n.children, child_it, indexable);

        for (children_iterator it = ++temp_it;
            it != n.children.end(); ++it)
        {
            Areas areas(n.children, it, indexable);

            if ( areas < min_areas )
            {
                child_it = it;
                min_areas = areas;
            }
        }

        // TODO: awulkiew - switch to indexes in the whole class?
        return child_it - n.children.begin();
    }

    struct branch_areas
    {
        typedef typename index::area_result<Box>::type area_type;

        template <typename Indexable>
        inline branch_areas(children_type const& ch, typename children_type::iterator const& k_it, Indexable const& indexable)
        {
            overlap_area = 0;
            for (typename children_type::const_iterator it = ch.begin(); it != ch.end(); ++it)
                if ( it != k_it )
                    overlap_area += index::overlap(k_it->first, it->first);

            area = index::area(k_it->first);

            diff_area = index::union_area(k_it->first, indexable) - area;
        }

        inline bool operator<(branch_areas &a) const
        {
            return overlap_area < a.overlap_area ||
                ( overlap_area == a.overlap_area && diff_area < a.diff_area ) ||
                ( diff_area == a.diff_area && area < a.area );
        }

        area_type overlap_area;
        area_type diff_area;
        area_type area;
    };

    struct internal_node_areas
    {
        typedef typename area_result<Box>::type area_type;

        template <typename Indexable>
        inline internal_node_areas(children_type const&, typename children_type::iterator const& k_it, Indexable const& indexable)
        {
            area = index::area(k_it->first);
            diff_area = index::union_area(k_it->first, indexable) - area;
        }

        inline bool operator<(internal_node_areas &a) const
        {
            return diff_area < a.diff_area ||
                ( diff_area == a.diff_area && area < a.area );
        }

        area_type diff_area;
        area_type area;
    };
};

}}} // namespace detail::rtree:rstar

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_RSTAR_CHOOSE_NEXT_NODE_HPP
