// Boost.Geometry Index
//
// Spatial query predicates
//
// Copyright (c) 2011-2012 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_PREDICATES_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_PREDICATES_HPP

#include <utility>
#include <boost/tuple/tuple.hpp>
#include <boost/mpl/assert.hpp>

// TODO: awulkiew - temporary
#include <boost/geometry/algorithms/covered_by.hpp>

namespace boost { namespace geometry { namespace index {

namespace detail {

// predicates

struct empty {};

template <typename ValuePredicate>
struct value
{
    value(ValuePredicate const& vpred) : value_predicate(vpred) {}
    ValuePredicate value_predicate;
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

} // namespace detail

// generators

/*!
Generate empty predicate.
*/
inline detail::empty empty()
{
    return detail::empty();
}

/*!
Generate value predicate. A wrapper around user-defined functor
describing if Value should be returned by spatial query.

\tparam ValuePredicate  Functor type.

\param vpred            The functor.
*/
template <typename ValuePredicate>
inline detail::value<ValuePredicate> value(ValuePredicate const& vpred)
{
    return detail::value<ValuePredicate>(vpred);
}

/*!
Generate a predicate defining Value and Geometry relationship.
Value will be returned by the query if bg::covered_by(Indexable, Geometry)
returns true.

\tparam Geometry    The Geometry type.

\param g            The Geometry object.
*/
template <typename Geometry>
inline detail::covered_by<Geometry> covered_by(Geometry const& g)
{
    return detail::covered_by<Geometry>(g);
}

/*!
Generate a predicate defining Value and Geometry relationship.
Value will be returned by the query if bg::disjoint(Indexable, Geometry)
returns true.

\tparam Geometry    The Geometry type.

\param g            The Geometry object.
*/
template <typename Geometry>
inline detail::disjoint<Geometry> disjoint(Geometry const& g)
{
    return detail::disjoint<Geometry>(g);
}

/*!
Generate a predicate defining Value and Geometry relationship.
Value will be returned by the query if bg::intersects(Indexable, Geometry)
returns true.

\tparam Geometry    The Geometry type.

\param g            The Geometry object.
*/
template <typename Geometry>
inline detail::intersects<Geometry> intersects(Geometry const& g)
{
    return detail::intersects<Geometry>(g);
}

/*!
Generate a predicate defining Value and Geometry relationship.
Value will be returned by the query if bg::overlaps(Indexable, Geometry)
returns true.

\tparam Geometry    The Geometry type.

\param g            The Geometry object.
*/
template <typename Geometry>
inline detail::overlaps<Geometry> overlaps(Geometry const& g)
{
    return detail::overlaps<Geometry>(g);
}

//*!
//Generate a predicate defining Value and Geometry relationship.
//Value will be returned by the query if bg::touches(Indexable, Geometry)
//returns true.
//
//\tparam Geometry    The Geometry type.
//
//\param g            The Geometry object.
//*/
//template <typename Geometry>
//inline detail::touches<Geometry> touches(Geometry const& g)
//{
//    return detail::touches<Geometry>(g);
//}

/*!
Generate a predicate defining Value and Geometry relationship.
Value will be returned by the query if bg::within(Indexable, Geometry)
returns true.

\tparam Geometry    The Geometry type.

\param g            The Geometry object.
*/
template <typename Geometry>
inline detail::within<Geometry> within(Geometry const& g)
{
    return detail::within<Geometry>(g);
}

namespace detail
{

// predicate check

// TODO: awulkiew - consider passing Value/Node and Translator instead of
//                  Value and Indexable

template <typename Geometry, typename Tag>
struct predicate_check
{
    template <typename Value, typename Indexable>
    static inline bool apply(Geometry const& g, Value const&, Indexable const& i)
    {
        return geometry::intersects(i, g);
    }
};

template <typename Tag>
struct predicate_check<empty, Tag>
{
    template <typename Geometry, typename Value, typename Indexable>
    static inline bool apply(Geometry const&, Value const&, Indexable const&)
    {
        return true;
    }
};

template <typename ValuePredicate, typename Tag>
struct predicate_check<value<ValuePredicate>, Tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(value<ValuePredicate> const& p, Value const& v, Indexable const&)
    {
        return p.value_predicate(v);
    }
};

template <typename Geometry, typename Tag>
struct predicate_check<covered_by<Geometry>, Tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(covered_by<Geometry> const& p, Value const&, Indexable const& i)
    {
        return geometry::covered_by(i, p.geometry);
    }
};

template <typename Geometry, typename Tag>
struct predicate_check<disjoint<Geometry>, Tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(disjoint<Geometry> const& p, Value const&, Indexable const& i)
    {
        return geometry::disjoint(i, p.geometry);
    }
};

template <typename Geometry, typename Tag>
struct predicate_check<intersects<Geometry>, Tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(intersects<Geometry> const& p, Value const&, Indexable const& i)
    {
        return geometry::intersects(i, p.geometry);
    }
};

template <typename Geometry, typename Tag>
struct predicate_check<overlaps<Geometry>, Tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(overlaps<Geometry> const& p, Value const&, Indexable const& i)
    {
        return geometry::overlaps(i, p.geometry);
    }
};

