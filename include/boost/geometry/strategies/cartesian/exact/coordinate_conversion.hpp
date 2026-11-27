// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_COORDINATE_CONVERSION_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_COORDINATE_CONVERSION_HPP

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <boost/geometry/strategies/cartesian/exact/arithmetic.hpp>
#include <boost/geometry/util/detail/binary_rounding.hpp>

#include <boost/geometry/util/detail/precise_fp.hpp>

BOOST_GEOMETRY_DETAIL_PRECISE_FP_BEGIN

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian { namespace coordinate_conversion
{

template <typename Coordinate>
inline scalar_approximation input_approximation(Coordinate value, std::false_type)
{
    double const approximate = static_cast<double>(value);
    if (!std::isfinite(approximate))
    {
        throw std::domain_error("An exact Cartesian input must be finite");
    }
    return {approximate, 0.0};
}

template <typename Coordinate>
inline scalar_approximation input_approximation(Coordinate value, std::true_type)
{
    double const approximate = static_cast<double>(value);
    // A full ULP encloses integral-to-double conversion, including INT64_MAX.
    // The inclusive cutoff is one binade below the first inexact integer.
    constexpr double limit = 1 / std::numeric_limits<double>::epsilon();
    if (std::numeric_limits<Coordinate>::digits <= std::numeric_limits<double>::digits
        || (approximate >= -limit && approximate <= limit))
    {
        return {approximate, 0.0};
    }
    return {approximate, arithmetic_detail::max_adjacent_spacing(approximate)};
}

using output_integer = boost::multiprecision::cpp_int;
using geometry::detail::binary_rounding::rounded_quotient;

template <typename Output, typename Enable = void>
struct output_rounder
{
    static Output apply(exact_number const&)
    {
        static_assert(std::is_integral<Output>::value
                      || std::is_same<Output, float>::value
                      || std::is_same<Output, double>::value,
                      "Exact Cartesian output supports integral, float and double, not long double");
        return Output();
    }
};

template <typename Output>
struct output_rounder<Output, typename std::enable_if<std::is_integral<Output>::value>::type>
{
    static Output apply(exact_number const& value)
    {
        output_integer numerator = boost::multiprecision::numerator(value);
        bool const negative = numerator < 0;
        if (negative)
        {
            numerator = -numerator;
        }
        output_integer result = rounded_quotient(numerator, boost::multiprecision::denominator(value));
        if (negative)
        {
            result = -result;
        }
        if (result < output_integer((std::numeric_limits<Output>::min)())
            || result > output_integer((std::numeric_limits<Output>::max)()))
        {
            throw std::overflow_error("Exact Cartesian integral output is out of range");
        }
        return result.template convert_to<Output>();
    }
};

template <>
struct output_rounder<exact_number, void>
{
    static exact_number const& apply(exact_number const& value) { return value; }
};

template <typename Output>
struct output_rounder<Output, typename std::enable_if<std::is_same<Output, float>::value
                                                  || std::is_same<Output, double>::value>::type>
{
    static Output apply(exact_number const& value)
    {
        return geometry::detail::binary_rounding::round<Output>(value).value;
    }
};

}}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian::coordinate_conversion

BOOST_GEOMETRY_DETAIL_PRECISE_FP_END

#endif
