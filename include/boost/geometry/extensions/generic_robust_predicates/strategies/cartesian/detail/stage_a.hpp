// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2020 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_STAGE_A_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_STAGE_A_HPP

#include <boost/mp11/list.hpp>
#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/set.hpp>

#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/expression_tree.hpp>
#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/coefficient_list.hpp>
#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/error_bound.hpp>
#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/interval_error_bound.hpp>

#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/semi_static_filter.hpp>
#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/almost_static_filter.hpp>
#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/static_filter.hpp>

namespace boost { namespace geometry
{

namespace detail { namespace generic_robust_predicates
{

template <typename Expression, typename CalculationType>
struct stage_a_error_bound_impl
{
private:
    using ct = CalculationType;
    using root = Expression;
    using stack = typename boost::mp11::mp_unique<post_order<Expression>>;
    using evals = typename boost::mp11::mp_remove_if<stack, is_leaf>;
    using interim_evals = typename boost::mp11::mp_remove
        <
            boost::mp11::mp_remove_if<stack, is_leaf>,
            root
        >;
    using interim_errors = evals_error<interim_evals>;
    using final_children = add_children
        <
            boost::mp11::mp_second
                <
                    boost::mp11::mp_map_find
                        <
                            interim_errors,
                            typename root::left
                        >
                >,
            boost::mp11::mp_second
                <
                    boost::mp11::mp_map_find
                        <
                            interim_errors,
                            typename root::right
                        >
                >
        >;
    using final_children_ltp = error_map_list_to_product<final_children>;
    using final_children_ltp_abs = abs_all<final_children_ltp>;
    using final_children_sum_fold = error_map_balanced_sum_up<final_children_ltp_abs>;
    using final_coeff = coeff_round
        <
            div_by_1_m_eps
                <
                    mult_by_1_p_eps
                        <
                            boost::mp11::mp_second<final_children_sum_fold>
                        >
                >
        >;
    static constexpr ct final_coeff_value = eval_eps_polynomial<ct, final_coeff>::value;

    struct final_coeff_constant : public static_constant_interface<ct>
    {
        static constexpr ct value = final_coeff_value;
        static constexpr bool non_negative = true;
    };

    using error_expression_variable = boost::mp11::mp_front<final_children_sum_fold>;
public:
    using type = product<final_coeff_constant, error_expression_variable>;
};

template
<
    typename Expression,
    typename CalculationType
>
using stage_a_error_bound =
    typename stage_a_error_bound_impl<Expression, CalculationType>::type;

template
<
    typename Expression,
    typename CalculationType
>
using stage_a_semi_static = semi_static_filter
        <
            Expression,
            CalculationType,
            stage_a_error_bound<Expression, CalculationType>
        >;

template
<
    typename Expression,
    typename CalculationType
>
using stage_a_static = static_filter
        <
            Expression,
            CalculationType,
            interval<stage_a_error_bound<Expression, CalculationType>>
        >;

template
<
    typename Expression,
    typename CalculationType
>
using stage_a_almost_static = almost_static_filter
        <
            Expression,
            CalculationType,
            stage_a_static<Expression, CalculationType>
        >;

}} // namespace detail::generic_robust_predicates

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_STAGE_A_HPP
