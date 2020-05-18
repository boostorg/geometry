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

template <typename Expression, std::size_t max_argn>
struct interval_min_impl
{
    using type = Expression;
};

template <typename Expression, std::size_t max_argn>
struct interval_max_impl
{
    using type = Expression;
};

template <typename Left, typename Right, std::size_t max_argn>
struct interval_min_impl<difference<Left, Right>, max_argn>
{
private:
    using min_left = typename interval_min_impl<Left, max_argn>::type;
    using max_right = typename interval_max_impl<Right, max_argn>::type;
public:
    using type = difference<min_left, max_right>;
};

template <typename Left, typename Right, std::size_t max_argn>
struct interval_min_impl<sum<Left, Right>, max_argn>
{
private:
    using min_left = typename interval_min_impl<Left, max_argn>::type;
    using min_right = typename interval_min_impl<Right, max_argn>::type;
public:
    using type = sum<min_left, min_right>;
};

template <typename Left, typename Right, std::size_t max_argn>
struct interval_max_impl<difference<Left, Right>, max_argn>
{
private:
    using max_left = typename interval_max_impl<Left, max_argn>::type;
    using min_right = typename interval_min_impl<Right, max_argn>::type;
public:
    using type = difference<max_left, min_right>;
};

template <typename Left, typename Right, std::size_t max_argn>
struct interval_max_impl<sum<Left, Right>, max_argn>
{
private:
    using max_left = typename interval_max_impl<Left, max_argn>::type;
    using max_right = typename interval_max_impl<Right, max_argn>::type;
public:
    using type = sum<max_left, max_right>;
};

template <typename Left, typename Right, std::size_t max_argn>
struct interval_min_impl<product<Left, Right>, max_argn>
{
private:
    using min_left = typename interval_min_impl<Left, max_argn>::type;
    using max_left = typename interval_max_impl<Left, max_argn>::type;
    using min_right = typename interval_min_impl<Right, max_argn>::type;
    using max_right = typename interval_max_impl<Right, max_argn>::type;
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

template <typename Child, std::size_t max_argn>
struct interval_min_impl<product<Child, Child>, max_argn>
{
private:
    using min_child = typename interval_min_impl<Child, max_argn>::type;
    using max_child = typename interval_max_impl<Child, max_argn>::type;
public:
    using type = min
        <
            product<min_child, min_child>,
            product<max_child, max_child>
        >;
};

template <typename Left, typename Right, std::size_t max_argn>
struct interval_max_impl<product<Left, Right>, max_argn>
{
private:
    using min_left = typename interval_min_impl<Left, max_argn>::type;
    using max_left = typename interval_max_impl<Left, max_argn>::type;
    using min_right = typename interval_min_impl<Right, max_argn>::type;
    using max_right = typename interval_max_impl<Right, max_argn>::type;
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

template <typename Child, std::size_t max_argn>
struct interval_max_impl<product<Child, Child>, max_argn>
{
private:
    using min_child = typename interval_min_impl<Child, max_argn>::type;
    using max_child = typename interval_max_impl<Child, max_argn>::type;
public:
    using type = max
        <
            product<min_child, min_child>,
            product<max_child, max_child>
        >;
};

template <typename Child, std::size_t max_argn>
struct interval_min_impl<abs<Child>, max_argn>
{
private:
    using min_child = typename interval_min_impl<Child, max_argn>::type;
    using max_child = typename interval_max_impl<Child, max_argn>::type;
public:
    using type = min<abs<min_child>, abs<max_child>>;
};

template <typename Child, std::size_t max_argn>
struct interval_max_impl<abs<Child>, max_argn>
{
private:
    using min_child = typename interval_min_impl<Child, max_argn>::type;
    using max_child = typename interval_max_impl<Child, max_argn>::type;
public:
    using type = max<abs<min_child>, abs<max_child>>;
};

template <std::size_t argn, std::size_t max_argn>
struct interval_min_impl<argument<argn>, max_argn>
{
    using type = argument<argn + max_argn>;
};

template <std::size_t argn, std::size_t max_argn>
struct interval_max_impl<argument<argn>, max_argn>
{
    using type = argument<argn>;
};

template <typename Expression, std::size_t max_argn>
struct interval_impl
{
    using type = Expression;
};

template <typename Child, std::size_t max_argn>
struct interval_impl<abs<Child>, max_argn>
{
private:
    using min_child = typename interval_min_impl<Child, max_argn>::type;
    using max_child = typename interval_max_impl<Child, max_argn>::type;
public:
    using type = max<abs<min_child>, abs<max_child>>;
};

template <typename Left, typename Right, std::size_t max_argn>
struct interval_impl<difference<Left, Right>, max_argn>
{
private:
    using left = typename interval_impl<Left, max_argn>::type;
    using right = typename interval_impl<Right, max_argn>::type;
public:
    using type = difference<left, right>;
};

template <typename Left, typename Right, std::size_t max_argn>
struct interval_impl<sum<Left, Right>, max_argn>
{
private:
    using left = typename interval_impl<Left, max_argn>::type;
    using right = typename interval_impl<Right, max_argn>::type;
public:
    using type = sum<left, right>;
};

template <typename Left, typename Right, std::size_t max_argn>
struct interval_impl<product<Left, Right>, max_argn>
{
private:
    using left = typename interval_impl<Left, max_argn>::type;
    using right = typename interval_impl<Right, max_argn>::type;
public:
    using type = product<left, right>;
};

template <typename Expression, std::size_t max_argn>
using interval_min = typename interval_min_impl<Expression, max_argn>::type;

template <typename Expression, std::size_t max_argn>
using interval_max = typename interval_max_impl<Expression, max_argn>::type;

template <typename Expression>
using interval =
    typename interval_impl<Expression, max_argn<Expression>::value>::type;

}} // namespace detail::generic_robust_predicates

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_INTERVAL_ERROR_BOUND_HPP
