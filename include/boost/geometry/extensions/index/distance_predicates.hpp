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

#include <boost/geometry/extensions/index/tuples.hpp>

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
struct near
{
    near(T const& v) : value(v) {}
    T value;
};

template <typename T>
struct centroid
{
    centroid(T const& v) : value(v) {}
    T value;
};

template <typename T>
struct far
{
    far(T const& v) : value(v) {}
    T value;
};

// tags

struct near_tag {};
struct centroid_tag {};
struct far_tag {};

// relation

template <typename T>
struct relation
{
    typedef T value_type;
    typedef near_tag tag;
    static inline T const& value(T const& v) { return v; }
    static inline T & value(T & v) { return v; }
};

template <typename T>
struct relation< near<T> >
{
    typedef T value_type;
    typedef near_tag tag;
    static inline T const& value(near<T> const& r) { return r.value; }
    static inline T & value(near<T> & r) { return r.value; }
};

template <typename T>
struct relation< centroid<T> >
{
    typedef T value_type;
    typedef centroid_tag tag;
    static inline T const& value(centroid<T> const& r) { return r.value; }
    static inline T & value(centroid<T> & r) { return r.value; }
};

template <typename T>
struct relation< far<T> >
{
    typedef T value_type;
    typedef far_tag tag;
    static inline T const& value(far<T> const& r) { return r.value; }
    static inline T & value(far<T> & r) { return r.value; }
};

} // namespace detail

// relations generators

template <typename T>
detail::near<T> near(T const& v)
{
    return detail::near<T>(v);
}

template <typename T>
detail::centroid<T> centroid(T const& v)
{
    return detail::centroid<T>(v);
}

template <typename T>
detail::far<T> far(T const& v)
{
    return detail::far<T>(v);
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

template <typename PointRelation>
inline detail::unbounded<PointRelation>
unbounded(PointRelation const& pr)
{
    return detail::unbounded<PointRelation>(pr);
}

template <typename PointRelation, typename MinRelation>
inline detail::min_bounded<PointRelation, MinRelation>
min_bounded(PointRelation const& pr, MinRelation const& minr)
{
    return detail::min_bounded<PointRelation, MinRelation>(pr, minr);
}

template <typename PointRelation, typename MaxRelation>
inline detail::max_bounded<PointRelation, MaxRelation>
max_bounded(PointRelation const& pr, MaxRelation const& maxr)
{
    return detail::max_bounded<PointRelation, MaxRelation>(pr, maxr);
}

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
        // TODO MPL_ASSERT tuples::equal<CDist, cdist<T, Tag>>::value
        return cd.value;
    }

    template <typename Tag2>
    static inline type const& get(cdist<T, Tag> const& cd)
    {
        // TODO MPL_ASSERT tuples::equal<CDist, cdist<T, Tag>>::value
        return cd.value;
    }
};

} // namespace detail

namespace detail {

// distances_calc_impl_rel

template <typename RelDist>
struct distances_calc_impl_rel
{
    // TODO MPL_ASSERT not implemented for this RelDist
};

template <typename T>
struct distances_calc_impl_rel<
    cdist<T, detail::near_tag>
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
    cdist<T, detail::centroid_tag>
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
    cdist<T, detail::far_tag>
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

// TODO explicitly define DistanceType ?
// Indexable/Box is used in distances_predicates_check only for purpose of
// this explicit DistanceType definition

// move distance_calc and distance_comp into geometry::index ?

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
