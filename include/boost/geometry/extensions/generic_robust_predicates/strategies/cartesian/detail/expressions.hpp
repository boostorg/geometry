// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2020 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_EXPRESSIONS_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_EXPRESSIONS_HPP

#include <boost/mp11/integral.hpp>
#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/list.hpp>
#include <boost/mp11/utility.hpp>

#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/expression_tree.hpp>

namespace boost { namespace geometry
{

namespace detail { namespace generic_robust_predicates
{

static constexpr std::size_t sizet_sqrt(std::size_t n)
{
    std::size_t out = 0;
    while (out * out < n)
    {
        ++out;
    }
    return out;
}

template <typename I, typename Exp>
struct indexed_exp
{
    using index = I;
    using exp = Exp;
};

template <typename IExp>
using extract_exp = typename IExp::exp;

template <typename IExps>
using extract_exps = boost::mp11::mp_transform<extract_exp, IExps>;

template <typename I, typename N>
struct remove_from_minor_q
{
    template <typename IExp>
    using fn = boost::mp11::mp_bool
        <
               IExp::index::value / N::value == I::value
            || IExp::index::value % N::value == 0
        >;
};

template <typename N, typename IExps>
struct minor_q
{
    template <typename I>
    using fn = boost::mp11::mp_remove_if_q<IExps, remove_from_minor_q<I, N>>;
};

template <typename N, typename IExps>
struct minor_factor_q
{
    template <typename I>
    using fn = typename boost::mp11::mp_at_c<IExps, I::value * N::value>::exp;
};

template <typename ...Exps>
struct alternating_sum_impl {};

template <typename Exp1, typename Exp2, typename Exp3, typename ...Exps>
struct alternating_sum_impl<Exp1, Exp2, Exp3, Exps...>
{
    using type = sum
        <
            difference<Exp1, Exp2>,
            typename alternating_sum_impl<Exp3, Exps...>::type
        >;
};

template <typename Exp1, typename Exp2>
struct alternating_sum_impl<Exp1, Exp2>
{
    using type = difference<Exp1, Exp2>;
};

template <typename Exp>
struct alternating_sum_impl<Exp>
{
    using type = Exp;
};

template <typename ...Exps>
using alternating_sum = typename alternating_sum_impl<Exps...>::type;

template <typename ...Exps>
struct det_impl;

template <typename Exps>
using make_det =
    typename boost::mp11::mp_apply<det_impl, Exps>::type;

template <typename ...Exps>
struct det_impl
{
private:
    static constexpr std::size_t N = sizet_sqrt(sizeof...(Exps));
    static_assert(N * N == sizeof...(Exps),
                  "Arguments must be a square matrix.");
    using indices = boost::mp11::mp_iota_c<N * N>;
    using minor_indices = boost::mp11::mp_iota_c<N>;
    using indexed_exps = boost::mp11::mp_transform
        <
            indexed_exp,
            indices,
            boost::mp11::mp_list<Exps...>
        >;
    using indexed_minors = boost::mp11::mp_transform_q
        <
            minor_q<boost::mp11::mp_size_t<N>, indexed_exps>,
            minor_indices
        >;
    using minors = boost::mp11::mp_transform<extract_exps, indexed_minors>;
    using minor_dets = boost::mp11::mp_transform<make_det, minors>;
    using minor_factors = boost::mp11::mp_transform_q
        <
            minor_factor_q<boost::mp11::mp_size_t<N>, indexed_exps>,
            minor_indices
        >;
    using alt_summands = boost::mp11::mp_transform
        <
            product,
            minor_factors,
            minor_dets
        >;
public:
    using type = boost::mp11::mp_apply
        <
            alternating_sum,
            alt_summands
        >;
};

template <typename Exp>
struct det_impl<Exp>
{
    using type = Exp;
};

template <typename ...Exps>
using det = typename det_impl<Exps...>::type;

template <std::size_t N>
struct orient_entry
{
    template <typename I>
    using fn = difference
        <
            argument<I::value + 1>,
            argument<N * N + I::value % N + 1>
        >;
};

template <typename I>
using collinear_entry = argument<I::value + 1>;

template <std::size_t N>
using orient = boost::mp11::mp_apply
    <
        det,
        boost::mp11::mp_transform_q
            <
                orient_entry<N>,
                boost::mp11::mp_iota_c< N * N >
            >
    >;

template <std::size_t N>
using collinear = boost::mp11::mp_apply
    <
        det,
        boost::mp11::mp_transform
            <
                collinear_entry,
                boost::mp11::mp_iota_c<N * N>
            >
    >;

template <std::size_t N>
struct incircle_entry;

template <std::size_t N, std::size_t Offset>
struct incircle_entry_offset
{
    template <typename I>
    using fn = typename incircle_entry<N>::template fn
        <
            boost::mp11::mp_size_t<I::value + Offset>
        >;
};

template <typename ...Exps>
struct multi_sum_impl;

template <typename Exp, typename ...Exps>
struct multi_sum_impl<Exp, Exps...>
{
    using type = sum<Exp, typename multi_sum_impl<Exps...>::type>;
};

template <typename Exp>
struct multi_sum_impl<Exp>
{
    using type = Exp;
};

template <>
struct multi_sum_impl<>
{
    using type = void;
};

template <std::size_t Offset>
struct size_t_offset_q
{
    template <typename I>
    using fn = boost::mp11::mp_size_t<I::value + Offset>;
};

template <std::size_t N>
struct incircle_entry
{
private:
    template <typename I>
    using diff = difference
            <
                argument
                    <
                          N * ((I::value) / (N + 1))
                        + (I::value) % (N + 1) + 1
                    >,
                argument
                    <
                        N * (N + 1) + (I::value) % (N + 1) + 1
                    >
            >;
    template <std::size_t I>
    using lift_indices = boost::mp11::mp_transform_q
        <
            size_t_offset_q<(N+ 1) * (I / (N + 1))>,
            boost::mp11::mp_iota_c<N>
        >;

