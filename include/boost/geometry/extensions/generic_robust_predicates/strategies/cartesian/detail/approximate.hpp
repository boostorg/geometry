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
#include <tuple>

#include <boost/mp11/integral.hpp>
#include <boost/mp11/list.hpp>
#include <boost/mp11/algorithm.hpp>

#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/expression_tree.hpp>
#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/result_propagation.hpp>

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
    typename InputList,
    typename Real,
    bool IsStaticConstant,
    typename ...InputArr
>
struct get_approx_impl
{
    static inline Real apply(const InputArr&... inputs)
    {
        using indices = index_pair<Node, InputList>;
        return std::get<indices::second_type::value>(
                std::get<indices::first_type::value>(
                    std::forward_as_tuple(inputs...)
                )
            );
    }
};

template
<
    typename Node,
    typename InputList,
    typename Real,
    typename ...InputArr
>
struct get_approx_impl
    <
        Node,
        InputList,
        Real,
        true,
        InputArr...
    >
{
    static inline Real apply(const InputArr&...)
    {
        return Node::value;
    }
};

template
<
    typename Node,
    typename InputList,
    typename Real,
    typename ...InputArr
>
inline Real get_approx(const InputArr&... inputs)
{
    return get_approx_impl
        <
            Node,
            InputList,
            Real,
            is_static_constant<Node>::value,
            InputArr...
        >::apply(inputs...);
}

template
<
    typename Remaining,
    typename InputList,
    typename Real,
    operator_types Op,
    typename ...Arr
>
struct approximate_interim_impl {};

template
<
    typename Remaining,
    typename InputList,
    typename Real,
    bool Empty,
    typename ...Arr
>
struct approximate_remainder_impl
{
    static inline void apply(Arr&... inputs)
    {
        using node = boost::mp11::mp_front<Remaining>;
        approximate_interim_impl
            <
                Remaining,
                InputList,
                Real,
                node::operator_type,
                Arr...
            >::apply(inputs...);
    }
};

template
<
    typename Remaining,
    typename InputList,
    typename Real,
    typename ...Arr
>
struct approximate_remainder_impl
    <
        Remaining,
        InputList,
        Real,
        true,
        Arr...
    >
{
    static inline void apply(Arr&...) {}
};

template
<
    typename Remaining,
    typename InputList,
    typename Real,
    typename ...Arr
>
inline void approximate_remainder(Arr&... inputs)
{
    approximate_remainder_impl
        <
            Remaining,
            InputList,
            Real,
            boost::mp11::mp_empty<Remaining>::value,
            Arr...
        >::apply(inputs...);
}

template
<
    typename Remaining,
    typename InputList,
    typename Real,
    typename ...Arr
>
struct approximate_interim_impl
    <
        Remaining,
        InputList,
        Real,
        operator_types::product,
        Arr...
    >
{
    static inline void apply(Arr&... inputs)
    {
        using node = boost::mp11::mp_front<Remaining>;
        using indices = index_pair<node, InputList>;
        Real& out = std::get<indices::second_type::value>(
                std::get<indices::first_type::value>(
                    std::forward_as_tuple(inputs...)
                )
            );
        out =   get_approx<typename node::left, InputList, Real>(inputs...)
              * get_approx<typename node::right, InputList, Real>(inputs...);
        approximate_remainder
            <
                boost::mp11::mp_pop_front<Remaining>,
                InputList,
                Real
            >(inputs...);
    }
};

template
<
    typename Remaining,
    typename InputList,
    typename Real,
    typename ...Arr
>
struct approximate_interim_impl
    <
        Remaining,
        InputList,
        Real,
        operator_types::max,
        Arr...
    >
{
    static inline void apply(Arr&... inputs)
    {
        using node = boost::mp11::mp_front<Remaining>;
        using indices = index_pair<node, InputList>;
        Real& out = std::get<indices::second_type::value>(
                std::get<indices::first_type::value>(
                    std::forward_as_tuple(inputs...)
                )
            );
        out = std::max(
                get_approx<typename node::left, InputList, Real>(inputs...),
                get_approx<typename node::right, InputList, Real>(inputs...)
            );
        approximate_remainder
            <
                boost::mp11::mp_pop_front<Remaining>,
                InputList,
                Real
            >(inputs...);
    }
};

template
<
    typename Remaining,
    typename InputList,
    typename Real,
    typename ...Arr
