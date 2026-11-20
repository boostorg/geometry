// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_OUTPUT_POLICY_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_OUTPUT_POLICY_HPP

#include <cstddef>
#include <type_traits>

namespace boost { namespace geometry { namespace strategies { namespace relate
{

// Emit the approximation already stored at the output boundary. Predicates
// remain exact, but emission neither refines nor certifies these coordinates.
struct approximate_output {};

// Root-bound intersections store the nearest binary64 at construction. Float
// emission rounds the exact coordinate directly to binary32. Owning exact_point
// keeps its stored error bounds and rounds floating coordinates at emission.
// Integral output follows BG numeric_cast conversion under every policy.
struct nearest_output {};

// Bound constructed binary64 output by N representable steps from nearest.
// Root-bound points retain no coordinate error fields; predicates use the
// construction's bound specialized on N.
template <std::size_t N>
struct bounded_ulp_output
{
    static_assert(N >= 1, "bounded_ulp_output requires at least one ULP");
    static constexpr std::size_t steps = N;
};

}}}} // namespace boost::geometry::strategies::relate

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

template <typename Policy> struct is_bounded_ulp_output : std::false_type {};
template <std::size_t N>
struct is_bounded_ulp_output<relate::bounded_ulp_output<N>> : std::true_type {};

}}}}}

#endif
