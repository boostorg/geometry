// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_UTIL_DETAIL_SEMI_STATIC_ERROR_HPP
#define BOOST_GEOMETRY_UTIL_DETAIL_SEMI_STATIC_ERROR_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>

#include <boost/geometry/util/detail/precise_fp.hpp>

BOOST_GEOMETRY_DETAIL_PRECISE_FP_BEGIN

namespace boost { namespace geometry { namespace detail { namespace semi_static_error
{

// Shallow polynomial rules: a product of leaf differences, then a sum or
// difference of products. Units are first-order coefficients in unit roundoff,
// not individually certified bounds. The terminal rule accounts for the rest.
template <typename Real, std::size_t Units, bool Propagated, bool Product = false>
struct term
{
    Real head, magnitude;
};

template <typename Real>
inline Real leaf_magnitude(Real head)
{
    return (std::max)(std::abs(head), (std::numeric_limits<Real>::min)());
}

// Rho*u*max(|head|, min_normal) is a producer's error relative to the
// true coordinate. For a native leaf Rho is zero.
template <std::size_t A, std::size_t B, typename Real>
inline auto difference(Real a, Real b)
{
    Real const head = a - b;
    constexpr std::size_t rho = A > B ? A : B;
    static_assert(rho <= (1u << 25) + 2, "Outside the binary64 certificate proof");
    // With exact leaves the rounded difference itself is a magnitude.
    // Otherwise subtraction propagates both leaf magnitudes even when
    // the computed difference vanishes.
    Real const magnitude = rho == 0 ? std::abs(head)
        : leaf_magnitude(a) + leaf_magnitude(b);
    return term<Real, rho + 1, rho != 0>{head, magnitude};
}

template <typename Real, std::size_t A, std::size_t B, bool AP, bool BP>
inline auto product(term<Real, A, AP> const& a, term<Real, B, BP> const& b)
{
    Real const head = a.head * b.head;
    Real const magnitude = AP || BP ? a.magnitude * b.magnitude : std::abs(head);
    return term<Real, A + B + 1, AP || BP, true>{head, magnitude};
}

// Native sign test, with no underflow/overflow. Rounding the products
// through r operations gives error ( (1-u)^-r - 1 ) relative to their heads.
// Include upper rounding of the difference and lower rounding of the
// magnitude and the final bound: multiply that error by (1+u)/(1-u)^2.
// For r<=3, the coefficient of u^2 is r*(r+7)/2 and the remaining
// terms are <64*u^3 for u<=2^-24. Round the quadratic coefficient
// to a multiple of four so r+quadratic*u is exact in the native format.
template <typename Real, std::size_t L, std::size_t R>
constexpr Real native_sign_coefficient(term<Real, L, false, true> const&,
                                       term<Real, R, false, true> const&)
{
    constexpr auto r = (std::max)(L, R);
    static_assert(r <= 3, "Prove the remainder for more rounded operations");
    constexpr auto quadratic = 4 * ((r * (r + 7) / 2 + 4) / 4);
    constexpr Real u = std::numeric_limits<Real>::epsilon() / 2;
    return (Real(r) + Real(quadratic) * u) * u;
}

// Binary64 absolute bound on a sum or difference of two products. First-order
// units are max(L,R) from the products and one from the final operation.
// Two more units cover product underflow, one covers rounding the bound
// itself in the subnormal range, and one covers all higher-order terms.
//
// To see the latter, write a=A*u/(1-u), b=B*u/(1-u) for the two factor
// errors: leaf subtraction is exact in the subnormal range. A product
// has error <= k*M+(1+k)*eta, k=(a+b+a*b+u)/(1-u), eta=u*min_normal.
// This applies to both a reused product head and a rounded scale product.
// For factor units <=2^25+3, k<(A+B+1+1/4)*u. The scale sum and final
// sum or subtraction are exact if subnormal. At normal magnitudes their rounding,
// the two k*eta terms and the final bound's relative rounding consume
// <1/4*u*max(fl(M1+M2), min_normal) beyond the first-order terms.
template <std::size_t L, std::size_t R, bool LP, bool RP>
inline double bilinear_bound(term<double, L, LP, true> const& left,
                             term<double, R, RP, true> const& right)
{
    constexpr auto r = (std::max)(L, R);
    static_assert(r <= 2 * ((1u << 25) + 3) + 1, "Outside the binary64 certificate proof");
    constexpr double u = std::numeric_limits<double>::epsilon() / 2;
    return (r + 5) * u * (std::max)(left.magnitude + right.magnitude,
                                    (std::numeric_limits<double>::min)());
}

}}}} // namespace boost::geometry::detail::semi_static_error

BOOST_GEOMETRY_DETAIL_PRECISE_FP_END

#endif
