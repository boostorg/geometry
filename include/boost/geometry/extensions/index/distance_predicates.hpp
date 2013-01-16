// Boost.Geometry Index
//
// Spatial index distance predicates, calculators and checkers used in nearest neighbor query
//
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_DISTANCE_PREDICATES_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_DISTANCE_PREDICATES_HPP

#include <boost/geometry/extensions/index/algorithms/comparable_distance_near.hpp>
#include <boost/geometry/extensions/index/algorithms/comparable_distance_far.hpp>
#include <boost/geometry/extensions/index/algorithms/comparable_distance_centroid.hpp>

#include <boost/geometry/extensions/index/tuples.hpp>

/*!
\defgroup distance_predicates Distance predicates (boost::geometry::index::)
*/

namespace boost { namespace geometry { namespace index {

//TODO: awulkiew - consider storing values instead of const references
// it may be faster and it eliminates problems with storing of references to temporaries
// moreover user may use boost::cref

// TODO: awulkiew - what with coordinate systems other than cartesian?
// do comparable_distance returns distance in coordinate system of objects used?
// what if objects are in different systems?
// should index algorithms work exactly like comparable_distance or not?

namespace detail {

// relations

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

// relation

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

} // namespace detail

// relations generators

/*!
\brief Generate to_nearest() Point-Indexable relationship.

Generate a nearest query Point and Value's Indexable relationship while calculating
distances. This function may be used to define that knn query should calculate distances
as smallest as possible between query Point and Indexable's points. In other words it
should be the distance to the nearest Indexable's point. This function may be also used
to define distances bounds which indicates that Indexable's nearest point should be
closer or further than value v. This is default relation.

\ingroup distance_predicates

\tparam T   Type of wrapped object. This may be a Point for PointRelation or some Value for
            MinRelation or MaxRelation

\param v    Point or bound value.
*/
template <typename T>
detail::to_nearest<T> to_nearest(T const& v)
{
    return detail::to_nearest<T>(v);
}

/*!
\brief Generate to_centroid() Point-Indexable relationship.

Generate a nearest query Point and Value's Indexable relationship while calculating
distances. This function may be used to define that knn query should calculate distances
between query Point and Indexable's centroid. This function may be also used
to define distances bounds which indicates that Indexable's centroid should be
closer or further than value v.

\ingroup distance_predicates

\tparam T   Type of wrapped object. This may be a Point for PointRelation or some Value for
            MinRelation or MaxRelation

\param v    Point or bound value.
*/
template <typename T>
detail::to_centroid<T> to_centroid(T const& v)
{
    return detail::to_centroid<T>(v);
}

/*!
\brief Generate to_furthest() Point-Indexable relationship.

Generate a nearest query Point and Value's Indexable relationship while calculating
distances. This function may be used to define that knn query should calculate distances
as biggest as possible between query Point and Indexable's points. In other words it
should be the distance to the furthest Indexable's point. This function may be also used
to define distances bounds which indicates that Indexable's furthest point should be
closer or further than value v.

\ingroup distance_predicates

\tparam T   Type of wrapped object. This may be a Point for PointRelation or some Value for
            MinRelation or MaxRelation

\param v    Point or bound value.
*/
template <typename T>
detail::to_furthest<T> to_furthest(T const& v)
{
    return detail::to_furthest<T>(v);
}

// distance predicates

namespace detail {

// TODO: awulkiew - consider storing points instead of PointRelations in predicates below

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

} // namespace detail

// distance predicates generators

/*!
\brief Generate unbounded() distance predicate.

Generate a distance predicate. This defines distances bounds which are used by knn query.
This function indicates that there is no distance bounds and Values should be returned
if distances between Point and Indexable are the smallest. Distance calculation is defined
by PointRelation. This is default nearest predicate.

\ingroup distance_predicates

\tparam PointRelation   PointRelation type.

\param pr               The point relation. This may be generated by \c index::to_nearest(),
                        \c index::to_centroid() or \c index::to_furthest() with \c Point passed as a parameter.
*/
template <typename PointRelation>
inline detail::unbounded<PointRelation>
unbounded(PointRelation const& pr)
{
    return detail::unbounded<PointRelation>(pr);
}

/*!
\brief Generate min_bounded() distance predicate.

Generate a distance predicate. This defines distances bounds which are used by knn query.
This function indicates that Values should be returned only if distances between Point and
Indexable are greater or equal to some min_distance passed in MinRelation. Check for closest Value is
defined by PointRelation. So it is possible e.g. to return Values with centroids closest to some
Point but only if nearest points are further than some distance.

\ingroup distance_predicates

\tparam PointRelation   PointRelation type.
\tparam MinRelation     MinRelation type.

\param pr               The point relation. This may be generated by \c to_nearest(),
                        \c to_centroid() or \c to_furthest() with \c Point passed as a parameter.
\param minr             The minimum bound relation. This may be generated by \c to_nearest(),
                        \c to_centroid() or \c to_furthest() with distance value passed as a parameter.
*/
template <typename PointRelation, typename MinRelation>
inline detail::min_bounded<PointRelation, MinRelation>
min_bounded(PointRelation const& pr, MinRelation const& minr)
{
    return detail::min_bounded<PointRelation, MinRelation>(pr, minr);
}

/*!
\brief Generate max_bounded() distance predicate.

Generate a distance predicate. This defines distances bounds which are used by knn query.
This function indicates that Values should be returned only if distances between Point and
Indexable are lesser or equal to some max_distance passed in MaxRelation. Check for closest Value is
defined by PointRelation. So it is possible e.g. to return Values with centroids closest to some
Point but only if nearest points are closer than some distance.

\ingroup distance_predicates

\tparam PointRelation   PointRelation type.
\tparam MaxRelation     MaxRelation type.

\param pr               The point relation. This may be generated by \c to_nearest(),
                        \c to_centroid() or \c to_furthest() with \c Point passed as a parameter.
\param maxr             The maximum bound relation. This may be generated by \c to_nearest(),
                        \c to_centroid() or \c to_furthest() with distance value passed as a parameter.
*/
template <typename PointRelation, typename MaxRelation>
inline detail::max_bounded<PointRelation, MaxRelation>
max_bounded(PointRelation const& pr, MaxRelation const& maxr)
{
    return detail::max_bounded<PointRelation, MaxRelation>(pr, maxr);
}

/*!
\brief Generate bounded() distance predicate.

Generate a distance predicate. This defines distances bounds which are used by knn query.
This function indicates that Values should be returned only if distances between Point and
Indexable are greater or equal to some min_distance passed in MinRelation and lesser or equal to
some max_distance passed in MaxRelation. Check for closest Value is defined by PointRelation.
So it is possible e.g. to return Values with centroids closest to some Point but only if nearest
points are further than some distance and closer than some other distance.

\ingroup distance_predicates

\tparam PointRelation   PointRelation type.
\tparam MinRelation     MinRelation type.
\tparam MaxRelation     MaxRelation type.

\param pr               The point relation. This may be generated by \c to_nearest(),
                        \c to_centroid() or \c to_furthest() with \c Point passed as a parameter.
\param minr             The minimum bound relation. This may be generated by \c to_nearest(),
                        \c to_centroid() or \c to_furthest() with distance value passed as a parameter.
\param maxr             The maximum bound relation. This may be generated by \c to_nearest(),
                        \c to_centroid() or \c to_furthest() with distance value passed as a parameter.
*/
template <typename PointRelation, typename MinRelation, typename MaxRelation>
inline detail::bounded<PointRelation, MinRelation, MaxRelation>
bounded(PointRelation const& pr, MinRelation const& minr, MaxRelation const& maxr)
{
    return detail::bounded<PointRelation, MinRelation, MaxRelation>(pr, minr, maxr);
}

// algorithms

namespace detail{

// point_relation

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


} // namespace detail

namespace detail
{

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
    typedef typename index::tuples::add_unique<
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
            index::tuples::find_index<
                CDistTuple,
                cdist<type, Tag>
            >::value
        >(cdtup).value;
    }

