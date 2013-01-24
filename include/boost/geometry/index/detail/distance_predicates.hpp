// Boost.Geometry Index
//
// Spatial index distance predicates, calculators and checkers
// used in nearest query - specialized for envelopes
//
// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_INDEX_DETAIL_DISTANCE_PREDICATES_HPP
#define BOOST_GEOMETRY_INDEX_DETAIL_DISTANCE_PREDICATES_HPP

#include <boost/geometry/index/detail/algorithms/comparable_distance_near.hpp>
#include <boost/geometry/index/detail/algorithms/comparable_distance_far.hpp>
#include <boost/geometry/index/detail/algorithms/comparable_distance_centroid.hpp>

#include <boost/geometry/index/detail/tuples.hpp>

#include <boost/geometry/index/detail/tags.hpp>

// TODO - optimization
// For Boxes and Points all types of distances may be calculated
// - near, far and centroid. For points those are the same distance
// calculate them for Boxes only!
// for Points calculate only 1 distance

namespace boost { namespace geometry { namespace index { namespace detail {

// ------------------------------------------------------------------ //
// predicate
// ------------------------------------------------------------------ //

template <typename DistancePredicates>
struct nearest
{
    static const bool is_one = false;

    nearest(DistancePredicates const& dpred, unsigned k)
        : distance_predicates(dpred)
        , count(k)
    {}
    DistancePredicates const& distance_predicates;
    unsigned count;
};

template <typename DistancePredicates>
struct predicate_check<nearest<DistancePredicates>, value_tag>
{
    template <typename Value, typename Box>
    static inline bool apply(nearest<DistancePredicates> const&, Value const&, Box const&)
    {
        return true;
    }
};

template <typename DistancePredicates>
struct predicate_check<nearest<DistancePredicates>, envelope_tag>
{
    template <typename Value, typename Box>
    static inline bool apply(nearest<DistancePredicates> const&, Value const&, Box const&)
    {
        return true;
    }
};

template <typename DistancePredicates>
struct nearest_one
{
    static const bool is_one = true;

    nearest_one(DistancePredicates const& dpred)
        : distance_predicates(dpred)
    {}
    DistancePredicates const& distance_predicates;
};

template <typename DistancePredicates>
struct predicate_check<nearest_one<DistancePredicates>, value_tag>
{
    template <typename Value, typename Box>
    static inline bool apply(nearest_one<DistancePredicates> const&, Value const&, Box const&)
    {
        return true;
    }
};

template <typename DistancePredicates>
struct predicate_check<nearest_one<DistancePredicates>, envelope_tag>
{
    template <typename Value, typename Box>
    static inline bool apply(nearest_one<DistancePredicates> const&, Value const&, Box const&)
    {
        return true;
    }
};

// predicates_is_nearest

template <typename P>
struct predicates_is_nearest
{
    static const unsigned value = 0;
};

template <typename DistancePredicates>
struct predicates_is_nearest< nearest<DistancePredicates> >
{
    static const unsigned value = 1;
};

template <typename DistancePredicates>
struct predicates_is_nearest< nearest_one<DistancePredicates> >
{
    static const unsigned value = 1;
};

// predicates_count_nearest

template <typename T>
struct predicates_count_nearest
{
    static const unsigned value = predicates_is_nearest<T>::value;
};

template <typename F, typename S>
struct predicates_count_nearest< std::pair<F, S> >
{
    static const unsigned value = predicates_is_nearest<F>::value
                                + predicates_is_nearest<S>::value;
};

template <typename Tuple, unsigned N>
struct predicates_count_nearest_tuple
{
    static const unsigned value =
        predicates_is_nearest<typename boost::tuples::element<N-1, Tuple>::type>::value
        + predicates_count_nearest_tuple<Tuple, N-1>::value;
};

template <typename Tuple>
struct predicates_count_nearest_tuple<Tuple, 1>
{
    static const unsigned value =
        predicates_is_nearest<typename boost::tuples::element<0, Tuple>::type>::value;
};

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
struct predicates_count_nearest< boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> >
{
    static const unsigned value = predicates_count_nearest_tuple<
        boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>,
        boost::tuples::length< boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> >::value
    >::value;
};

template <typename Head, typename Tail>
struct predicates_count_nearest< boost::tuples::cons<Head, Tail> >
{
    static const unsigned value = predicates_count_nearest_tuple<
        boost::tuples::cons<Head, Tail>,
        boost::tuples::length< boost::tuples::cons<Head, Tail> >::value
    >::value;
};

// predicates_find_nearest

template <typename T>
struct predicates_find_nearest
{
    static const unsigned value = predicates_is_nearest<T>::value ? 0 : 1;
};

template <typename F, typename S>
struct predicates_find_nearest< std::pair<F, S> >
{
    static const unsigned value = predicates_is_nearest<F>::value ? 0 :
                                    (predicates_is_nearest<S>::value ? 1 : 2);
};

template <typename Tuple, unsigned N>
struct predicates_find_nearest_tuple
{
    static const bool is_found = predicates_find_nearest_tuple<Tuple, N-1>::is_found
                                || predicates_is_nearest<typename boost::tuples::element<N-1, Tuple>::type>::value;

