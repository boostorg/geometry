// Boost.Geometry
// Unit Test

// Copyright (c) 2019 Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <geometry_test_common.hpp>

#include <boost/geometry/util/tuples.hpp>

namespace bt = boost::tuples;
namespace bgt = boost::geometry::tuples;
namespace bm = boost::mpl;

template <typename T>
struct is_double
    : boost::is_same<T, double>
{};

template <typename T>
struct is_float
    : boost::is_same<T, float>
{};


template <typename Tuple>
struct is_boost_tuple
    : boost::integral_constant<bool, false>
{};

template <class T0, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8, class T9>
struct is_boost_tuple<boost::tuple<T0, T1, T2, T3, T4, T5, T6, T7, T8, T9> >
    : boost::integral_constant<bool, true>
{};

template <typename Tuple>
struct is_boost_tuples_cons
    : boost::integral_constant<bool, false>
{};

template <typename HT, typename TT>
struct is_boost_tuples_cons<boost::tuples::cons<HT, TT> >
    : boost::integral_constant<bool, true>
{};

template <typename Tuple>
struct is_std_pair
    : boost::integral_constant<bool, false>
{};

template <typename F, typename S>
struct is_std_pair<std::pair<F, S> >
    : boost::integral_constant<bool, true>
{};

#if !defined(BOOST_NO_CXX11_HDR_TUPLE) && !defined(BOOST_NO_VARIADIC_TEMPLATES)

template <typename Tuple>
struct is_std_tuple
    : boost::integral_constant<bool, false>
{};

template <typename ...Ts>
struct is_std_tuple<std::tuple<Ts...> >
    : boost::integral_constant<bool, true>
{};

#endif

template <typename Tuple>
void test_all()
{
    typedef Tuple tuple_id;
    tuple_id tup_id(1, 2.0);

    BOOST_CHECK_EQUAL((bgt::get<0>(tup_id)), 1);
    BOOST_CHECK_EQUAL((bgt::get<1>(tup_id)), 2.0);

    BOOST_CHECK_EQUAL(int(bgt::size<tuple_id>::value), 2);

    BOOST_CHECK_EQUAL((bgt::find_index_if<tuple_id, is_double>::value), 1);
    BOOST_CHECK((boost::is_same<typename bgt::find_if<tuple_id, is_double>::type, double>::value));

    BOOST_CHECK_EQUAL((bgt::find_index_if<tuple_id, is_float>::value), 2);
    BOOST_CHECK((boost::is_same<typename bgt::find_if<tuple_id, is_float>::type, boost::tuples::null_type>::value));

    typedef typename bgt::push_back<tuple_id, float>::type tuple_idf;
    tuple_idf tup_idf = bgt::push_back<tuple_id, float>::apply(tup_id, 3.0f);

    BOOST_CHECK_EQUAL((bgt::get<0>(tup_idf)), 1);
    BOOST_CHECK_EQUAL((bgt::get<1>(tup_idf)), 2.0);
    BOOST_CHECK_EQUAL((bgt::get<2>(tup_idf)), 3.0f);

    BOOST_CHECK_EQUAL(int(bgt::size<tuple_idf>::value), 3);

    BOOST_CHECK_EQUAL((bgt::find_index_if<tuple_idf, is_float>::value), 2);
    BOOST_CHECK((boost::is_same<typename bgt::find_if<tuple_idf, is_float>::type, float>::value));

#if !defined(BOOST_NO_CXX11_HDR_TUPLE) && !defined(BOOST_NO_VARIADIC_TEMPLATES)

    BOOST_CHECK((
        (is_boost_tuple<tuple_id>::value && is_boost_tuples_cons<tuple_idf>::value)
     || (!is_boost_tuple<tuple_id>::value && is_std_tuple<tuple_idf>::value)
        ));

#if !defined(BOOST_NO_CXX11_RVALUE_REFERENCES)

    tup_idf = bgt::push_back<tuple_id, float>::apply(std::move(tup_id), 3.0f);

    BOOST_CHECK_EQUAL((bgt::get<0>(tup_idf)), 1);
    BOOST_CHECK_EQUAL((bgt::get<1>(tup_idf)), 2.0);
    BOOST_CHECK_EQUAL((bgt::get<2>(tup_idf)), 3.0f);

#endif

#else

    BOOST_CHECK((is_boost_tuples_cons<tuple_idf>::type, float>::value));

#endif
}

int test_main(int, char* [])
{
    test_all<boost::tuple<int, double> >();
    test_all<std::pair<int, double> >();

#if !defined(BOOST_NO_CXX11_HDR_TUPLE) && !defined(BOOST_NO_VARIADIC_TEMPLATES)

    test_all<std::tuple<int, double> >();

#endif
    
    return 0;
}
