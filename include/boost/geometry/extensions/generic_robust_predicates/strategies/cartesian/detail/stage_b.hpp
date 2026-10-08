// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2020 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_STAGE_B_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_STAGE_B_HPP

#include <array>
#include <iterator>

#include <boost/mp11/list.hpp>
#include <boost/mp11/algorithm.hpp>

#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/stage_d.hpp>

namespace boost { namespace geometry
{

namespace detail { namespace generic_robust_predicates
{

template
<
    typename Expression,
    bool IsDifference = Expression::operator_type == operator_types::difference
>
struct is_leaf_difference_impl
{
    using type = boost::mp11::mp_false;
};

template
<
    typename Expression
>
struct is_leaf_difference_impl <Expression, true>
{
    using type = boost::mp11::mp_bool
        <
            Expression::left::is_leaf && Expression::right::is_leaf
        >;
};

template <typename Expression>
using is_leaf_difference = typename is_leaf_difference_impl<Expression>::type;

template
<
    typename Evals,
    typename LeafDifferences,
    typename AccumulatedSizes,
    typename Iter,
    typename CT,
    std::size_t RemainingDifferences =
        boost::mp11::mp_size<LeafDifferences>::value
>
struct all_differences_zero_tail
{
    template <typename ...CTs>
    static bool apply(Iter begin, Iter end, CTs const&... args)
    {
        using eval = boost::mp11::mp_front<LeafDifferences>;
        using left = typename eval::left;
        using right = typename eval::right;
        std::array<CT, sizeof...(CTs)> input
            {{ static_cast<CT>(args)... }};
        CT left_val = input[left::argn - 1];
        CT right_val = input[right::argn - 1];
        using eval_index = boost::mp11::mp_find<Evals, eval>;
        constexpr std::size_t start =
            boost::mp11::mp_at<AccumulatedSizes, eval_index>::value;
        *(begin + start) = left_val - right_val;
        if (two_difference_tail(left_val, right_val, *(begin + start))
                == CT(0))
        {
            return all_differences_zero_tail
                <
                    Evals,
                    boost::mp11::mp_pop_front<LeafDifferences>,
                    AccumulatedSizes,
                    Iter,
                    CT
                >::apply(begin, end, args...);
        }
        return false;
    }
};

template
<
    typename Evals,
    typename LeafDifferences,
    typename AccumulatedSizes,
    typename Iter,
    typename CT
>
struct all_differences_zero_tail
    <
        Evals,
        LeafDifferences,
        AccumulatedSizes,
        Iter,
        CT,
        0
    >
{
    template <typename ...CTs>
    static bool apply(Iter, Iter, CTs const&...)
    {
        return true;
    }
};

template
<
    typename Expression,
    typename CalculationType,
    template <int> class ZEPolicy = default_zero_elimination_policy,
    template <int, int> class FEPolicy = default_fast_expansion_sum_policy
>
struct stage_b
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
                expansion_size_stage_b,
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
                                expansion_size_stage_b
                                    <
                                        typename Expression::left
                                    >::value,
                                expansion_size_stage_b
                                    <
                                        typename Expression::right
                                    >::value
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
            std::array
                <
                    ct,
                    boost::mp11::mp_back<accumulated_sizes>::value
                >;
        result_array results;

        using leaf_differences =
            boost::mp11::mp_copy_if<evals, is_leaf_difference>;

        bool all_zero = all_differences_zero_tail
            <
                evals,
                leaf_differences,
                accumulated_sizes,
                typename result_array::iterator,
                ct
            >::apply(results.begin(), results.end(), args...);

        if ( !all_zero )
        {
            return sign_uncertain;
        }

        using remainder =
            typename boost::mp11::mp_remove_if
                <
                    evals,
                    is_leaf_difference
                >;

        using ze_evals = boost::mp11::mp_copy_if_q
        <
            remainder,
            is_zero_elim_q<ZEPolicy, true>
        >;
        std::array
            <
                typename result_array::iterator,
                boost::mp11::mp_size<ze_evals>::value
            > ze_ends;

        auto most_significant = eval_expansions_impl
            <
                evals,
                remainder,
                sizes,
                accumulated_sizes,
                ze_evals,
                decltype(results.begin()),
                ct,
                ZEPolicy,
                FEPolicy,
                true
            >::apply(results.begin(), results.end(), ze_ends, args...) - 1;

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

#endif // BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_STAGE_B_HPP
