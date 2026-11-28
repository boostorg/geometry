// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_DETAIL_OPERATION_BINDING_HPP
#define BOOST_GEOMETRY_STRATEGIES_DETAIL_OPERATION_BINDING_HPP

#include <type_traits>
#include <utility>
#include <boost/geometry/strategies/relate/services.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{

// An opt-in strategy can supply operation-local geometries as well as a kernel.
template <typename Strategy, typename A, typename B, typename Apply>
decltype(auto) with_operation_operands(Strategy const& strategy, A const& a, B const& b, Apply&& run)
{
    auto const hook = [&](auto const& s) -> decltype(s.with_operands(a, b, std::forward<Apply>(run)))
    { return s.with_operands(a, b, std::forward<Apply>(run)); };
    if constexpr (std::is_invocable_v<decltype(hook), Strategy const&>)
    {
        return hook(strategy);
    }
    else
    {
        return std::forward<Apply>(run)(a, b, strategy);
    }
}

template <typename Strategy, typename A, typename B, typename Apply>
decltype(auto) with_predicate_operands(Strategy const& strategy, A const& a, B const& b, Apply&& run)
{
    auto const& predicate = relate::services::predicate_strategy<Strategy>::get(strategy, a, b);
    return strategies::detail::with_operation_operands(predicate, a, b, std::forward<Apply>(run));
}

}}}}

#endif
