// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Boost.SpatialIndex - tags related to rtree used in various files
//
// Copyright 2011 Adam Wulkiewicz.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_INDEX_TUPLES_HPP
#define BOOST_GEOMETRY_EXTENSIONS_INDEX_TUPLES_HPP

#include <boost/tuple/tuple.hpp>

// TODO move this to index/tuples and separate algorithms

namespace boost { namespace geometry { namespace index {

namespace tuples {

// find_index

namespace detail {

template <typename Tuple, typename El, size_t N>
struct find_index;

template <typename Tuple, typename El, size_t N, typename CurrentEl>
struct find_index_impl
{
    static const size_t value = find_index<Tuple, El, N - 1>::value;
};

template <typename Tuple, typename El, size_t N>
struct find_index_impl<Tuple, El, N, El>
{
    static const size_t value = N - 1;
};

template <typename Tuple, typename El, typename CurrentEl>
struct find_index_impl<Tuple, El, 1, CurrentEl>
{
    BOOST_MPL_ASSERT_MSG(
        (false),
        ELEMENT_NOT_FOUND,
        (find_index_impl));
};

template <typename Tuple, typename El>
struct find_index_impl<Tuple, El, 1, El>
{
    static const size_t value = 0;
};

template <typename Tuple, typename El, size_t N>
struct find_index
{
    static const size_t value =
        find_index_impl<
            Tuple,
            El,
            N,
            typename boost::tuples::element<N - 1, Tuple>::type
        >::value;
};

} // namespace detail

template <typename Tuple, typename El>
struct find_index
{
    static const size_t value =
        detail::find_index<
            Tuple,
            El,
            boost::tuples::length<Tuple>::value
        >::value;
};

// types_equal

template <typename T1, typename T2>
struct equal
{
    static const bool value = false;
};

template <typename T>
struct equal<T, T>
{
    static const bool value = true;
};

// has

namespace detail {

template <typename Tuple, typename El, size_t N>
struct has
{
    static const bool value
        = equal<
            typename boost::tuples::element<N - 1, Tuple>::type,
            El
        >::value
        || has<Tuple, El, N - 1>::value;
};

template <typename Tuple, typename El>
struct has<Tuple, El, 1>
{
    static const bool value
        = equal<
            typename boost::tuples::element<0, Tuple>::type,
            El
        >::value;
};

} // namespace detail

template <typename Tuple, typename El>
struct has
{
    static const bool value
        = detail::has<
            Tuple,
            El,
            boost::tuples::length<Tuple>::value
        >::value;
};

// add

template <typename Tuple, typename El>
struct add
{
    BOOST_MPL_ASSERT_MSG(
        (false),
        NOT_IMPLEMENTED_FOR_THIS_TUPLE_TYPE,
        (add));
};

template <typename T1, typename T>
struct add<boost::tuple<T1>, T>
{
    typedef boost::tuple<T1, T> type;
};

template <typename T1, typename T2, typename T>
struct add<boost::tuple<T1, T2>, T>
{
    typedef boost::tuple<T1, T2, T> type;
};

// add_if

template <typename Tuple, typename El, bool Cond>
struct add_if
{
    typedef Tuple type;
};

template <typename Tuple, typename El>
struct add_if<Tuple, El, true>
{
    typedef typename add<Tuple, El>::type type;
};

// add_unique

template <typename Tuple, typename El>
struct add_unique
{
    typedef typename add_if<
        Tuple,
        El,
        !has<Tuple, El>::value
    >::type type;
};

} // namespace tuples

}}} // namespace boost::geometry::index

#endif // BOOST_GEOMETRY_EXTENSIONS_INDEX_RTREE_TAGS_HPP
