// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Licensed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_GEOMETRY_STRATEGIES_DETAIL_POINT_CONVERSION_HPP
#define BOOST_GEOMETRY_STRATEGIES_DETAIL_POINT_CONVERSION_HPP

#include <type_traits>
#include <boost/geometry/algorithms/convert.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{

template <typename Strategy, typename Source, typename Destination>
inline void convert_geometry(Strategy const& strategy, Source const& source,
                             Destination& destination)
{
    auto const hook = [&](auto const& s) -> decltype(s.convert_geometry(source, destination), void())
    { s.convert_geometry(source, destination); };
    if constexpr (std::is_invocable_v<decltype(hook), Strategy const&>)
    {
        hook(strategy);
    }
    else
    {
        geometry::convert(source, destination);
    }
}

template <typename Strategy, typename Point, typename Output>
inline void convert_output_point(Strategy const& strategy, Point const& point, Output& output)
{
    auto const hook = [&](auto const& s) -> decltype(s.convert_output_point(point, output), void())
    { s.convert_output_point(point, output); };
    if constexpr (std::is_invocable_v<decltype(hook), Strategy const&>)
    {
        hook(strategy);
    }
    else
    {
        strategies::detail::convert_geometry(strategy, point, output);
    }
}

template <typename Strategy, typename Iterator, typename Destination>
inline void assign_point_from_iterator(Strategy const& strategy, Iterator const& iterator,
                                       Destination& destination)
{
    auto const hook = [&](auto const& s) -> decltype(s.section().assign_point(iterator, destination), void())
    { s.section().assign_point(iterator, destination); };
    if constexpr (std::is_invocable_v<decltype(hook), Strategy const&>)
    {
        hook(strategy);
    }
    else
    {
        auto copy = iterator;
        strategies::detail::convert_geometry(strategy, *copy, destination);
    }
}

}}}} // namespace boost::geometry::strategies::detail

#endif
