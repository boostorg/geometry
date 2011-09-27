// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - Spatial index distance predicates, calculators and checkers
//                      used in nearest query - rtree's nodes specializations
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_DISTANCE_PREDICATES_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_DISTANCE_PREDICATES_HPP

#include <boost/geometry/extensions/index/distance_predicates.hpp>
#include <boost/geometry/extensions/index/rtree/tags.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail {

// distance_calc

template <typename PointRelation, typename Box>
struct distances_calc<
    PointRelation,
    Box,
    rtree::node_tag>
{
    typedef typename detail::relation<PointRelation>::value_type point_type;
    typedef typename geometry::default_distance_result<point_type, Box>::type distance_type;

    typedef detail::cdist<distance_type, detail::near_tag> result_type;

    static inline result_type apply(PointRelation const& p, Box const& i)
    {
        result_type res;
        distances_calc_impl<result_type, point_type, Box>
            ::apply(res, relation<PointRelation>::value(p), i);
        return res;
    }
};

template <typename PointRelation, typename Box>
struct distances_calc<
    detail::unbounded<PointRelation>,
    Box,
    rtree::node_tag
>
{
    typedef typename detail::relation<PointRelation>::value_type point_type;
    typedef typename geometry::default_distance_result<point_type, Box>::type distance_type;

    typedef detail::cdist<distance_type, detail::near_tag> result_type;

    static inline result_type apply(detail::unbounded<PointRelation> const& pp, Box const& i)
    {
        result_type res;
        distances_calc_impl<result_type, point_type, Box>
            ::apply(res, relation<PointRelation>::value(pp.point_relation), i);
        return res;
    }
};

template <typename PointRelation, typename MinRelation, typename Box>
struct distances_calc<
    detail::min_bounded<PointRelation, MinRelation>,
    Box,
    rtree::node_tag
>
{
    typedef typename detail::relation<PointRelation>::value_type point_type;
    typedef typename geometry::default_distance_result<point_type, Box>::type distance_type;
    
    typedef typename detail::cdist_merge<
        cdist<distance_type, detail::near_tag>,
        cdist<distance_type, detail::far_tag>
    >::type result_type;

    static inline result_type apply(detail::min_bounded<PointRelation, MinRelation> const& pp, Box const& i)
    {
        result_type res;
        distances_calc_impl<result_type, point_type, Box>
            ::apply(res, relation<PointRelation>::value(pp.point_relation), i);
        return res;
    }
};

template <typename PointRelation, typename MaxRelation, typename Box>
struct distances_calc<
    detail::max_bounded<PointRelation, MaxRelation>,
    Box,
    rtree::node_tag
>
{
    typedef typename detail::relation<PointRelation>::value_type point_type;
    typedef typename geometry::default_distance_result<point_type, Box>::type distance_type;

    typedef cdist<distance_type, detail::near_tag> result_type;

    static inline result_type apply(detail::max_bounded<PointRelation, MaxRelation> const& pp, Box const& i)
    {
        result_type res;
        distances_calc_impl<result_type, point_type, Box>
            ::apply(res, relation<PointRelation>::value(pp.point_relation), i);
        return res;
    }
};

template <typename PointRelation, typename MinRelation, typename MaxRelation, typename Box>
struct distances_calc<
    detail::bounded<PointRelation, MinRelation, MaxRelation>,
    Box,
    rtree::node_tag
>
{
    typedef typename detail::relation<PointRelation>::value_type point_type;
    typedef typename geometry::default_distance_result<point_type, Box>::type distance_type;

    typedef typename detail::cdist_merge<
        cdist<distance_type, detail::near_tag>,
        cdist<distance_type, detail::far_tag>
    >::type result_type;

    static inline result_type apply(detail::bounded<PointRelation, MinRelation, MaxRelation> const& pp, Box const& i)
    {
        result_type res;
        distances_calc_impl<result_type, point_type, Box>
            ::apply(res, relation<PointRelation>::value(pp.point_relation), i);
        return res;
    }
};

// distance_predicate_check

template <typename PointRelation, typename Box>
struct distances_predicates_check<
    PointRelation,
    Box,
    rtree::node_tag>
{
    template <typename Distances>
    static inline bool apply(PointRelation const&, Distances const&)
    {
        return true;
    }
};

template <typename PointRelation, typename Box>
struct distances_predicates_check<
    detail::unbounded<PointRelation>,
    Box,
    rtree::node_tag>
{
    template <typename Distances>
    static inline bool apply(
        detail::unbounded<PointRelation> const&,
        Distances const&)
    {
        return true;
    }
};

template <typename PointRelation, typename MinRelation, typename Box>
struct distances_predicates_check<
    detail::min_bounded<PointRelation, MinRelation>,
    Box,
    rtree::node_tag>
{
    template <typename Distances>
    static inline bool apply(
        detail::min_bounded<PointRelation, MinRelation> const& pred,
        Distances const& d)
    {
        return pred.comparable_min
            <= cdist_value<Distances>::template get<detail::far_tag>(d);
    }
};

template <typename PointRelation, typename MaxRelation, typename Box>
struct distances_predicates_check<
    detail::max_bounded<PointRelation, MaxRelation>,
    Box,
    rtree::node_tag>
{
    template <typename Distances>
    static inline bool apply(
        detail::max_bounded<PointRelation, MaxRelation> const& pred,
        Distances const& d)
    {
        return cdist_value<Distances>::template get<detail::near_tag>(d)
            <= pred.comparable_max;
    }
};

template <typename PointRelation, typename MinRelation, typename MaxRelation, typename Box>
struct distances_predicates_check<
    detail::bounded<PointRelation, MinRelation, MaxRelation>,
    Box,
    rtree::node_tag>
{
    template <typename Distances>
    static inline bool apply(
        detail::bounded<PointRelation, MinRelation, MaxRelation> const& pred,
        Distances const& d)
    {
        return pred.comparable_min
                <= cdist_value<Distances>::template get<detail::far_tag>(d)
            && cdist_value<Distances>::template get<detail::near_tag>(d)
                <= pred.comparable_max;
    }
};

} // namespace detail

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_DISTANCE_PREDICATES_HPP