//template <typename Geometry, typename Tag>
//struct predicate_check<touches<Geometry>, Tag>
//{
//    template <typename Value, typename Indexable>
//    static inline bool apply(touches<Geometry> const& p, Value const&, Indexable const& i)
//    {
//        return geometry::touches(i, p.geometry);
//    }
//};

template <typename Geometry, typename Tag>
struct predicate_check<within<Geometry>, Tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(within<Geometry> const& p, Value const&, Indexable const& i)
    {
        return geometry::within(i, p.geometry);
    }
};

template <typename Geometry, typename Tag>
struct predicate_check<not_covered_by<Geometry>, Tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(not_covered_by<Geometry> const& p, Value const&, Indexable const& i)
    {
        return !geometry::covered_by(i, p.geometry);
    }
};

template <typename Geometry, typename Tag>
struct predicate_check<not_disjoint<Geometry>, Tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(not_disjoint<Geometry> const& p, Value const&, Indexable const& i)
    {
        return !geometry::disjoint(i, p.geometry);
    }
};

template <typename Geometry, typename Tag>
struct predicate_check<not_intersects<Geometry>, Tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(not_intersects<Geometry> const& p, Value const&, Indexable const& i)
    {
        return !geometry::intersects(i, p.geometry);
    }
};

template <typename Geometry, typename Tag>
struct predicate_check<not_overlaps<Geometry>, Tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(not_overlaps<Geometry> const& p, Value const&, Indexable const& i)
    {
        return !geometry::overlaps(i, p.geometry);
    }
};

//template <typename Geometry, typename Tag>
//struct predicate_check<not_touches<Geometry>, Tag>
//{
//    template <typename Value, typename Indexable>
//    static inline bool apply(not_touches<Geometry> const& p, Value const&, Indexable const& i)
//    {
//        return !geometry::touches(i, p.geometry);
//    }
//};

template <typename Geometry, typename Tag>
struct predicate_check<not_within<Geometry>, Tag>
{
    template <typename Value, typename Indexable>
    static inline bool apply(not_within<Geometry> const& p, Value const&, Indexable const& i)
    {
        return !geometry::within(i, p.geometry);
    }
};

// predicates check

template <typename TuplePredicates, typename Tag, unsigned int N>
struct predicates_check_tuple
{
    template <typename Value, typename Indexable>
    static inline bool apply(TuplePredicates const& p, Value const& v, Indexable const& i)
    {
        return predicates_check_tuple<TuplePredicates, Tag, N - 1>::apply(p, v, i)
            && predicate_check<
                typename boost::tuples::element<N - 1, TuplePredicates>::type,
                Tag
            >::apply(boost::get<N - 1>(p), v, i);
    }
};

template <typename TuplePredicates, typename Tag>
struct predicates_check_tuple<TuplePredicates, Tag, 1>
{
    template <typename Value, typename Indexable>
    static inline bool apply(TuplePredicates const& p, Value const& v, Indexable const& i)
    {
        return predicate_check<
            typename boost::tuples::element<0, TuplePredicates>::type,
            Tag
        >::apply(boost::get<0>(p), v, i);
    }
};