    static const unsigned value = predicates_find_nearest_tuple<Tuple, N-1>::is_found ?
        predicates_find_nearest_tuple<Tuple, N-1>::value :
        (predicates_is_nearest<typename boost::tuples::element<N-1, Tuple>::type>::value ?
            N-1 : boost::tuples::length<Tuple>::value);
};

template <typename Tuple>
struct predicates_find_nearest_tuple<Tuple, 1>
{
    static const bool is_found = predicates_is_nearest<typename boost::tuples::element<0, Tuple>::type>::value;
    static const unsigned value = is_found ? 0 : boost::tuples::length<Tuple>::value;
};

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
struct predicates_find_nearest< boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> >
{
    static const unsigned value = predicates_find_nearest_tuple<
        boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>,
        boost::tuples::length< boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> >::value
    >::value;
};

template <typename Head, typename Tail>
struct predicates_find_nearest< boost::tuples::cons<Head, Tail> >
{
    static const unsigned value = predicates_find_nearest_tuple<
        boost::tuples::cons<Head, Tail>,
        boost::tuples::length< boost::tuples::cons<Head, Tail> >::value
    >::value;
};

// ------------------------------------------------------------------ //
// relations
// ------------------------------------------------------------------ //

template <typename T>
struct to_nearest
{
    to_nearest(T const& v) : value(v) {}
    T value;
};

template <typename T>
struct to_centroid
{
    to_centroid(T const& v) : value(v) {}
    T value;
};

template <typename T>
struct to_furthest
{
    to_furthest(T const& v) : value(v) {}
    T value;
};

// tags

struct to_nearest_tag {};
struct to_centroid_tag {};
struct to_furthest_tag {};

// ------------------------------------------------------------------ //
// relation traits and access
// ------------------------------------------------------------------ //

template <typename T>
struct relation
{
    typedef T value_type;
    typedef to_nearest_tag tag;
    static inline T const& value(T const& v) { return v; }
    static inline T & value(T & v) { return v; }
};

template <typename T>
struct relation< to_nearest<T> >
{
    typedef T value_type;
    typedef to_nearest_tag tag;
    static inline T const& value(to_nearest<T> const& r) { return r.value; }
    static inline T & value(to_nearest<T> & r) { return r.value; }
};

template <typename T>
struct relation< to_centroid<T> >
{
    typedef T value_type;
    typedef to_centroid_tag tag;
    static inline T const& value(to_centroid<T> const& r) { return r.value; }
    static inline T & value(to_centroid<T> & r) { return r.value; }
};

template <typename T>
struct relation< to_furthest<T> >
{
    typedef T value_type;
    typedef to_furthest_tag tag;
    static inline T const& value(to_furthest<T> const& r) { return r.value; }
    static inline T & value(to_furthest<T> & r) { return r.value; }
};

// ------------------------------------------------------------------ //
// distance predicates
// ------------------------------------------------------------------ //

template <typename PointRelation>
struct unbounded
    : nonassignable
{
    inline explicit unbounded(PointRelation const& pr)
        : point_relation(pr)
    {}

    PointRelation point_relation;
};

template <typename PointRelation, typename MinRelation>
struct min_bounded
    : nonassignable
{
    typedef typename detail::relation<PointRelation>::value_type point_type;
    typedef typename geometry::default_distance_result<point_type, point_type>::type distance_type;

    inline min_bounded(PointRelation const& pr, MinRelation const& min_rel)
        : point_relation(pr)
        , comparable_min(
            relation<MinRelation>::value(min_rel) *
            relation<MinRelation>::value(min_rel) )
    {}

    PointRelation point_relation;
    distance_type comparable_min;
};

template <typename PointRelation, typename MaxRelation>
struct max_bounded
    : nonassignable
{
    typedef typename detail::relation<PointRelation>::value_type point_type;
    typedef typename geometry::default_distance_result<point_type, point_type>::type distance_type;

    inline max_bounded(PointRelation const& pr, MaxRelation const& max_rel)
        : point_relation(pr)
        , comparable_max(
            relation<MaxRelation>::value(max_rel) *
            relation<MaxRelation>::value(max_rel) )
    {}

    PointRelation point_relation;
    distance_type comparable_max;
};

template <typename PointRelation, typename MinRelation, typename MaxRelation>
struct bounded
    : nonassignable
{
    typedef typename detail::relation<PointRelation>::value_type point_type;
    typedef typename geometry::default_distance_result<point_type, point_type>::type distance_type;

    inline bounded(PointRelation const& pr, MinRelation const& min_rel, MaxRelation const& max_rel)
        : point_relation(pr)
        , comparable_min(
            relation<MinRelation>::value(min_rel) *
            relation<MinRelation>::value(min_rel) )
        , comparable_max(
            relation<MaxRelation>::value(max_rel) *
            relation<MaxRelation>::value(max_rel) )
    {}

    PointRelation point_relation;
    distance_type comparable_min;
    distance_type comparable_max;
};

// ------------------------------------------------------------------ //
// point_relation trait
// ------------------------------------------------------------------ //

template <typename PointRelation>
struct point_relation
{
    typedef PointRelation type;
};

template <typename PointRelation>
struct point_relation< detail::unbounded<PointRelation> >
{
   typedef PointRelation type;
};

template <typename PointRelation, typename MinRelation>
struct point_relation< detail::min_bounded<PointRelation, MinRelation> >
{
    typedef PointRelation type;
};

template <typename PointRelation, typename MaxRelation>
struct point_relation< detail::max_bounded<PointRelation, MaxRelation> >
{
    typedef PointRelation type;
};

template <typename PointRelation, typename MinRelation, typename MaxRelation>
struct point_relation< detail::bounded<PointRelation, MinRelation, MaxRelation> >
{
    typedef PointRelation type;
};

// ------------------------------------------------------------------ //
// helpers
// ------------------------------------------------------------------ //

// algorithms

// cdist

template <typename T, typename Tag>
struct cdist
{
    T value;
};

// cdist_merge

template <typename CDistTuple, typename CDist>
struct cdist_merge
{
    typedef typename detail::tuples::add_unique<
        CDistTuple,
        CDist
    >::type type;
};

template <typename T, typename Tag, typename OtherTag>
struct cdist_merge<
    cdist<T, Tag>,
    cdist<T, OtherTag> >
{
    typedef boost::tuple<
        cdist<T, Tag>,
        cdist<T, OtherTag>
    > type;
};

template <typename T, typename Tag>
struct cdist_merge<
    cdist<T, Tag>,
    cdist<T, Tag> >
{
    typedef cdist<T, Tag> type;
};

// cdist_value_type

template <typename CDistTuple>
struct cdist_value
{
    typedef typename cdist_value<
        typename boost::tuples::element<0, CDistTuple>::type
    >::type type;

