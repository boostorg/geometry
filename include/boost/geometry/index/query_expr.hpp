// Boost.Geometry Index
//
// Query expr
//
// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_INDEX_QUERY_EXPR_HPP
#define BOOST_GEOMETRY_INDEX_QUERY_EXPR_HPP

#include <boost/type_traits/is_same.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/mpl/assert.hpp>

namespace boost { namespace geometry { namespace index { namespace qe {

namespace detail {

// tags

struct intersects_tag {};
struct within_tag {};

struct nearest_tag {};

// predicates

template <typename T, typename Tag>
struct predicate
{
    typedef Tag tag_type;
    predicate(T const& d) : data(d) {}
    T data;
};

template <typename T>
struct predicate<T, nearest_tag>
{
    typedef nearest_tag tag_type;
    predicate(T const& d, size_t k) : data(d), count(k) {}
    T data;
    size_t count;
};

template <typename T, typename Tuple, size_t I, size_t N>
struct push_back_impl
{
    typedef
    boost::tuples::cons<
        typename boost::tuples::element<I, Tuple>::type,
        typename push_back_impl<T, Tuple, I+1, N>::type
    > type;

    static type apply(T const& t, Tuple const& tup)
    {
        return
        type(
            boost::get<I>(tup),
            push_back_impl<T, Tuple, I+1, N>::apply(t, tup)
        );
    }
};

template <typename T, typename Tuple, size_t N>
struct push_back_impl<T, Tuple, N, N>
{
    typedef boost::tuples::cons<T, boost::tuples::null_type> type;

    static type apply(T const& t, Tuple const&)
    {
        return type(t, boost::tuples::null_type());
    }
};

template <typename T1, typename Tag1, typename T2, typename Tag2> inline
boost::tuples::cons<
    predicate<T1, Tag1>,
    boost::tuples::cons<predicate<T2, Tag2>, boost::tuples::null_type>
>
operator&&(predicate<T1, Tag1> const& p1, predicate<T2, Tag2> const& p2)
{
    return
    boost::tuples::cons<
        predicate<T1, Tag1>,
        boost::tuples::cons<predicate<T2, Tag2>, boost::tuples::null_type>
    >(
        p1,
        boost::tuples::cons<predicate<T2, Tag2>, boost::tuples::null_type>(p2, boost::tuples::null_type())
    );
}

template <typename Head, typename Tail, typename T, typename Tag> inline
typename push_back_impl<
    predicate<T, Tag>,
    boost::tuples::cons<Head, Tail>,
    0,
    boost::tuples::length<boost::tuples::cons<Head, Tail> >::value
>::type
operator&&(boost::tuples::cons<Head, Tail> const& t, predicate<T, Tag> const& p)
{
    return
    push_back_impl<
        predicate<T, Tag>,
        boost::tuples::cons<Head, Tail>,
        0,
        boost::tuples::length<boost::tuples::cons<Head, Tail> >::value
    >::apply(p, t);
}

template <typename Tuple, size_t I, size_t N>
struct find_nearest_impl
{
    static const size_t value =
        boost::is_same<
            typename boost::tuples::element<I, Tuple>::type::tag_type,
            nearest_tag
        >::value ?
        I :
        find_nearest_impl<Tuple, I+1, N>::value;
};

template <typename Tuple, size_t N>
struct find_nearest_impl<Tuple, N, N>
{
    static const size_t value = N;
};

} // namespace detail

// predicates generators

template <typename Geometry>
detail::predicate<Geometry, detail::intersects_tag> intersects(Geometry const& g)
{
    return detail::predicate<Geometry, detail::intersects_tag>(g);
}

template <typename DistancePredicates>
detail::predicate<DistancePredicates, detail::nearest_tag> nearest(DistancePredicates const& p, size_t k)
{
    return detail::predicate<DistancePredicates, detail::nearest_tag>(p, k);
}

}}}} // namespace boost::geometry::index::qe

#endif // BOOST_GEOMETRY_INDEX_QUERY_EXPR_HPP
