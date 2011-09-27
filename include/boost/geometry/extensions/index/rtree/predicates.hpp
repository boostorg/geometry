// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - Spatial index query predicates
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_PREDICATES_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_PREDICATES_HPP

#include <boost/geometry/extensions/index/predicates.hpp>
#include <boost/geometry/extensions/index/rtree/tags.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail {

//template <typename Geometry>
//struct predicate_check<Geometry, rtree::node_predicates_tag>
//{
//    template <typename Box>
//    static inline bool apply(Geometry const& g, Box const& i)
//    {
//        return geometry::intersects(i, g);
//    }
//};

template <typename Geometry>
struct predicate_check<covered_by<Geometry>, rtree::node_tag>
{
    template <typename Box>
    static bool apply(covered_by<Geometry> const& p, Box const& i)
    {
        return geometry::intersects(i, p.geometry);
    }
};

//template <typename Geometry>
//struct predicate_check<intersects<Geometry>, rtree::node_predicates_tag>
//{
//    template <typename Box>
//    static inline bool apply(intersects<Geometry> const& p, Box const& i)
//    {
//        return geometry::intersects(i, p.geometry);
//    }
//};

template <typename Geometry>
struct predicate_check<overlaps<Geometry>, rtree::node_tag>
{
    template <typename Box>
    static inline bool apply(overlaps<Geometry> const& p, Box const& i)
    {
        // TODO: awulkiew - possibly change to the version without border case
        // e.g. intersects_without_border(0,0x1,1, 1,1x2,2) should give false
        return geometry::intersects(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<within<Geometry>, rtree::node_tag>
{
    template <typename Box>
    static bool apply(within<Geometry> const& p, Box const& i)
    {
        // TODO: awulkiew - possibly change to the version without border case
        // e.g. intersects_without_border(0,0x1,1, 1,1x2,2) should give false
        return geometry::intersects(i, p.geometry);
    }
};

} // namespace detail

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_PREDICATES_HPP
