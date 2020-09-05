// Boost.Geometry

// Copyright (c) 2020, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_UTIL_SEQUENCE_HPP
#define BOOST_GEOMETRY_UTIL_SEQUENCE_HPP


#include <type_traits>


namespace boost { namespace geometry
{

namespace detail
{


// An alternative would be to use std:tuple and std::pair
//   but it would add dependency.


template <typename ...Ts>
struct type_sequence {};

template <typename First, typename Second>
struct type_pair
{
    typedef First first_type;
    typedef Second second_type;
};

template <typename T, T First, T Second>
struct integer_pair
{
    static_assert(std::is_integral<T>::value, "T has to be an integral type.");
    typedef T value_type;
    static const T first_value = First;
    static const T second_value = Second;
};



// merge<type_sequence<A, B>, type_sequence<C, D>>::type is
//   type_sequence<A, B, C, D>
template <typename ...Sequences>
struct merge;

template <typename S>
struct merge<S>
{
    typedef S type;
};

template <typename ...T1s, typename ...T2s>
struct merge<type_sequence<T1s...>, type_sequence<T2s...>>
{
    typedef type_sequence<T1s..., T2s...> type;
};

template <typename T, T ...I1s, T ...I2s>
struct merge<std::integer_sequence<T, I1s...>, std::integer_sequence<T, I2s...>>
{
    typedef std::integer_sequence<T, I1s..., I2s...> type;
};

template <typename S1, typename S2, typename ...Sequences>
struct merge<S1, S2, Sequences...>
{
    typedef typename merge
        <
            typename merge<S1, S2>::type,
            typename merge<Sequences...>::type
        >::type type;
};


// combine<type_sequence<A, B>, type_sequence<C, D>>::type is
//   type_sequence<type_pair<A, C>, type_pair<A, D>,
//                 type_pair<B, C>, type_pair<B, D>>
template <typename Sequence1, typename Sequence2>
struct combine;

template <typename ...T1s, typename ...T2s>
struct combine<type_sequence<T1s...>, type_sequence<T2s...>>
{
    template <typename U1, typename ...U2s>
    using type_sequence_t = type_sequence<type_pair<U1, U2s>...>;

    typedef typename merge
        <
            type_sequence_t<T1s, T2s...>...
        >::type type;
};

// combine<integer_sequence<T, 1, 2>, integer_sequence<T, 3, 4>>::type is
//   type_sequence<integer_pair<1, 3>, integer_pair<1, 4>,
//                 integer_pair<2, 3>, integer_pair<2, 4>>
template <typename T, T ...I1s, T ...I2s>
struct combine<std::integer_sequence<T, I1s...>, std::integer_sequence<T, I2s...>>
{
    template <T J1, T ...J2s>
    using type_sequence_t = type_sequence<integer_pair<T, J1, J2s>...>;

    typedef typename merge
        <
            type_sequence_t<I1s, I2s...>...
        >::type type;
};



// Selects least element from a parameter pack based on
// LessPred<T1, T2>::value comparison, similar to std::min_element
template
<
    template <typename, typename> class LessPred,
    typename ...Ts
>
struct select_pack_element;

template
<
    template <typename, typename> class LessPred,
    typename T
>
struct select_pack_element<LessPred, T>
{
    typedef T type;
};

template
<
    template <typename, typename> class LessPred,
    typename T1, typename T2
>
struct select_pack_element<LessPred, T1, T2>
{
    typedef std::conditional_t<LessPred<T1, T2>::value, T1, T2> type;
};

template
<
    template <typename, typename> class LessPred,
    typename T1, typename T2, typename ...Ts
>
struct select_pack_element<LessPred, T1, T2, Ts...>
{
    typedef typename select_pack_element
        <
            LessPred,
            typename select_pack_element<LessPred, T1, T2>::type,
            typename select_pack_element<LessPred, Ts...>::type
        >::type type;
};


// Selects least element from a sequence based on
// LessPred<T1, T2>::value comparison, similar to std::min_element
template
<
    typename Sequence,
    template <typename, typename> class LessPred    
>
struct select_element;

template
<
    typename ...Ts,
    template <typename, typename> class LessPred
>
struct select_element<type_sequence<Ts...>, LessPred>
{
    typedef typename select_pack_element<LessPred, Ts...>::type type;
};


// Selects least pair of elements from a parameter pack based on
// LessPred<xxx_pair<T11, T12>, xxx_pair<T21, T22>>::value comparison
template
<
    typename Sequence1,
    typename Sequence2,
    template <typename, typename> class LessPred
>
struct select_combination_element
{
    typedef typename select_element
        <
            typename combine<Sequence1, Sequence2>::type,
            LessPred
        >::type type;
};


} // namespace detail


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_UTIL_SEQUENCE_HPP
