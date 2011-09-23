// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - Spatial index distance predicates, calculators and checkers
//                      used in nearest query - rtree's specializations
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_DISTANCE_PREDICATES_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_DISTANCE_PREDICATES_HPP

namespace boost { namespace geometry { namespace index {

namespace detail {

namespace rtree
{

struct value_distance_predicate_tag {};
struct node_distance_predicate_tag {};

} // namespace rtree

// distance_calc

template <typename Point, typename Indexable>
struct distance_calc<Point, Indexable, node_distance_predicate_tag>
{
    typedef typename geometry::default_distance_result<Point, Indexable>::type result_type;

    static inline result_type apply(Point const& p, Indexable const& i)
    {
        return index::comparable_distance_near(p, i);
    }
};

// TODO - finish specializations for nodes

template <typename Point, typename AlgoTag, typename Indexable, typename Tag>
struct distance_calc<
    detail::distance_unbounded<Point, AlgoTag>,
    Indexable,
    Tag>
{
    typedef typename distance_calc_impl<Point, Indexable, AlgoTag>::result_type result_type;

    static inline result_type apply(
        detail::distance_unbounded<Point, AlgoTag> const& dx,
        Indexable const& i)
    {
        return distance_calc_impl<Point, Indexable, AlgoTag>::apply(dx.point, i);
    }
};

template <typename Point, typename AlgoTag, typename LimitTag, typename Indexable, typename Tag>
struct distance_calc<
    detail::distance_half_bounded<Point, AlgoTag, LimitTag>,
    Indexable,
    Tag>
{
    typedef typename distance_calc_impl<Point, Indexable, AlgoTag>::result_type result_type;

    static inline result_type apply(
        detail::distance_half_bounded<Point, AlgoTag, LimitTag> const& dx,
        Indexable const& i)
    {
        return distance_calc_impl<Point, Indexable, AlgoTag>::apply(dx.point, i);
    }
};

template <typename Point, typename AlgoTag, typename Indexable, typename Tag>
struct distance_calc<
    detail::distance_bounded<Point, AlgoTag>,
    Indexable,
    Tag>
{
    typedef typename distance_calc_impl<Point, Indexable, AlgoTag>::result_type result_type;

    static inline result_type apply(
        detail::distance_bounded<Point, AlgoTag> const& dx,
        Indexable const& i)
    {
        return distance_calc_impl<Point, Indexable, AlgoTag>::apply(dx.point, i);
    }
};

// distance_check

template <typename Point, typename Tag>
struct distance_check
{
    template <typename DistanceType>
    static inline bool apply(Point const&, DistanceType const&)
    {
        return true;
    }
};

template <typename Point, typename AlgoTag, typename Tag>
struct distance_check<
    detail::distance_unbounded<Point, AlgoTag>,
    Tag>
{
    template <typename DistanceType>
    static inline bool apply(
        detail::distance_unbounded<Point, AlgoTag> const&,
        DistanceType const&)
    {
        return true;
    }
};

template <typename Point, typename AlgoTag, typename Tag>
struct distance_check<
    detail::distance_half_bounded<Point, AlgoTag, detail::distance_min_tag>,
    Tag>
{
    template <typename DistanceType>
    static inline bool apply(
        detail::distance_half_bounded<Point, AlgoTag, detail::distance_min_tag> const& dx,
        DistanceType const& comparable_d)
    {
        return dx.comparable_limit <= comparable_d;
    }
};

template <typename Point, typename AlgoTag, typename Tag>
struct distance_check<
    detail::distance_half_bounded<Point, AlgoTag, detail::distance_max_tag>,
    Tag>
{
    template <typename DistanceType>
    static inline bool apply(
        detail::distance_half_bounded<Point, AlgoTag, detail::distance_max_tag> const& dx,
        DistanceType const& comparable_d)
    {
        return comparable_d <= dx.comparable_limit;
    }
};

template <typename Point, typename AlgoTag, typename Tag>
struct distance_check<
    detail::distance_bounded<Point, AlgoTag>,
    Tag>
{
    template <typename DistanceType>
    static inline bool apply(
        detail::distance_bounded<Point, AlgoTag> const& dx,
        DistanceType const& comparable_d)
    {
        return dx.comparable_min <= comparable_d && comparable_d <= dx.comparable_max;
    }
};

// move distance_calc and distance_comp into geometry::index ?

// TODO: awulkiew - pruning for nodes! <- inside detail::rtree so NOT HERE
// if 0 < comp_near node is pruned if maxdist(point, node_box) < comp_near
// if comp_far < INF node is pruned if comp_far < min_dist(point, node_box)
// still nodes must be sorted by min_dist(point, node_box)

// for values, proper distance values are calculated min, max or centroid
// and tested with comp_near and/or comp_far

// + something in case of nodes
// additional calculation of maxdist in case of distance_between and
// distance_xxxxx<more> 

} // namespace detail

//template <typename PointData, typename Indexable>
//inline typename detail::distance_calc<PointData, Indexable>::distance_type
//distance_calc(PointData const& p, Indexable const& i)
//{
//    return detail::distance_calc<PointData, Indexable>
//        ::apply(p, i);
//}
//
//template <typename PointData, typename DistanceType>
//inline bool
//distance_comp(PointData const& p, DistanceType const& d)
//{
//    return detail::distance_comp<PointData>
//        ::apply(p, d);
//}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_DISTANCE_PREDICATES_HPP
