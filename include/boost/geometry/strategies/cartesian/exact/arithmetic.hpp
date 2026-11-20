// Boost.Geometry
//
// Copyright (c) 2026 Tinko Bartels, Berlin, Germany.
//
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_ARITHMETIC_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_ARITHMETIC_HPP

#if defined(__FAST_MATH__)
#error Exact Cartesian arithmetic does not support fast-math
#endif

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>

#include <boost/config.hpp>
#include <boost/multiprecision/cpp_int.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{
namespace exact_cartesian
{

using exact_number = boost::multiprecision::cpp_rational;

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

inline double infinity()
{
    return (std::numeric_limits<double>::infinity)();
}

inline double downward(double value)
{
    return std::nextafter(value, -infinity());
}

inline double upward(double value)
{
    return std::nextafter(value, infinity());
}

// The supported build contract requires binary64 evaluation without excess
// precision, reassociation or implicit contraction. Explicit std::fma stays fused.
inline double rounded_add(double a, double b) { return a + b; }
inline double rounded_sub(double a, double b) { return a - b; }
inline double rounded_mul(double a, double b) { return a * b; }
inline double rounded_div(double a, double b) { return a / b; }

} // namespace arithmetic_detail

// Represents the closed real interval [approximate - error, approximate + error].
// The error is an absolute bound, not a relative error or a heuristic epsilon.
// Certification requires IEC 60559 binary double arithmetic, gradual underflow,
// correctly rounded basic operations, and no fast-math/reassociation or FTZ/DAZ.
// A single nextafter step encloses the exact result of each rounded operation,
// including underflow, in any of the standard rounding modes. Non-finite
// intermediates or invalid representations deliberately lose all certainty.
struct interval
{
    double approximate = 0;
    double error = 0;

    bool valid() const
    {
        return std::isfinite(approximate) && std::isfinite(error) && error >= 0;
    }

    double lower() const
    {
        if (!valid())
        {
            return -arithmetic_detail::infinity();
        }
        return error == 0 ? approximate
                         : arithmetic_detail::downward(arithmetic_detail::rounded_sub(approximate, error));
    }

    double upper() const
    {
        if (!valid())
        {
            return arithmetic_detail::infinity();
        }
        return error == 0 ? approximate
                         : arithmetic_detail::upward(arithmetic_detail::rounded_add(approximate, error));
    }

    static interval uncertain()
    {
        return {0, arithmetic_detail::infinity()};
    }
};

static_assert(std::numeric_limits<double>::is_iec559
              && std::numeric_limits<double>::radix == 2,
              "Certified intervals require IEC 60559 binary double");

namespace arithmetic_detail
{

inline interval enclose(double approximate, double lower, double upper)
{
    if (!std::isfinite(approximate) || !std::isfinite(lower) || !std::isfinite(upper))
    {
        return interval::uncertain();
    }

    // Each subtraction is rounded; one outward step bounds the exact distance
    // from the chosen center to either endpoint. The extra width is intentional.
    double const error = (std::max)(upward(rounded_sub(approximate, lower)),
                                   upward(rounded_sub(upper, approximate)));
    return std::isfinite(error) ? interval{approximate, error}
                                : interval::uncertain();
}

inline bool finite_bounds(double al, double au, double bl, double bu)
{
    return std::isfinite(al) && std::isfinite(au)
        && std::isfinite(bl) && std::isfinite(bu);
}

// For multiplication/division, extrema occur at corners of the input rectangle
// (division only when its denominator interval excludes zero). Round the corner
// extrema outwards before converting them to a center and absolute error.
inline interval enclose_corners(double approximate, double p, double q, double r, double s)
{
    if (!finite_bounds(p, q, r, s))
    {
        return interval::uncertain();
    }
    return enclose(approximate,
                   downward((std::min)((std::min)(p, q), (std::min)(r, s))),
                   upward((std::max)((std::max)(p, q), (std::max)(r, s))));
}

} // namespace arithmetic_detail

