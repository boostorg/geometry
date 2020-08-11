// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2020 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_ERROR_BOUND_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_ERROR_BOUND_HPP

#include <limits>

#include <boost/mp11/integral.hpp>
#include <boost/mp11/list.hpp>
#include <boost/mp11/map.hpp>

#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/coefficient_list.hpp>

namespace boost { namespace geometry
{

namespace detail { namespace generic_robust_predicates
{

//The templates in this file are utilities for the generation of error
//expressions for given arithmetic expressions. This is done using forward
//error analysis methods at compile time. This analysis produces an error map
//that is folded into an error expression.
//
//An Error Map is a template fulfilling the boost::mp11 map concept. The keys K
//are expressions (as in expression trees and the values V are lists of
//integers. Each key-value pair represents the contribution of an error term to
//the total error, i.e. the error expression is something like the sum of
//abs<K> * (V[0] * epsilon + V[1] * epsilon^2 + ...)
//
//over all key-value pairs KV in the error map.
//
//See p. 39 in "Adaptive Precision Floating-Point Arithmetic and Fast Robust
//Geometric Predicates" by Jonathan Richard Shewchuk (can be downloaded from
//https://www.cs.cmu.edu/~quake/robust.html) for the inspiration for the
//methods implemented below.

template <typename KV> using second_is_coeff_list =
    is_coeff_list< boost::mp11::mp_second<KV> >;

template <typename EM> using is_error_map = boost::mp11::mp_and
    <
        boost::mp11::mp_is_map<EM>,
        boost::mp11::mp_all_of<EM, second_is_coeff_list>,
        boost::mp11::mp_similar<EM, boost::mp11::mp_list<>
    >
    //TODO: It would be desirable to also validate that the keys are good
>;

template <typename KV, typename M>
struct error_map_insert_impl
{
private:
    using key = boost::mp11::mp_front<KV>;
    using value = boost::mp11::mp_second<KV>;
    using other_value = typename mp_map_at_second_or_void<M, key>::type;
    using merged_value = coeff_merge<value, other_value>;
    using nkv = boost::mp11::mp_list<key, merged_value>;
public:
    using type = boost::mp11::mp_map_replace<M, nkv>;
};

template <typename KV, typename M> using error_map_insert =
    typename error_map_insert_impl<KV, M>::type;

template <typename M1, typename M2>
struct add_fold_operator
{
public:
    template <typename M, typename K> using fn = boost::mp11::mp_map_insert
        <
            M,
            boost::mp11::mp_list
                <
                    K,
                    coeff_merge
                        <
                            typename mp_map_at_second_or_void<M1, K>::type,
                            typename mp_map_at_second_or_void<M2, K>::type
                        >
                >
        >;
};

template <typename M1, typename M2> using add_children = boost::mp11::mp_fold
    <
        boost::mp11::mp_set_union
            <
                boost::mp11::mp_map_keys<M1>,
                boost::mp11::mp_map_keys<M2>
            >,
        boost::mp11::mp_list<>,
        add_fold_operator<M1, M2>::template fn
    >;

template
<
    typename Exp,
    typename EM,
    typename Out,
    typename LErr = typename val_or_empty_list<EM, typename Exp::left>::type,
    typename RErr = typename val_or_empty_list<EM, typename Exp::right>::type,
    typename skip_decompose =
        boost::mp11::mp_or
            <
                boost::mp11::mp_not
                    <
                        boost::mp11::mp_same
                            <
                                typename Exp::error_type,
                                sum_error_type
                            >
                    >,
                boost::mp11::mp_same
                    <
                        boost::mp11::mp_list<>,
                        LErr,
                        RErr
                    >
            >
>
struct decompose_add_impl
{
private:
    using decomp_left = typename decompose_add_impl
        <
            typename Exp::left,
            EM,
            Out
        >::type;
    using decomp_right = typename decompose_add_impl
        <
            typename Exp::right,
            EM,
            decomp_left
        >::type;
public:
    using type = decomp_right;
};

template
<
    typename Exp,
    typename EM,
    typename Out,
    typename LErr,
    typename RErr
>
struct decompose_add_impl
    <
        Exp,
        EM,
        Out,
        LErr,
        RErr,
        boost::mp11::mp_true
    >
{
    using type = error_map_insert
        <
            boost::mp11::mp_list
                <
                    Exp,
                    boost::mp11::mp_list<boost::mp11::mp_int<1>>
                >,
            Out
        >;
};

template
<
    typename Exp,
    typename EM,
    typename Out
>
using decompose_add = typename decompose_add_impl<Exp, EM, Out>::type;

template
<
    typename Exp,
    typename EM,
    typename LErr = typename val_or_empty_list<EM, typename Exp::left>::type,
    typename RErr = typename val_or_empty_list<EM, typename Exp::right>::type,
    typename Children_Empty =
        boost::mp11::mp_and
            <
                typename empty_or_void<LErr>::type,
                typename empty_or_void<RErr>::type
            >
>
struct sum_err_impl
{
private:
    static_assert(is_error_map<LErr>::value, "LErr needs to be a valid error map.");
    static_assert(is_error_map<RErr>::value, "RErr needs to be a valid error map.");
    using children = add_children<LErr, RErr>;
    static_assert(
            is_error_map<children>::value, "children needs to be a valid error map.");
public:
    /*
    using type = boost::mp11::mp_map_insert<
        children,
        boost::mp11::mp_list<Exp, boost::mp11::mp_list<boost::mp11::mp_int<1>>>
    >;*/
    using type = decompose_add<Exp, EM, children>;
    static_assert(is_error_map<type>::value, "type needs to be a valid error map.");
};

template
<
    typename Exp,
    typename EM,
    typename LErr,
    typename RErr
>
struct sum_err_impl
    <
        Exp,
        EM,
        LErr,
        RErr,
        boost::mp11::mp_true
    >
{
    static_assert(is_error_map<LErr>::value, "LErr needs to be a valid error map.");
    static_assert(is_error_map<RErr>::value, "RErr needs to be a valid error map.");
    using type = boost::mp11::mp_list
        <
            boost::mp11::mp_list
                <
                    Exp,
                    boost::mp11::mp_list<boost::mp11::mp_int<1>>
                >
        >;
    static_assert(is_error_map<type>::value, "type needs to be a valid error map.");
};

template <typename Exp, typename EM> using sum_err =
    typename sum_err_impl<Exp, EM>::type;

template <typename L> using pad_second = boost::mp11::mp_list
    <
        boost::mp11::mp_front<L>,
        boost::mp11::mp_push_front
            <
                boost::mp11::mp_second<L>,
                boost::mp11::mp_int<0>
            >
    >;

template <typename L> using pop_front_second = boost::mp11::mp_list
    <
        boost::mp11::mp_front<L>,
        boost::mp11::mp_pop_front<boost::mp11::mp_second<L>>
    >;

template <typename K, typename V> using increment_first_of_second =
    boost::mp11::mp_transform_front<V, inc>;

template <typename KV1, typename KV2> using prod_entry_merge =
    boost::mp11::mp_list
        <
            boost::mp11::mp_flatten
                <
                    boost::mp11::mp_list
                        <
                            boost::mp11::mp_first<KV1>,
                            boost::mp11::mp_first<KV2>
                        >
                >,
            list_product
                <
                    boost::mp11::mp_second<KV1>,
                    boost::mp11::mp_second<KV2>
                >
        >;

template
<
    typename Exp,
    typename EM,
    typename LErr = typename val_or_empty_list<EM, typename Exp::left>::type,
    typename RErr = typename val_or_empty_list<EM, typename Exp::right>::type
>
struct prod_children_impl
{
private:
    static_assert(is_error_map<LErr>::value, "LErr needs to be a valid error map.");
    static_assert(is_error_map<RErr>::value, "RErr needs to be a valid error map.");
    using left = typename Exp::left;
    using right = typename Exp::right;
    using padded_lerr = boost::mp11::mp_transform<pad_second, LErr>;
    using added_left_lerr = decompose_add<left, EM, padded_lerr>;
    using padded_rerr = boost::mp11::mp_transform<pad_second, RErr>;
    using added_right_rerr = decompose_add<right, EM, padded_rerr>;
    using prod = boost::mp11::mp_product
        <
            prod_entry_merge,
            added_left_lerr,
            added_right_rerr
        >;
    using stripped_prod = boost::mp11::mp_transform<pop_front_second, prod>;
public:
    using type = stripped_prod;
    static_assert(is_error_map<type>::value, "type needs to be a valid error map.");
};

template <typename Exp, typename EM> using prod_children =
    typename prod_children_impl<Exp, EM>::type;

template
<
    typename Exp,
    typename EM,
    typename LErr = typename val_or_empty_list<EM, typename Exp::left>::type,
    typename RErr = typename val_or_empty_list<EM, typename Exp::right>::type
>
struct product_err_impl
{
private:
    static_assert(is_error_map<LErr>::value, "LErr needs to be a valid error map.");
    static_assert(is_error_map<RErr>::value, "RErr needs to be a valid error map.");
    using children = prod_children<Exp, EM>;
    static_assert(is_error_map<children>::value, "children needs to be a valid error map.");
public:
    using type = boost::mp11::mp_map_insert
        <
            children,
            boost::mp11::mp_list
                <
                    Exp,
                    boost::mp11::mp_list< boost::mp11::mp_int<1> >
                >
        >;
    static_assert(is_error_map<type>::value, "type needs to be a valid error map.");
};

template <typename Exp, typename EM> using product_err =
    typename product_err_impl<Exp, EM>::type;

template
<
    typename Errors,
    typename Exp,
    typename IsSum = boost::mp11::mp_same
        <
            typename Exp::error_type,
            sum_error_type
        >
>
struct sum_or_product_err_impl
{
    using type = sum_err<Exp, Errors>;
};

template
<
    typename Errors,
    typename Exp
>
struct sum_or_product_err_impl<Errors, Exp, boost::mp11::mp_false>
{
    using type = product_err<Exp, Errors>;
};

template <typename Errors, typename Exp> using sum_or_product_err =
    typename sum_or_product_err_impl<Errors, Exp>::type;

template
<
    typename Errors,
    typename Exp
>
struct error_fold_impl
{
private:
    using lerr = typename val_or_empty_list<Errors, typename Exp::left>::type;
    using rerr = typename val_or_empty_list<Errors, typename Exp::right>::type;
    using err = sum_or_product_err<Errors, Exp>;
public:
    using type = boost::mp11::mp_map_insert
        <
            Errors,
            boost::mp11::mp_list<Exp, err>
        >;
};


template <typename Errors, typename Exp> using error_fold =
    typename error_fold_impl<Errors, Exp>::type;

template <typename Evals> using evals_error =
    boost::mp11::mp_fold
        <
            Evals,
            boost::mp11::mp_list<>,
            error_fold
        >;

template <typename T> using is_mp_list = boost::mp11::mp_similar
    <
        boost::mp11::mp_list<>,
        T
    >;

template <typename KV>
struct list_to_product_impl
{
private:
    using key = boost::mp11::mp_front<KV>;
    using value = boost::mp11::mp_second<KV>;
    using multiplications = boost::mp11::mp_int<boost::mp11::mp_size<key>::value - 1>;
    using nvalue = mult_by_1_p_eps_pow<value, multiplications>;
    using nkey = boost::mp11::mp_reverse_fold<
        boost::mp11::mp_pop_back<key>,
        boost::mp11::mp_back<key>,
        product
    >;
public:
    using type = boost::mp11::mp_list<nkey, nvalue>;
};

template <typename KV> using list_to_product = typename list_to_product_impl<KV>::type;

template
<
    typename M,
    typename KV,
    typename KeyMPList = is_mp_list< boost::mp11::mp_front<KV> >
>
struct error_map_list_to_product_fold_impl
{
    using type = error_map_insert<list_to_product<KV>, M>;
};

template <typename M, typename KV>
struct error_map_list_to_product_fold_impl
    <
        M,
        KV,
        boost::mp11::mp_false
    >
{
    using type = error_map_insert<KV, M>;
};

template <typename M, typename KV> using error_map_list_to_product_fold =
    typename error_map_list_to_product_fold_impl<M, KV>::type;

template <typename M>
struct error_map_list_to_product_impl
{
    using type = boost::mp11::mp_fold
        <
            M,
            boost::mp11::mp_list<>,
            error_map_list_to_product_fold
        >;
};

template <typename M> using error_map_list_to_product =
    typename error_map_list_to_product_impl<M>::type;

template <typename Expression>
struct abs_unless_non_negative
{
    using type = boost::mp11::mp_if_c
        <
            Expression::non_negative,
            Expression,
            abs<Expression>
        >;
};

template <typename EM, typename KV> using abs_fold =
    boost::mp11::mp_push_back
        <
            EM,
            boost::mp11::mp_list
                <
                    typename abs_unless_non_negative<boost::mp11::mp_front<KV>>
                        ::type,
                    boost::mp11::mp_second<KV>
                >
        >;

template<typename EM> using abs_all =
    boost::mp11::mp_fold
        <
            EM,
            boost::mp11::mp_list<>,
            abs_fold
        >;

template
<
    typename KV1,
    typename KV2
>
struct abs_sum_error_term_impl
{
private:
    using key1 = boost::mp11::mp_front<KV1>;
    using key2 = boost::mp11::mp_front<KV2>;
    using nkey = sum<key1, key2>;
    using val1 = boost::mp11::mp_second<KV1>;
    using val2 = boost::mp11::mp_second<KV2>;
    using mval = coeff_max<val1, val2>;
    static_assert(is_coeff_list<mval>::value, "merged coefficient list fails coefficient list check");
    using nval = mult_by_1_p_eps<mval>;
public:
    using type = boost::mp11::mp_list<nkey, nval>;
};

template<typename KV1, typename KV2> using abs_sum_error_term = typename abs_sum_error_term_impl<KV1, KV2>::type;

//TODO: The following could be probably optimized for potentially produce better error bounds in some cases
//      if the error map is treated as a minheap by ordering of epsilon-polynomial.
template<typename M> using error_map_sum_up =
    boost::mp11::mp_fold
        <
            boost::mp11::mp_pop_front<M>,
            boost::mp11::mp_first<M>,
            abs_sum_error_term
        >;

template<typename M, std::size_t MS = boost::mp11::mp_size<M>::value>
struct error_map_balanced_sum_up_impl
{
private:
    static constexpr std::size_t mid = MS/2;
    using left = typename error_map_balanced_sum_up_impl
        <
            boost::mp11::mp_erase_c<M, 0, mid>
        >::type;
    using right = typename error_map_balanced_sum_up_impl
        <
            boost::mp11::mp_erase_c<M, mid, MS>
        >::type;
public:
    using type = abs_sum_error_term<left, right>;
};

template<typename M>
struct error_map_balanced_sum_up_impl<M, 1>
{
    using type = boost::mp11::mp_front<M>;
};

template<typename M> using error_map_balanced_sum_up =
    typename error_map_balanced_sum_up_impl<M>::type;

template
<
    typename Real,
    typename Exp
>
struct eps_pow
{
    static constexpr Real value =
          std::numeric_limits<Real>::epsilon()/2.0
        * eps_pow<Real, boost::mp11::mp_size_t< Exp::value - 1 >>::value;
};

template <typename Real> struct eps_pow<Real, boost::mp11::mp_size_t<0>>
{
    static constexpr Real value = 1.0;
};

template
<
    typename Real,
    typename L,
    typename S = boost::mp11::mp_size<L>
>
struct eval_eps_polynomial
{
private:
    using last = boost::mp11::mp_back<L>;
    using s2last = boost::mp11::mp_back< boost::mp11::mp_pop_back<L> >;
public:
    static constexpr Real value =
          s2last::value
        * eps_pow<Real, boost::mp11::mp_size_t<S::value - 1>>::value
        + last::value * eps_pow<Real, S>::value;
};

template
<
    typename Real,
    typename L
>
struct eval_eps_polynomial<Real, L, boost::mp11::mp_size_t<1>>
{
    static constexpr Real value =
          boost::mp11::mp_front<L>::value
        * std::numeric_limits<Real>::epsilon() / 2.0;
};

template
<
    typename Real,
    typename L
>
struct eval_eps_polynomial<Real, L, boost::mp11::mp_size_t<0>>
{
    static constexpr Real value = 0;
};

}} // namespace detail::generic_robust_predicates

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_ERROR_BOUND_HPP
