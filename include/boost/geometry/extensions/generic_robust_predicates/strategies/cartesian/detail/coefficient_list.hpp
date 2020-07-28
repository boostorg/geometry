// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2020 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_COEFFICIENT_LIST_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_COEFFICIENT_LIST_HPP

#include <cstddef>

#include <boost/mp11/integral.hpp>
#include <boost/mp11/list.hpp>

#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/static_util.hpp>

namespace boost { namespace geometry
{

namespace detail { namespace generic_robust_predicates
{

//only meant to be used in assertions
template <typename T> using is_mp_int =
    boost::mp11::mp_same<T, boost::mp11::mp_int<T::value>>;

template <typename L> using is_coeff_list = boost::mp11::mp_and
    <
        boost::mp11::mp_all_of<L, is_mp_int>,
        boost::mp11::mp_similar<L, boost::mp11::mp_list<>>
    >;

template <typename L>
struct coeff_truncate_impl
{
private:
    static_assert(is_coeff_list<L>::value, "L must be a coefficient list");
    using first_nz = boost::mp11::mp_find_if<L, is_not_zero>;
    using after_second_nz =
        boost::mp11::mp_min<
            boost::mp11::mp_size_t<first_nz::value + 2>,
            boost::mp11::mp_size<L>
        >;
    using tail = boost::mp11::mp_erase_c<L, 0, after_second_nz::value>;
    using head = boost::mp11::mp_erase
            <
                L,
                after_second_nz,
                boost::mp11::mp_size<L>
            >;
    using round_up = boost::mp11::mp_less
            <
                boost::mp11::mp_find_if<tail, is_not_zero>,
                boost::mp11::mp_size<tail>
            >;
public:
    using type = boost::mp11::mp_if
            <
                round_up,
                boost::mp11::mp_push_back<head, boost::mp11::mp_int<1>>,
                head
            >;
    static_assert(is_coeff_list<L>::value, "type should be a coefficient list");
};

template <typename L> using coeff_truncate = typename coeff_truncate_impl<L>::type;

template <typename L> using app_zero_b =
    boost::mp11::mp_push_front<L, boost::mp11::mp_int<0>>;
template <typename L> using app_zero_f =
    boost::mp11::mp_push_back<L, boost::mp11::mp_int<0>>;
template <typename L> using mult_by_1_p_eps = coeff_truncate
        <
            boost::mp11::mp_transform
                <
                    boost::mp11::mp_plus,
                    app_zero_b<L>,
                    app_zero_f<L>
                >
        >;

template
<
    typename L,
    typename N,
    typename done = boost::mp11::mp_bool<N::value == 0>
>
struct mult_by_1_p_eps_pow_impl
{
private:
    using next = mult_by_1_p_eps<L>;
public:
    using type =
        typename mult_by_1_p_eps_pow_impl
            <
                next,
                boost::mp11::mp_int<N::value - 1>
            >::type;
};

template <typename L, typename N>
struct mult_by_1_p_eps_pow_impl<L, N, boost::mp11::mp_true>
{
public:
    using type = L;
};

template <typename L, typename N> using mult_by_1_p_eps_pow = coeff_truncate
        <
            typename mult_by_1_p_eps_pow_impl<L, N>::type
        >;

template <typename L> using div_by_1_m_eps_helper =
    boost::mp11::mp_partial_sum
        <
            L,
            boost::mp11::mp_int<0>,
            boost::mp11::mp_plus
        >;

template <typename L> using div_by_1_m_eps = coeff_truncate
    <
        boost::mp11::mp_push_back
            <
                boost::mp11::mp_pop_back<div_by_1_m_eps_helper<L>>,
                inc<boost::mp11::mp_back<div_by_1_m_eps_helper<L>>>
            >
    >;

template
<
    typename L1,
    typename L2,
    typename L,
    typename L1_Empty = typename empty_or_void<L1>::type,
    typename L2_Empty = typename empty_or_void<L2>::type
>
struct coeff_merge_impl {};

template
<
    typename L1,
    typename L2,
    typename L
>
struct coeff_merge_impl
<
    L1,
    L2,
    L,
    boost::mp11::mp_false,
    boost::mp11::mp_false
>
{
    using type = typename coeff_merge_impl
        <
            boost::mp11::mp_pop_front<L1>,
            boost::mp11::mp_pop_front<L2>,
            boost::mp11::mp_push_back
                <
                    L,
                    boost::mp11::mp_plus
                        <
                            boost::mp11::mp_front<L1>,
                            boost::mp11::mp_front<L2>
                        >
                >
        >::type;
};

template
<
    typename L1,
    typename L2,
    typename L
>
struct coeff_merge_impl
    <
        L1,
        L2,
        L,
        boost::mp11::mp_true,
        boost::mp11::mp_true
    >
{
    using type = L;
};

template
<
    typename L1,
    typename L2,
    typename L
>
struct coeff_merge_impl
    <
        L1,
        L2,
        L,
        boost::mp11::mp_true,
        boost::mp11::mp_false
    >
{
    using type = boost::mp11::mp_append<L, L2>;
};

template
<
    typename L1,
    typename L2,
    typename L
>
struct coeff_merge_impl
    <
        L1,
        L2,
        L,
        boost::mp11::mp_false,
        boost::mp11::mp_true
    >
{
    using type = boost::mp11::mp_append<L, L1>;
};

template <typename L1, typename L2> using coeff_merge =
    coeff_truncate
        <
            typename coeff_merge_impl
                <
                    L1,
                    L2,
                    boost::mp11::mp_list<>
                >::type
        >;

template
<
    typename L1,
    typename L2,
    typename L,
    typename L1_Empty = typename empty_or_void<L1>::type,
    typename L2_Empty = typename empty_or_void<L2>::type
>
struct coeff_max_impl {};

template
<
    typename L1,
    typename L2,
    typename L
>
struct coeff_max_impl<L1, L2, L, boost::mp11::mp_false, boost::mp11::mp_false>
{
    using type = typename coeff_max_impl
        <
            boost::mp11::mp_if
                <
                    boost::mp11::mp_less
                        <
                            boost::mp11::mp_front<L1>,
                            boost::mp11::mp_front<L2>
                        >,
                    boost::mp11::mp_list<>,
                    boost::mp11::mp_pop_front<L1>
                >,
            boost::mp11::mp_if
                <
                    boost::mp11::mp_less
                        <
                            boost::mp11::mp_front<L2>,
                            boost::mp11::mp_front<L1>
                        >,
                    boost::mp11::mp_list<>,
                    boost::mp11::mp_pop_front<L2>
                >,
            boost::mp11::mp_push_back
                <
                    L,
                    boost::mp11::mp_max
                        <
                            boost::mp11::mp_front<L1>,
                            boost::mp11::mp_front<L2>
                        >
                >
        >::type;
};

template
<
    typename L1,
    typename L2,
    typename L
>
struct coeff_max_impl
    <
        L1,
        L2,
        L,
        boost::mp11::mp_true,
        boost::mp11::mp_true
    >
{
    using type = L;
};

template
<
    typename L1,
    typename L2,
    typename L
>
struct coeff_max_impl
    <
        L1,
        L2,
        L,
        boost::mp11::mp_true,
        boost::mp11::mp_false
    >
{
    using type = boost::mp11::mp_append<L, L2>;
};

template
<
    typename L1,
    typename L2,
    typename L
>
struct coeff_max_impl
    <
        L1,
        L2,
        L,
        boost::mp11::mp_false,
        boost::mp11::mp_true
    >
{
    using type = boost::mp11::mp_append<L, L1>;
};

template <typename L1, typename L2> using coeff_max =
    coeff_truncate
        <
            typename coeff_max_impl
                <
                    L1,
                    L2,
                    boost::mp11::mp_list<>
                >::type
        >;

template
<
    typename L,
    std::size_t Tail_Size =
          boost::mp11::mp_size<L>::value
        - boost::mp11::mp_find_if<L, is_not_zero>::value
>
struct coeff_round_impl
{
private:
    using first_nz = boost::mp11::mp_find_if<L, is_not_zero>;
    using tail = boost::mp11::mp_erase_c<L, 0, first_nz::value + 2>;
    using zero_tail = boost::mp11::mp_same
        <
            boost::mp11::mp_find_if<tail, is_not_zero>,
            boost::mp11::mp_size<tail>
        >;
    using head = boost::mp11::mp_erase_c
        <
            L,
            first_nz::value + 2,
            boost::mp11::mp_size<L>::value
        >;
    using minor = boost::mp11::mp_if
        <
            zero_tail,
            boost::mp11::mp_back<head>,
            inc<boost::mp11::mp_back<head>>
        >;
    using major = boost::mp11::mp_at<head, first_nz>;
    using major_rounded = boost::mp11::mp_int< 1 << log_2_ceil<major>::value >;
    using minor_rounded = boost::mp11::mp_int
        <
            (minor::value / major_rounded::value) * major_rounded::value < minor::value ?
                  (minor::value / major_rounded::value + 1) * major_rounded::value
                : (minor::value / major_rounded::value) * major_rounded::value
        >;
public:
    using type = boost::mp11::mp_push_back
        <
            boost::mp11::mp_pop_back<head>,
            minor_rounded
        >;
};

template <typename L>
struct coeff_round_impl<L, 0> { using type = L; };

template <typename L>
struct coeff_round_impl<L, 1> { using type = L; };

template <typename L> using coeff_round = typename coeff_round_impl<L>::type;

template <typename IV1, typename IV2> using indexed_value_product =
    boost::mp11::mp_list
        <
            boost::mp11::mp_plus
                <
                    boost::mp11::mp_first<IV1>,
                    boost::mp11::mp_first<IV2>
                >,
            boost::mp11::mp_int
                <
                      boost::mp11::mp_second<IV1>::value
                    * boost::mp11::mp_second<IV2>::value
                >
        >;

template <typename V1> struct add_nested
{
    template <typename K, typename V2> using fn =
        boost::mp11::mp_plus<V1, V2>;
};

template <typename M, typename IV> using list_product_fold =
    boost::mp11::mp_map_update_q
        <
            M,
            IV,
            add_nested< boost::mp11::mp_second<IV> >
        >;

template<typename IV1, typename IV2> using index_less =
    boost::mp11::mp_less
        <
            boost::mp11::mp_first<IV1>,
            boost::mp11::mp_first<IV2>
        >;

template <typename L1, typename L2> using list_product =
    strip_index
        <
            boost::mp11::mp_sort
                <
                    boost::mp11::mp_fold
                        <
                            boost::mp11::mp_product
                                <
                                    indexed_value_product,
                                    indexed<L1>,
                                    indexed<L2>
                                >,
                            boost::mp11::mp_list<>,
                            list_product_fold
                        >,
                    index_less
                >
        >;

}} // namespace detail::generic_robust_predicates

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_COEFFICIENT_LIST_HPP