inline interval operator+(interval const& a, interval const& b)
{
    double const al = a.lower(), au = a.upper(), bl = b.lower(), bu = b.upper();
    if (!arithmetic_detail::finite_bounds(al, au, bl, bu))
    {
        return interval::uncertain();
    }
    return arithmetic_detail::enclose(arithmetic_detail::rounded_add(a.approximate, b.approximate),
                                     arithmetic_detail::downward(arithmetic_detail::rounded_add(al, bl)),
                                     arithmetic_detail::upward(arithmetic_detail::rounded_add(au, bu)));
}

inline interval operator-(interval const& a, interval const& b)
{
    double const al = a.lower(), au = a.upper(), bl = b.lower(), bu = b.upper();
    if (!arithmetic_detail::finite_bounds(al, au, bl, bu))
    {
        return interval::uncertain();
    }
    return arithmetic_detail::enclose(arithmetic_detail::rounded_sub(a.approximate, b.approximate),
                                     arithmetic_detail::downward(arithmetic_detail::rounded_sub(al, bu)),
                                     arithmetic_detail::upward(arithmetic_detail::rounded_sub(au, bl)));
}

inline interval operator*(interval const& a, interval const& b)
{
    double const al = a.lower(), au = a.upper(), bl = b.lower(), bu = b.upper();
    if (!arithmetic_detail::finite_bounds(al, au, bl, bu))
    {
        return interval::uncertain();
    }
    return arithmetic_detail::enclose_corners(arithmetic_detail::rounded_mul(a.approximate, b.approximate),
        arithmetic_detail::rounded_mul(al, bl), arithmetic_detail::rounded_mul(al, bu),
        arithmetic_detail::rounded_mul(au, bl), arithmetic_detail::rounded_mul(au, bu));
}

inline interval operator/(interval const& a, interval const& b)
{
    double const al = a.lower(), au = a.upper(), bl = b.lower(), bu = b.upper();
    if (!arithmetic_detail::finite_bounds(al, au, bl, bu) || (bl <= 0 && bu >= 0))
    {
        return interval::uncertain();
    }
    return arithmetic_detail::enclose_corners(arithmetic_detail::rounded_div(a.approximate, b.approximate),
        arithmetic_detail::rounded_div(al, bl), arithmetic_detail::rounded_div(al, bu),
        arithmetic_detail::rounded_div(au, bl), arithmetic_detail::rounded_div(au, bu));
}

// value is -1, 0, or +1, but is meaningful ONLY when certain is true.
// In particular, {0, false} means "unknown", not "exactly zero".
struct filter_result
{
    int value;
    bool certain;
};

inline filter_result sign(interval const& value)
{
    if (value.lower() > 0)
    {
        return {1, true};
    }
    if (value.upper() < 0)
    {
        return {-1, true};
    }
    if (value.valid() && value.error == 0 && value.approximate == 0)
    {
        return {0, true};
    }
    return {0, false};
}

inline filter_result compare(interval const& a, interval const& b)
{
    if (a.upper() < b.lower())
    {
        return {-1, true};
    }
    if (a.lower() > b.upper())
    {
        return {1, true};
    }
    if (a.valid() && b.valid() && a.error == 0 && b.error == 0
        && a.approximate == b.approximate)
    {
        return {0, true};
    }
    return {0, false};
}

