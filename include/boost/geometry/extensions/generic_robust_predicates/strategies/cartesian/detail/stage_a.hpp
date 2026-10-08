// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2020 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_STAGE_A_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_STAGE_A_HPP

#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/expression_tree.hpp>
#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/stage_a_error_bound.hpp>
#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/interval_error_bound.hpp>

#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/semi_static_filter.hpp>
#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/almost_static_filter.hpp>
#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/static_filter.hpp>

namespace boost { namespace geometry
{

namespace detail { namespace generic_robust_predicates
{

//The following templates apply the ideas implemented in error_bound.hpp and
//interval_error_bound.hpp to generate error expressions from expressions.

template
<
    typename Expression,
    typename CalculationType
>
using stage_a_error_bound_expression =
    typename stage_a_error_bound_expression_impl
        <
            Expression,
            CalculationType
        >::type;

template
<
    typename Expression,
    typename CalculationType
>
using stage_a_semi_static = semi_static_filter
    <
        Expression,
        CalculationType,
        stage_a_error_bound_expression
            <
                Expression,
                CalculationType
            >
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
        interval
            <
                stage_a_error_bound_expression
                    <
                        Expression,
                        CalculationType
                    >
            >
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