    template <typename Tag>
    static inline type & get(CDistTuple & cdtup)
    {
        return boost::get<
            tuples::find_index<
                CDistTuple,
                cdist<type, Tag>
            >::value
        >(cdtup).value;
    }

    template <typename Tag>
    static inline type const& get(CDistTuple const& cdtup)
    {
        return boost::get<
            tuples::find_index<
                CDistTuple,
                cdist<type, Tag>
            >::value
        >(cdtup).value;
    }
};

template <typename T, typename Tag>
struct cdist_value<
    cdist<T, Tag>
>
{
    typedef T type;

    template <typename Tag2>
    static inline type & get(cdist<T, Tag> & cd)
    {
        BOOST_MPL_ASSERT_MSG(
            (boost::is_same< cdist<T, Tag2>, cdist<T, Tag> >::value),
            TAGS_DO_NOT_MATCH,
            (cdist_value));

        return cd.value;
    }

    template <typename Tag2>
    static inline type const& get(cdist<T, Tag> const& cd)
    {
        BOOST_MPL_ASSERT_MSG(
            (boost::is_same< cdist<T, Tag2>, cdist<T, Tag> >::value),
            TAGS_DO_NOT_MATCH,
            (cdist_value));

        return cd.value;
    }
};

// distances_calc_impl_rel

template <typename RelDist>
struct distances_calc_impl_rel
{
    BOOST_MPL_ASSERT_MSG(
        (false),
        NOT_IMPLEMENTED_FOR_THIS_RELATION,
        (distances_calc_impl_rel));
};

template <typename T>
struct distances_calc_impl_rel<
    cdist<T, detail::to_nearest_tag>
>
{
    template <typename Point, typename Indexable>
    typename geometry::default_distance_result<Point, Indexable>::type
    static inline apply(Point const& p, Indexable const& i)
    {
        return index::detail::comparable_distance_near(p, i);
    }
};

template <typename T>
struct distances_calc_impl_rel<
    cdist<T, detail::to_centroid_tag>
>
{
    template <typename Point, typename Indexable>
    typename geometry::default_distance_result<Point, Indexable>::type
    static inline apply(Point const& p, Indexable const& i)
    {
        return index::detail::comparable_distance_centroid(p, i);
    }
};

template <typename T>
struct distances_calc_impl_rel<
    cdist<T, detail::to_furthest_tag>
>
{
    template <typename Point, typename Indexable>
    typename geometry::default_distance_result<Point, Indexable>::type
    static inline apply(Point const& p, Indexable const& i)
    {
        return index::detail::comparable_distance_far(p, i);
    }
};

// distances_calc_impl_tuple

template <typename Distances, typename Point, typename Indexable, unsigned N>
struct distances_calc_impl_tuple
{
    // TODO MPL ASSERT 0 < N
    static inline void apply(Distances & d, Point const& p, Indexable const&i)
    {
        boost::get<N - 1>(d).value =
            distances_calc_impl_rel<
                typename boost::tuples::element<N - 1, Distances>::type
            >::apply(p, i);

        distances_calc_impl_tuple<
            Distances,
            Point,
            Indexable,
            N - 1
        >::apply(d, p, i);
    }
};

template <typename Distances, typename Point, typename Indexable>
struct distances_calc_impl_tuple<Distances, Point, Indexable, 1>
{
    static inline void apply(Distances & d, Point const& p, Indexable const&i)
    {
        boost::get<0>(d).value =
            distances_calc_impl_rel<
                typename boost::tuples::element<0, Distances>::type
            >::apply(p, i);
    }
};

// distances_calc_impl

template <typename Distances, typename Point, typename Indexable>
struct distances_calc_impl
{
    static inline void apply(Distances & d, Point const& p, Indexable const&i)
    {
        distances_calc_impl_tuple<
            Distances,
            Point,
            Indexable,
            boost::tuples::length<Distances>::value
        >::apply(d, p, i);
    }
};

template <typename T, typename Tag, typename Point, typename Indexable>
struct distances_calc_impl<
    cdist<T, Tag>,
    Point,
    Indexable
>
{
    static inline void apply(cdist<T, Tag> & d, Point const& p, Indexable const&i)
    {
        d.value = distances_calc_impl_rel<
            cdist<T, Tag>
        >::apply(p, i);
    }
};

// ------------------------------------------------------------------ //
// distance_calc and distances_predicates_check
// ------------------------------------------------------------------ //

template <typename PointRelation, typename Indexable, typename Tag>
struct distances_calc
{
    BOOST_MPL_ASSERT_MSG(
        (false),
        NOT_IMPLEMENTED_FOR_THIS_TAG,
        (distances_calc));
};

template <typename PointRelation, typename Indexable, typename Tag>
struct distances_predicates_check
{
    BOOST_MPL_ASSERT_MSG(
        (false),
        NOT_IMPLEMENTED_FOR_THIS_TAG,
        (distances_predicates_check));
};

// ------------------------------------------------------------------ //
// distance_calc for value_tag
// ------------------------------------------------------------------ //

template <typename PointRelation, typename Indexable>
struct distances_calc<PointRelation, Indexable, value_tag>
{
    typedef typename detail::relation<PointRelation>::value_type point_type;
    typedef typename detail::relation<PointRelation>::tag point_relation_tag;
    typedef typename geometry::default_distance_result<point_type, Indexable>::type distance_type;

