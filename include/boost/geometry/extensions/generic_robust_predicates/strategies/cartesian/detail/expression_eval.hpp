// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_EXPRESSION_EVAL_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_EXPRESSION_EVAL_HPP

#include <cstddef>
#include <cmath>
#include <array>

#include <boost/mp11/integral.hpp>
#include <boost/mp11/list.hpp>
#include <boost/mp11/algorithm.hpp>

#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/expression_tree.hpp>

namespace boost { namespace geometry
{

namespace detail { namespace generic_robust_predicates
{

//The templates in this file are meant to be used for the evaluation of
//expressions with floating-point precision and floating-point rounding.

template <operator_types Op>
struct evaluate_expression_unary_impl {};

template <>
struct evaluate_expression_unary_impl<operator_types::abs>
{
    template <typename CT>
    static constexpr void apply(CT const& c, CT& out)
    {
        out = std::abs(c);
    }
};

template <operator_types Op>
struct evaluate_expression_binary_impl {};

template <>
struct evaluate_expression_binary_impl<operator_types::sum>
{
    template <typename CT>
    static constexpr void apply(CT const& l, CT const& r, CT& out)
    {
        out = l + r;
    }
};

template <>
struct evaluate_expression_binary_impl<operator_types::product>
{
    template <typename CT>
    static constexpr void apply(CT const& l, CT const& r, CT& out)
    {
        out = l * r;
    }
};

template <>
struct evaluate_expression_binary_impl<operator_types::difference>
{
    template <typename CT>
    static constexpr void apply(CT const& l, CT const& r, CT& out)
    {
        out = l - r;
    }
};

template <>
struct evaluate_expression_binary_impl<operator_types::min>
{
    template <typename CT>
    static constexpr void apply(CT const& l, CT const& r, CT& out)
    {
        out = std::min(l, r);
    }
};

template <>
struct evaluate_expression_binary_impl<operator_types::max>
{
    template <typename CT>
    static constexpr void apply(CT const& l, CT const& r, CT& out)
    {
        out = std::max(l, r);
    }
};

template
<
    std::size_t I,
    typename Expression,
    bool IsLeaf = Expression::is_leaf
>
struct get_arg_or_out_impl {};

template <std::size_t I, typename Expression>
struct get_arg_or_out_impl<I, Expression, false>
{
    template <typename In, typename Out>
    static constexpr auto apply(In const&, Out const& out)
    {
        return out[I];
    }
};

template <typename Expression, std::size_t Argn = Expression::argn>
struct get_arg_or_const
{
    template <typename In>
    static constexpr auto apply(In const& in)
    {
        return in[Argn - 1];
    }
};

template <typename Expression>
struct get_arg_or_const<Expression, 0>
{
    template <typename In>
    static constexpr auto apply(In const&)
    {
        return Expression::value;
    }
};

template <std::size_t I, typename Expression>
struct get_arg_or_out_impl<I, Expression, true>
{
    template <typename In, typename Out>
    static constexpr auto apply(In const& in, Out const&)
    {
        return get_arg_or_const<Expression>::apply(in);
    }
};

template
<
    typename InputArr,
    typename OutputArr,
    typename Expressions,
    typename Expression,
    operator_arities Op = Expression::operator_arity
>
struct evaluate_expression_impl {};

template
<
    typename InputArr,
    typename OutputArr,
    typename Expressions,
    typename Expression
>
struct evaluate_expression_impl
    <
        InputArr,
        OutputArr,
        Expressions,
        Expression,
        operator_arities::binary
    >
{
    using left = typename Expression::left;
    using right = typename Expression::right;
    static constexpr std::size_t i_out =
        boost::mp11::mp_find<Expressions, Expression>::value;
    static constexpr std::size_t i_left =
        boost::mp11::mp_find<Expressions, left>::value;
    static constexpr std::size_t i_right =
        boost::mp11::mp_find<Expressions, right>::value;
    static constexpr void apply(
            InputArr const& input,
            OutputArr& output
        )
    {
        auto& out = output[i_out];
        auto const& l =
            get_arg_or_out_impl<i_left, left>::apply(input, output);
        auto const& r =
            get_arg_or_out_impl<i_right, right>::apply(input, output);
        evaluate_expression_binary_impl<Expression::operator_type>
            ::apply(l, r, out);
    }
};

template
<
    typename InputArr,
    typename OutputArr,
    typename Expressions,
    typename Expression
>
struct evaluate_expression_impl
    <
        InputArr,
        OutputArr,
        Expressions,
        Expression,
        operator_arities::unary
    >
{
    using child = typename Expression::child;
    static constexpr std::size_t i_out =
        boost::mp11::mp_find<Expressions, Expression>::value;
    static constexpr std::size_t i_child =
        boost::mp11::mp_find<Expressions, child>::value;
    static constexpr void apply(
            InputArr const& input,
            OutputArr& output
        )
    {
        auto& out = output[i_out];
        auto const& c = get_arg_or_out_impl
            <
                i_child,
                child
            >::apply(input, output);
        evaluate_expression_unary_impl<Expression::operator_type>
            ::apply(c, out);
    }
};

//Expects a list of variadic list of expressions that are to be evaluated.
//The results are stored in output.

template
<
    typename InputArr,
    typename OutputArr,
    typename ...Expressions
>
constexpr void evaluate_expressions(InputArr const& input,
                                    OutputArr& output,
                                    boost::mp11::mp_list<Expressions...>)
{
    using exp_list = boost::mp11::mp_list<Expressions...>;
    //Below we use a bit of a hack to do a comma-fold in C++14.
    using dummy = int[];
    (void)dummy{
        0,
        (evaluate_expression_impl
            <
                InputArr,
                OutputArr,
                exp_list,
                Expressions
            >::apply(input, output), 0)...
    };
}

template
<
    typename Expression,
    typename InputArr
>
constexpr auto evaluate_expression(InputArr const& input)
{
    using stack = typename boost::mp11::mp_unique<post_order<Expression>>;
    using evals = typename boost::mp11::mp_remove_if<stack, is_leaf>;
    std::array
        <
            typename InputArr::value_type,
            boost::mp11::mp_size<evals>::value
        > results;
    evaluate_expressions(input, results, evals{});
    return results.back();
}

}} // namespace detail::generic_robust_predicates

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_EXPRESSION_EVAL_HPP
