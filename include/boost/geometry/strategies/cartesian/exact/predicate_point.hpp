// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_PREDICATE_POINT_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_PREDICATE_POINT_HPP

#include <cstdint>
#include <type_traits>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/strategies/cartesian/exact/fixed_arithmetic.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

template <typename Point, typename = void>
struct root_of { using type = void; };

template <typename Point>
struct root_of<Point, std::void_t<typename std::decay_t<Point>::root_type>>
{ using type = typename std::decay_t<Point>::root_type; };

// Indirect points must reach the predicate in their own exact format;
// their BG coordinate accessors can expose rounded coordinates.
template <typename Point, typename Function>
decltype(auto) with_homogeneous(Point const& point, Function const& function)
{
    if constexpr (!std::is_void<typename root_of<Point>::type>::value)
        return function(point.homogeneous());
    else return function(fixed_input_homogeneous(geometry::get<0>(point), geometry::get<1>(point)));
}

template <typename A, typename B, typename Function>
decltype(auto) with_homogeneous(A const& a, B const& b, Function const& function)
{
    return with_homogeneous(a, [&](auto const& x) -> decltype(auto)
    { return with_homogeneous(b, [&](auto const& y) -> decltype(auto) { return function(x, y); }); });
}

template <typename A, typename B, typename P, typename Function>
decltype(auto) with_homogeneous(A const& a, B const& b, P const& point, Function const& function)
{
    return with_homogeneous(a, b, [&](auto const& x, auto const& y) -> decltype(auto)
    { return with_homogeneous(point, [&](auto const& z) -> decltype(auto) { return function(x, y, z); }); });
}

template <typename Point>
inline bool small_integer_coordinate(Point const& point, std::size_t axis, std::int64_t& output)
{
    typename Point::input_coordinate_type native;
    if (!point.native_coordinate(axis, native)) { return false; }
    double const value = static_cast<double>(native);
    constexpr double limit = 1073741824.0; // 2^30; integers in this range convert exactly.
    if (!(value >= -limit && value <= limit)) { return false; }
    auto const integer = static_cast<std::int64_t>(value);
    if (static_cast<typename Point::input_coordinate_type>(integer) != native) { return false; }
    output = integer;
    return true;
}

template <typename A, typename B>
inline bool small_integer_squared_distance(A const& a, B const& b,
                                           std::uint64_t& output)
{
    std::int64_t ax, ay, bx, by;
    if (!small_integer_coordinate(a, 0, ax) || !small_integer_coordinate(a, 1, ay)
        || !small_integer_coordinate(b, 0, bx) || !small_integer_coordinate(b, 1, by)) { return false; }
    auto const dx = bx - ax, dy = by - ay;
    auto const x = static_cast<std::uint64_t>(dx < 0 ? -dx : dx);
    auto const y = static_cast<std::uint64_t>(dy < 0 ? -dy : dy);
    // |dx|,|dy| <= 2^31, so their sum of squares is <= 2^63.
    output = x * x + y * y;
    return true;
}

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

#endif
