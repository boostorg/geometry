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
    satisfies(base const& b) : base(b) {}
};

template <typename Fun>
struct not_satisfies
    : satisfies_impl<Fun, ::boost::is_function<Fun>::value>
{
    typedef satisfies_impl<Fun, ::boost::is_function<Fun>::value> base;

    not_satisfies(Fun const& f) : base(f) {}
    not_satisfies(base const& b) : base(b) {}
};

// ------------------------------------------------------------------ //

struct covered_by_tag {};
struct within_tag {};
struct disjoint_tag {};
struct intersects_tag {};
struct overlaps_tag {};
struct touches_tag {};

template <typename Geometry, typename Tag, bool Negated, bool Reversed>
struct spatial_predicate
{
    spatial_predicate(Geometry const& g) : geometry(g) {}
    Geometry geometry;
};

// ------------------------------------------------------------------ //

template <typename PointOrRelation>
struct nearest
{
    nearest(PointOrRelation const& por, unsigned k)
        : point_or_relation(por)
        , count(k)
    {}
    PointOrRelation point_or_relation;
    unsigned count;
};

template <typename SegmentOrLinestring>
struct path
{
    path(SegmentOrLinestring const& g, unsigned k)
        : geometry(g)
        , count(k)
    {}
    SegmentOrLinestring geometry;
    unsigned count;
};

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

template <typename Fun>
struct predicate_check<satisfies<Fun>, value_tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(satisfies<Fun> const& p, Value const& v, Indexable const&)
    {
        return p.fun(v);
    }
};

template <typename Fun>
struct predicate_check<not_satisfies<Fun>, value_tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(not_satisfies<Fun> const& p, Value const& v, Indexable const&)
    {
        return !p.fun(v);
    }
};

// ------------------------------------------------------------------ //

template <typename Tag>
struct spatial_predicate_call
{
    BOOST_MPL_ASSERT_MSG(false, NOT_IMPLEMENTED_FOR_THIS_TAG, (Tag));
};

template <>
struct spatial_predicate_call<covered_by_tag>
{
    template <typename G1, typename G2>
    static inline bool apply(G1 const& g1, G2 const& g2)
    {
        return geometry::covered_by(g1, g2);
    }
};

template <>
struct spatial_predicate_call<disjoint_tag>
{
    template <typename G1, typename G2>
    static inline bool apply(G1 const& g1, G2 const& g2)
    {
        return geometry::disjoint(g1, g2);
    }
};

template <>
struct spatial_predicate_call<intersects_tag>
{
    template <typename G1, typename G2>
    static inline bool apply(G1 const& g1, G2 const& g2)
    {
        return geometry::intersects(g1, g2);
    }
};

template <>
struct spatial_predicate_call<overlaps_tag>
{
    template <typename G1, typename G2>
    static inline bool apply(G1 const& g1, G2 const& g2)
    {
        return geometry::overlaps(g1, g2);
    }
};

template <>
struct spatial_predicate_call<touches_tag>
{
    template <typename G1, typename G2>
    static inline bool apply(G1 const& g1, G2 const& g2)
    {
        return geometry::touches(g1, g2);
    }
};

template <>
struct spatial_predicate_call<within_tag>
{
    template <typename G1, typename G2>
    static inline bool apply(G1 const& g1, G2 const& g2)
    {
        return geometry::within(g1, g2);
    }
};

// ------------------------------------------------------------------ //

// spatial predicate
template <typename Geometry, typename Tag>
struct predicate_check<spatial_predicate<Geometry, Tag, false, false>, value_tag>
{
    typedef spatial_predicate<Geometry, Tag, false, false> Pred;

    template <typename Value, typename Indexable>
    static inline bool apply(Pred const& p, Value const&, Indexable const& i)
    {
        return spatial_predicate_call<Tag>::apply(i, p.geometry);
    }
};

// negated spatial predicate
template <typename Geometry, typename Tag>
struct predicate_check<spatial_predicate<Geometry, Tag, true, false>, value_tag>
{
    typedef spatial_predicate<Geometry, Tag, true, false> Pred;

    template <typename Value, typename Indexable>
    static inline bool apply(Pred const& p, Value const&, Indexable const& i)
    {
        return !spatial_predicate_call<Tag>::apply(i, p.geometry);
    }
};

// reversed spatial predicate
template <typename Geometry, typename Tag>
struct predicate_check<spatial_predicate<Geometry, Tag, false, true>, value_tag>
{
    typedef spatial_predicate<Geometry, Tag, false, true> Pred;

    template <typename Value, typename Indexable>
    static inline bool apply(Pred const& p, Value const&, Indexable const& i)
    {
        return spatial_predicate_call<Tag>::apply(p.geometry, i);
    }
};

