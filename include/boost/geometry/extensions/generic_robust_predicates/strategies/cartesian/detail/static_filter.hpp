// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2020 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_STATIC_FILTER_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_STATIC_FILTER_HPP

#include <array>

#include <boost/mp11/list.hpp>
#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/map.hpp>
#include <boost/mp11/set.hpp>

#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/stage_a.hpp>

namespace boost { namespace geometry
{

namespace detail { namespace generic_robust_predicates
{

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
    using stack = typename boost::mp11::mp_unique<post_order<Expression>>;
    using evals = typename boost::mp11::mp_remove_if<stack, is_leaf>;
    using error_eval_stack = boost::mp11::mp_unique
        <
            post_order<ErrorExpression>
        >;
    ct m_error_bound;
public:
    inline static_filter() {}

    ct error_bound() const { return m_error_bound; }

    template <typename ...Reals>
    inline static_filter(const Reals&... args)
        : m_error_bound(approximate_value<ErrorExpression, ct>(
                std::array<ct, sizeof...(Reals)>
                    {static_cast<ct>(args)...}))
    {
        static_assert(sizeof...(Reals) == max_argn<ErrorExpression>::value,
                      "Number of constructor arguments is incompatible with error expression.");
    }

    template <typename ...Reals>
    inline int apply(const Reals&... args) const
    {
        std::array<ct, sizeof...(Reals)> input {static_cast<ct>(args)...};
        std::array<ct, boost::mp11::mp_size<evals>::value> results;
        approximate_interim<evals, evals, ct>(results, input);
        const ct det = get_approx<evals, Expression, ct>(results, input);
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

    template <typename ...Reals>
    inline int operator()(const Reals&... args) const
    {
        return apply(args...);
    }
};

}} // namespace detail::generic_robust_predicates

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_STATIC_FILTER_HPP
