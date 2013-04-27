// Boost.Geometry Index
//
// Spatial query predicates definition and checks.
//
// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_INDEX_DETAIL_PREDICATES_HPP
#define BOOST_GEOMETRY_INDEX_DETAIL_PREDICATES_HPP

#include <boost/geometry/index/predicates.hpp>
#include <boost/geometry/index/detail/tags.hpp>

namespace boost { namespace geometry { namespace index { namespace detail {

// ------------------------------------------------------------------ //
// predicates
// ------------------------------------------------------------------ //

//struct empty {};

template <typename Fun, bool IsFunction>
struct satisfies_impl
{
    satisfies_impl(Fun f) : fun(f) {}
    Fun * fun;
};

template <typename Fun>
struct satisfies_impl<Fun, false>
{
    satisfies_impl(Fun const& f) : fun(f) {}
    Fun fun;
};

template <typename Fun>
struct satisfies
    : satisfies_impl<Fun, ::boost::is_function<Fun>::value>
{
    typedef satisfies_impl<Fun, ::boost::is_function<Fun>::value> base;

    satisfies(Fun const& f) : base(f) {}
};

template <typename Geometry>
struct covered_by
{
    covered_by(Geometry const& g) : geometry(g) {}
    Geometry geometry;
};

template <typename Geometry>
struct disjoint
{
    disjoint(Geometry const& g) : geometry(g) {}
    Geometry geometry;
};

template <typename Geometry>
struct intersects
{
    intersects(Geometry const& g) : geometry(g) {}
    Geometry geometry;
};

template <typename Geometry>
struct overlaps
{
    overlaps(Geometry const& g) : geometry(g) {}
    Geometry geometry;
};

//template <typename Geometry>
//struct touches
//{
//    touches(Geometry const& g) : geometry(g) {}
//    Geometry geometry;
//};

template <typename Geometry>
struct within
{
    within(Geometry const& g) : geometry(g) {}
    Geometry geometry;
};

template <typename Geometry>
struct not_covered_by
{
    not_covered_by(Geometry const& g) : geometry(g) {}
    Geometry geometry;
};

template <typename Geometry>
struct not_disjoint
{
    not_disjoint(Geometry const& g) : geometry(g) {}
    Geometry geometry;
};

template <typename Geometry>
struct not_intersects
{
    not_intersects(Geometry const& g) : geometry(g) {}
    Geometry geometry;
};

template <typename Geometry>
struct not_overlaps
{
    not_overlaps(Geometry const& g) : geometry(g) {}
    Geometry geometry;
};

//template <typename Geometry>
//struct not_touches
//{
//    not_touches(Geometry const& g) : geometry(g) {}
//    Geometry geometry;
//};

template <typename Geometry>
struct not_within
{
    not_within(Geometry const& g) : geometry(g) {}
    Geometry geometry;
};

template <typename DistancePredicates>
struct nearest
{
    nearest(DistancePredicates const& dpred, unsigned k)
        : distance_predicates(dpred)
        , count(k)
    {}
    DistancePredicates distance_predicates;
    unsigned count;
};

// ------------------------------------------------------------------ //
// is_predicate
// ------------------------------------------------------------------ //

//template <typename P> struct is_predicate { static const bool value = false; };
////template <> struct is_predicate< empty > { static const bool value = true; };
//template <typename UP> struct is_predicate< satisfies<UP> > { static const bool value = true; };
//template <typename G> struct is_predicate< covered_by<G> > { static const bool value = true; };
//template <typename G> struct is_predicate< disjoint<G> > { static const bool value = true; };
//template <typename G> struct is_predicate< intersects<G> > { static const bool value = true; };
//template <typename G> struct is_predicate< overlaps<G> > { static const bool value = true; };
////template <typename G> struct is_predicate< touches<G> > { static const bool value = true; };
//template <typename G> struct is_predicate< within<G> > { static const bool value = true; };
//template <typename G> struct is_predicate< not_covered_by<G> > { static const bool value = true; };
//template <typename G> struct is_predicate< not_disjoint<G> > { static const bool value = true; };
//template <typename G> struct is_predicate< not_intersects<G> > { static const bool value = true; };
//template <typename G> struct is_predicate< not_overlaps<G> > { static const bool value = true; };
////template <typename G> struct is_predicate< not_touches<G> > { static const bool value = true; };
//template <typename G> struct is_predicate< not_within<G> > { static const bool value = true; };
//template <typename P> struct is_predicate< nearest<P> > { static const bool value = true; };

// ------------------------------------------------------------------ //
// predicate_check_default
// ------------------------------------------------------------------ //

//template <typename GeometryOrUnary, typename GeometryTag, typename Tag>
//struct predicate_check_default
//{
//    BOOST_MPL_ASSERT_MSG(
//        (false),
//        NOT_IMPLEMENTED_FOR_THESE_TAGS,
//        (predicate_check_default));
//};

// ------------------------------------------------------------------ //
// predicate_check
// ------------------------------------------------------------------ //

template <typename Predicate, typename Tag>
struct predicate_check
{
    BOOST_MPL_ASSERT_MSG(
        (false),
        NOT_IMPLEMENTED_FOR_THIS_PREDICATE_OR_TAG,
        (predicate_check));
};

// ------------------------------------------------------------------ //
// predicate_check_default for value
// ------------------------------------------------------------------ //

//template <typename Geometry, typename GeometryTag>
//struct predicate_check_default<Geometry, GeometryTag, value_tag>
//{
//    template <typename Value, typename Indexable>
//    static inline bool apply(Geometry const& g, Value const&, Indexable const& i)
//    {
//        return geometry::intersects(i, g);
//    }
//};
//
//template <typename Unary>
//struct predicate_check_default<Unary, void, value_tag>
//{
//    template <typename Value, typename Indexable>
//    static inline bool apply(Unary const& u, Value const& v, Indexable const&)
//    {
//        return u(v);
//    }
//};

// ------------------------------------------------------------------ //
// predicate_check for value
// ------------------------------------------------------------------ //

//template <typename GeometryOrUnary>
//struct predicate_check<GeometryOrUnary, value_tag>
//{
//    template <typename Value, typename Indexable>
//    static inline bool apply(GeometryOrUnary const& g, Value const& v, Indexable const& i)
//    {
//        return predicate_check_default<
//            GeometryOrUnary, typename geometry::traits::tag<GeometryOrUnary>::type, bounds_tag
//        >::apply(g, v, i);
//    }
//};

//template <>
//struct predicate_check<empty, value_tag>
//{
//    template <typename Value, typename Indexable>
//    static inline bool apply(empty const&, Value const&, Indexable const&)
//    {
//        return true;
//    }
//};

template <typename Fun>
struct predicate_check<satisfies<Fun>, value_tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(satisfies<Fun> const& p, Value const& v, Indexable const&)
    {
        return p.fun(v);
    }
};

template <typename Geometry>
struct predicate_check<covered_by<Geometry>, value_tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(covered_by<Geometry> const& p, Value const&, Indexable const& i)
    {
        return geometry::covered_by(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<disjoint<Geometry>, value_tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(disjoint<Geometry> const& p, Value const&, Indexable const& i)
    {
        return geometry::disjoint(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<intersects<Geometry>, value_tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(intersects<Geometry> const& p, Value const&, Indexable const& i)
    {
        return geometry::intersects(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<overlaps<Geometry>, value_tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(overlaps<Geometry> const& p, Value const&, Indexable const& i)
    {
        return geometry::overlaps(i, p.geometry);
    }
};

//template <typename Geometry>
//struct predicate_check<touches<Geometry>, value_tag>
//{
//    template <typename Value, typename Indexable>
//    static inline bool apply(touches<Geometry> const& p, Value const&, Indexable const& i)
//    {
//        return geometry::touches(i, p.geometry);
//    }
//};

template <typename Geometry>
struct predicate_check<within<Geometry>, value_tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(within<Geometry> const& p, Value const&, Indexable const& i)
    {
        return geometry::within(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<not_covered_by<Geometry>, value_tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(not_covered_by<Geometry> const& p, Value const&, Indexable const& i)
    {
        return !geometry::covered_by(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<not_disjoint<Geometry>, value_tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(not_disjoint<Geometry> const& p, Value const&, Indexable const& i)
    {
        return !geometry::disjoint(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<not_intersects<Geometry>, value_tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(not_intersects<Geometry> const& p, Value const&, Indexable const& i)
    {
        return !geometry::intersects(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<not_overlaps<Geometry>, value_tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(not_overlaps<Geometry> const& p, Value const&, Indexable const& i)
    {
        return !geometry::overlaps(i, p.geometry);
    }
};

//template <typename Geometry>
//struct predicate_check<not_touches<Geometry>, value_tag>
//{
//    template <typename Value, typename Indexable>
//    static inline bool apply(not_touches<Geometry> const& p, Value const&, Indexable const& i)
//    {
//        return !geometry::touches(i, p.geometry);
//    }
//};

template <typename Geometry>
struct predicate_check<not_within<Geometry>, value_tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(not_within<Geometry> const& p, Value const&, Indexable const& i)
    {
        return !geometry::within(i, p.geometry);
    }
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

// ------------------------------------------------------------------ //
// predicate_check_default for bounds
// ------------------------------------------------------------------ //

//template <typename Geometry, typename GeometryTag>
//struct predicate_check_default<Geometry, GeometryTag, bounds_tag>
//{
//    template <typename Value, typename Indexable>
//    static inline bool apply(Geometry const& g, Value const&, Indexable const& i)
//    {
//        return geometry::intersects(i, g);
//    }
//};
//
//template <typename Unary>
//struct predicate_check_default<Unary, void, bounds_tag>
//{
//    template <typename Value, typename Indexable>
//    static inline bool apply(Unary const&, Value const&, Indexable const&)
//    {
//        return true;
//    }
//};

// ------------------------------------------------------------------ //
// predicates_chec for bounds
// ------------------------------------------------------------------ //

//template <typename GeometryOrUnary>
//struct predicate_check<GeometryOrUnary, bounds_tag>
//{
//    template <typename Value, typename Indexable>
//    static inline bool apply(GeometryOrUnary const& g, Value const& v, Indexable const& i)
//    {
//        return predicate_check_default<
//            GeometryOrUnary, typename geometry::traits::tag<GeometryOrUnary>::type, bounds_tag
//        >::apply(g, v, i);
//    }
//};

//template <>
//struct predicate_check<empty, bounds_tag>
//{
//    template <typename Geometry, typename Value, typename Indexable>
//    static inline bool apply(Geometry const&, Value const&, Indexable const&)
//    {
//        return true;
//    }
//};

template <typename Fun>
struct predicate_check<satisfies<Fun>, bounds_tag>
{
    template <typename Value, typename Box>
    static bool apply(satisfies<Fun> const&, Value const&, Box const&)
    {
        return true;
    }
};

template <typename Geometry>
struct predicate_check<covered_by<Geometry>, bounds_tag>
{
    template <typename Value, typename Box>
    static bool apply(covered_by<Geometry> const& p, Value const&, Box const& i)
    {
        return geometry::intersects(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<disjoint<Geometry>, bounds_tag>
{
    template <typename Value, typename Box>
    static bool apply(disjoint<Geometry> const& p, Value const&, Box const& i)
    {
        return !geometry::covered_by(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<intersects<Geometry>, bounds_tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(intersects<Geometry> const& p, Value const&, Indexable const& i)
    {
        return geometry::intersects(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<overlaps<Geometry>, bounds_tag>
{
    template <typename Value, typename Box>
    static inline bool apply(overlaps<Geometry> const& p, Value const&, Box const& i)
    {
        // TODO: awulkiew - possibly change to the version without border case
        // e.g. intersects_without_border(0,0x1,1, 1,1x2,2) should give false
        return geometry::intersects(i, p.geometry);
    }
};

//template <typename Geometry>
//struct predicate_check<touches<Geometry>, bounds_tag>
//{
//    template <typename Value, typename Box>
//    static bool apply(touches<Geometry> const& p, Value const&, Box const& i)
//    {
//        return geometry::intersects(i, p.geometry);
//    }
//};

template <typename Geometry>
struct predicate_check<within<Geometry>, bounds_tag>
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
struct predicate_check<not_covered_by<Geometry>, bounds_tag>
{
    template <typename Value, typename Box>
    static bool apply(not_covered_by<Geometry> const& p, Value const&, Box const& i)
    {
        return !geometry::covered_by(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<not_disjoint<Geometry>, bounds_tag>
{
    template <typename Value, typename Box>
    static bool apply(not_disjoint<Geometry> const& p, Value const&, Box const& i)
    {
        return !geometry::disjoint(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<not_intersects<Geometry>, bounds_tag>
{
    template <typename Value, typename Box>
    static bool apply(not_intersects<Geometry> const& p, Value const&, Box const& i)
    {
        return !geometry::covered_by(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<not_overlaps<Geometry>, bounds_tag>
{
    template <typename Value, typename Box>
    static bool apply(not_overlaps<Geometry> const& , Value const&, Box const& )
    {
        return true;
    }
};

//template <typename Geometry>
//struct predicate_check<not_touches<Geometry>, bounds_tag>
//{
//    template <typename Value, typename Box>
//    static bool apply(not_touches<Geometry> const& p, Value const&, Box const& i)
//    {
//        return !geometry::intersects(i, p.geometry);
//    }
//};

template <typename Geometry>
struct predicate_check<not_within<Geometry>, bounds_tag>
{
    template <typename Value, typename Box>
    static bool apply(not_within<Geometry> const& p, Value const&, Box const& i)
    {
        return !geometry::within(i, p.geometry);
    }
};

template <typename DistancePredicates>
struct predicate_check<nearest<DistancePredicates>, bounds_tag>
{
    template <typename Value, typename Box>
    static inline bool apply(nearest<DistancePredicates> const&, Value const&, Box const&)
    {
        return true;
    }
};

// ------------------------------------------------------------------ //
// predicates_length
// ------------------------------------------------------------------ //

template <typename T>
struct predicates_length
{
    static const unsigned value = 1;
};

//template <typename F, typename S>
//struct predicates_length< std::pair<F, S> >
//{
//    static const unsigned value = 2;
//};

//template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
//struct predicates_length< boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> >
//{
//    static const unsigned value = boost::tuples::length< boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> >::value;
//};

template <typename Head, typename Tail>
struct predicates_length< boost::tuples::cons<Head, Tail> >
{
    static const unsigned value = boost::tuples::length< boost::tuples::cons<Head, Tail> >::value;
};

// ------------------------------------------------------------------ //
// predicates_element
// ------------------------------------------------------------------ //

template <unsigned I, typename T>
struct predicates_element
{
    BOOST_MPL_ASSERT_MSG((I < 1), INVALID_INDEX, (predicates_element));
    typedef T type;
    static type const& get(T const& p) { return p; }
};

//template <unsigned I, typename F, typename S>
//struct predicates_element< I, std::pair<F, S> >
//{
//    BOOST_MPL_ASSERT_MSG((I < 2), INVALID_INDEX, (predicates_element));
//
//    typedef F type;
//    static type const& get(std::pair<F, S> const& p) { return p.first; }
//};
//
//template <typename F, typename S>
//struct predicates_element< 1, std::pair<F, S> >
//{
//    typedef S type;
//    static type const& get(std::pair<F, S> const& p) { return p.second; }
//};
//
//template <unsigned I, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
//struct predicates_element< I, boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> >
//{
//    typedef boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> predicate_type;
//
//    typedef typename boost::tuples::element<I, predicate_type>::type type;
//    static type const& get(predicate_type const& p) { return boost::get<I>(p); }
//};

template <unsigned I, typename Head, typename Tail>
struct predicates_element< I, boost::tuples::cons<Head, Tail> >
{
    typedef boost::tuples::cons<Head, Tail> predicate_type;

    typedef typename boost::tuples::element<I, predicate_type>::type type;
    static type const& get(predicate_type const& p) { return boost::get<I>(p); }
};

// ------------------------------------------------------------------ //
// predicates_check
// ------------------------------------------------------------------ //

//template <typename PairPredicates, typename Tag, unsigned First, unsigned Last>
//struct predicates_check_pair {};
//
//template <typename PairPredicates, typename Tag, unsigned I>
//struct predicates_check_pair<PairPredicates, Tag, I, I>
//{
//    template <typename Value, typename Indexable>
//    static inline bool apply(PairPredicates const& , Value const& , Indexable const& )
//    {
//        return true;
//    }
//};
//
//template <typename PairPredicates, typename Tag>
//struct predicates_check_pair<PairPredicates, Tag, 0, 1>
//{
//    template <typename Value, typename Indexable>
//    static inline bool apply(PairPredicates const& p, Value const& v, Indexable const& i)
//    {
//        return predicate_check<typename PairPredicates::first_type, Tag>::apply(p.first, v, i);
//    }
//};
//
//template <typename PairPredicates, typename Tag>
//struct predicates_check_pair<PairPredicates, Tag, 1, 2>
//{
//    template <typename Value, typename Indexable>
//    static inline bool apply(PairPredicates const& p, Value const& v, Indexable const& i)
//    {
//        return predicate_check<typename PairPredicates::second_type, Tag>::apply(p.second, v, i);
//    }
//};
//
//template <typename PairPredicates, typename Tag>
//struct predicates_check_pair<PairPredicates, Tag, 0, 2>
//{
//    template <typename Value, typename Indexable>
//    static inline bool apply(PairPredicates const& p, Value const& v, Indexable const& i)
//    {
//        return predicate_check<typename PairPredicates::first_type, Tag>::apply(p.first, v, i)
//            && predicate_check<typename PairPredicates::second_type, Tag>::apply(p.second, v, i);
//    }
//};

template <typename TuplePredicates, typename Tag, unsigned First, unsigned Last>
struct predicates_check_tuple
{
    template <typename Value, typename Indexable>
    static inline bool apply(TuplePredicates const& p, Value const& v, Indexable const& i)
    {
        return
        predicate_check<
            typename boost::tuples::element<First, TuplePredicates>::type,
            Tag
        >::apply(boost::get<First>(p), v, i) &&
        predicates_check_tuple<TuplePredicates, Tag, First+1, Last>::apply(p, v, i);
    }
};

template <typename TuplePredicates, typename Tag, unsigned First>
struct predicates_check_tuple<TuplePredicates, Tag, First, First>
{
    template <typename Value, typename Indexable>
    static inline bool apply(TuplePredicates const& , Value const& , Indexable const& )
    {
        return true;
    }
};

template <typename Predicate, typename Tag, unsigned First, unsigned Last>
struct predicates_check_impl
{
    static const bool check = First < 1 && Last <= 1 && First <= Last;
    BOOST_MPL_ASSERT_MSG((check), INVALID_INDEXES, (predicates_check_impl));

    template <typename Value, typename Indexable>
    static inline bool apply(Predicate const& p, Value const& v, Indexable const& i)
    {
        return predicate_check<Predicate, Tag>::apply(p, v, i);
    }
};

//template <typename Predicate1, typename Predicate2, typename Tag, size_t First, size_t Last>
//struct predicates_check_impl<std::pair<Predicate1, Predicate2>, Tag, First, Last>
//{
//    BOOST_MPL_ASSERT_MSG((First < 2 && Last <= 2 && First <= Last), INVALID_INDEXES, (predicates_check_impl));
//
//    template <typename Value, typename Indexable>
//    static inline bool apply(std::pair<Predicate1, Predicate2> const& p, Value const& v, Indexable const& i)
//    {
//        return predicate_check<Predicate1, Tag>::apply(p.first, v, i)
//            && predicate_check<Predicate2, Tag>::apply(p.second, v, i);
//    }
//};
//
//template <
//    typename T0, typename T1, typename T2, typename T3, typename T4,
//    typename T5, typename T6, typename T7, typename T8, typename T9,
//    typename Tag, unsigned First, unsigned Last
//>
//struct predicates_check_impl<
//    boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>,
//    Tag, First, Last
//>
//{
//    typedef boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> predicates_type;
//
//    static const unsigned pred_len = boost::tuples::length<predicates_type>::value;
//    BOOST_MPL_ASSERT_MSG((First < pred_len && Last <= pred_len && First <= Last), INVALID_INDEXES, (predicates_check_impl));
//
//    template <typename Value, typename Indexable>
//    static inline bool apply(predicates_type const& p, Value const& v, Indexable const& i)
//    {
//        return predicates_check_tuple<
//            predicates_type,
//            Tag, First, Last
//        >::apply(p, v, i);
//    }
//};

template <typename Head, typename Tail, typename Tag, unsigned First, unsigned Last>
struct predicates_check_impl<
    boost::tuples::cons<Head, Tail>,
    Tag, First, Last
>
{
    typedef boost::tuples::cons<Head, Tail> predicates_type;

    static const unsigned pred_len = boost::tuples::length<predicates_type>::value;
    static const bool check = First < pred_len && Last <= pred_len && First <= Last;
    BOOST_MPL_ASSERT_MSG((check), INVALID_INDEXES, (predicates_check_impl));

    template <typename Value, typename Indexable>
    static inline bool apply(predicates_type const& p, Value const& v, Indexable const& i)
    {
        return predicates_check_tuple<
            predicates_type,
            Tag, First, Last
        >::apply(p, v, i);
    }
};

template <typename Tag, unsigned First, unsigned Last, typename Predicates, typename Value, typename Indexable>
inline bool predicates_check(Predicates const& p, Value const& v, Indexable const& i)
{
    return detail::predicates_check_impl<Predicates, Tag, First, Last>
        ::apply(p, v, i);
}

// ------------------------------------------------------------------ //
// nearest predicate helpers
// ------------------------------------------------------------------ //

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

// predicates_count_nearest

template <typename T>
struct predicates_count_nearest
{
    static const unsigned value = predicates_is_nearest<T>::value;
};

//template <typename F, typename S>
//struct predicates_count_nearest< std::pair<F, S> >
//{
//    static const unsigned value = predicates_is_nearest<F>::value
//                                + predicates_is_nearest<S>::value;
//};

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

//template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
//struct predicates_count_nearest< boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> >
//{
//    static const unsigned value = predicates_count_nearest_tuple<
//        boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>,
//        boost::tuples::length< boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> >::value
//    >::value;
//};

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

//template <typename F, typename S>
//struct predicates_find_nearest< std::pair<F, S> >
//{
//    static const unsigned value = predicates_is_nearest<F>::value ? 0 :
//                                    (predicates_is_nearest<S>::value ? 1 : 2);
//};

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

//template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
//struct predicates_find_nearest< boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> >
//{
//    static const unsigned value = predicates_find_nearest_tuple<
//        boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>,
//        boost::tuples::length< boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> >::value
//    >::value;
//};

template <typename Head, typename Tail>
struct predicates_find_nearest< boost::tuples::cons<Head, Tail> >
{
    static const unsigned value = predicates_find_nearest_tuple<
        boost::tuples::cons<Head, Tail>,
        boost::tuples::length< boost::tuples::cons<Head, Tail> >::value
    >::value;
};

}}}} // namespace boost::geometry::index::detail

#endif // BOOST_GEOMETRY_INDEX_DETAIL_PREDICATES_HPP