    typedef detail::cdist<distance_type, point_relation_tag> result_type;

    static inline result_type apply(PointRelation const& p, Indexable const& i)
    {
        result_type res;
        distances_calc_impl<result_type, point_type, Indexable>
            ::apply(res, relation<PointRelation>::value(p), i);
        return res;
    }
};

template <typename PointRelation, typename Indexable>
struct distances_calc<
    detail::unbounded<PointRelation>,
    Indexable,
    value_tag
>
{
    typedef typename detail::relation<PointRelation>::value_type point_type;
    typedef typename detail::relation<PointRelation>::tag point_relation_tag;
    typedef typename geometry::default_distance_result<point_type, Indexable>::type distance_type;

    typedef detail::cdist<distance_type, point_relation_tag> result_type;

    static inline result_type apply(detail::unbounded<PointRelation> const& pp, Indexable const& i)
    {
        result_type res;
        distances_calc_impl<result_type, point_type, Indexable>
            ::apply(res, relation<PointRelation>::value(pp.point_relation), i);
        return res;
    }
};

template <typename PointRelation, typename MinRelation, typename Indexable>
struct distances_calc<
    detail::min_bounded<PointRelation, MinRelation>,
    Indexable,
    value_tag
>
{
    typedef typename detail::relation<PointRelation>::value_type point_type;
    typedef typename detail::relation<PointRelation>::tag point_relation_tag;
    typedef typename geometry::default_distance_result<point_type, Indexable>::type distance_type;
    typedef typename detail::relation<MinRelation>::tag min_relation_tag;

    typedef typename detail::cdist_merge<
        cdist<distance_type, point_relation_tag>,
        cdist<distance_type, min_relation_tag>
    >::type result_type;

    static inline result_type apply(detail::min_bounded<PointRelation, MinRelation> const& pp, Indexable const& i)
    {
        result_type res;
        distances_calc_impl<result_type, point_type, Indexable>
            ::apply(res, relation<PointRelation>::value(pp.point_relation), i);
        return res;
    }
};

template <typename PointRelation, typename MaxRelation, typename Indexable>
struct distances_calc<
    detail::max_bounded<PointRelation, MaxRelation>,
    Indexable,
    value_tag
>
{
    typedef typename detail::relation<PointRelation>::value_type point_type;
    typedef typename detail::relation<PointRelation>::tag point_relation_tag;
    typedef typename geometry::default_distance_result<point_type, Indexable>::type distance_type;
    typedef typename detail::relation<MaxRelation>::tag max_relation_tag;

    typedef typename detail::cdist_merge<
        cdist<distance_type, point_relation_tag>,
        cdist<distance_type, max_relation_tag>
    >::type result_type;

    static inline result_type apply(detail::max_bounded<PointRelation, MaxRelation> const& pp, Indexable const& i)
    {
        result_type res;
        distances_calc_impl<result_type, point_type, Indexable>
            ::apply(res, relation<PointRelation>::value(pp.point_relation), i);
        return res;
    }
};

template <typename PointRelation, typename MinRelation, typename MaxRelation, typename Indexable>
struct distances_calc<
    detail::bounded<PointRelation, MinRelation, MaxRelation>,
    Indexable,
    value_tag
>
{
    typedef typename detail::relation<PointRelation>::value_type point_type;
    typedef typename detail::relation<PointRelation>::tag point_relation_tag;
    typedef typename geometry::default_distance_result<point_type, Indexable>::type distance_type;
    typedef typename detail::relation<MinRelation>::tag min_relation_tag;
    typedef typename detail::relation<MaxRelation>::tag max_relation_tag;

    typedef typename detail::cdist_merge<
        typename detail::cdist_merge<
            cdist<distance_type, point_relation_tag>,
            cdist<distance_type, min_relation_tag>
        >::type,
        cdist<distance_type, max_relation_tag>
    >::type result_type;

    static inline result_type apply(
        detail::bounded<PointRelation, MinRelation, MaxRelation> const& pp,
        Indexable const& i)
    {
        result_type res;
        distances_calc_impl<result_type, point_type, Indexable>
            ::apply(res, relation<PointRelation>::value(pp.point_relation), i);
        return res;
    }
};

// ------------------------------------------------------------------ //
// distance_predicates_check for value_tag
// ------------------------------------------------------------------ //

template <typename PointRelation, typename Indexable>
struct distances_predicates_check<PointRelation, Indexable, value_tag>
{
    template <typename Distances>
    static inline bool apply(PointRelation const&, Distances const&)
    {
        return true;
    }
};

template <typename PointRelation, typename Indexable>
struct distances_predicates_check<
    detail::unbounded<PointRelation>,
    Indexable,
    value_tag
>
{
    template <typename Distances>
    static inline bool apply(detail::unbounded<PointRelation> const&, Distances const&)
    {
        return true;
    }
};

template <typename PointRelation, typename MinRelation, typename Indexable>
struct distances_predicates_check<
    detail::min_bounded<PointRelation, MinRelation>,
    Indexable,
    value_tag
>
{
    typedef typename detail::relation<MinRelation>::tag min_relation_tag;

    template <typename Distances>
    static inline bool apply(
        detail::min_bounded<PointRelation, MinRelation> const& pred,
        Distances const& d)
    {
        return pred.comparable_min <=
            detail::cdist_value<Distances>::template get<min_relation_tag>(d);
    }
};

template <typename PointRelation, typename MaxRelation, typename Indexable>
struct distances_predicates_check<
    detail::max_bounded<PointRelation, MaxRelation>,
    Indexable,
    value_tag
>
{
    typedef typename detail::relation<MaxRelation>::tag max_relation_tag;

    template <typename Distances>
    static inline bool apply(
        detail::max_bounded<PointRelation, MaxRelation> const& pred,
        Distances const& d)
    {
        return pred.comparable_max <=
            detail::cdist_value<Distances>::template get<max_relation_tag>(d);
    }
};

template <typename PointRelation, typename MinRelation, typename MaxRelation, typename Indexable>
struct distances_predicates_check<
    detail::bounded<PointRelation, MinRelation, MaxRelation>,
    Indexable,
    value_tag
>
{
    typedef typename detail::relation<MinRelation>::tag min_relation_tag;
    typedef typename detail::relation<MaxRelation>::tag max_relation_tag;

    template <typename Distances>
    static inline bool apply(
        detail::bounded<PointRelation, MinRelation, MaxRelation> const& pred,
        Distances const& d)
    {
        return pred.comparable_min
                <= detail::cdist_value<Distances>::template get<min_relation_tag>(d)
            && detail::cdist_value<Distances>::template get<max_relation_tag>(d)
                <= pred.comparable_max;
    }
};

// ------------------------------------------------------------------ //
// distance_calc for envelope_tag
// ------------------------------------------------------------------ //

template <typename PointRelation, typename Box>
struct distances_calc<
    PointRelation,
    Box,
    envelope_tag>
{
    typedef typename detail::relation<PointRelation>::value_type point_type;
    typedef typename geometry::default_distance_result<point_type, Box>::type distance_type;

    typedef detail::cdist<distance_type, detail::to_nearest_tag> result_type;

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
    envelope_tag
>
{
    typedef typename detail::relation<PointRelation>::value_type point_type;
    typedef typename geometry::default_distance_result<point_type, Box>::type distance_type;

    typedef detail::cdist<distance_type, detail::to_nearest_tag> result_type;

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
    envelope_tag
>
{
    typedef typename detail::relation<PointRelation>::value_type point_type;
    typedef typename geometry::default_distance_result<point_type, Box>::type distance_type;
    
    typedef typename detail::cdist_merge<
        cdist<distance_type, detail::to_nearest_tag>,
        cdist<distance_type, detail::to_furthest_tag>
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
    envelope_tag
>
{
    typedef typename detail::relation<PointRelation>::value_type point_type;
    typedef typename geometry::default_distance_result<point_type, Box>::type distance_type;

    typedef cdist<distance_type, detail::to_nearest_tag> result_type;

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
    envelope_tag
>
{
    typedef typename detail::relation<PointRelation>::value_type point_type;
    typedef typename geometry::default_distance_result<point_type, Box>::type distance_type;

    typedef typename detail::cdist_merge<
        cdist<distance_type, detail::to_nearest_tag>,
        cdist<distance_type, detail::to_furthest_tag>
    >::type result_type;

    static inline result_type apply(detail::bounded<PointRelation, MinRelation, MaxRelation> const& pp, Box const& i)
    {
        result_type res;
        distances_calc_impl<result_type, point_type, Box>
            ::apply(res, relation<PointRelation>::value(pp.point_relation), i);
        return res;
    }
};

// ------------------------------------------------------------------ //
// distance_predicates_check for envelope_tag
// ------------------------------------------------------------------ //

template <typename PointRelation, typename Box>
struct distances_predicates_check<
    PointRelation,
    Box,
    envelope_tag>
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
    envelope_tag>
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
    envelope_tag>
{
    template <typename Distances>
    static inline bool apply(
        detail::min_bounded<PointRelation, MinRelation> const& pred,
        Distances const& d)
    {
        return pred.comparable_min
            <= cdist_value<Distances>::template get<detail::to_furthest_tag>(d);
    }
};

template <typename PointRelation, typename MaxRelation, typename Box>
struct distances_predicates_check<
    detail::max_bounded<PointRelation, MaxRelation>,
    Box,
    envelope_tag>
{
    template <typename Distances>
    static inline bool apply(
        detail::max_bounded<PointRelation, MaxRelation> const& pred,
        Distances const& d)
    {
        return cdist_value<Distances>::template get<detail::to_nearest_tag>(d)
            <= pred.comparable_max;
    }
};

template <typename PointRelation, typename MinRelation, typename MaxRelation, typename Box>
struct distances_predicates_check<
    detail::bounded<PointRelation, MinRelation, MaxRelation>,
    Box,
    envelope_tag>
{
    template <typename Distances>
    static inline bool apply(
        detail::bounded<PointRelation, MinRelation, MaxRelation> const& pred,
        Distances const& d)
    {
        return pred.comparable_min
                <= cdist_value<Distances>::template get<detail::to_furthest_tag>(d)
            && cdist_value<Distances>::template get<detail::to_nearest_tag>(d)
                <= pred.comparable_max;
    }
};

}}}} // namespace boost::geometry::index::detail

#endif // BOOST_GEOMETRY_INDEX_RTREE_DISTANCE_PREDICATES_HPP
