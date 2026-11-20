// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_RATIONAL_ROUNDING_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_RATIONAL_ROUNDING_HPP

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

#include <boost/geometry/strategies/cartesian/exact/arithmetic.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian { namespace rational_rounding
{

using boost::multiprecision::cpp_int;

inline cpp_int rounded_quotient(cpp_int const& numerator, cpp_int const& denominator,
                                bool* exact = nullptr)
{
    cpp_int result, remainder;
    boost::multiprecision::divide_qr(numerator, denominator, result, remainder);
    if (exact) { *exact = remainder == 0; }
    cpp_int const twice = remainder << 1;
    if (twice > denominator || (twice == denominator && (result & 1) != 0))
    {
        ++result;
    }
    return result;
}

template <typename Output>
struct rounded_value
{
    Output value;
    bool exact;
};

template <typename Output>
inline rounded_value<Output> round(exact_number const& value)
{
    using limits = std::numeric_limits<Output>;
    static_assert(limits::radix == 2 && limits::is_iec559 && limits::digits < 64,
                  "Exact Cartesian output requires IEEE binary arithmetic");
    cpp_int numerator = boost::multiprecision::numerator(value);
    if (numerator == 0) { return {Output(0), true}; }
    bool const negative = numerator < 0;
    if (negative) { numerator = -numerator; }
    cpp_int denominator = boost::multiprecision::denominator(value);
    constexpr int digits = limits::digits;
    constexpr int minimum_step = limits::min_exponent - digits;
    Output const infinity = (limits::infinity)();
    Output const zero = negative ? -Output(0) : Output(0);

    std::intmax_t exponent = std::intmax_t(boost::multiprecision::msb(numerator))
                           - std::intmax_t(boost::multiprecision::msb(denominator));
    // floor(log2(value)) is either this difference or one less. Discard
    // extreme magnitudes before shifting either arbitrary-precision operand.
    if (exponent > limits::max_exponent) { return {negative ? -infinity : infinity, false}; }
    if (exponent < minimum_step - 1) { return {zero, false}; }
    if (exponent >= 0 ? numerator < (denominator << exponent)
                      : (numerator << -exponent) < denominator)
    {
        --exponent;
    }
    if (exponent >= limits::max_exponent) { return {negative ? -infinity : infinity, false}; }
    if (exponent < minimum_step - 1) { return {zero, false}; }

    int const step = int((std::max)(exponent - digits + 1, std::intmax_t(minimum_step)));
    if (step < 0) { numerator <<= -step; }
    else { denominator <<= step; }
    bool exact;
    cpp_int const significand = rounded_quotient(numerator, denominator, &exact);
    // A zero significand has no magnitude to scale. Return the prescribed
    // sign directly, independently of the active floating-point rounding mode.
    if (significand == 0) { return {zero, false}; }
    if (exponent == limits::max_exponent - 1 && significand == (cpp_int(1) << digits))
    {
        return {negative ? -infinity : infinity, false};
    }
    // The integer fits in the significand (or is its carry power of two).
    // Both the conversion and ldexp are exact, even in the subnormal range.
    Output const magnitude = std::ldexp(static_cast<Output>(significand.convert_to<std::uint64_t>()), step);
    return {negative ? -magnitude : magnitude, exact};
}

}}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian::rational_rounding

#endif
