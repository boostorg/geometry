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

// TODO: awulkiew - consider removing Value from parameters
//                  then predicates_check must be implemented for nodes as well

template <typename Geometry>
struct predicate_check<Geometry, rtree::node_tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(Geometry const& g, Value const&, Indexable const& i)
    {
        return geometry::intersects(i, g);
    }
};

template <>
struct predicate_check<empty, rtree::node_tag>
{
    template <typename Geometry, typename Value, typename Indexable>
    static inline bool apply(Geometry const&, Value const&, Indexable const&)
    {
        return true;
    }
};

template <typename ValuePredicate>
struct predicate_check<value<ValuePredicate>, rtree::node_tag>
{
    template <typename Value, typename Box>
    static bool apply(value<ValuePredicate> const&, Value const&, Box const&)
    {
        return true;
    }
};

template <typename Geometry>
struct predicate_check<covered_by<Geometry>, rtree::node_tag>
{
    template <typename Value, typename Box>
    static bool apply(covered_by<Geometry> const& p, Value const&, Box const& i)
    {
        return geometry::intersects(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<disjoint<Geometry>, rtree::node_tag>
{
    template <typename Value, typename Box>
    static bool apply(disjoint<Geometry> const& p, Value const&, Box const& i)
    {
        return !geometry::covered_by(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<intersects<Geometry>, rtree::node_tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(intersects<Geometry> const& p, Value const&, Indexable const& i)
    {
        return geometry::intersects(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<overlaps<Geometry>, rtree::node_tag>
{
    template <typename Value, typename Box>
    static inline bool apply(overlaps<Geometry> const& p, Value const&, Box const& i)
    {
        // TODO: awulkiew - possibly change to the version without border case
        // e.g. intersects_without_border(0,0x1,1, 1,1x2,2) should give false
        return geometry::intersects(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<within<Geometry>, rtree::node_tag>
{
    template <typename Value, typename Box>
    static bool apply(within<Geometry> const& p, Value const&, Box const& i)
    {
        // TODO: awulkiew - possibly change to the version without border case
        // e.g. intersects_without_border(0,0x1,1, 1,1x2,2) should give false
        return geometry::intersects(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<not_covered_by<Geometry>, rtree::node_tag>
{
    template <typename Value, typename Box>
    static bool apply(not_covered_by<Geometry> const& p, Value const&, Box const& i)
    {
        return !geometry::covered_by(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<not_disjoint<Geometry>, rtree::node_tag>
{
    template <typename Value, typename Box>
    static bool apply(not_disjoint<Geometry> const& p, Value const&, Box const& i)
    {
        return !geometry::disjoint(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<not_intersects<Geometry>, rtree::node_tag>
{
    template <typename Value, typename Box>
    static bool apply(not_intersects<Geometry> const& p, Value const&, Box const& i)
    {
        return !geometry::covered_by(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<not_overlaps<Geometry>, rtree::node_tag>
{
    template <typename Value, typename Box>
    static bool apply(not_overlaps<Geometry> const& p, Value const&, Box const& i)
    {
        return true;
    }
};

template <typename Geometry>
struct predicate_check<not_within<Geometry>, rtree::node_tag>
{
    template <typename Value, typename Box>
    static bool apply(not_within<Geometry> const& p, Value const&, Box const& i)
    {
        return !geometry::within(i, p.geometry);
    }
};

} // namespace detail

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_PREDICATES_HPP
