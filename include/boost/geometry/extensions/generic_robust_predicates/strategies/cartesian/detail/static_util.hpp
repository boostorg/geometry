// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2020 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_STATIC_UTIL_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_STATIC_UTIL_HPP

#include <boost/mp11/integral.hpp>
#include <boost/mp11/list.hpp>
#include <boost/mp11/map.hpp>


namespace boost { namespace geometry
{

namespace detail { namespace generic_robust_predicates
{

template
<
    typename LOV,
    typename Void = boost::mp11::mp_same<LOV, void>
>
struct empty_or_void
{
    using type = boost::mp11::mp_empty<LOV>;
};

template <typename LOV>
struct empty_or_void<LOV, boost::mp11::mp_true>
{
    using type = boost::mp11::mp_true;
};

template <typename N>
struct log_2_floor_impl
{
    using type = typename boost::mp11::mp_plus
        <
            typename log_2_floor_impl<boost::mp11::mp_int<N::value / 2>>::type,
            boost::mp11::mp_int<1>
        >;
};

template <>
struct log_2_floor_impl<boost::mp11::mp_int<1>>
{
    using type = boost::mp11::mp_int<0>;
};

template <typename N> using log_2_floor = typename log_2_floor_impl<N>::type;

template <typename N>
struct log_2_ceil_impl
{
private:
    using floor = log_2_floor<N>;
public:
    using type = boost::mp11::mp_int
        <
            (1 << floor::value) == N::value ? floor::value : floor::value + 1
        >;
};

template <typename N> using log_2_ceil = typename log_2_ceil_impl<N>::type;

template <typename S> using is_not_zero = boost::mp11::mp_bool<S::value != 0>;

template
<
    typename M,
    typename K,
    typename Contained = boost::mp11::mp_map_contains<M, K>
>
struct mp_map_at_second_or_void
{
    using type = void;
};

template<typename M, typename K>
struct mp_map_at_second_or_void<M, K, boost::mp11::mp_true>
{
    using type = boost::mp11::mp_second<boost::mp11::mp_map_find<M, K>>;
};

template <typename T> using inc = boost::mp11::mp_int<T::value + 1>;

template <typename L> using indexed = boost::mp11::mp_transform
    <
        boost::mp11::mp_list,
        boost::mp11::mp_iota<boost::mp11::mp_size<L>>,
        L
    >;

template <typename L> using strip_index =
    boost::mp11::mp_transform<boost::mp11::mp_second, L>;

template
<
    typename Map,
    typename Key,
    typename Contains = boost::mp11::mp_map_contains<Map, Key>
>
struct val_or_empty_list
{
    using type = boost::mp11::mp_second<boost::mp11::mp_map_find<Map, Key>>;
};

template
<
    typename Map,
    typename Key
>
struct val_or_empty_list<Map, Key, boost::mp11::mp_false>
{
    using type = boost::mp11::mp_list<>;
};

}} // namespace detail::generic_robust_predicates

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_STATIC_UTIL_HPP