template <typename Predicate, typename Tag>
struct predicates_check
{
    template <typename Value, typename Indexable>
    static inline bool apply(Predicate const& p, Value const& v, Indexable const& i)
    {
        return predicate_check<Predicate, Tag>::apply(p, v, i);
    }
};

template <typename Predicate1, typename Predicate2, typename Tag>
struct predicates_check<std::pair<Predicate1, Predicate2>, Tag>
{
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
    typename Tag
>
struct predicates_check<
    boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9>,
    Tag
>
{
    typedef boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> predicates_type;

    template <typename Value, typename Indexable>
    static inline bool apply(predicates_type const& p, Value const& v, Indexable const& i)
    {
        return predicates_check_tuple<
            predicates_type,
            Tag,
            boost::tuples::length<predicates_type>::value
        >::apply(p, v, i);
    }
};

} // namespace detail

template <typename Tag, typename Predicates, typename Value, typename Indexable>
inline bool predicates_check(Predicates const& p, Value const& v, Indexable const& i)
{
    return detail::predicates_check<Predicates, Tag>
        ::apply(p, v, i);
}

}}} // namespace boost::geometry::index

// operator! generators

template <typename Geometry>
boost::geometry::index::detail::not_covered_by<Geometry>
operator!(boost::geometry::index::detail::covered_by<Geometry> const& p)
{
    return boost::geometry::index::detail::not_covered_by<Geometry>(p.geometry);
}

template <typename Geometry>
boost::geometry::index::detail::covered_by<Geometry>
operator!(boost::geometry::index::detail::not_covered_by<Geometry> const& p)
{
    return boost::geometry::index::detail::covered_by<Geometry>(p.geometry);
}

template <typename Geometry>
boost::geometry::index::detail::not_disjoint<Geometry>
operator!(boost::geometry::index::detail::disjoint<Geometry> const& p)
{
    return boost::geometry::index::detail::not_disjoint<Geometry>(p.geometry);
}

template <typename Geometry>
boost::geometry::index::detail::disjoint<Geometry>
operator!(boost::geometry::index::detail::not_disjoint<Geometry> const& p)
{
    return boost::geometry::index::detail::disjoint<Geometry>(p.geometry);
}

template <typename Geometry>
boost::geometry::index::detail::not_intersects<Geometry>
operator!(boost::geometry::index::detail::intersects<Geometry> const& p)
{
    return boost::geometry::index::detail::not_intersects<Geometry>(p.geometry);
}

template <typename Geometry>
boost::geometry::index::detail::intersects<Geometry>
operator!(boost::geometry::index::detail::not_intersects<Geometry> const& p)
{
    return boost::geometry::index::detail::intersects<Geometry>(p.geometry);
}

template <typename Geometry>
boost::geometry::index::detail::not_overlaps<Geometry>
operator!(boost::geometry::index::detail::overlaps<Geometry> const& p)
{
    return boost::geometry::index::detail::not_overlaps<Geometry>(p.geometry);
}

template <typename Geometry>
boost::geometry::index::detail::overlaps<Geometry>
operator!(boost::geometry::index::detail::not_overlaps<Geometry> const& p)
{
    return boost::geometry::index::detail::overlaps<Geometry>(p.geometry);
}

//template <typename Geometry>
//boost::geometry::index::detail::not_touches<Geometry>
//operator!(boost::geometry::index::detail::touches<Geometry> const& p)
//{
//    return boost::geometry::index::detail::not_touches<Geometry>(p.geometry);
//}
//
//template <typename Geometry>
//boost::geometry::index::detail::touches<Geometry>
//operator!(boost::geometry::index::detail::not_touches<Geometry> const& p)
//{
//    return boost::geometry::index::detail::touches<Geometry>(p.geometry);
//}

template <typename Geometry>
boost::geometry::index::detail::not_within<Geometry>
operator!(boost::geometry::index::detail::within<Geometry> const& p)
{
    return boost::geometry::index::detail::not_within<Geometry>(p.geometry);
}

template <typename Geometry>
boost::geometry::index::detail::within<Geometry>
operator!(boost::geometry::index::detail::not_within<Geometry> const& p)
{
    return boost::geometry::index::detail::within<Geometry>(p.geometry);
}

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_PREDICATES_HPP
