// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2020 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_STAGE_D_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_STAGE_D_HPP

#include <cstddef>
#include <array>
#include <algorithm>

#include <boost/mp11/integral.hpp>
#include <boost/mp11/list.hpp>
#include <boost/mp11/function.hpp>
#include <boost/mp11/algorithm.hpp>

#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/expression_tree.hpp>
#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/expansion_eval.hpp>

namespace boost { namespace geometry
{

namespace detail { namespace generic_robust_predicates
{

template
<
    typename Expression,
    typename CalculationType,
    template <int> class ZEPolicy = default_zero_elimination_policy,
    template <int, int> class FEPolicy = default_fast_expansion_sum_policy
>
struct stage_d
{
private:
    using ct = CalculationType;
public:
    static constexpr bool stateful = false;
    static constexpr bool updates = false;

    template <typename ...CTs>
    static inline int apply(CTs const&... args)
    {
        using stack = typename boost::mp11::mp_unique<post_order<Expression>>;
        using evals = typename boost::mp11::mp_remove_if<stack, is_leaf>;
        using sizes_pre = boost::mp11::mp_transform
            <
                expansion_size,
                boost::mp11::mp_pop_back<evals>
            >;
        using sizes = boost::mp11::mp_push_back
            <
                sizes_pre,
                boost::mp11::mp_size_t
                    <
                        final_expansion_size
                            <
                                Expression,
                                expansion_size<typename Expression::left>::value,
                                expansion_size<typename Expression::right>::value
                            >()
                    >
            >;
        using accumulated_sizes = boost::mp11::mp_push_front
            <
                boost::mp11::mp_partial_sum
                    <
                        sizes,
                        boost::mp11::mp_size_t<0>,
                        boost::mp11::mp_plus
                    >,
                boost::mp11::mp_size_t<0>
            >;

        using result_array =
            std::array<ct, boost::mp11::mp_back<accumulated_sizes>::value>;
        result_array results;

        auto most_significant = eval_expansions
            <
                evals,
                sizes,
                accumulated_sizes,
                decltype(results.begin()),
                ct,
                false,
                ZEPolicy,
                FEPolicy
            >(results.begin(), results.end(), args...) - 1;
        if ( *most_significant == 0)
        {
            return 0;
        }
        else if ( *most_significant > 0 )
        {
            return 1;
        }
        else
        {
            return -1;
        }
    }
};

}} // namespace detail::generic_robust_predicates

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_STAGE_D_HPP
