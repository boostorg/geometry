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

/*!
\defgroup predicates Spatial predicates (boost::geometry::index::)
*/

namespace boost { namespace geometry { namespace index {

/*!
\brief Generate empty predicate.

\ingroup predicates
*/
inline detail::empty empty()
{
    return detail::empty();
}

/*!
\brief Generate value predicate.

A wrapper around user-defined functor
describing if Value should be returned by spatial query.

\ingroup predicates

\tparam ValuePredicate  Functor type.

\param vpred            The functor.
*/
template <typename ValuePredicate>
inline detail::value<ValuePredicate> value(ValuePredicate const& vpred)
{
    return detail::value<ValuePredicate>(vpred);
}

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

#endif // BOOST_GEOMETRY_INDEX_PREDICATES_HPP
