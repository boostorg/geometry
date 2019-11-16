// Boost.Geometry Index
//
// Copyright (c) 2011-2013 Adam Wulkiewicz, Lodz, Poland.
//
// This file was modified by Oracle on 2019.
// Modifications copyright (c) 2019 Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_UTIL_TUPLES_HPP
#define BOOST_GEOMETRY_UTIL_TUPLES_HPP

#include <utility>

#include <boost/tuple/tuple.hpp>
#include <boost/type_traits/integral_constant.hpp>
#include <boost/type_traits/is_same.hpp>

namespace boost { namespace geometry { namespace tuples {


// find_index_if
// Searches for the index of an element for which UnaryPredicate returns true
// If such element is not found the result is N

template
<
    typename Tuple,
    template <typename> class UnaryPred,
    int I = 0,
    int N = boost::tuples::length<Tuple>::value
>
struct find_index_if
    : boost::mpl::if_c
        <
            UnaryPred<typename boost::tuples::element<I, Tuple>::type>::value,
            boost::integral_constant<int, I>,
            typename find_index_if<Tuple, UnaryPred, I+1, N>::type
        >::type
{};

template
<
    typename Tuple,
    template <typename> class UnaryPred,
    int N
>
struct find_index_if<Tuple, UnaryPred, N, N>
    : boost::integral_constant<int, N>
{};


// find_if
// Searches for an element for which UnaryPredicate returns true
// If such element is not found the result is boost::tuples::null_type

template
<
    typename Tuple,
    template <typename> class UnaryPred,
    int I = 0,
    int N = boost::tuples::length<Tuple>::value
>
struct find_if
    : boost::mpl::if_c
        <
            UnaryPred<typename boost::tuples::element<I, Tuple>::type>::value,
            boost::tuples::element<I, Tuple>,
            typename find_if<Tuple, UnaryPred, I+1, N>
        >::type
{};

template
<
    typename Tuple,
    template <typename> class UnaryPred,
    int N
>
struct find_if<Tuple, UnaryPred, N, N>
{
    typedef boost::tuples::null_type type;
};


// is_found
// Returns true if a type T (the result of find_if) was found.

template <typename T>
struct is_found
    : boost::mpl::not_<boost::is_same<T, boost::tuples::null_type> >
{};


// is_not_found
// Returns true if a type T (the result of find_if) was not found.

template <typename T>
struct is_not_found
    : boost::is_same<T, boost::tuples::null_type>
{};


// exists_if
// Returns true if search for element meeting UnaryPred can be found.

template <typename Tuple, template <typename> class UnaryPred>
struct exists_if
    : is_found<typename find_if<Tuple, UnaryPred>::type>
{};


// push_back
// A utility used to create a type/object of a Tuple containing
//   all types/objects stored in another Tuple plus additional one.

template <typename Tuple,
          typename T,
          size_t I = 0,
          size_t N = boost::tuples::length<Tuple>::value>
struct push_back
{
    typedef
    boost::tuples::cons<
        typename boost::tuples::element<I, Tuple>::type,
        typename push_back<Tuple, T, I+1, N>::type
    > type;

    static type apply(Tuple const& tup, T const& t)
    {
        return
        type(
            boost::get<I>(tup),
            push_back<Tuple, T, I+1, N>::apply(tup, t)
        );
    }
};

template <typename Tuple, typename T, size_t N>
struct push_back<Tuple, T, N, N>
{
    typedef boost::tuples::cons<T, boost::tuples::null_type> type;

    static type apply(Tuple const&, T const& t)
    {
        return type(t, boost::tuples::null_type());
    }
};

}}} // namespace boost::geometry::tuples

#endif // BOOST_GEOMETRY_UTIL_TUPLES_HPP