// negated reversed spatial predicate
template <typename Geometry, typename Tag>
struct predicate_check<spatial_predicate<Geometry, Tag, true, true>, value_tag>
{
    typedef spatial_predicate<Geometry, Tag, true, true> Pred;

    template <typename Value, typename Indexable>
    static inline bool apply(Pred const& p, Value const&, Indexable const& i)
    {
        return !spatial_predicate_call<Tag>::apply(p.geometry, i);
    }
};

// ------------------------------------------------------------------ //

template <typename DistancePredicates>
struct predicate_check<nearest<DistancePredicates>, value_tag>
{
    template <typename Value, typename Box>
    static inline bool apply(nearest<DistancePredicates> const&, Value const&, Box const&)
    {
        return true;
    }
};

template <typename Linestring>
struct predicate_check<path<Linestring>, value_tag>
{
    template <typename Value, typename Box>
    static inline bool apply(path<Linestring> const&, Value const&, Box const&)
    {
        return true;
    }
};

// ------------------------------------------------------------------ //
// predicates_check for bounds
// ------------------------------------------------------------------ //

template <typename Fun>
struct predicate_check<satisfies<Fun>, bounds_tag>
{
    template <typename Value, typename Box>
    static bool apply(satisfies<Fun> const&, Value const&, Box const&)
    {
        return true;
    }
};

template <typename Fun>
struct predicate_check<not_satisfies<Fun>, bounds_tag>
{
    template <typename Value, typename Box>
    static bool apply(not_satisfies<Fun> const&, Value const&, Box const&)
    {
        return true;
    }
};

// ------------------------------------------------------------------ //

// NOT REVERSED
// value_tag        bounds_tag
// ---------------------------
// covered_by(I,G)  intersects(I,G)
// disjoint(I,G)    !covered_by(I,G)
// intersects(I,G)  intersects(I,G)
// overlaps(I,G)    intersects(I,G)  - possibly change to the version without border case, e.g. intersects_without_border(0,0x1,1, 1,1x2,2) should give false
// touches(I,G)     intersects(I,G)
// within(I,G)      intersects(I,G)  - possibly change to the version without border case, e.g. intersects_without_border(0,0x1,1, 1,1x2,2) should give false

// spatial predicate - default
template <typename Geometry, typename Tag>
struct predicate_check<spatial_predicate<Geometry, Tag, false, false>, bounds_tag>
{
    typedef spatial_predicate<Geometry, Tag, false, false> Pred;

    template <typename Value, typename Indexable>
    static inline bool apply(Pred const& p, Value const&, Indexable const& i)
    {
        return spatial_predicate_call<intersects_tag>::apply(i, p.geometry);
    }
};

// spatial predicate - disjoint
template <typename Geometry>
struct predicate_check<spatial_predicate<Geometry, disjoint_tag, false, false>, bounds_tag>
{
    typedef spatial_predicate<Geometry, disjoint_tag, false, false> Pred;

    template <typename Value, typename Indexable>
    static inline bool apply(Pred const& p, Value const&, Indexable const& i)
    {
        return !spatial_predicate_call<covered_by_tag>::apply(i, p.geometry);
    }
};

// NEGATED, NOT REVERSED
// value_tag        bounds_tag
// ---------------------------
// !covered_by(I,G) !covered_by(I,G)
// !disjoint(I,G)   !disjoint(I,G)
// !intersects(I,G) !covered_by(I,G)
// !overlaps(I,G)   TRUE
// !touches(I,G)    !intersects(I,G)
// !within(I,G)     !within(I,G)

// negated spatial predicate - default
template <typename Geometry, typename Tag>
struct predicate_check<spatial_predicate<Geometry, Tag, true, false>, bounds_tag>
{
    typedef spatial_predicate<Geometry, Tag, true, false> Pred;

    template <typename Value, typename Indexable>
    static inline bool apply(Pred const& p, Value const&, Indexable const& i)
    {
        return !spatial_predicate_call<Tag>::apply(i, p.geometry);
    }
};

// negated spatial predicate - intersects
template <typename Geometry>
struct predicate_check<spatial_predicate<Geometry, intersects_tag, true, false>, bounds_tag>
{
    typedef spatial_predicate<Geometry, intersects_tag, true, false> Pred;

    template <typename Value, typename Indexable>
    static inline bool apply(Pred const& p, Value const&, Indexable const& i)
    {
        return !spatial_predicate_call<covered_by_tag>::apply(i, p.geometry);
    }
};

// negated spatial predicate - overlaps
template <typename Geometry>
struct predicate_check<spatial_predicate<Geometry, overlaps_tag, true, false>, bounds_tag>
{
    typedef spatial_predicate<Geometry, overlaps_tag, true, false> Pred;

