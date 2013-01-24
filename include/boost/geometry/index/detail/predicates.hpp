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

struct empty {};

template <typename Fun>
struct value
{
    value(Fun const& f) : fun(f) {}
    Fun fun;
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

// ------------------------------------------------------------------ //
// predicate_check
// ------------------------------------------------------------------ //

template <typename Geometry, typename Tag>
struct predicate_check
{
    BOOST_MPL_ASSERT_MSG(
        (false),
        NOT_IMPLEMENTED_FOR_THIS_TAG,
        (predicate_check));
};

// ------------------------------------------------------------------ //
// predicate_check for value
// ------------------------------------------------------------------ //

template <typename Geometry>
struct predicate_check<Geometry, value_tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(Geometry const& g, Value const&, Indexable const& i)
    {
        return geometry::intersects(i, g);
    }
};

template <>
struct predicate_check<empty, value_tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(empty const&, Value const&, Indexable const&)
    {
        return true;
    }
};

template <typename Fun>
struct predicate_check<value<Fun>, value_tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(value<Fun> const& p, Value const& v, Indexable const&)
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

// ------------------------------------------------------------------ //
// predicates_chec for envelope
// ------------------------------------------------------------------ //

template <typename Geometry>
struct predicate_check<Geometry, envelope_tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(Geometry const& g, Value const&, Indexable const& i)
    {
        return geometry::intersects(i, g);
    }
};

template <>
struct predicate_check<empty, envelope_tag>
{
    template <typename Geometry, typename Value, typename Indexable>
    static inline bool apply(Geometry const&, Value const&, Indexable const&)
    {
        return true;
    }
};

template <typename Fun>
struct predicate_check<value<Fun>, envelope_tag>
{
    template <typename Value, typename Box>
    static bool apply(value<Fun> const&, Value const&, Box const&)
    {
        return true;
    }
};

