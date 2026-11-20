// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_ULP_FILTER_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_ULP_FILTER_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <type_traits>

#include <boost/geometry/strategies/cartesian/exact/arithmetic.hpp>
#include <boost/geometry/strategies/cartesian/exact/coordinate_certificate.hpp>
#include <boost/geometry/strategies/cartesian/exact/forward_error.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian { namespace ulp_filter
{

// Binary64, nearest/ties-even, gradual underflow, no contraction or reassociation.
// A declined sign requests the exact predicate. See exact_lean_n_ulp_filters.md.

using coordinates = std::array<double, 2>;
static_assert(std::numeric_limits<double>::is_iec559
    && std::numeric_limits<double>::radix == 2
    && std::numeric_limits<double>::digits == 53
    && std::numeric_limits<double>::min_exponent == -1021
    && std::numeric_limits<double>::max_exponent == 1024,
    "ULP filters require IEEE binary64");

namespace detail
{
namespace arithmetic = arithmetic_detail;
constexpr double unit_roundoff = std::numeric_limits<double>::epsilon() / 2;
constexpr double minimum_normal = (std::numeric_limits<double>::min)();

inline bool sign(double value, double bound, int& result)
{
    if (!std::isfinite(value) || !std::isfinite(bound)) return false;
    if (value > bound) { result = 1; return true; }
    if (value < -bound) { result = -1; return true; }
    return false;
}

inline std::uint64_t step_index(double x)
{
    std::uint64_t bits;
    static_assert(sizeof(bits) == sizeof(x), "Binary64 encoding required");
    std::memcpy(&bits, &x, sizeof(bits));
    constexpr std::uint64_t center = std::uint64_t(1) << 63;
    return bits & center ? center - (bits - center) : center + bits;
}

inline bool in_measure_range(coordinates const& a)
{
    return std::abs(a[0]) <= 0x1p500 && std::abs(a[1]) <= 0x1p500;
}

struct n_vector
{
    coordinates value, scale;
};

template <bool AI, bool BI>
inline n_vector n_difference(coordinates const& a, coordinates const& b)
{
    n_vector v;
    for (std::size_t i = 0; i < 2; ++i)
    {
        v.value[i] = arithmetic::rounded_sub(b[i], a[i]);
        if constexpr (AI || BI)
            v.scale[i] = arithmetic::rounded_add((std::max)(std::abs(a[i]), minimum_normal),
                                                 (std::max)(std::abs(b[i]), minimum_normal));
        else v.scale[i] = std::abs(v.value[i]);
    }
    return v;
}

template <typename Certificate, bool Dot>
inline forward_error::scalar_approximation n_bilinear(n_vector const& a, n_vector const& b)
{
    constexpr std::size_t i = Dot ? 0 : 1, j = 1 - i;
    double const x = arithmetic::rounded_mul(a.value[0], b.value[i]);
    double const y = arithmetic::rounded_mul(a.value[1], b.value[j]);
    double const scale = arithmetic::rounded_add(arithmetic::rounded_mul(a.scale[0], b.scale[i]),
                                                 arithmetic::rounded_mul(a.scale[1], b.scale[j]));
    constexpr double factor = Certificate::bilinear_units * unit_roundoff;
    return {Dot ? arithmetic::rounded_add(x, y) : arithmetic::rounded_sub(x, y),
            arithmetic::rounded_mul(factor, (std::max)(scale, minimum_normal))};
}
} // namespace detail

template <std::size_t N, bool AI, bool BI>
inline bool try_compare_n(double a, double b, int& result)
{
    if (!std::isfinite(a) || !std::isfinite(b)) return false;
    if constexpr ((AI || BI) && N == 0)
    {
        // RN is monotone, including ties. Equal heads do not prove equality.
        if (a == b) return false;
    }
    else if constexpr (AI || BI)
    {
        auto const x = detail::step_index(a), y = detail::step_index(b);
        constexpr auto limit = (unsigned(AI) + unsigned(BI)) * effective_steps<N>;
        if ((x < y ? y - x : x - y) <= limit) return false;
    }
    result = int(a > b) - int(a < b);
    return true;
}

template <typename Certificate, bool AI, bool BI, bool PI>
inline bool try_signed_determinant(coordinates const& a, coordinates const& b,
                                   coordinates const& p, forward_error::scalar_approximation& result)
{
    if (!detail::in_measure_range(a) || !detail::in_measure_range(b)
        || !detail::in_measure_range(p)) return false;
    result = detail::n_bilinear<std::conditional_t<AI || BI || PI, Certificate, exact_certificate>, false>(
        detail::n_difference<AI, BI>(a, b), detail::n_difference<AI, PI>(a, p));
    return true;
}

template <typename Certificate, bool AI, bool BI, bool PI>
inline bool try_orientation(coordinates const& a, coordinates const& b,
                            coordinates const& p, int& result)
{
    forward_error::scalar_approximation d;
    return try_signed_determinant<Certificate, AI, BI, PI>(a, b, p, d)
        && detail::sign(d.value, d.error, result);
}

template <typename Certificate, bool AI, bool BI, bool PI>
inline bool try_dot(coordinates const& a, coordinates const& b, coordinates const& p, int& result)
{
    if (!detail::in_measure_range(a) || !detail::in_measure_range(b)
        || !detail::in_measure_range(p)) return false;
    auto const d = detail::n_bilinear<std::conditional_t<AI || BI || PI, Certificate, exact_certificate>, true>(
        detail::n_difference<AI, BI>(a, b), detail::n_difference<BI, PI>(b, p));
    return detail::sign(d.value, d.error, result);
}

template <typename Certificate, bool AI, bool BI>
inline bool try_squared_distance(coordinates const& a, coordinates const& b,
                                 forward_error::scalar_approximation& result)
{
    if (!detail::in_measure_range(a) || !detail::in_measure_range(b)) return false;
    auto const d = detail::n_difference<AI, BI>(a, b);
    result = detail::n_bilinear<std::conditional_t<AI || BI, Certificate, exact_certificate>, true>(d, d);
    return true;
}

}}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian::ulp_filter

#endif
