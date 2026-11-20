// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_CONSTRUCTION_CAPACITY_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_CONSTRUCTION_CAPACITY_HPP

#include <cstddef>
#include <type_traits>
#include <boost/geometry/strategies/cartesian/exact/fixed_arithmetic.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

// These describe construction preconditions, not just interchangeable storage.
// Query results must not acquire original-edge provenance by fitting a budget.
enum class construction_family
{
    native_vertex,
    original_support_intersection,
    midpoint_query,
    splice_query,
    native_axis_clip_query
};

namespace construction_capacity_detail
{
template <std::size_t N, std::size_t D, bool GeometryPoint = false>
struct point_capacity
{
    static constexpr std::size_t numerator_bits = N;
    static constexpr std::size_t denominator_bits = D;
    static constexpr bool is_geometry_point = GeometryPoint;
    using homogeneous_type = fixed_homogeneous<N, D>;
};
} // namespace construction_capacity_detail

template <typename Coordinate, construction_family Family, typename = void>
struct construction_capacity;

template <typename C>
struct construction_capacity<C, construction_family::native_vertex,
    std::enable_if_t<fixed_native_supported<C>::value>>
    : construction_capacity_detail::point_capacity<native_fixed_traits<C>::coordinate_bits,
        native_fixed_traits<C>::scale_bits + 1, true>
{};

template <typename C>
struct construction_capacity<C, construction_family::original_support_intersection,
    std::enable_if_t<fixed_native_supported<C>::value>>
    : construction_capacity_detail::point_capacity<native_fixed_traits<C>::intersection_numerator_bits,
        native_fixed_traits<C>::intersection_denominator_bits, true>
{};

// Both operands are native vertices or original-support intersections, not
// arbitrary values in the query budget. These match fixed_midpoint/fixed_splice.
template <typename C>
struct construction_capacity<C, construction_family::midpoint_query,
    std::enable_if_t<fixed_native_supported<C>::value>>
    : construction_capacity_detail::point_capacity<native_fixed_traits<C>::intersection_numerator_bits
        + native_fixed_traits<C>::intersection_denominator_bits + 1,
        2 * native_fixed_traits<C>::intersection_denominator_bits + 1>
{};

template <typename C>
struct construction_capacity<C, construction_family::splice_query,
    std::enable_if_t<fixed_native_supported<C>::value>>
    : construction_capacity_detail::point_capacity<native_fixed_traits<C>::intersection_numerator_bits
        + native_fixed_traits<C>::intersection_denominator_bits,
        2 * native_fixed_traits<C>::intersection_denominator_bits>
{};

// Original grid line (U,V,C), native bound p/Q: vertical clipping gives
// (V*p, -(U*p+C), Q*V). Horizontal clipping swaps U/V and X/Y.
// Cancel Q algebraically first; the general four-point cross needs more scratch.
template <typename C>
struct construction_capacity<C, construction_family::native_axis_clip_query,
    std::enable_if_t<fixed_native_supported<C>::value>>
    : construction_capacity_detail::point_capacity<2 * native_fixed_traits<C>::coordinate_bits + 2,
        native_fixed_traits<C>::coordinate_bits + native_fixed_traits<C>::scale_bits + 1>
{};

// Match the declared operand formats, including any widening at the call site.
// Scratch bounds do not authorize promotion into any construction family.
template <typename A, typename B, typename C = B>
struct construction_predicate_capacity
{
    static constexpr std::size_t coordinate_compare_bits = fixed_detail::maximum(
        A::numerator_bits + B::denominator_bits, B::numerator_bits + A::denominator_bits) + 1;
    static constexpr std::size_t orientation_bits = fixed_detail::maximum(
        A::numerator_bits + B::numerator_bits + C::denominator_bits,
        fixed_detail::maximum(A::numerator_bits + B::denominator_bits + C::numerator_bits,
            A::denominator_bits + B::numerator_bits + C::numerator_bits)) + 3;
    static constexpr std::size_t direction_bits = coordinate_compare_bits + fixed_detail::maximum(
        B::numerator_bits + C::denominator_bits, C::numerator_bits + B::denominator_bits) + 2;

    using coordinate_compare_type = fixed_integer<coordinate_compare_bits>;
    using orientation_type = fixed_integer<orientation_bits>;
    using direction_type = fixed_integer<direction_bits>;
};

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

#endif
