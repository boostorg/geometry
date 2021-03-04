// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2020 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_STATIC_FILTER_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_STATIC_FILTER_HPP

#include <limits>
#include <array>

#include <boost/mp11/list.hpp>
#include <boost/mp11/algorithm.hpp>

#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/expression_tree.hpp>
#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/expression_eval.hpp>

namespace boost { namespace geometry
{

namespace detail { namespace generic_robust_predicates
{

//The static filter works similar to the semi static filter with the exception
//that the error bound is only computed once at construction time. For this
//purpose, the filter is stateful and it requires upper and lower bounds on the
//inputs at compile time like this:
//
//static_filter<...> f(max1, max2, ..., min1, min2, ...);
//
//for each call to the filter, i.e. f.apply(arg1, arg2, ...); it must hold that
//min1 <= arg1 <= max1 and so on.

template
<
    typename Expression,
    typename CalculationType,
    typename ErrorExpression
>
class static_filter
{
private:
    using ct = CalculationType;
    ct m_error_bound;
    static constexpr std::size_t extrema_count = max_argn<ErrorExpression>;
public:
    static constexpr bool stateful = true;
    static constexpr bool updates = false;

    inline static_filter() : m_error_bound(std::numeric_limits<ct>::max()) {}

    inline ct error_bound() const { return m_error_bound; }

    template <typename ...CTs>
    inline static_filter(CTs const&... args)
        : m_error_bound(evaluate_expression<ErrorExpression>(
                std::array<ct, extrema_count>
                    {static_cast<ct>(args)...}))
    {
        static_assert(sizeof...(CTs) == extrema_count,
                      "Number of constructor arguments is incompatible with error expression.");
    }

    inline static_filter(std::array<ct, extrema_count> const& extrema)
        : m_error_bound(evaluate_expression<ErrorExpression>(extrema)) {}

    inline static_filter(ct const& b) : m_error_bound(b) {}

    template <typename ...CTs>
    inline int apply(CTs const&... args) const
    {
        std::array<ct, sizeof...(CTs)> input {static_cast<ct>(args)...};
        const ct det = evaluate_expression<Expression>(input);
        if (det > m_error_bound)
        {
            return 1;
        }
        else if (det < -m_error_bound)
        {
            return -1;
        }
        else if (m_error_bound == 0 && det == 0)
        {
            return 0;
        }
        else
        {
            return sign_uncertain;
        }
    }

    template <typename ...CTs>
    inline int operator()(CTs const&... args) const
    {
        return apply(args...);
    }
};

}} // namespace detail::generic_robust_predicates

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_STATIC_FILTER_HPP
