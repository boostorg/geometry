// Boost.Geometry
//
// Copyright (c) 2026 Tinko Bartels, Berlin, Germany.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_ARITHMETIC_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_ARITHMETIC_HPP

#if defined(__FAST_MATH__) || defined(_M_FP_FAST) \
    || (defined(__FINITE_MATH_ONLY__) && __FINITE_MATH_ONLY__ > 0)
#error Exact Cartesian arithmetic does not support fast-math
#endif

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>

#include <boost/config.hpp>
#include <boost/core/bit.hpp>
#include <boost/math/ccmath/ldexp.hpp>
#include <boost/multiprecision/cpp_int.hpp>

#include <boost/geometry/util/detail/precise_fp.hpp>

BOOST_GEOMETRY_DETAIL_PRECISE_FP_BEGIN

namespace boost { namespace geometry { namespace strategies { namespace detail
{
namespace exact_cartesian
{

using exact_number = boost::multiprecision::cpp_rational;

namespace exponent_range
{
using limits = std::numeric_limits<double>;
constexpr int significand_bits = limits::digits;
constexpr int fraction_bits = significand_bits - 1;
constexpr int min_normal = limits::min_exponent - 1;
constexpr int max_normal = limits::max_exponent - 1;

template <int Exponent>
constexpr double power_of_two = boost::math::ccmath::ldexp(1.0, Exponent);

template <int E>
inline bool contains(double value)
{
    double const magnitude = std::abs(value);
    return value == 0 || (magnitude >= power_of_two<-E> && magnitude <= power_of_two<E>);
}

// Coordinates in [2^-E, 2^E] (or zero) are integer multiples of
// 2^-(E + fraction_bits). A product of Degree such coordinates is on
// the product lattice. Allow extra shifts at the lower end and extra
// factors from coefficients and intermediate sums at the upper end.
constexpr int polynomial_band(int degree, int lower_extra, int upper_extra)
{
    return (std::min)((-min_normal - lower_extra) / degree - fraction_bits,
                      (max_normal - upper_extra) / degree);
}
} // namespace exponent_range

inline exact_number const& exact_value(exact_number const& value) { return value; }

template <typename T>
inline typename std::enable_if<std::is_integral<T>::value, exact_number>::type
exact_value(T value)
{
    return exact_number(value);
}

// Deliberately not a general floating-point overload: long double is not part
// of this interface. Multiprecision's rational_adaptor assignment extracts the
// binary significand with frexp/ldexp and shifts integer numerator/denominator;
// it does not pass through a decimal string or an intermediate double.
template <typename T>
inline typename std::enable_if
    <std::is_same<T, float>::value || std::is_same<T, double>::value, exact_number>::type
exact_value(T value)
{
    static_assert(std::numeric_limits<T>::radix == 2, "Binary input required");
    if (!std::isfinite(value))
    {
        throw std::domain_error("exact_value requires finite float/double input");
    }
    return exact_number(value);
}

namespace arithmetic_detail
{

inline double max_adjacent_spacing(double value)
{
    double const infinity = (std::numeric_limits<double>::infinity)();
    return (std::max)(std::nextafter(value, infinity) - value,
                      value - std::nextafter(value, -infinity));
}

inline bool binary64_even(double value)
{
    return (boost::core::bit_cast<std::uint64_t>(value) & 1) == 0;
}
} // namespace arithmetic_detail

// A floating-point head and an absolute error bound.
// When valid, |exact_value - value| <= error.
struct scalar_approximation
{
    double value = 0;
    double error = 0;

    bool valid() const
    { return std::isfinite(value) && std::isfinite(error) && error >= 0; }
};

static_assert(std::numeric_limits<double>::is_iec559
              && std::numeric_limits<double>::radix == 2,
              "Certified approximations require IEC 60559 binary double");

namespace arithmetic_detail
{

template <typename T>
inline int integer_orientation(T ax, T ay, T bx, T by, T cx, T cy, std::true_type)
{
    // At most 16 input bits (including a sign bit): |difference| <= 2^16-1,
    // so each product is < 2^32, and even their difference fits in int64_t.
    std::int64_t const dx1 = std::int64_t(bx) - std::int64_t(ax);
    std::int64_t const dy1 = std::int64_t(by) - std::int64_t(ay);
    std::int64_t const dx2 = std::int64_t(cx) - std::int64_t(ax);
    std::int64_t const dy2 = std::int64_t(cy) - std::int64_t(ay);
    std::int64_t const determinant = dx1 * dy2 - dy1 * dx2;
    return (determinant > 0) - (determinant < 0);
}

#if !defined(BOOST_HAS_INT128)

template <typename T>
inline bool fits_int64(T value)
{
    return std::numeric_limits<T>::digits <= 63
        || (value <= static_cast<T>((std::numeric_limits<std::int64_t>::max)())
            && (!std::numeric_limits<T>::is_signed
                || value >= static_cast<T>((std::numeric_limits<std::int64_t>::min)())));
}

inline bool subtract_int64(std::int64_t a, std::int64_t b, std::int64_t& result)
{
    if ((b > 0 && a < (std::numeric_limits<std::int64_t>::min)() + b)
        || (b < 0 && a > (std::numeric_limits<std::int64_t>::max)() + b))
    {
        return false;
    }
    result = a - b;
    return true;
}

#endif

} // namespace arithmetic_detail

} // namespace exact_cartesian
}}}} // namespace boost::geometry::strategies::detail

BOOST_GEOMETRY_DETAIL_PRECISE_FP_END

#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_ARITHMETIC_HPP
