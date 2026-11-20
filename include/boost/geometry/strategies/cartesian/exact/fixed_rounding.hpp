// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_FIXED_ROUNDING_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_FIXED_ROUNDING_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>

#include <boost/geometry/strategies/cartesian/exact/fixed_arithmetic.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{
namespace exact_cartesian
{

namespace fixed_rounding_detail
{

// Both operands are nonnegative, with denominator>0. Comparing r with d-r
// avoids a doubled-remainder intermediate and implements nearest, ties-to-even.
template <typename Integer>
inline Integer rounded_quotient(Integer const& numerator, Integer const& denominator)
{
    Integer quotient, remainder;
    boost::multiprecision::divide_qr(numerator, denominator, quotient, remainder);
    Integer const complement = denominator - remainder;
    if (remainder > complement || (remainder == complement && (quotient & 1) != 0))
    {
        ++quotient;
    }
    return quotient;
}

template <typename Output, typename Enable = void>
struct rounder
{
    template <std::size_t N, std::size_t D>
    static bool apply(fixed_integer<N> const&, fixed_integer<D> const&, Output&) { return false; }
};

template <typename Output>
struct rounder<Output, typename std::enable_if<std::is_integral<Output>::value
    && (std::numeric_limits<Output>::digits <= 64)>::type>
{
    template <std::size_t N, std::size_t D>
    static bool apply(fixed_integer<N> const& numerator, fixed_integer<D> const& denominator, Output& output)
    {
        if (denominator <= 0) { return false; }
        // Reserve one bit for rounding carry and enough room for native limits.
        using integer = fixed_integer<fixed_detail::maximum(fixed_detail::maximum(N, D), 65) + 1>;
        bool const negative = numerator < 0;
        integer const magnitude = negative ? integer(-numerator) : integer(numerator);
        integer result = rounded_quotient(magnitude, integer(denominator));
        if (negative) { result = -result; }
        if (result < integer((std::numeric_limits<Output>::min)())
            || result > integer((std::numeric_limits<Output>::max)()))
        {
            throw std::overflow_error("Exact Cartesian integral output is out of range");
        }
        output = result.template convert_to<Output>();
        return true;
    }
};

template <typename Output>
struct rounder<Output, typename std::enable_if<std::is_same<Output, float>::value
    || std::is_same<Output, double>::value>::type>
{
    template <std::size_t N, std::size_t D>
    static bool apply(fixed_integer<N> const& numerator, fixed_integer<D> const& denominator, Output& output)
    {
        static_assert(std::numeric_limits<Output>::is_iec559 && std::numeric_limits<Output>::radix == 2,
                      "Fixed Cartesian floating output requires IEEE binary arithmetic");
        if (denominator <= 0) { return false; }
        if (numerator == 0) { output = Output(0); return true; }
        constexpr int digits = std::numeric_limits<Output>::digits;
        constexpr int minimum_step = std::numeric_limits<Output>::min_exponent - digits;
        // A retained finite exponent needs at most -minimum_step numerator
        // shifts, or max_exponent denominator shifts, plus two spare bits.
        // For double these budgets are N+1076 and D+1026, respectively.
        using integer = fixed_integer<fixed_detail::maximum(N + std::size_t(-minimum_step) + 2,
            D + std::numeric_limits<Output>::max_exponent + 2)>;
        bool const negative = numerator < 0;
        integer n = negative ? integer(-numerator) : integer(numerator);
        integer d(denominator);
        std::intmax_t exponent = std::intmax_t(boost::multiprecision::msb(n))
                              - std::intmax_t(boost::multiprecision::msb(d));
        // Shifted operands have the same msb as the other operand, so these
        // comparisons fit max(N,D) bits even before the additional headroom.
        if (exponent >= 0 ? n < (d << exponent) : (n << -exponent) < d) { --exponent; }
        if (exponent >= std::numeric_limits<Output>::max_exponent)
        {
            output = negative ? -(std::numeric_limits<Output>::infinity)()
                              : (std::numeric_limits<Output>::infinity)();
            return true;
        }
        if (exponent < minimum_step - 1)
        {
            output = negative ? -Output(0) : Output(0);
            return true;
        }
        int const step = int((std::max)(exponent - digits + 1, std::intmax_t(minimum_step)));
        if (step < 0) { n <<= -step; }
        else { d <<= step; }
        integer const significand = rounded_quotient(n, d);
        if (exponent == std::numeric_limits<Output>::max_exponent - 1
            && significand == (integer(1) << digits))
        {
            output = negative ? -(std::numeric_limits<Output>::infinity)()
                              : (std::numeric_limits<Output>::infinity)();
            return true;
        }
        // q<=2^digits; all such integers (including the carry power of two)
        // convert exactly. The chosen grid makes ldexp exact, even subnormally,
        // so neither operation depends on the ambient floating rounding mode.
        auto const native_significand = significand.template convert_to<std::uint64_t>();
        Output const magnitude = std::ldexp(static_cast<Output>(native_significand), step);
        output = negative ? -magnitude : magnitude;
        return true;
    }
};

} // namespace fixed_rounding_detail

// Unsupported outputs and nonpositive denominators leave output unchanged.
// Supported integral outputs throw on range overflow, as the rational rounder
// does. Floating outputs include gradual underflow, signed zero and infinity.
template <typename Output, std::size_t N, std::size_t D>
inline bool try_fixed_round(fixed_integer<N> const& numerator, fixed_integer<D> const& denominator,
                            Output& output)
{
    return fixed_rounding_detail::rounder<Output>::apply(numerator, denominator, output);
}

template <typename Output, std::size_t N, std::size_t D>
inline bool try_fixed_round(fixed_homogeneous<N, D> const& point, std::size_t dimension, Output& output)
{
    if (dimension > 1) { throw std::out_of_range("Exact Cartesian coordinate dimension"); }
    return try_fixed_round<Output>(dimension == 0 ? point.X : point.Y, point.W, output);
}

} // namespace exact_cartesian
}}}} // namespace boost::geometry::strategies::detail

#endif
