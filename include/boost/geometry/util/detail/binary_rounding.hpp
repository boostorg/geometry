// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_UTIL_DETAIL_BINARY_ROUNDING_HPP
#define BOOST_GEOMETRY_UTIL_DETAIL_BINARY_ROUNDING_HPP

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>

#include <boost/multiprecision/cpp_int.hpp>

namespace boost { namespace geometry { namespace detail { namespace binary_rounding
{

// Compare the remainder with its complement to avoid widening fixed integers.
template <typename Integer>
inline Integer rounded_quotient(Integer const& numerator, Integer const& denominator,
                                bool* exact = nullptr)
{
    Integer quotient, remainder;
    boost::multiprecision::divide_qr(numerator, denominator, quotient, remainder);
    if (exact) { *exact = remainder == 0; }
    Integer const complement = denominator - remainder;
    if (remainder > complement || (remainder == complement && (quotient & 1) != 0))
    {
        ++quotient;
    }
    return quotient;
}

template <typename Output>
struct rounded_value
{
    Output value;
    bool exact;
};

// The caller owns the capacity proof for fixed integers. Unbounded operands
// can have arbitrarily distant leading bits, so avoid shifting them in that case.
template <typename Output, bool Unbounded, typename Integer>
inline rounded_value<Output> round(Integer numerator, Integer denominator)
{
    using limits = std::numeric_limits<Output>;
    static_assert(limits::radix == 2 && limits::is_iec559 && limits::digits < 64,
                  "Binary rounding requires IEEE binary arithmetic");
    if (numerator == 0) { return {Output(0), true}; }
    bool const negative = numerator < 0;
    if (negative) { numerator = -numerator; }
    constexpr int digits = limits::digits;
    constexpr int minimum_step = limits::min_exponent - digits;
    Output const infinity = (limits::infinity)();
    Output const zero = negative ? -Output(0) : Output(0);

    std::intmax_t exponent = std::intmax_t(boost::multiprecision::msb(numerator))
                           - std::intmax_t(boost::multiprecision::msb(denominator));
    if constexpr (Unbounded)
    {
        if (exponent > limits::max_exponent) { return {negative ? -infinity : infinity, false}; }
        if (exponent < minimum_step - 1) { return {zero, false}; }
    }
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
    Integer const significand = rounded_quotient(numerator, denominator, &exact);
    if (significand == 0) { return {zero, false}; }
    if (exponent == limits::max_exponent - 1 && significand == (Integer(1) << digits))
        return {negative ? -infinity : infinity, false};

    // Both conversions are exact, including subnormals and a significand carry.
    Output const magnitude = std::ldexp(static_cast<Output>(significand.template convert_to<std::uint64_t>()), step);
    return {negative ? -magnitude : magnitude, exact};
}

template <typename Output>
inline rounded_value<Output> round(boost::multiprecision::cpp_rational const& value)
{
    return round<Output, true>(boost::multiprecision::numerator(value),
                               boost::multiprecision::denominator(value));
}

}}}} // namespace boost::geometry::detail::binary_rounding

#endif