    template <typename Tag>
    static inline type const& get(CDistTuple const& cdtup)
    {
        return boost::get<
            index::tuples::find_index<
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
            (tuples::equal< cdist<T, Tag2>, cdist<T, Tag> >::value),
            TAGS_DO_NOT_MATCH,
            (cdist_value));

        return cd.value;
    }

    template <typename Tag2>
    static inline type const& get(cdist<T, Tag> const& cd)
    {
        BOOST_MPL_ASSERT_MSG(
            (tuples::equal< cdist<T, Tag2>, cdist<T, Tag> >::value),
            TAGS_DO_NOT_MATCH,
            (cdist_value));

        return cd.value;
    }
};

} // namespace detail

namespace detail {

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
        return index::comparable_distance_near(p, i);
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
        return index::comparable_distance_centroid(p, i);
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
        return index::comparable_distance_far(p, i);
    }
};

// distances_calc_impl_tuple

template <typename Distances, typename Point, typename Indexable, size_t N>
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

// distances_calc

template <typename PointRelation, typename Indexable, typename Tag>
struct distances_calc
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

template <typename PointRelation, typename Indexable, typename Tag>
struct distances_calc<
    detail::unbounded<PointRelation>,
    Indexable,
    Tag
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

template <typename PointRelation, typename MinRelation, typename Indexable, typename Tag>
struct distances_calc<
    detail::min_bounded<PointRelation, MinRelation>,
    Indexable,
    Tag
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

template <typename PointRelation, typename MaxRelation, typename Indexable, typename Tag>
struct distances_calc<
    detail::max_bounded<PointRelation, MaxRelation>,
    Indexable,
    Tag
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

template <typename PointRelation, typename MinRelation, typename MaxRelation, typename Indexable, typename Tag>
struct distances_calc<
    detail::bounded<PointRelation, MinRelation, MaxRelation>,
    Indexable,
    Tag
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

// distances_predicates_check

// TODO explicitly define Distances type ?
// TODO move distance_calc and distance_comp into geometry::index ?

template <typename PointRelation, typename Indexable, typename Tag>
struct distances_predicates_check
{
    template <typename Distances>
    static inline bool apply(PointRelation const&, Distances const&)
    {
        return true;
    }
};

template <typename PointRelation, typename Indexable, typename Tag>
struct distances_predicates_check<
    detail::unbounded<PointRelation>,
    Indexable,
    Tag
>
{
    template <typename Distances>
    static inline bool apply(detail::unbounded<PointRelation> const&, Distances const&)
    {
        return true;
    }
};

template <typename PointRelation, typename MinRelation, typename Indexable, typename Tag>
struct distances_predicates_check<
    detail::min_bounded<PointRelation, MinRelation>,
    Indexable,
    Tag
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

template <typename PointRelation, typename MaxRelation, typename Indexable, typename Tag>
struct distances_predicates_check<
    detail::max_bounded<PointRelation, MaxRelation>,
    Indexable,
    Tag
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

template <typename PointRelation, typename MinRelation, typename MaxRelation, typename Indexable, typename Tag>
struct distances_predicates_check<
    detail::bounded<PointRelation, MinRelation, MaxRelation>,
    Indexable,
    Tag
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

// TODO calculate all of this for Boxes only!
// for Points calculate only 1 distance allways

} // namespace detail

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_DISTANCE_PREDICATES_HPP
