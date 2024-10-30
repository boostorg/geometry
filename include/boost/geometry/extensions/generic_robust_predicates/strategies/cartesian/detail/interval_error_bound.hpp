// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2020 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_INTERVAL_ERROR_BOUND_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_INTERVAL_ERROR_BOUND_HPP

#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/expression_tree.hpp>

namespace boost { namespace geometry
{

namespace detail { namespace generic_robust_predicates
{

//This file contains methods to manipulate expression trees based on the ideas
//found in https://en.wikipedia.org/wiki/Interval_arithmetic. The purpose of
//this is to transform error expressions for semi-static filters, i.e. error
//expressions that compute error bounds for specific inputs, to error
//expressions for static filters that compute error bounds for upper and lower
//bounds on input values.

template <typename Expression, std::size_t MaxArgn>
struct interval_min_impl
{
    using type = Expression;
};

template <typename Expression, std::size_t MaxArgn>
struct interval_max_impl
{
    using type = Expression;
};

template <typename Left, typename Right, std::size_t MaxArgn>
struct interval_min_impl<difference<Left, Right>, MaxArgn>
{
private:
    using min_left = typename interval_min_impl<Left, MaxArgn>::type;
    using max_right = typename interval_max_impl<Right, MaxArgn>::type;
public:
    using type = difference<min_left, max_right>;
};

template <typename Left, typename Right, std::size_t MaxArgn>
struct interval_min_impl<sum<Left, Right>, MaxArgn>
{
private:
    using min_left = typename interval_min_impl<Left, MaxArgn>::type;
    using min_right = typename interval_min_impl<Right, MaxArgn>::type;
public:
    using type = sum<min_left, min_right>;
};

template <typename Left, typename Right, std::size_t MaxArgn>
struct interval_max_impl<difference<Left, Right>, MaxArgn>
{
private:
    using max_left = typename interval_max_impl<Left, MaxArgn>::type;
    using min_right = typename interval_min_impl<Right, MaxArgn>::type;
public:
    using type = difference<max_left, min_right>;
};

template <typename Left, typename Right, std::size_t MaxArgn>
struct interval_max_impl<sum<Left, Right>, MaxArgn>
{
private:
    using max_left = typename interval_max_impl<Left, MaxArgn>::type;
    using max_right = typename interval_max_impl<Right, MaxArgn>::type;
public:
    using type = sum<max_left, max_right>;
};

template <typename Left, typename Right, std::size_t MaxArgn>
struct interval_min_impl<product<Left, Right>, MaxArgn>
{
private:
    using min_left = typename interval_min_impl<Left, MaxArgn>::type;
    using max_left = typename interval_max_impl<Left, MaxArgn>::type;
    using min_right = typename interval_min_impl<Right, MaxArgn>::type;
    using max_right = typename interval_max_impl<Right, MaxArgn>::type;
public:
    using type = min
        <
            min
                <
                    product<min_left, min_right>,
                    product<min_left, max_right>
                >,
            min
                <
                    product<max_left, min_right>,
                    product<max_left, max_right>
                >
        >;
};

template <typename Child, std::size_t MaxArgn>
struct interval_min_impl<product<Child, Child>, MaxArgn>
{
private:
    using min_child = typename interval_min_impl<Child, MaxArgn>::type;
    using max_child = typename interval_max_impl<Child, MaxArgn>::type;
public:
    using type = min
        <
            product<min_child, min_child>,
            product<max_child, max_child>
        >;
};

template <typename Left, typename Right, std::size_t MaxArgn>
struct interval_max_impl<product<Left, Right>, MaxArgn>
{
private:
    using min_left = typename interval_min_impl<Left, MaxArgn>::type;
    using max_left = typename interval_max_impl<Left, MaxArgn>::type;
    using min_right = typename interval_min_impl<Right, MaxArgn>::type;
    using max_right = typename interval_max_impl<Right, MaxArgn>::type;
public:
    using type = max
        <
            max
                <
                    product<min_left, min_right>,
                    product<min_left, max_right>
                >,
            max
                <
                    product<max_left, min_right>,
                    product<max_left, max_right>
                >
        >;
};

template <typename Child, std::size_t MaxArgn>
struct interval_max_impl<product<Child, Child>, MaxArgn>
{
private:
    using min_child = typename interval_min_impl<Child, MaxArgn>::type;
    using max_child = typename interval_max_impl<Child, MaxArgn>::type;
public:
    using type = max
        <
            product<min_child, min_child>,
            product<max_child, max_child>
        >;
};

template <typename Child, std::size_t MaxArgn>
struct interval_min_impl<abs<Child>, MaxArgn>
{
private:
    using min_child = typename interval_min_impl<Child, MaxArgn>::type;
    using max_child = typename interval_max_impl<Child, MaxArgn>::type;
public:
    using type = min<abs<min_child>, abs<max_child>>;
};

template <typename Child, std::size_t MaxArgn>
struct interval_max_impl<abs<Child>, MaxArgn>
{
private:
    using min_child = typename interval_min_impl<Child, MaxArgn>::type;
    using max_child = typename interval_max_impl<Child, MaxArgn>::type;
public:
    using type = max<abs<min_child>, abs<max_child>>;
};

template <std::size_t argn, std::size_t MaxArgn>
struct interval_min_impl<argument<argn>, MaxArgn>
{
    using type = argument<argn + MaxArgn>;
};

template <std::size_t argn, std::size_t MaxArgn>
struct interval_max_impl<argument<argn>, MaxArgn>
{
    using type = argument<argn>;
};

template <typename Expression, std::size_t MaxArgn>
struct interval_impl
{
    using type = Expression;
};

template <typename Child, std::size_t MaxArgn>
struct interval_impl<abs<Child>, MaxArgn>
{
private:
    using min_child = typename interval_min_impl<Child, MaxArgn>::type;
    using max_child = typename interval_max_impl<Child, MaxArgn>::type;
public:
    using type = max<abs<min_child>, abs<max_child>>;
};

template <typename Left, typename Right, std::size_t MaxArgn>
struct interval_impl<max<Left, Right>, MaxArgn>
{
private:
    using left = typename interval_impl<Left, MaxArgn>::type;
    using right = typename interval_impl<Right, MaxArgn>::type;
public:
    using type = max<left, right>;
};

template <typename Left, typename Right, std::size_t MaxArgn>
struct interval_impl<difference<Left, Right>, MaxArgn>
{
private:
    using left = typename interval_impl<Left, MaxArgn>::type;
    using right = typename interval_impl<Right, MaxArgn>::type;
public:
    using type = difference<left, right>;
};

template <typename Left, typename Right, std::size_t MaxArgn>
struct interval_impl<sum<Left, Right>, MaxArgn>
{
private:
    using left = typename interval_impl<Left, MaxArgn>::type;
    using right = typename interval_impl<Right, MaxArgn>::type;
public:
    using type = sum<left, right>;
};

template <typename Left, typename Right, std::size_t MaxArgn>
struct interval_impl<product<Left, Right>, MaxArgn>
{
private:
    using left = typename interval_impl<Left, MaxArgn>::type;
    using right = typename interval_impl<Right, MaxArgn>::type;
public:
    using type = product<left, right>;
};

template <typename Expression, std::size_t MaxArgn>
using interval_min = typename interval_min_impl<Expression, MaxArgn>::type;

template <typename Expression, std::size_t MaxArgn>
using interval_max = typename interval_max_impl<Expression, MaxArgn>::type;

template <typename Expression>
using interval =
    typename interval_impl<Expression, max_argn<Expression>>::type;

}} // namespace detail::generic_robust_predicates

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_INTERVAL_ERROR_BOUND_HPP
