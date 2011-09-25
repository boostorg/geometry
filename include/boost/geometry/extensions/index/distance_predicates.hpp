// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - Spatial index distance predicates, calculators and checkers used in nearest query
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_DISTANCE_PREDICATES_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_DISTANCE_PREDICATES_HPP

#include <boost/geometry/extensions/index/algorithms/comparable_distance_near.hpp>
#include <boost/geometry/extensions/index/algorithms/comparable_distance_far.hpp>
#include <boost/geometry/extensions/index/algorithms/comparable_distance_centroid.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail {

//TODO: awulkiew - consider storing values instead of const references
// it may be faster and it eliminates problems with storing of references to temporaries
// moreover user may use boost::cref

// TODO: awulkiew - what with coordinate systems other than cartesian?
// do comparable_distance returns distance in coordinate system of objects used?
// what if objects are in different systems?
// should index algorithms work exactly like comparable_distance or not?

// data

struct distance_near_tag {};
struct distance_far_tag {};
struct distance_centroid_tag {};

struct distance_min_tag {};
struct distance_max_tag {};

template <typename Point, typename Tag>
struct distance_unbounded
    : nonassignable
{
    typedef typename index::traits::coordinate_type<Point>::type coordinate_type;

    inline explicit distance_unbounded(Point const& p)
        : point(p)
    {}

    Point const& point;
};

template <typename Point, typename Tag, typename LimitTag>
struct distance_half_bounded
    : nonassignable
{
    typedef typename index::traits::coordinate_type<Point>::type coordinate_type;
    typedef typename geometry::default_distance_result<Point, Point>::type distance_type;

    inline explicit distance_half_bounded(Point const& p, coordinate_type const& distance_limit)
        : point(p)
        , comparable_limit(distance_limit * distance_limit)
    {}

    Point const& point;
    distance_type comparable_limit;
};

template <typename Point, typename Tag>
struct distance_bounded
    : nonassignable
{
    typedef typename index::traits::coordinate_type<Point>::type coordinate_type;
    typedef typename geometry::default_distance_result<Point, Point>::type distance_type;

    inline explicit distance_bounded(Point const& p, coordinate_type const& distance_min, coordinate_type const& distance_max)
        : point(p)
        , comparable_min(distance_min * distance_min)
        , comparable_max(distance_max * distance_max)
    {}

    Point const& point;
    distance_type comparable_min;
    distance_type comparable_max;
};

} // namespace detail

// generators

template <typename Point>
inline detail::distance_unbounded<Point, detail::distance_near_tag>
distance_near(Point const& p)
{
    return detail::distance_unbounded<Point, detail::distance_near_tag>(p);
}

template <typename Point>
inline detail::distance_unbounded<Point, detail::distance_far_tag>
distance_far(Point const& p)
{
    return detail::distance_unbounded<Point, detail::distance_far_tag>(p);
}

template <typename Point>
inline detail::distance_unbounded<Point, detail::distance_centroid_tag>
distance_centroid(Point const& p)
{
    return detail::distance_unbounded<Point, detail::distance_centroid_tag>(p);
}

template <typename Point>
inline detail::distance_half_bounded<Point, detail::distance_near_tag, detail::distance_min_tag>
distance_near(
    Point const& p,
    typename index::traits::coordinate_type<Point>::type const& distance_min)
{
    return detail::distance_half_bounded<Point, detail::distance_near_tag, detail::distance_min_tag>(p, distance_min);
}

template <typename Point>
inline detail::distance_half_bounded<Point, detail::distance_far_tag, detail::distance_min_tag>
distance_far(
    Point const& p,
    typename index::traits::coordinate_type<Point>::type const& distance_min)
{
    return detail::distance_half_bounded<Point, detail::distance_far_tag, detail::distance_min_tag>(p, distance_min);
}

template <typename Point>
inline detail::distance_half_bounded<Point, detail::distance_centroid_tag, detail::distance_min_tag>
distance_centroid(
    Point const& p,
    typename index::traits::coordinate_type<Point>::type const& distance_min)
{
    return detail::distance_half_bounded<Point, detail::distance_centroid_tag, detail::distance_min_tag>(p, distance_min);
}

