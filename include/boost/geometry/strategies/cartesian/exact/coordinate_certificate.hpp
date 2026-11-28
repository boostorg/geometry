// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_COORDINATE_CERTIFICATE_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_COORDINATE_CERTIFICATE_HPP

#include <cstddef>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian { namespace ulp_filter
{

constexpr std::size_t max_steps = 1u << 24;
template <std::size_t N>
constexpr std::size_t effective_steps = N < max_steps ? N : max_steps;

// |X-z| <= rho*u*max(|z|, min_normal). This is a bound on the true ordinate,
// not an ordering guarantee. The proof of the bilinear coefficient covers
// this range of rho, including gradual underflow in magnitude arithmetic.
template <std::size_t Rho>
struct coordinate_certificate
{
    static_assert(Rho <= 2 * max_steps + 2, "Coordinate error exceeds the bilinear filter proof");
    static constexpr std::size_t rho = Rho;
};

using nearest_certificate = coordinate_certificate<1>; // half a local spacing
template <std::size_t N> using producer_certificate =
    coordinate_certificate<N == 0 ? 1 : effective_steps<N> + 2>;

}}}}}}

#endif