    template <typename Value, typename Indexable>
    static inline bool apply(Pred const& p, Value const&, Indexable const& i)
    {
        return true;
    }
};

// negated spatial predicate - touches
template <typename Geometry>
struct predicate_check<spatial_predicate<Geometry, touches_tag, true, false>, bounds_tag>
{
    typedef spatial_predicate<Geometry, touches_tag, true, false> Pred;

    template <typename Value, typename Indexable>
    static inline bool apply(Pred const& p, Value const&, Indexable const& i)
    {
        return !spatial_predicate_call<intersects_tag>::apply(i, p.geometry);
    }
};

// REVERSED
// value_tag        bounds_tag
// ---------------------------
// covered_by(G,I)  covered_by(G,I)
// disjoint(G,I)    !covered_by(I,G)
// intersects(G,I)  intersects(I,G)
// overlaps(G,I)    intersects(I,G)  - possibly change to the version without border case, e.g. intersects_without_border(0,0x1,1, 1,1x2,2) should give false
// touches(G,I)     intersects(I,G)
// within(G,I)      within(G,I)

// reversed spatial predicate - default
template <typename Geometry, typename Tag>
struct predicate_check<spatial_predicate<Geometry, Tag, false, true>, bounds_tag>
{
    typedef spatial_predicate<Geometry, Tag, false, true> Pred;

    template <typename Value, typename Indexable>
    static inline bool apply(Pred const& p, Value const&, Indexable const& i)
    {
        return spatial_predicate_call<intersects_tag>::apply(i, p.geometry);
    }
};

// reversed spatial predicate - covered_by
template <typename Geometry>
struct predicate_check<spatial_predicate<Geometry, covered_by_tag, false, true>, bounds_tag>
{
    typedef spatial_predicate<Geometry, covered_by_tag, false, true> Pred;

    template <typename Value, typename Indexable>
    static inline bool apply(Pred const& p, Value const&, Indexable const& i)
    {
        return spatial_predicate_call<covered_by_tag>::apply(p.geometry, i);
    }
};

// reversed spatial predicate - disjoint
template <typename Geometry>
struct predicate_check<spatial_predicate<Geometry, disjoint_tag, false, true>, bounds_tag>
{
    typedef spatial_predicate<Geometry, disjoint_tag, false, true> Pred;

    template <typename Value, typename Indexable>
    static inline bool apply(Pred const& p, Value const&, Indexable const& i)
    {
        return !spatial_predicate_call<covered_by_tag>::apply(i, p.geometry);
    }
};

// reversed spatial predicate - within
template <typename Geometry>
struct predicate_check<spatial_predicate<Geometry, within_tag, false, true>, bounds_tag>
{
    typedef spatial_predicate<Geometry, within_tag, false, true> Pred;

    template <typename Value, typename Indexable>
    static inline bool apply(Pred const& p, Value const&, Indexable const& i)
    {
        return spatial_predicate_call<within_tag>::apply(p.geometry, i);
    }
};

// NEGATED, REVERSED
// value_tag        bounds_tag
// ---------------------------
// !covered_by(G,I) TRUE
// !disjoint(G,I)   !disjoint(I,G)
// !intersects(G,I) !covered_by(I,G)
// !overlaps(G,I)   TRUE
// !touches(G,I)    !intersects(I,G)
// !within(G,I)     TRUE

// negated reversed spatial predicate - default
template <typename Geometry, typename Tag>
struct predicate_check<spatial_predicate<Geometry, Tag, true, true>, bounds_tag>
{
    typedef spatial_predicate<Geometry, Tag, true, true> Pred;

    template <typename Value, typename Indexable>
    static inline bool apply(Pred const& p, Value const&, Indexable const& i)
    {
        return true;
    }
};

// negated reversed spatial predicate - disjoint
template <typename Geometry>
struct predicate_check<spatial_predicate<Geometry, disjoint_tag, true, true>, bounds_tag>
{
    typedef spatial_predicate<Geometry, disjoint_tag, true, true> Pred;

    template <typename Value, typename Indexable>
    static inline bool apply(Pred const& p, Value const&, Indexable const& i)
    {
        return !spatial_predicate_call<disjoint_tag>::apply(i, p.geometry);
    }
};

// negated reversed spatial predicate - disjoint
template <typename Geometry>
struct predicate_check<spatial_predicate<Geometry, intersects_tag, true, true>, bounds_tag>
{
    typedef spatial_predicate<Geometry, intersects_tag, true, true> Pred;

    template <typename Value, typename Indexable>
    static inline bool apply(Pred const& p, Value const&, Indexable const& i)
    {
        return !spatial_predicate_call<covered_by_tag>::apply(i, p.geometry);
    }
};