template <typename Point>
inline detail::distance_bounded<Point, detail::distance_near_tag>
distance_near(
    Point const& p,
    typename index::traits::coordinate_type<Point>::type const& distance_min,
    typename index::traits::coordinate_type<Point>::type const& distance_max)
{
    return detail::distance_bounded<Point, detail::distance_near_tag>(p, distance_min, distance_max);
}

template <typename Point>
inline detail::distance_bounded<Point, detail::distance_far_tag>
distance_far(
    Point const& p,
    typename index::traits::coordinate_type<Point>::type const& distance_min,
    typename index::traits::coordinate_type<Point>::type const& distance_max)
{
    return detail::distance_bounded<Point, detail::distance_far_tag>(p, distance_min, distance_max);
}

template <typename Point>
inline detail::distance_bounded<Point, detail::distance_centroid_tag>
distance_centroid(
    Point const& p,
    typename index::traits::coordinate_type<Point>::type const& distance_min,
    typename index::traits::coordinate_type<Point>::type const& distance_max)
{
    return detail::distance_bounded<Point, detail::distance_centroid_tag>(p, distance_min, distance_max);
}

// algorithms

namespace detail
{

// distance_calc_impl

template <typename Point, typename Indexable, typename AlgoTag>
struct distance_calc_impl
{
    BOOST_MPL_ASSERT_MSG(
        (false),
        NOT_IMPLEMENTED_FOR_THIS_TAG_TYPE,
        (distance_calc_impl));
};

template <typename Point, typename Indexable>
struct distance_calc_impl<Point, Indexable, detail::distance_near_tag>
{
    typedef typename geometry::default_distance_result<Point, Indexable>::type result_type;

    static inline result_type apply(Point const& p, Indexable const& i)
    {
        return index::comparable_distance_near(p, i);
    }
};

template <typename Point, typename Indexable>
struct distance_calc_impl<Point, Indexable, detail::distance_far_tag>
{
    typedef typename geometry::default_distance_result<Point, Indexable>::type result_type;

    static inline result_type apply(Point const& p, Indexable const& i)
    {
        return index::comparable_distance_far(p, i);
    }
};

template <typename Point, typename Indexable>
struct distance_calc_impl<Point, Indexable, detail::distance_centroid_tag>
{
    typedef typename geometry::default_distance_result<Point, Indexable>::type result_type;

    static inline result_type apply(Point const& p, Indexable const& i)
    {
        return index::comparable_distance_centroid(p, i);
    }
};

// distance_calc

template <typename Point, typename Indexable, typename Tag>
struct distance_calc
{
    typedef typename geometry::default_distance_result<Point, Indexable>::type result_type;

    static inline result_type apply(Point const& p, Indexable const& i)
    {
        return index::comparable_distance_near(p, i);
    }
};

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

// distance_predicate_check

template <typename Point, typename Indexable, typename Tag>
struct distance_predicate_check
{
    template <typename DistanceType>
    static inline bool apply(Point const&, DistanceType const&)
    {
        return true;
    }
};

template <typename Point, typename AlgoTag, typename Indexable, typename Tag>
struct distance_predicate_check<
    detail::distance_unbounded<Point, AlgoTag>,
    Indexable,
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

template <typename Point, typename AlgoTag, typename Indexable, typename Tag>
struct distance_predicate_check<
    detail::distance_half_bounded<Point, AlgoTag, detail::distance_min_tag>,
    Indexable,
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

template <typename Point, typename AlgoTag, typename Indexable, typename Tag>
struct distance_predicate_check<
    detail::distance_half_bounded<Point, AlgoTag, detail::distance_max_tag>,
    Indexable,
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

template <typename Point, typename AlgoTag, typename Indexable, typename Tag>
struct distance_predicate_check<
    detail::distance_bounded<Point, AlgoTag>,
    Indexable,
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

// distance_point

template <typename Point>
struct distance_point
{
    typedef Point type;
};

template <typename Point, typename AlgoTag>
struct distance_point< detail::distance_unbounded<Point, AlgoTag> >
{
   typedef Point type;
};

template <typename Point, typename AlgoTag, typename LimitTag>
struct distance_point< detail::distance_half_bounded<Point, AlgoTag, LimitTag> >
{
    typedef Point type;
};

template <typename Point, typename AlgoTag>
struct distance_point< detail::distance_bounded<Point, AlgoTag> >
{
    typedef Point type;
};

} // namespace detail

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_DISTANCE_PREDICATES_HPP
