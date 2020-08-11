// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2020 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_APPROXIMATE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_APPROXIMATE_HPP

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
//The most important template in this file is approximate_interim.

template
<
    typename Node,
    std::size_t N,
    typename Real,
    typename InputArr
>
struct get_nth_real_impl
{
    static inline Real apply(InputArr const& input)
    {
        return input[N - 1];
    }
};

template
<
    typename Node,
    typename Real,
    typename InputArr
>
struct get_nth_real_impl<Node, 0, Real, InputArr>
{
    static inline Real apply(InputArr const&)
    {
        return Node::value;
    }
};

template
<
    typename Node,
    std::size_t N,
    typename Real,
    typename InputArr
>
inline Real get_nth_real(InputArr const& input)
{
    return get_nth_real_impl<Node, Node::argn, Real, InputArr>::apply(input);
}

template
<
    typename All,
    typename Node,
    typename Real,
    typename Arr,
    bool is_leaf,
    typename InputArr
>
struct get_approx_impl
{
    static inline Real apply(Arr& interim_results, InputArr const&)
    {
        return interim_results[boost::mp11::mp_find<All, Node>::value];
    }
};

template
<
    typename All,
    typename Node,
    typename Real,
    typename Arr,
    typename InputArr
>
struct get_approx_impl<All, Node, Real, Arr, true, InputArr>
{
    static inline Real apply(Arr&, InputArr const& input)
    {
        return get_nth_real<Node, Node::argn, Real, InputArr>(input);
    }
};

template
<
    typename All,
    typename Node,
    typename Real,
    typename Arr,
    typename InputArr
>
inline Real get_approx(Arr& interim_results, InputArr const& input)
{
    return get_approx_impl
        <
            All,
            Node,
            Real,
            Arr,
            is_leaf<Node>::value,
            InputArr
        >::apply(interim_results, input);
}

template
<
    typename All,
    typename Remaining,
    typename Real,
    typename Arr,
    operator_types Op,
    typename InputArr
>
struct approximate_interim_impl {};

template
<
    typename All,
    typename Remaining,
    typename Real,
    typename Arr,
    bool Empty,
    typename InputArr
>
struct approximate_remainder_impl
{
    static inline void apply(Arr& interim_results, InputArr const& input)
    {
        using node = boost::mp11::mp_front<Remaining>;
        approximate_interim_impl
            <
                All,
                Remaining,
                Real,
                Arr,
                node::operator_type,
                InputArr
            >::apply(interim_results, input);
    }
};

template
<
    typename All,
    typename Remaining,
    typename Real,
    typename Arr,
    typename InputArr
>
struct approximate_remainder_impl
    <
        All,
        Remaining,
        Real,
        Arr,
        true,
        InputArr
    >
{
    static inline void apply(Arr&, InputArr const&) {}
};

template
<
    typename All,
    typename Remaining,
    typename Real,
    typename Arr,
    typename InputArr
>
inline void approximate_remainder(Arr& interim_results, InputArr const& input)
{
    approximate_remainder_impl
        <
            All,
            Remaining,
            Real,
            Arr,
            boost::mp11::mp_empty<Remaining>::value,
            InputArr
        >::apply(interim_results, input);
}

template
<
    typename All,
    typename Remaining,
    typename Real,
    typename Arr,
    typename InputArr
>
struct approximate_interim_impl
    <
        All,
        Remaining,
        Real,
        Arr,
        operator_types::product,
        InputArr
    >
{
    static inline void apply(Arr& interim_results, InputArr const& input)
    {
        using node = boost::mp11::mp_front<Remaining>;
        interim_results[boost::mp11::mp_find<All, node>::value] =
                  get_approx<All, typename node::left, Real>(interim_results,
                                                             input)
                * get_approx<All, typename node::right, Real>(interim_results,
                                                              input);
        approximate_remainder
            <
                All,
                boost::mp11::mp_pop_front<Remaining>,
                Real
            >(interim_results, input);
    }
};

template
<
    typename All,
    typename Remaining,
    typename Real,
    typename Arr,
    typename InputArr
>
struct approximate_interim_impl
    <
        All,
        Remaining,
        Real,
        Arr,
        operator_types::max,
        InputArr
    >
{
    static inline void apply(Arr& interim_results, InputArr const& input)
    {
        using node = boost::mp11::mp_front<Remaining>;
        interim_results[boost::mp11::mp_find<All, node>::value] = std::max(
                  get_approx<All, typename node::left, Real>(interim_results,
                                                             input),
                  get_approx<All, typename node::right, Real>(interim_results,
                                                              input));
        approximate_remainder
            <
                All,
                boost::mp11::mp_pop_front<Remaining>,
                Real
            >(interim_results, input);
    }
};