template <typename Geometry>
struct predicate_check<covered_by<Geometry>, envelope_tag>
{
    template <typename Value, typename Box>
    static bool apply(covered_by<Geometry> const& p, Value const&, Box const& i)
    {
        return geometry::intersects(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<disjoint<Geometry>, envelope_tag>
{
    template <typename Value, typename Box>
    static bool apply(disjoint<Geometry> const& p, Value const&, Box const& i)
    {
        return !geometry::covered_by(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<intersects<Geometry>, envelope_tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(intersects<Geometry> const& p, Value const&, Indexable const& i)
    {
        return geometry::intersects(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<overlaps<Geometry>, envelope_tag>
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
//struct predicate_check<touches<Geometry>, envelope_tag>
//{
//    template <typename Value, typename Box>
//    static bool apply(touches<Geometry> const& p, Value const&, Box const& i)
//    {
//        return geometry::intersects(i, p.geometry);
//    }
//};

template <typename Geometry>
struct predicate_check<within<Geometry>, envelope_tag>
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
struct predicate_check<not_covered_by<Geometry>, envelope_tag>
{
    template <typename Value, typename Box>
    static bool apply(not_covered_by<Geometry> const& p, Value const&, Box const& i)
    {
        return !geometry::covered_by(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<not_disjoint<Geometry>, envelope_tag>
{
    template <typename Value, typename Box>
    static bool apply(not_disjoint<Geometry> const& p, Value const&, Box const& i)
    {
        return !geometry::disjoint(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<not_intersects<Geometry>, envelope_tag>
{
    template <typename Value, typename Box>
    static bool apply(not_intersects<Geometry> const& p, Value const&, Box const& i)
    {
        return !geometry::covered_by(i, p.geometry);
    }
};

template <typename Geometry>
struct predicate_check<not_overlaps<Geometry>, envelope_tag>
{
    template <typename Value, typename Box>
    static bool apply(not_overlaps<Geometry> const& p, Value const&, Box const& i)
    {
        return true;
    }
};

//template <typename Geometry>
//struct predicate_check<not_touches<Geometry>, envelope_tag>
//{
//    template <typename Value, typename Box>
//    static bool apply(not_touches<Geometry> const& p, Value const&, Box const& i)
//    {
//        return !geometry::intersects(i, p.geometry);
//    }
//};

template <typename Geometry>
struct predicate_check<not_within<Geometry>, envelope_tag>
{
    template <typename Value, typename Box>
    static bool apply(not_within<Geometry> const& p, Value const&, Box const& i)
    {
        return !geometry::within(i, p.geometry);
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

template <typename F, typename S>
struct predicates_length< std::pair<F, S> >
{
    static const unsigned value = 2;
};

template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
struct predicates_length< boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> >
{
    static const unsigned value = boost::tuples::length< boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> >::value;
};

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

template <unsigned I, typename F, typename S>
struct predicates_element< I, std::pair<F, S> >
{
    BOOST_MPL_ASSERT_MSG((I < 2), INVALID_INDEX, (predicates_element));

    typedef F type;
    static type const& get(std::pair<F, S> const& p) { return p.first; }
};

template <typename F, typename S>
struct predicates_element< 1, std::pair<F, S> >
{
    typedef S type;
    static type const& get(std::pair<F, S> const& p) { return p.second; }
};

template <unsigned I, typename T0, typename T1, typename T2, typename T3, typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
struct predicates_element< I, boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> >
{
    typedef boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> predicate_type;

    typedef typename boost::tuples::element<I, predicate_type>::type type;
    static type const& get(predicate_type const& p) { return boost::get<I>(p); }
};

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

template <typename PairPredicates, typename Tag, unsigned First, unsigned Last>
struct predicates_check_pair {};

template <typename PairPredicates, typename Tag, unsigned I>
struct predicates_check_pair<PairPredicates, Tag, I, I>
{
    template <typename Value, typename Indexable>
    static inline bool apply(PairPredicates const& p, Value const& v, Indexable const& i)
    {
        return true;
    }
};

template <typename PairPredicates, typename Tag>
struct predicates_check_pair<PairPredicates, Tag, 0, 1>
{
    template <typename Value, typename Indexable>
    static inline bool apply(PairPredicates const& p, Value const& v, Indexable const& i)
    {
        return predicate_check<typename PairPredicates::first_type, Tag>::apply(p.first, v, i);
    }
};

template <typename PairPredicates, typename Tag>
struct predicates_check_pair<PairPredicates, Tag, 1, 2>
{
    template <typename Value, typename Indexable>
    static inline bool apply(PairPredicates const& p, Value const& v, Indexable const& i)
    {
        return predicate_check<typename PairPredicates::second_type, Tag>::apply(p.second, v, i);
    }
};

template <typename PairPredicates, typename Tag>
struct predicates_check_pair<PairPredicates, Tag, 0, 2>
{
    template <typename Value, typename Indexable>
    static inline bool apply(PairPredicates const& p, Value const& v, Indexable const& i)
    {
        return predicate_check<typename PairPredicates::first_type, Tag>::apply(p.first, v, i)
            && predicate_check<typename PairPredicates::second_type, Tag>::apply(p.second, v, i);
    }
};

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
    static inline bool apply(TuplePredicates const& p, Value const& v, Indexable const& i)
    {
        return true;
    }
};

template <typename Predicate, typename Tag, size_t First, size_t Last>
struct predicates_check_impl
{
    BOOST_MPL_ASSERT_MSG((First < 1 && Last <= 1 && First <= Last), INVALID_INDEXES, (predicates_check_impl));

    template <typename Value, typename Indexable>
    static inline bool apply(Predicate const& p, Value const& v, Indexable const& i)
    {
        return predicate_check<Predicate, Tag>::apply(p, v, i);
    }
};

template <typename Predicate1, typename Predicate2, typename Tag, size_t First, size_t Last>
struct predicates_check_impl<std::pair<Predicate1, Predicate2>, Tag, First, Last>
{
    BOOST_MPL_ASSERT_MSG((First < 2 && Last <= 2 && First <= Last), INVALID_INDEXES, (predicates_check_impl));

    template <typename Value, typename Indexable>
    static inline bool apply(std::pair<Predicate1, Predicate2> const& p, Value const& v, Indexable const& i)
    {
        return predicate_check<Predicate1, Tag>::apply(p.first, v, i)
            && predicate_check<Predicate2, Tag>::apply(p.second, v, i);
    }
};

template <
    typename T0, typename T1, typename T2, typename T3, typename T4,
    typename T5, typename T6, typename T7, typename T8, typename T9,
    typename Tag, unsigned First, unsigned Last
>
struct predicates_check_impl<
    boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>,
    Tag, First, Last
>
{
    typedef boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> predicates_type;

    static const unsigned pred_len = boost::tuples::length<predicates_type>::value;
    BOOST_MPL_ASSERT_MSG((First < pred_len && Last <= pred_len && First <= Last), INVALID_INDEXES, (predicates_check_impl));

    template <typename Value, typename Indexable>
    static inline bool apply(predicates_type const& p, Value const& v, Indexable const& i)
    {
        return predicates_check_tuple<
            predicates_type,
            Tag, First, Last
        >::apply(p, v, i);
    }
};

template <typename Head, typename Tail, typename Tag, unsigned First, unsigned Last>
struct predicates_check_impl<
    boost::tuples::cons<Head, Tail>,
    Tag, First, Last
>
{
    typedef boost::tuples::cons<Head, Tail> predicates_type;

    static const unsigned pred_len = boost::tuples::length<predicates_type>::value;
    BOOST_MPL_ASSERT_MSG((First < pred_len && Last <= pred_len && First <= Last), INVALID_INDEXES, (predicates_check_impl));

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

}}}} // namespace boost::geometry::index::detail

#endif // BOOST_GEOMETRY_INDEX_DETAIL_PREDICATES_HPP