// negated reversed spatial predicate - touches
template <typename Geometry>
struct predicate_check<spatial_predicate<Geometry, touches_tag, true, true>, bounds_tag>
{
    typedef spatial_predicate<Geometry, touches_tag, true, true> Pred;

    template <typename Value, typename Indexable>
    static inline bool apply(Pred const& p, Value const&, Indexable const& i)
    {
        return !spatial_predicate_call<intersects_tag>::apply(i, p.geometry);
    }
};

// ------------------------------------------------------------------ //

template <typename DistancePredicates>
struct predicate_check<nearest<DistancePredicates>, bounds_tag>
{
    template <typename Value, typename Box>
    static inline bool apply(nearest<DistancePredicates> const&, Value const&, Box const&)
    {
        return true;
    }
};

template <typename Linestring>
struct predicate_check<path<Linestring>, bounds_tag>
{
    template <typename Value, typename Box>
    static inline bool apply(path<Linestring> const&, Value const&, Box const&)
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
struct predicates_is_distance
{
    static const unsigned value = 0;
};

template <typename DistancePredicates>
struct predicates_is_distance< nearest<DistancePredicates> >
{
    static const unsigned value = 1;
};

template <typename Linestring>
struct predicates_is_distance< path<Linestring> >
{
    static const unsigned value = 1;
};

// predicates_count_nearest

template <typename T>
struct predicates_count_distance
{
    static const unsigned value = predicates_is_distance<T>::value;
};

//template <typename F, typename S>
//struct predicates_count_distance< std::pair<F, S> >
//{
//    static const unsigned value = predicates_is_distance<F>::value
//                                + predicates_is_distance<S>::value;
//};

template <typename Tuple, unsigned N>
struct predicates_count_distance_tuple
{
    static const unsigned value =
        predicates_is_distance<typename boost::tuples::element<N-1, Tuple>::type>::value
        + predicates_count_distance_tuple<Tuple, N-1>::value;
};

template <typename Tuple>
struct predicates_count_distance_tuple<Tuple, 1>
{
    static const unsigned value =
        predicates_is_distance<typename boost::tuples::element<0, Tuple>::type>::value;
};

//template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
//struct predicates_count_distance< boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> >
//{
//    static const unsigned value = predicates_count_distance_tuple<
//        boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>,
//        boost::tuples::length< boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> >::value
//    >::value;
//};

template <typename Head, typename Tail>
struct predicates_count_distance< boost::tuples::cons<Head, Tail> >
{
    static const unsigned value = predicates_count_distance_tuple<
        boost::tuples::cons<Head, Tail>,
        boost::tuples::length< boost::tuples::cons<Head, Tail> >::value
    >::value;
};

// predicates_find_nearest

template <typename T>
struct predicates_find_distance
{
    static const unsigned value = predicates_is_distance<T>::value ? 0 : 1;
};

//template <typename F, typename S>
//struct predicates_find_distance< std::pair<F, S> >
//{
//    static const unsigned value = predicates_is_distance<F>::value ? 0 :
//                                    (predicates_is_distance<S>::value ? 1 : 2);
//};

template <typename Tuple, unsigned N>
struct predicates_find_distance_tuple
{
    static const bool is_found = predicates_find_distance_tuple<Tuple, N-1>::is_found
                                || predicates_is_distance<typename boost::tuples::element<N-1, Tuple>::type>::value;

    static const unsigned value = predicates_find_distance_tuple<Tuple, N-1>::is_found ?
        predicates_find_distance_tuple<Tuple, N-1>::value :
        (predicates_is_distance<typename boost::tuples::element<N-1, Tuple>::type>::value ?
            N-1 : boost::tuples::length<Tuple>::value);
};

template <typename Tuple>
struct predicates_find_distance_tuple<Tuple, 1>
{
    static const bool is_found = predicates_is_distance<typename boost::tuples::element<0, Tuple>::type>::value;
    static const unsigned value = is_found ? 0 : boost::tuples::length<Tuple>::value;
};

//template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
//struct predicates_find_distance< boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> >
//{
//    static const unsigned value = predicates_find_distance_tuple<
//        boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>,
//        boost::tuples::length< boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> >::value
//    >::value;
//};

template <typename Head, typename Tail>
struct predicates_find_distance< boost::tuples::cons<Head, Tail> >
{
    static const unsigned value = predicates_find_distance_tuple<
        boost::tuples::cons<Head, Tail>,
        boost::tuples::length< boost::tuples::cons<Head, Tail> >::value
    >::value;
};

}}}} // namespace boost::geometry::index::detail

#endif // BOOST_GEOMETRY_INDEX_DETAIL_PREDICATES_HPP