namespace arithmetic_detail
{

template <typename T>
inline int limb_orientation(T ax, T ay, T bx, T by, T cx, T cy)
{
    using boost::multiprecision::cpp_int;
    cpp_int const dx1 = cpp_int(bx) - cpp_int(ax);
    cpp_int const dy1 = cpp_int(by) - cpp_int(ay);
    cpp_int const dx2 = cpp_int(cx) - cpp_int(ax);
    cpp_int const dy2 = cpp_int(cy) - cpp_int(ay);
    cpp_int const left = dx1 * dy2;
    cpp_int const right = dy1 * dx2;
    return (left > right) - (left < right);
}

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

#if defined(BOOST_HAS_INT128)

template <typename T>
inline int wide_orientation(T ax, T ay, T bx, T by, T cx, T cy, std::true_type)
{
    // All signed/unsigned <=64-bit coordinates fit BEFORE subtraction. The
    // differences need at most 65 signed bits. Bound them before multiplication:
    // (2^63-1)^2 < 2^126, so products and their comparison cannot overflow.
    using wide = boost::int128_type;
    wide const dx1 = wide(bx) - wide(ax), dy1 = wide(by) - wide(ay);
    wide const dx2 = wide(cx) - wide(ax), dy2 = wide(cy) - wide(ay);
    wide const bound = (std::numeric_limits<std::int64_t>::max)();
    if (dx1 >= -bound && dx1 <= bound && dy1 >= -bound && dy1 <= bound
        && dx2 >= -bound && dx2 <= bound && dy2 >= -bound && dy2 <= bound)
    {
        wide const left = dx1 * dy2, right = dy1 * dx2;
        return (left > right) - (left < right);
    }
    return limb_orientation(ax, ay, bx, by, cx, cy);
}

template <typename T>
inline int wide_orientation(T ax, T ay, T bx, T by, T cx, T cy, std::false_type)
{
    return limb_orientation(ax, ay, bx, by, cx, cy);
}

#else

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

template <typename T>
inline int wide_orientation(T ax, T ay, T bx, T by, T cx, T cy)
{
    if (fits_int64(ax) && fits_int64(ay) && fits_int64(bx)
        && fits_int64(by) && fits_int64(cx) && fits_int64(cy))
    {
        std::int64_t dx1, dy1, dx2, dy2;
        std::int64_t const bound = 2147483647;
        // Checked subtraction, then |difference| <= 2^31-1 proves that each
        // product fits in int64_t. Compare products without subtracting them.
        if (subtract_int64(std::int64_t(bx), std::int64_t(ax), dx1)
            && subtract_int64(std::int64_t(by), std::int64_t(ay), dy1)
            && subtract_int64(std::int64_t(cx), std::int64_t(ax), dx2)
            && subtract_int64(std::int64_t(cy), std::int64_t(ay), dy2)
            && dx1 >= -bound && dx1 <= bound && dy1 >= -bound && dy1 <= bound
            && dx2 >= -bound && dx2 <= bound && dy2 >= -bound && dy2 <= bound)
        {
            std::int64_t const left = dx1 * dy2, right = dy1 * dx2;
            return (left > right) - (left < right);
        }
    }
    return limb_orientation(ax, ay, bx, by, cx, cy);
}

#endif

template <typename T>
inline int integer_orientation(T ax, T ay, T bx, T by, T cx, T cy, std::false_type)
{
#if defined(BOOST_HAS_INT128)
    using fits_wide = std::integral_constant<bool, (std::numeric_limits<T>::digits <= 64)>;
    return wide_orientation(ax, ay, bx, by, cx, cy, fits_wide{});
#else
    return wide_orientation(ax, ay, bx, by, cx, cy);
#endif
}

} // namespace arithmetic_detail

// Exact sign of (bx-ax)*(cy-ay) - (by-ay)*(cx-ax), including uint64_t input.
// No input subtraction or conversion through floating point is performed.
template <typename T>
inline typename std::enable_if<std::is_integral<T>::value, int>::type
integer_orientation(T ax, T ay, T bx, T by, T cx, T cy)
{
    using small = std::integral_constant<bool,
        (std::numeric_limits<T>::digits + std::numeric_limits<T>::is_signed <= 16)>;
    return arithmetic_detail::integer_orientation(ax, ay, bx, by, cx, cy, small{});
}

} // namespace exact_cartesian
}}}} // namespace boost::geometry::strategies::detail

#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_ARITHMETIC_HPP
