// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2020 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_EXPRESSION_TREE_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_EXPRESSION_TREE_HPP

#include <cstddef>
#include <type_traits>
#include <algorithm>

#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/integral.hpp>
#include <boost/mp11/list.hpp>

namespace boost { namespace geometry
{

namespace detail { namespace generic_robust_predicates
{

//The templates in this file are designed to represent arithmetic expressions
//In the type system so that they can be processed at compile-time for the
//automatic creation of floating-point filters.
//
//E.g. the arithmetic expression a * b + c can be represented by the type
//
//sum< product < argument<1>, argument<2> >, argument<3> >
//
//The arguments represent the input variables and are the leaves of the
//expression tree (other possible leaves are constants). Because they represent
//placeholders their indexing is one-based rather than zero-based, similar to
//the placeholders for std::bind.

enum class operator_types {
    sum, difference, product, abs, no_op, max, min
};

enum class operator_arities { nullary, unary, binary };

constexpr int sign_uncertain = -2;

struct sum_error_type {};
struct product_error_type {};
struct no_error_type {};

template <typename... Children>
struct internal_node
{
    static constexpr bool is_leaf = false;
    static constexpr bool non_negative = false;
    using all_children = boost::mp11::mp_list<Children...>; //for convenience
};

template <typename Left, typename Right>
struct internal_binary_node : internal_node<Left, Right>
{
    using left  = Left;
    using right = Right;
    static constexpr operator_arities operator_arity =
        operator_arities::binary;
};

template <typename Child>
struct internal_unary_node : internal_node<Child>
{
    using child = Child;
    static constexpr operator_arities operator_arity = operator_arities::unary;
};

template <typename Left, typename Right>
struct sum : public internal_binary_node<Left, Right>
{
    static constexpr bool sign_exact =
           (Left::is_leaf && Right::is_leaf)
        || (   Left::sign_exact && Right::sign_exact
            && Left::non_negative && Right::non_negative);
    static constexpr bool non_negative =    Left::non_negative
                                         && Right::non_negative;
    static constexpr operator_types operator_type = operator_types::sum;
    using error_type = sum_error_type;
};

template <typename Left, typename Right>
struct difference : public internal_binary_node<Left, Right>
{
    static constexpr bool sign_exact = Left::is_leaf && Right::is_leaf;
    static constexpr bool non_negative = false;
    static constexpr operator_types operator_type = operator_types::difference;
    using error_type = sum_error_type;
};

template <typename Left, typename Right>
struct product : public internal_binary_node<Left, Right>
{
    static constexpr bool sign_exact = Left::sign_exact && Right::sign_exact;
    static constexpr bool non_negative =
           (Left::non_negative && Right::non_negative)
        || std::is_same<Left, Right>::value;
    static constexpr operator_types operator_type = operator_types::product;
    using error_type = product_error_type;
};

template <typename Left, typename Right>
struct max : public internal_binary_node<Left, Right>
{
    static constexpr bool sign_exact = Left::sign_exact && Right::sign_exact;
    static constexpr bool non_negative =
        Left::non_negative || Right::non_negative;
    static constexpr operator_types operator_type = operator_types::max;
    using error_type = no_error_type;
};

template <typename Left, typename Right>
struct min : public internal_binary_node<Left, Right>
{
    static constexpr bool sign_exact = Left::sign_exact && Right::sign_exact;
    static constexpr bool non_negative =
        Left::non_negative && Right::non_negative;
    static constexpr operator_types operator_type = operator_types::min;
    using error_type = no_error_type;
};

template <typename Child>
struct abs : public internal_unary_node<Child>
{
    using error_type = no_error_type;
    static constexpr operator_types operator_type = operator_types::abs;
    static constexpr bool sign_exact = Child::sign_exact;
    static constexpr bool non_negative = true;
};

struct leaf
{
    static constexpr bool is_leaf = true;
    static constexpr bool sign_exact = true;
    static constexpr bool non_negative = false;
    static constexpr operator_types operator_type = operator_types::no_op;
    static constexpr operator_arities operator_arity = operator_arities::nullary;
};

template <std::size_t Argn>
struct argument : public leaf
{
    static constexpr std::size_t argn = Argn;
};

template <typename NumberType>
struct static_constant_interface : public leaf
{
    using value_type = NumberType;
    static constexpr NumberType value = 0; //override
    static constexpr std::size_t argn = 0;
};

template <typename Node>
using is_leaf = boost::mp11::mp_bool<Node::is_leaf>;

//post_order_impl and post_order are templates for compile-time traversion of
//expression trees. The post_order traversal was chosen because it guarantees
//that children (subexpressions) are evaluated before there parents as is
//necessary for the evaluation of the arithmetic expressions that these
//expression trees represent.

template
<
    typename In,
    typename Out,
    template <typename> class Anchor = is_leaf,
    bool IsBinary = In::operator_arity == operator_arities::binary,
    bool AtAnchor = Anchor<In>::value
>
struct post_order_impl;

template
<
    typename In,
    typename Out,
    template <typename> class Anchor,
    bool IsBinary
>
struct post_order_impl<In, Out, Anchor, IsBinary, true>
{
    using type = Out;
};

template <typename In, typename Out, template <typename> class Anchor>
struct post_order_impl<In, Out, Anchor, true, false>
{
    using leftl = typename post_order_impl
            <
                typename In::left,
                boost::mp11::mp_list<>,
                Anchor
            >::type;
    using rightl = typename post_order_impl
            <
                typename In::right,
                boost::mp11::mp_list<>,
                Anchor
            >::type;
    using merged = boost::mp11::mp_append<Out, leftl, rightl>;
    using type   =
        boost::mp11::mp_unique<boost::mp11::mp_push_back<merged, In>>;
};

template <typename In, typename Out, template <typename> class Anchor>
struct post_order_impl<In, Out, Anchor, false, false>
{
    using childl = typename post_order_impl
            <
                typename In::child,
                boost::mp11::mp_list<>,
                Anchor
            >::type;
    using merged = boost::mp11::mp_append<Out, childl>;
    using type   =
        boost::mp11::mp_unique<boost::mp11::mp_push_back<merged, In>>;
};

template <typename In, template <typename> class Anchor = is_leaf>
using post_order =
    typename post_order_impl<In, boost::mp11::mp_list<>, Anchor>::type;

template <typename Expression, operator_arities = Expression::operator_arity>
constexpr std::size_t max_argn = 0;

template <typename Expression>
constexpr std::size_t max_argn<Expression, operator_arities::binary> =
    std::max(max_argn<typename Expression::left>,
             max_argn<typename Expression::right>);

template <typename Expression>
constexpr std::size_t max_argn<Expression, operator_arities::unary> =
    max_argn<typename Expression::child>;

template <typename Expression>
constexpr std::size_t max_argn<Expression, operator_arities::nullary> =
    Expression::argn;

using  _1 = argument<1>;
using  _2 = argument<2>;
using  _3 = argument<3>;
using  _4 = argument<4>;
using  _5 = argument<5>;
using  _6 = argument<6>;
using  _7 = argument<7>;
using  _8 = argument<8>;
using  _9 = argument<9>;
using _10 = argument<10>;
using _11 = argument<11>;
using _12 = argument<12>;
using _13 = argument<13>;
using _14 = argument<14>;
using _15 = argument<15>;

}} // namespace detail::generic_robust_predicates

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_EXPRESSION_TREE_HPP