>
struct approximate_interim_impl
    <
        Remaining,
        InputList,
        Real,
        operator_types::min,
        Arr...
    >
{
    static inline void apply(Arr&... inputs)
    {
        using node = boost::mp11::mp_front<Remaining>;
        using indices = index_pair<node, InputList>;
        Real& out = std::get<indices::second_type::value>(
                std::get<indices::first_type::value>(
                    std::forward_as_tuple(inputs...)
                )
            );
        out = std::min(
                get_approx<typename node::left, InputList, Real>(inputs...),
                get_approx<typename node::right, InputList, Real>(inputs...)
            );
        approximate_remainder
            <
                boost::mp11::mp_pop_front<Remaining>,
                InputList,
                Real
            >(inputs...);
    }
};

template
<
    typename Remaining,
    typename InputList,
    typename Real,
    typename ...Arr
>
struct approximate_interim_impl
    <
        Remaining,
        InputList,
        Real,
        operator_types::sum,
        Arr...
    >
{
    static inline void apply(Arr&... inputs)
    {
        using node = boost::mp11::mp_front<Remaining>;
        using indices = index_pair<node, InputList>;
        Real& out = std::get<indices::second_type::value>(
                std::get<indices::first_type::value>(
                    std::forward_as_tuple(inputs...)
                )
            );
        out =   get_approx<typename node::left, InputList, Real>(inputs...)
              + get_approx<typename node::right, InputList, Real>(inputs...);
        approximate_remainder
            <
                boost::mp11::mp_pop_front<Remaining>,
                InputList,
                Real
            >(inputs...);
    }
};

template
<
    typename Remaining,
    typename InputList,
    typename Real,
    typename ...Arr
>
struct approximate_interim_impl
    <
        Remaining,
        InputList,
        Real,
        operator_types::difference,
        Arr...
    >
{
    static inline void apply(Arr&... inputs)
    {
        using node = boost::mp11::mp_front<Remaining>;
        using indices = index_pair<node, InputList>;
        Real& out = std::get<indices::second_type::value>(
                std::get<indices::first_type::value>(
                    std::forward_as_tuple(inputs...)
                )
            );
        out =   get_approx<typename node::left, InputList, Real>(inputs...)
              - get_approx<typename node::right, InputList, Real>(inputs...);
        approximate_remainder
            <
                boost::mp11::mp_pop_front<Remaining>,
                InputList,
                Real
            >(inputs...);
    }
};

template
<
    typename Remaining,
    typename InputList,
    typename Real,
    typename ...Arr
>
struct approximate_interim_impl
    <
        Remaining,
        InputList,
        Real,
        operator_types::abs,
        Arr...
    >
{
    static inline void apply(Arr&... inputs)
    {
        using node = boost::mp11::mp_front<Remaining>;
        using indices = index_pair<node, InputList>;
        Real& out = std::get<indices::second_type::value>(
                std::get<indices::first_type::value>(
                    std::forward_as_tuple(inputs...)
                )
            );
        out = std::abs(
                get_approx<typename node::child, InputList, Real>(inputs...));
        approximate_remainder
            <
                boost::mp11::mp_pop_front<Remaining>,
                InputList,
                Real
            >(inputs...);
    }
};

template
<
    typename Remaining,
    typename InputList,
    typename Real,
    typename ...Arr
>
struct approximate_interim_impl
    <
        Remaining,
        InputList,
        Real,
        operator_types::no_op,
        Arr...
    >
{
    static inline void apply(Arr&... inputs)
    {
        approximate_remainder
            <
                boost::mp11::mp_pop_front<Remaining>,
                InputList,
                Real
            >(inputs...);
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
    typename Remaining,
    typename InputList,
    typename Real,
    typename ...Arr
>
inline void approximate_interim(Arr&... inputs)
{
    approximate_remainder
        <
            Remaining,
            InputList,
            Real
        >(inputs...);
}

template<typename Expression, typename Real, typename InputArr>
inline Real approximate_value(const InputArr& input)
{
    using stack = typename boost::mp11::mp_unique<post_order<Expression>>;
    using evals = typename boost::mp11::mp_remove_if<stack, is_leaf>;
    using arg_list = boost::mp11::mp_list
        <
            evals,
            argument_list<std::tuple_size<InputArr>::value>
        >;
    std::array<Real, boost::mp11::mp_size<evals>::value> results;
    approximate_interim<evals, arg_list, Real>(results, input);
    return results.back();
}

}} // namespace detail::generic_robust_predicates

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_APPROXIMATE_HPP
