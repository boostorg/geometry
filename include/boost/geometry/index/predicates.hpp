// Boost.Geometry Index
//
// Spatial query predicates
//
// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_INDEX_PREDICATES_HPP
#define BOOST_GEOMETRY_INDEX_PREDICATES_HPP

#include <utility>
#include <boost/tuple/tuple.hpp>
#include <boost/mpl/assert.hpp>

// TODO: awulkiew - temporary
#include <boost/geometry/algorithms/covered_by.hpp>

#include <boost/geometry/index/detail/predicates.hpp>
#include <boost/geometry/index/detail/tuples.hpp>

/*!
\defgroup predicates Spatial predicates (boost::geometry::index::)
*/

namespace boost { namespace geometry { namespace index {

/*!
\brief Generate \c covered_by() predicate.

Generate a predicate defining Value and Geometry relationship.
Value will be returned by the query if <tt>bg::covered_by(Indexable, Geometry)</tt>
returns true.

\ingroup predicates

\tparam Geometry    The Geometry type.

\param g            The Geometry object.
*/
template <typename Geometry>
inline detail::covered_by<Geometry> covered_by(Geometry const& g)
{
    return detail::covered_by<Geometry>(g);
}

/*!
\brief Generate \c disjoint() predicate.

Generate a predicate defining Value and Geometry relationship.
Value will be returned by the query if <tt>bg::disjoint(Indexable, Geometry)</tt>
returns true.

\ingroup predicates

\tparam Geometry    The Geometry type.

\param g            The Geometry object.
*/
template <typename Geometry>
inline detail::disjoint<Geometry> disjoint(Geometry const& g)
{
    return detail::disjoint<Geometry>(g);
}

/*!
\brief Generate \c intersects() predicate.

Generate a predicate defining Value and Geometry relationship.
Value will be returned by the query if <tt>bg::intersects(Indexable, Geometry)</tt>
returns true.

\ingroup predicates

\tparam Geometry    The Geometry type.

\param g            The Geometry object.
*/
template <typename Geometry>
inline detail::intersects<Geometry> intersects(Geometry const& g)
{
    return detail::intersects<Geometry>(g);
}

/*!
\brief Generate \c overlaps() predicate.

Generate a predicate defining Value and Geometry relationship.
Value will be returned by the query if <tt>bg::overlaps(Indexable, Geometry)</tt>
returns true.

\ingroup predicates

\tparam Geometry    The Geometry type.

\param g            The Geometry object.
*/
template <typename Geometry>
inline detail::overlaps<Geometry> overlaps(Geometry const& g)
{
    return detail::overlaps<Geometry>(g);
}

//*!
//\brief Generate \c touches() predicate.
//
//Generate a predicate defining Value and Geometry relationship.
//Value will be returned by the query if <tt>bg::touches(Indexable, Geometry)</tt>
//returns true.
//
//\ingroup predicates
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
\brief Generate \c within() predicate.

Generate a predicate defining Value and Geometry relationship.
Value will be returned by the query if <tt>bg::within(Indexable, Geometry)</tt>
returns true.

\ingroup predicates

\tparam Geometry    The Geometry type.

\param g            The Geometry object.
*/
template <typename Geometry>
inline detail::within<Geometry> within(Geometry const& g)
{
    return detail::within<Geometry>(g);
}

/*!
\brief Generate value predicate.

A wrapper around user-defined functor
describing if Value should be returned by spatial query.

\ingroup predicates

\tparam Fun  Functor type.

\param fun   The functor.
*/
template <typename Fun>
inline detail::value<Fun> value(Fun const& fun)
{
    return detail::value<Fun>(fun);
}

namespace detail {

// operator! generators

template <typename Geometry>
not_covered_by<Geometry>
operator!(covered_by<Geometry> const& p)
{
    return not_covered_by<Geometry>(p.geometry);
}

template <typename Geometry>
covered_by<Geometry>
operator!(not_covered_by<Geometry> const& p)
{
    return covered_by<Geometry>(p.geometry);
}

template <typename Geometry>
not_disjoint<Geometry>
operator!(disjoint<Geometry> const& p)
{
    return not_disjoint<Geometry>(p.geometry);
}

template <typename Geometry>
disjoint<Geometry>
operator!(not_disjoint<Geometry> const& p)
{
    return disjoint<Geometry>(p.geometry);
}

template <typename Geometry>
not_intersects<Geometry>
operator!(intersects<Geometry> const& p)
{
    return not_intersects<Geometry>(p.geometry);
}

template <typename Geometry>
intersects<Geometry>
operator!(not_intersects<Geometry> const& p)
{
    return intersects<Geometry>(p.geometry);
}

template <typename Geometry>
not_overlaps<Geometry>
operator!(overlaps<Geometry> const& p)
{
    return not_overlaps<Geometry>(p.geometry);
}

template <typename Geometry>
overlaps<Geometry>
operator!(not_overlaps<Geometry> const& p)
{
    return overlaps<Geometry>(p.geometry);
}

//template <typename Geometry>
//not_touches<Geometry>
//operator!(touches<Geometry> const& p)
//{
//    return not_touches<Geometry>(p.geometry);
//}
//
//template <typename Geometry>
//touches<Geometry>
//operator!(not_touches<Geometry> const& p)
//{
//    return touches<Geometry>(p.geometry);
//}

template <typename Geometry>
not_within<Geometry>
operator!(within<Geometry> const& p)
{
    return not_within<Geometry>(p.geometry);
}

template <typename Geometry>
within<Geometry>
operator!(not_within<Geometry> const& p)
{
    return within<Geometry>(p.geometry);
}

// operator&& generators

template <typename Pred1, typename Pred2> inline
boost::tuples::cons<
    Pred1,
    boost::tuples::cons<Pred2, boost::tuples::null_type>
>
operator&&(Pred1 const& p1, Pred2 const& p2)
{
    return
    boost::tuples::cons<
        Pred1,
        boost::tuples::cons<Pred2, boost::tuples::null_type>
    >(
        p1,
        boost::tuples::cons<Pred2, boost::tuples::null_type>(p2, boost::tuples::null_type())
    );
}

template <typename Head, typename Tail, typename Pred> inline
typename tuples::push_back_impl<
    boost::tuples::cons<Head, Tail>,
    Pred,
    0,
    boost::tuples::length<boost::tuples::cons<Head, Tail> >::value
>::type
operator&&(boost::tuples::cons<Head, Tail> const& t, Pred const& p)
{
    return
    tuples::push_back_impl<
        boost::tuples::cons<Head, Tail>,
        Pred,
        0,
        boost::tuples::length<boost::tuples::cons<Head, Tail> >::value
    >::apply(t, p);
}
    
} // namespace detail

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_INDEX_PREDICATES_HPP
