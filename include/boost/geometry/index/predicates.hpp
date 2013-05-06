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
\defgroup predicates Predicates (boost::geometry::index::)
*/

namespace boost { namespace geometry { namespace index {

/*!
\brief Generate \c covered_by() predicate.

Generate a predicate defining Value and Geometry relationship.
Value will be returned by the query if <tt>bg::covered_by(Indexable, Geometry)</tt>
returns true.

\par Example
\verbatim
bgi::query(spatial_index, bgi::covered_by(box), std::back_inserter(result));
\endverbatim

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

\par Example
\verbatim
bgi::query(spatial_index, bgi::disjoint(box), std::back_inserter(result));
\endverbatim

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

\par Example
\verbatim
bgi::query(spatial_index, bgi::intersects(box), std::back_inserter(result));
bgi::query(spatial_index, bgi::intersects(ring), std::back_inserter(result));
bgi::query(spatial_index, bgi::intersects(polygon), std::back_inserter(result));
\endverbatim

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

\par Example
\verbatim
bgi::query(spatial_index, bgi::overlaps(box), std::back_inserter(result));
\endverbatim

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

\par Example
\verbatim
bgi::query(spatial_index, bgi::within(box), std::back_inserter(result));
\endverbatim

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
\brief Generate satisfies() predicate.

A wrapper around user-defined UnaryPredicate checking if Value should be returned by spatial query.

\par Example
\verbatim
bool is_red(Value const& v) { return v.is_red(); }

struct is_red_o {
template <typename Value> bool operator()(Value const& v) { return v.is_red(); }
}

// ...

rt.query(index::intersects(box) && index::satisfies(is_red),
std::back_inserter(result));

rt.query(index::intersects(box) && index::satisfies(is_red_o()),
std::back_inserter(result));

#ifndef BOOST_NO_CXX11_LAMBDAS
rt.query(index::intersects(box) && index::satisfies([](Value const& v) { return v.is_red(); }),
std::back_inserter(result));
#endif
\endverbatim

\ingroup predicates

\tparam UnaryPredicate  A type of unary predicate function or function object.

\param pred             The unary predicate function or function object.
*/
template <typename UnaryPredicate>
inline detail::satisfies<UnaryPredicate> satisfies(UnaryPredicate const& pred)
{
    return detail::satisfies<UnaryPredicate>(pred);
}

/*!
\brief Generate nearest() predicate.

When nearest predicate is passed to the query, k-nearest neighbour search will be performed.

The simplest way of defining the knn query is passing a \c Point to which \c Values must be closest.

\par Example
\verbatim
bgi::query(spatial_index, bgi::nearest(pt, 5), std::back_inserter(result));
bgi::query(spatial_index, bgi::nearest(pt, 5) && bgi::intersects(box), std::back_inserter(result));
\endverbatim

\warning
Only one \c nearest() predicate may be used in a query.

\ingroup predicates

\param point        The point from which distance is calculated.
\param k            The maximum number of values to return.
*/
template <typename Point> inline
detail::nearest<Point>
nearest(Point const& point, unsigned k)
{
    return detail::nearest<Point>(point, k);
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
    /*typedef typename boost::mpl::if_c<is_predicate<Pred1>::value, Pred1, Pred1 const&>::type stored1;
    typedef typename boost::mpl::if_c<is_predicate<Pred2>::value, Pred2, Pred2 const&>::type stored2;*/
    namespace bt = boost::tuples;

    return
    bt::cons< Pred1, bt::cons<Pred2, bt::null_type> >
        ( p1, bt::cons<Pred2, bt::null_type>(p2, bt::null_type()) );
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
    //typedef typename boost::mpl::if_c<is_predicate<Pred>::value, Pred, Pred const&>::type stored;
    namespace bt = boost::tuples;

    return
    tuples::push_back_impl<
        bt::cons<Head, Tail>, Pred, 0, bt::length< bt::cons<Head, Tail> >::value
    >::apply(t, p);
}
    
} // namespace detail

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_INDEX_PREDICATES_HPP
