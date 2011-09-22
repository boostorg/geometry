// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - Spatial index distances calculators used in nearest query
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_DISTANCE_CALC_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_DISTANCE_CALC_HPP

namespace boost { namespace geometry { namespace index {

namespace detail {

//TODO: awulkiew - consider storing values instead of const references
// it may be faster and it eliminates problems with storing of references to temporaries

struct distance_near_tag {};
struct distance_far_tag {};
struct distance_centroid_tag {};

template <typename Point, typename Tag>
struct distance_xxx
    : nonassignable
{
    typedef typename index::traits::coordinate_type<Point>::type coordinate_type;
    typedef typename geometry::default_distance_result<Point, Point>::type distance_type;

    inline explicit distance_xxx(
        Point const& p,
        coordinate_type const& distance_near = coordinate_type(0),
        coordinate_type const& distance_far = std::numeric_limits<coordinate_type>::max()
    )
        : point(p)
        , comparable_near(distance_near * distance_near)
        , comparable_far(distance_far * distance_far)
    {}

    Point const& point;
    distance_type comparable_near;
    distance_type comparable_far;
};

} // namespace detail

template <typename Point>
inline detail::distance_xxx<Point, detail::distance_near_tag>
distance(
    Point const& p,
    typename index::traits::coordinate_type<Point>::type const& near
        = typename index::traits::coordinate_type<Point>::type(0),
    typename index::traits::coordinate_type<Point>::type const& far
        = std::numeric_limits<typename index::traits::coordinate_type<Point>::type>::max()
)
{
    return detail::detail::distance_xxx<Point, detail::distance_near_tag>(p, near, far);
}

template <typename Point>
inline detail::distance_xxx<Point, detail::distance_near_tag>
distance_near(
    Point const& p,
    typename index::traits::coordinate_type<Point>::type const& near
        = typename index::traits::coordinate_type<Point>::type(0),
    typename index::traits::coordinate_type<Point>::type const& far
        = std::numeric_limits<typename index::traits::coordinate_type<Point>::type>::max()
)
{
    return detail::detail::distance_xxx<Point, detail::distance_near_tag>(p, near, far);
}

template <typename Point>
inline detail::distance_xxx<Point, detail::distance_far_tag>
distance_far(
    Point const& p,
    typename index::traits::coordinate_type<Point>::type const& near
        = typename index::traits::coordinate_type<Point>::type(0),
    typename index::traits::coordinate_type<Point>::type const& far
        = std::numeric_limits<typename index::traits::coordinate_type<Point>::type>::max()
    )
{
    return detail::detail::distance_xxx<Point, detail::distance_far_tag>(p, near, far);
}

template <typename Point>
inline detail::distance_xxx<Point, detail::distance_centroid_tag>
distance_centroid(
    Point const& p,
    typename index::traits::coordinate_type<Point>::type const& near
        = typename index::traits::coordinate_type<Point>::type(0),
    typename index::traits::coordinate_type<Point>::type const& far
        = std::numeric_limits<typename index::traits::coordinate_type<Point>::type>::max()
    )
{
    return detail::detail::distance_xxx<Point, detail::distance_centroid_tag>(p, near, far);
}

namespace detail
{

template <typename Point, typename Indexable>
struct distance_calc
{
    typedef typename geometry::default_distance_result<Point, Indexable>::type distance_type;

    static inline distance_type apply(Point const& p, Indexable const& i)
    {
        return index::mindist(p, i);
    }
};

template <typename Point, typename Indexable>
struct distance_calc<
    detail::distance_xxx<Point, detail::distance_near_tag>,
    Indexable>
{
    typedef typename geometry::default_distance_result<Point, Indexable>::type distance_type;

    static inline distance_type apply(
        detail::distance_xxx<Point, detail::distance_near_tag> const& dx,
        Indexable const& i)
    {
        return index::mindist(dx.point, i);
    }
};

// TODO distance_centroid

template <typename Point, typename Indexable>
struct distance_calc<
    detail::distance_xxx<Point, detail::distance_far_tag>,
    Indexable>
{
    typedef typename geometry::default_distance_result<Point, Indexable>::type distance_type;

    static inline distance_type apply(
        detail::distance_xxx<Point, detail::distance_far_tag> const& dx,
        Indexable const& i)
    {
        return index::maxdist(dx.point, i);
    }
};

template <typename Point>
struct distance_comp
{
    template <typename DistanceType>
    static inline bool apply(Point const&, DistanceType const&)
    {
        return true;
    }
};

template <typename Point, typename Tag>
struct distance_comp< detail::distance_xxx<Point, Tag> >
{
    template <typename DistanceType>
    static inline bool apply(
        detail::distance_xxx<Point, Tag> const& dx,
        DistanceType const& d)
    {
        return dx.comparable_near <= d && d <= dx.comparable_far;
    }
};

// TODO: awulkiew - pruning for nodes!
// if 0 < comp_near node is pruned if maxdist(point, node_box) < comp_near
// if comp_far < INF node is pruned if comp_far < min_dist(point, node_box)
// still nodes must be sorted by min_dist(point, node_box)

// for values, proper distance values are calculated min, max or centroid
// and tested with comp_near and/or comp_far

// implement versions with only one comp or without comp distances?
// less tests == speed increase
// near_between, near_lesser, near_greater
// far_xxx
// centroid_xxx, center_xxx

// distance_range, distance_bound, distance_upper_bound

// distance_between<near | far| centroid tag> - now distance_xxx
// distance_xxxxxx<near|far|centroid, less|more>
// distance_point_only

// distance_calc for each <near|far|centroid>
// distance_comp for each class and xxxxxx<less|more>

// + something in case of nodes
// additional calculation of maxdist in case of distance_between and
// distance_xxxxx<more> 

} // namespace detail

template <typename PointData, typename Indexable>
inline typename detail::distance_calc<PointData, Indexable>::distance_type
distance_calc(PointData const& p, Indexable const& i)
{
    return detail::distance_calc<PointData, Indexable>
        ::apply(p, i);
}

template <typename PointData, typename DistanceType>
inline bool
distance_comp(PointData const& p, DistanceType const& d)
{
    return detail::distance_comp<PointData>
        ::apply(p, d);
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_DISTANCE_CALC_HPP
