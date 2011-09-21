// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - Spatial index query predicates
//
// Copyright 2011 Adam Wulkiewicz.
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

// predicates

namespace detail {

struct empty {};

//TODO: awulkiew - consider storing Geometry instead of Geometry const&
// it's faster and eliminates problems with storing of references to temporaries

template <typename Geometry>
struct covered_by
    : nonassignable
{
    covered_by(Geometry const& g) : geometry(g) {}
    Geometry const& geometry;
};

template <typename Geometry>
struct intersects
    : nonassignable
{
    intersects(Geometry const& g) : geometry(g) {}
    Geometry const& geometry;
};

template <typename Geometry>
struct overlaps
    : nonassignable
{
    overlaps(Geometry const& g) : geometry(g) {}
    Geometry const& geometry;
};

template <typename Geometry>
struct within
    : nonassignable
{
    within(Geometry const& g) : geometry(g) {}
    Geometry const& geometry;
};

} // namespace detail

inline detail::empty empty()
{
    return detail::empty();
}

template <typename Geometry>
inline detail::covered_by<Geometry> covered_by(Geometry const& g)
{
    return detail::covered_by<Geometry>(g);
}

template <typename Geometry>
inline detail::intersects<Geometry> intersects(Geometry const& g)
{
    return detail::intersects<Geometry>(g);
}

template <typename Geometry>
inline detail::overlaps<Geometry> overlaps(Geometry const& g)
{
    return detail::overlaps<Geometry>(g);
}

template <typename Geometry>
inline detail::within<Geometry> within(Geometry const& g)
{
    return detail::within<Geometry>(g);
}

namespace detail
{

// predicate check

// TODO: use empty definitions here + MPL_ASSERT ?
// implement default values predicates applied to values in leafs, as a function/functor as simple as possible
// bool fun(Value const& v);
// distinguish between geometries and other types by use of geometry::tag
// in predicate_check_default<..., GeomTag> -> predicate_check_default<..., void>

template <typename Geometry, typename Tag>
struct predicate_check
{
    template <typename Indexable>
    static inline bool apply(Geometry const& g, Indexable const& i)
    {
        return geometry::intersects(i, g);
    }
};

template <typename Tag>
struct predicate_check<empty, Tag>
{
    template <typename Geometry, typename Indexable>
    static inline bool apply(Geometry const&, Indexable const&)
    {
        return true;
    }
};

template <typename Geometry, typename Tag>
struct predicate_check<covered_by<Geometry>, Tag>
{
    template <typename Indexable>
    static inline bool apply(covered_by<Geometry> const& p, Indexable const& i)
    {
        return geometry::covered_by(i, p.geometry);
    }
};

template <typename Geometry, typename Tag>
struct predicate_check<intersects<Geometry>, Tag>
{
    template <typename Indexable>
    static inline bool apply(intersects<Geometry> const& p, Indexable const& i)
    {
        return geometry::intersects(i, p.geometry);
    }
};

template <typename Geometry, typename Tag>
struct predicate_check<overlaps<Geometry>, Tag>
{
    template <typename Indexable>
    static inline bool apply(overlaps<Geometry> const& p, Indexable const& i)
    {
        return geometry::overlaps(i, p.geometry);
    }
};

template <typename Geometry, typename Tag>
struct predicate_check<within<Geometry>, Tag>
{
    template <typename Indexable>
    static inline bool apply(within<Geometry> const& p, Indexable const& i)
    {
        return geometry::within(i, p.geometry);
    }
};

// predicates check

template <typename TuplePredicates, typename Tag, unsigned int N>
struct predicates_check_tuple
{
    template <typename Indexable>
    static inline bool apply(TuplePredicates const& p, Indexable const& i)
    {
        return predicates_check_tuple<TuplePredicates, Tag, N - 1>::apply(p, i)
            && predicate_check<
                typename boost::tuples::element<N - 1, TuplePredicates>::type,
                Tag
            >::apply(boost::get<N - 1>(p), i);
    }
};

template <typename TuplePredicates, typename Tag>
struct predicates_check_tuple<TuplePredicates, Tag, 0>
{
    template <typename Indexable>
    static inline bool apply(TuplePredicates const& p, Indexable const& i)
    {
        return predicate_check<
            typename boost::tuples::element<0, TuplePredicates>::type,
            Tag
        >::apply(boost::get<0>(p), i);
    }
};

template <typename Predicate, typename Tag>
struct predicates_check
{
    template <typename Indexable>
    static inline bool apply(Predicate const& p, Indexable const& i)
    {
        return predicate_check<Predicate, Tag>::apply(p, i);
    }
};

template <typename Predicate1, typename Predicate2, typename Tag>
struct predicates_check<std::pair<Predicate1, Predicate2>, Tag>
{
    template <typename Indexable>
    static inline bool apply(std::pair<Predicate1, Predicate2> const& p, Indexable const& i)
    {
        return predicate_check<Predicate1, Tag>::apply(p.first, i)
            && predicate_check<Predicate2, Tag>::apply(p.second, i);
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

    template <typename Indexable>
    static inline bool apply(predicates_type const& p, Indexable const& i)
    {
        return predicates_check_tuple<
            predicates_type,
            Tag,
            boost::tuples::length<predicates_type>::value
        >::apply(p, i);
    }
};

} // namespace detail

template <typename Tag, typename Predicates, typename Indexable>
inline bool predicates_check(Predicates const& p, Indexable const& i)
{
    return detail::predicates_check<Predicates, Tag>
        ::apply(p, i);
}

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_PREDICATES_HPP