    template <std::size_t I>
    using line_diffs =
        boost::mp11::mp_transform
            <
                diff,
                lift_indices<I>
            >;

    template <std::size_t I>
    using line_squares =
        boost::mp11::mp_transform
            <
                product,
                line_diffs<I>,
                line_diffs<I>
            >;

    template <std::size_t I>
    using lift = typename boost::mp11::mp_apply
        <
            multi_sum_impl,
            line_squares<I>
        >::type;
public:
    template <typename I>
    using fn =
        boost::mp11::mp_if_c
            <
                (I::value % (N + 1) != 0),
                diff<boost::mp11::mp_size_t<I::value - 1>>,
                lift<I::value + N>
            >;
};

template <std::size_t N>
using innsphere = boost::mp11::mp_apply
    <
        det,
        boost::mp11::mp_transform_q
            <
                incircle_entry<N>,
                boost::mp11::mp_iota_c< (N + 1) * (N + 1) >
            >
    >;

using orient2d = orient<2>;
using orient3d = orient<3>;
using incircle = innsphere<2>;
using insphere = innsphere<3>;

struct incircle_simplified_impl
{
private:
    using qpx = difference<argument<3>, argument<7>>;
    using qpy = difference<argument<4>, argument<8>>;
    using rpx = difference<argument<1>, argument<7>>;
    using rpy = difference<argument<2>, argument<8>>;
    using tpx = difference<argument<5>, argument<7>>;
    using tpy = difference<argument<6>, argument<8>>;
public:
    using type = det
        <
            difference
                <
                    product<qpx, tpy>,
                    product<qpy, tpx>
                >,
            sum
                <
                    product
                        <
                            tpx,
                            difference < argument<5>, argument<3> >
                        >,
                    product
                        <
                            tpy,
                            difference < argument<6>, argument<4> >
                        >
                >,
            difference
                <
                    product<qpx, rpy>,
                    product<qpy, rpx>
                >,
            sum
                <
                    product
                        <
                            rpx,
                            difference < argument<1>, argument<3> >
                        >,
                    product
                        <
                            rpy,
                            difference < argument<2>, argument<4> >
                        >
                >
        >;
};

using incircle_simplified = incircle_simplified_impl::type;

struct orient2d_no_translation_impl
{
private:
    using adet = det
        <
            argument<3>, argument<4>,
            argument<5>, argument<6>
        >;
    using bdet = det
        <
            argument<1>, argument<2>,
            argument<5>, argument<6>
        >;
    using cdet = det
        <
            argument<1>, argument<2>,
            argument<3>, argument<4>
        >;
public:
    using type = sum< difference<adet, bdet>, cdet >;
};

using orient2d_no_translation = orient2d_no_translation_impl::type;

struct orient3d_no_translation_impl
{
private:
    using adet = det
        <
            argument<4>, argument<5>, argument<6>,
            argument<7>, argument<8>, argument<9>,
            argument<10>,argument<11>,argument<12>
        >;
    using bdet = det
        <
            argument<1>, argument<2>, argument<3>,
            argument<7>, argument<8>, argument<9>,
            argument<10>,argument<11>,argument<12>
        >;
    using cdet = det
        <
            argument<1>, argument<2>, argument<3>,
            argument<4>, argument<5>, argument<6>,
            argument<10>,argument<11>,argument<12>
        >;
    using ddet = det
        <
            argument<1>, argument<2>, argument<3>,
            argument<4>, argument<5>, argument<6>,
            argument<7>, argument<8>, argument<9>
        >;
public:
    using type = sum< difference<bdet, adet>, difference<ddet, cdet> >;
};

using orient3d_no_translation = orient3d_no_translation_impl::type;

struct incircle_no_translation_impl
{
private:
    using alift = sum< product<argument<1>, argument<1>>,
                       product<argument<2>, argument<2>> >;
    using blift = sum< product<argument<3>, argument<3>>,
                       product<argument<4>, argument<4>> >;
    using clift = sum< product<argument<5>, argument<5>>,
                       product<argument<6>, argument<6>> >;
    using dlift = sum< product<argument<7>, argument<7>>,
                       product<argument<8>, argument<8>> >;
    using adet = det < argument<3>, argument<4>, blift,
                       argument<5>, argument<6>, clift,
                       argument<7>, argument<8>, dlift >;
    using bdet = det < argument<1>, argument<2>, alift,
                       argument<5>, argument<6>, clift,
                       argument<7>, argument<8>, dlift >;
    using cdet = det < argument<1>, argument<2>, alift,
                       argument<3>, argument<4>, blift,
                       argument<7>, argument<8>, dlift >;
    using ddet = det < argument<1>, argument<2>, alift,
                       argument<3>, argument<4>, blift,
                       argument<5>, argument<6>, clift >;
public:
    using type = sum < difference<bdet, adet>, difference<ddet, cdet> >;
};

using incircle_no_translation = incircle_no_translation_impl::type;

struct insphere_no_translation_impl
{
private:
    using alift = sum< sum<product<argument<1>, argument<1>>,
                           product<argument<2>, argument<2>>>,
                       product<argument<3>, argument<3>> >;
    using blift = sum< sum<product<argument<4>, argument<4>>,
                           product<argument<5>, argument<5>>>,
                       product<argument<6>, argument<6>> >;
    using clift = sum< sum<product<argument<7>, argument<7>>,
                           product<argument<8>, argument<8>>>,
                       product<argument<9>, argument<9>> >;
    using dlift = sum< sum<product<argument<10>, argument<10>>,
                           product<argument<11>, argument<11>>>,
                       product<argument<12>, argument<12>> >;
    using elift = sum< sum<product<argument<13>, argument<13>>,
                           product<argument<14>, argument<14>>>,
                       product<argument<15>, argument<15>> >;
    using adet = det < argument<4>,  argument<5>,  argument<6>,  blift,
                       argument<7>,  argument<8>,  argument<9>,  clift,
                       argument<10>, argument<11>, argument<12>, dlift,
                       argument<13>, argument<14>, argument<15>, elift>;
    using bdet = det < argument<1>,  argument<2>,  argument<3>,  alift,
                       argument<7>,  argument<8>,  argument<9>,  clift,
                       argument<10>, argument<11>, argument<12>, dlift,
                       argument<13>, argument<14>, argument<15>, elift>;
    using cdet = det < argument<1>,  argument<2>,  argument<3>,  alift,
                       argument<4>,  argument<5>,  argument<6>,  blift,
                       argument<10>, argument<11>, argument<12>, dlift,
                       argument<13>, argument<14>, argument<15>, elift>;
    using ddet = det < argument<1>,  argument<2>,  argument<3>,  alift,
                       argument<4>,  argument<5>,  argument<6>,  blift,
                       argument<7>,  argument<8>,  argument<9>,  clift,
                       argument<13>, argument<14>, argument<15>, elift>;
    using edet = det < argument<1>,  argument<2>,  argument<3>,  alift,
                       argument<4>,  argument<5>,  argument<6>,  blift,
                       argument<7>,  argument<8>,  argument<9>,  clift,
                       argument<10>, argument<11>, argument<12>, dlift>;
public:
    using type = sum
        <
            difference<bdet, adet>,
            sum
                <
                    difference<ddet, cdet>,
                    edet
                >
        >;
};

using insphere_no_translation = insphere_no_translation_impl::type;

}} // namespace detail::generic_robust_predicates

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_EXPRESSIONS_HPP
