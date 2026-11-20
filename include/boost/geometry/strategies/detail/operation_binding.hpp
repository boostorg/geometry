// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_DETAIL_OPERATION_BINDING_HPP
#define BOOST_GEOMETRY_STRATEGIES_DETAIL_OPERATION_BINDING_HPP

#include <utility>

namespace boost { namespace geometry { namespace strategies { namespace detail
{
namespace operation_binding_detail
{
template <typename Strategy, typename A, typename B, typename Apply>
auto apply(Strategy const& strategy, A const& a, B const& b, Apply&& run, int)
    -> decltype(strategy.with_sources(a, b, std::forward<Apply>(run)))
{ return strategy.with_sources(a, b, std::forward<Apply>(run)); }

template <typename Strategy, typename A, typename B, typename Apply>
decltype(auto) apply(Strategy const& strategy, A const&, B const&, Apply&& run, long)
{ return std::forward<Apply>(run)(strategy); }
}

template <typename Strategy, typename A, typename B, typename Apply>
decltype(auto) with_operation_sources(Strategy const& strategy, A const& a, B const& b, Apply&& run)
{ return operation_binding_detail::apply(strategy, a, b, std::forward<Apply>(run), 0); }

}}}}

#endif
