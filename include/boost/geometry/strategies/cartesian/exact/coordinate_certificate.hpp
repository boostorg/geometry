// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_COORDINATE_CERTIFICATE_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_COORDINATE_CERTIFICATE_HPP

#include <cstddef>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian { namespace ulp_filter
{

// |X-z| <= rho*u*max(|z|, min_normal). This is a bound on the true ordinate,
// not an ordering guarantee. The proof of the bilinear coefficient covers
// this range of rho, including gradual underflow in magnitude arithmetic.
template <std::size_t Rho>
struct coordinate_certificate
{
    static constexpr std::size_t rho = Rho;
};

using nearest_certificate = coordinate_certificate<1>; // half a local spacing
}}}}}}

#endif