template
<
    typename All,
    typename Remaining,
    typename Real,
    typename Arr,
    typename InputArr
>
struct approximate_interim_impl
    <
        All,
        Remaining,
        Real,
        Arr,
        operator_types::min,
        InputArr
    >
{
    static inline void apply(Arr& interim_results, InputArr const& input)
    {
        using node = boost::mp11::mp_front<Remaining>;
        interim_results[boost::mp11::mp_find<All, node>::value] = std::min(
                  get_approx<All, typename node::left, Real>(interim_results,
                                                             input),
                  get_approx<All, typename node::right, Real>(interim_results,
                                                              input));
        approximate_remainder
            <
                All,
                boost::mp11::mp_pop_front<Remaining>,
                Real
            >(interim_results, input);
    }
};

template
<
    typename All,
    typename Remaining,
    typename Real,
    typename Arr,
    typename InputArr
>
struct approximate_interim_impl
    <
        All,
        Remaining,
        Real,
        Arr,
        operator_types::sum,
        InputArr
    >
{
    static inline void apply(Arr& interim_results, InputArr const& input)
    {
        using node = boost::mp11::mp_front<Remaining>;
        interim_results[boost::mp11::mp_find<All, node>::value] =
                  get_approx<All, typename node::left, Real>(interim_results,
                                                             input)
                + get_approx<All, typename node::right, Real>(interim_results,
                                                              input);
        approximate_remainder
            <
                All,
                boost::mp11::mp_pop_front<Remaining>,
                Real
            >(interim_results, input);
    }
};

template
<
    typename All,
    typename Remaining,
    typename Real,
    typename Arr,
    typename InputArr
>
struct approximate_interim_impl
    <
        All,
        Remaining,
        Real,
        Arr,
        operator_types::difference,
        InputArr
    >
{
    static inline void apply(Arr& interim_results, InputArr const& input)
    {
        using node = boost::mp11::mp_front<Remaining>;
        interim_results[boost::mp11::mp_find<All, node>::value] =
                  get_approx<All, typename node::left, Real>(interim_results,
                                                             input)
                - get_approx<All, typename node::right, Real>(interim_results,
                                                              input);
        approximate_remainder
            <
                All,
                boost::mp11::mp_pop_front<Remaining>,
                Real
            >(interim_results, input);
    }
};

template
<
    typename All,
    typename Remaining,
    typename Real,
    typename Arr,
    typename InputArr
>
struct approximate_interim_impl
    <
        All,
        Remaining,
        Real,
        Arr,
        operator_types::abs,
        InputArr
    >
{
    static inline void apply(Arr& interim_results, InputArr const& input)
    {
        using node = boost::mp11::mp_front<Remaining>;
        interim_results[boost::mp11::mp_find<All, node>::value] =
            std::abs(get_approx
                <
                    All,
                    typename node::child,
                    Real
                >(interim_results, input));
        approximate_remainder
            <
                All,
                boost::mp11::mp_pop_front<Remaining>,
                Real
            >(interim_results, input);
    }
};

template
<
    typename All,
    typename Remaining,
    typename Real,
    typename Arr,
    typename InputArr
>
struct approximate_interim_impl
    <
        All,
        Remaining,
        Real,
        Arr,
        operator_types::no_op,
        InputArr
    >
{
    static inline void apply(Arr& interim_results, InputArr const& input)
    {
        approximate_remainder
            <
                All,
                boost::mp11::mp_pop_front<Remaining>,
                Real
            >(interim_results, input);
    }
};

//All expects an boost::mp11::mp_list of all expressions that need to be
//evaluated. Remaining expects an boost::mp11::mp_list of the expressions
//that are left to be evaluated. In the first call it is expected to be
//equal to All and it serves as an anchor for template recursion. Real is a
//floating-point type. The remaining template arguments are deduced from
//parameters.

template
<
    typename All,
    typename Remaining,
    typename Real,
    typename Arr,
    typename InputArr
>
inline void approximate_interim(Arr& interim_results, InputArr const& input)
{
    approximate_remainder
        <
            All,
            Remaining,
            Real
        >(interim_results, input);
}

template<typename Expression, typename Real, typename InputArr>
inline Real approximate_value(InputArr const& input)
{
    using stack = typename boost::mp11::mp_unique<post_order<Expression>>;
    using evals = typename boost::mp11::mp_remove_if<stack, is_leaf>;
    std::array<Real, boost::mp11::mp_size<evals>::value> results;
        approximate_interim<evals, evals, Real>(results, input);
    return results.back();
}

}} // namespace detail::generic_robust_predicates

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_APPROXIMATE_HPP
