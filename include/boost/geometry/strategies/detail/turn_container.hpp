// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Licensed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_GEOMETRY_STRATEGIES_DETAIL_TURN_CONTAINER_HPP
#define BOOST_GEOMETRY_STRATEGIES_DETAIL_TURN_CONTAINER_HPP

#include <type_traits>

namespace boost { namespace geometry { namespace strategies { namespace detail
{

// Local storage is valid only when no turn point outlives its container.
enum class turn_storage { operation, local };

template <typename DefaultContainer, turn_storage Storage = turn_storage::operation,
          typename Strategy, typename Geometry1, typename Geometry2>
inline decltype(auto) make_turn_container(Strategy const& strategy,
                                         Geometry1 const& geometry1, Geometry2 const& geometry2)
{
    auto const hook = [&](auto const& s) -> decltype(
        s.template make_turn_container<DefaultContainer, Storage == turn_storage::local>(geometry1, geometry2))
    {
        return s.template make_turn_container<DefaultContainer, Storage == turn_storage::local>(geometry1, geometry2);
    };
    if constexpr (std::is_invocable_v<decltype(hook), Strategy const&>)
        return hook(strategy);
    else
        return DefaultContainer{};
}

struct empty_segment_pair_scope {};

template <bool Reverse1, bool Reverse2, typename Strategy, typename Identifier>
inline decltype(auto) segment_pair_scope(Strategy const& strategy, Identifier const& a, Identifier const& b)
{
    auto const hook = [&](auto const& s) -> decltype(s.template segment_pair_scope<Reverse1, Reverse2>(a, b))
    { return s.template segment_pair_scope<Reverse1, Reverse2>(a, b); };
    if constexpr (std::is_invocable_v<decltype(hook), Strategy const&>)
    {
        return hook(strategy);
    }
    else
    {
        return empty_segment_pair_scope{};
    }
}

}}}} // namespace boost::geometry::strategies::detail

#endif // BOOST_GEOMETRY_STRATEGIES_DETAIL_TURN_CONTAINER_HPP
