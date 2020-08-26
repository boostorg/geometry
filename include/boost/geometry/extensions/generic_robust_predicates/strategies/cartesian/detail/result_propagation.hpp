// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020 Tinko Bartels, Berlin, Germany.

// Contributed and/or modified by Tinko Bartels,
//   as part of Google Summer of Code 2020 program.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_RESULT_PROPAGATION_HPP
#define BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_RESULT_PROPAGATION_HPP

#include <boost/mp11/list.hpp>
#include <boost/mp11/algorithm.hpp>
#include <boost/mp11/bind.hpp>

#include <boost/geometry/extensions/generic_robust_predicates/strategies/cartesian/detail/expression_tree.hpp>

namespace boost { namespace geometry
{

namespace detail { namespace generic_robust_predicates
{

template
<
    std::size_t Last,
    typename In = boost::mp11::mp_list<>,
    std::size_t At = 1
>
struct argument_list_impl
{
private:
    using next = boost::mp11::mp_push_back<In, argument<At>>;
public:
    using type = typename argument_list_impl<Last, next, At + 1>::type;
};

template
<
    std::size_t Last,
    typename In
>
struct argument_list_impl
    <
        Last,
        In,
        Last
    >
{
    using type = boost::mp11::mp_push_back<In, argument<Last>>;
};

template <std::size_t Last> using argument_list =
    typename argument_list_impl<Last>::type;

template
<
    typename Needle,
    typename Haystacks
>
struct index_pair_impl
{
private:
    template <typename List> 
    using contains = typename boost::mp11::mp_bind_back
        <
            boost::mp11::mp_contains,
            Needle
        >::template fn<List>;
    using outer = boost::mp11::mp_find_if<Haystacks, contains>;
    using inner_list = boost::mp11::mp_at<Haystacks, outer>;
    using inner = boost::mp11::mp_find<inner_list, Needle>;
public:
    using type = std::pair<outer, inner>;
};
 
template
<
    typename Needle,
    typename Haystacks
>
using index_pair = typename index_pair_impl<Needle, Haystacks>::type;

}} // namespace detail::generic_robust_predicates

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_EXTENSIONS_GENERIC_ROBUST_PREDICATES_STRATEGIES_CARTESIAN_DETAIL_RESULT_PROPAGATION_HPP
