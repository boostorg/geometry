// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2020 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_SEMI_STATIC_FILTER_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_SEMI_STATIC_FILTER_HPP

#include <array>

#include <boost/mp11/list.hpp>
#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/set.hpp>

#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/expression_tree.hpp>
#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/expression_eval.hpp>

namespace boost { namespace geometry
{

namespace detail { namespace generic_robust_predicates
{

//The semi static filter evaluates an Expression and an ErrorExpression. On
//application it evaluates both expressions and verifies the sign of the
//Expression by the following rules:
//If the absolute value of the number resulting from the evaluation of the
//expression is larger than or equal to the result for the error expression,
//then the sign can be returned. If not, then a constant is returned to
//represent an uncertain sign.
//
//The filters that are build from this template are meant to be semi-static in
//the sense that the error expression including constants that depend only the
//epsilon of the calculation type are known statically at compile-time but the
//final value of the error bound depends on the specific inputs for each call
//to the filter and parts of it need to be computed at each call.
//
//This filter is stateless.

template
<
    typename Expression,
    typename CalculationType,
    typename ErrorExpression
>
struct semi_static_filter
{
private:
    using stack = typename boost::mp11::mp_unique<post_order<Expression>>;
    using evals = typename boost::mp11::mp_remove_if<stack, is_leaf>;
    using error_eval_stack = boost::mp11::mp_unique
        <
            post_order<ErrorExpression>
        >;
    using error_eval_stack_remainder = boost::mp11::mp_set_difference
        <
            error_eval_stack,
            evals
        >;
    using all_evals = boost::mp11::mp_append
        <
            evals,
            error_eval_stack_remainder
        >;
    using ct = CalculationType;
public:
    static constexpr bool stateful = false;
    static constexpr bool updates = false;

    template <typename ...CTs>
    static inline ct error_bound(CTs const&... args)
    {
        std::array<ct, sizeof...(CTs)> input
            {{ static_cast<ct>(args)... }};
        return evaluate_expression<ErrorExpression>(input);
    }

    template <typename ...CTs>
    static inline int apply(CTs const&... args)
    {
        std::array<ct, sizeof...(CTs)> input
            {{ static_cast<ct>(args)... }};
        std::array<ct, boost::mp11::mp_size<all_evals>::value> results;
        evaluate_expressions(input, results, all_evals{});
        constexpr std::size_t i_eb =
            boost::mp11::mp_find<all_evals, ErrorExpression>::value;
        const ct error_bound = results[i_eb];
        constexpr std::size_t i_e =
            boost::mp11::mp_find<all_evals, Expression>::value;
        const ct det = results[i_e];
        if (det > error_bound)
        {
            return 1;
        }
        else if (det < -error_bound)
        {
            return -1;
        }
        else if (error_bound == 0 && det == 0)
        {
            return 0;
        }
        else
        {
            return sign_uncertain;
        }
    }
};

}} // namespace detail::generic_robust_predicates

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_SEMI_STATIC_FILTER_HPP
