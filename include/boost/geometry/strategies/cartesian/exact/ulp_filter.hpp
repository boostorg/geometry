// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_ULP_FILTER_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_ULP_FILTER_HPP

#include <array>
#include <cmath>
#include <cstddef>
#include <limits>

#include <boost/geometry/strategies/cartesian/exact/coordinate_certificate.hpp>
#include <boost/geometry/strategies/cartesian/exact/forward_error.hpp>
#include <boost/geometry/util/detail/semi_static_error.hpp>

#include <boost/geometry/util/detail/precise_fp.hpp>

BOOST_GEOMETRY_DETAIL_PRECISE_FP_BEGIN

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
namespace bound_rules = geometry::detail::semi_static_error;

inline bool sign(double value, double bound, int& result)
{
    if (!std::isfinite(value) || !std::isfinite(bound)) return false;
    if (value > bound) { result = 1; return true; }
    if (value < -bound) { result = -1; return true; }
    return false;
}

inline bool in_measure_range(coordinates const& a)
{
    // For |x| <= B each difference (or leaf-magnitude sum) is <= 2B,
    // so the two products sum to at most 8B^2. The error coefficient is
    // less than one, and bilinear_bound also covers subnormal results.
    constexpr auto limit = exponent_range::power_of_two<(exponent_range::max_normal - 3) / 2>;
    return std::abs(a[0]) <= limit && std::abs(a[1]) <= limit;
}

template <bool Implicit>
struct certified_point { coordinates value; };

template <typename Certificate>
struct bilinear_ops
{
    template <bool AI, bool BI>
    static auto difference(certified_point<AI> const& a, certified_point<BI> const& b)
    {
        return linear_formula::difference(a.value, b.value, [](double x, double y)
        {
            return bound_rules::difference<BI ? Certificate::rho : 0, AI ? Certificate::rho : 0>(x, y);
        });
    }

    template <bool Dot, typename A, typename B>
    static scalar_approximation evaluate(A const& a, B const& b)
    {
        auto const p = linear_formula::bilinear_products<Dot>(a, b, [](auto const& x, auto const& y)
        { return bound_rules::product(x, y); });
        return {Dot ? p[0].head + p[1].head : p[0].head - p[1].head,
                bound_rules::bilinear_bound(p[0], p[1])};
    }

    template <typename A, typename B>
    static auto cross(A const& a, B const& b) { return evaluate<false>(a, b); }

    template <typename A, typename B>
    static auto dot(A const& a, B const& b) { return evaluate<true>(a, b); }
};
} // namespace detail

template <bool AI, bool BI>
inline bool try_compare_nearest(double a, double b, int& result)
{
    if (!std::isfinite(a) || !std::isfinite(b)) return false;
    if constexpr (AI || BI)
    {
        // RN is monotone, including ties. Equal heads do not prove equality.
        if (a == b) return false;
    }
    result = int(a > b) - int(a < b);
    return true;
}

template <typename Certificate, bool AI, bool BI, bool PI>
inline bool try_signed_determinant(coordinates const& a, coordinates const& b,
                                   coordinates const& p, scalar_approximation& result)
{
    if (!detail::in_measure_range(a) || !detail::in_measure_range(b)
        || !detail::in_measure_range(p)) return false;
    result = linear_formula::orientation::evaluate(detail::bilinear_ops<Certificate>{},
        detail::certified_point<AI>{a}, detail::certified_point<BI>{b}, detail::certified_point<PI>{p});
    return true;
}

template <typename Certificate, bool AI, bool BI, bool PI>
inline bool try_orientation(coordinates const& a, coordinates const& b,
                            coordinates const& p, int& result)
{
    scalar_approximation d;
    return try_signed_determinant<Certificate, AI, BI, PI>(a, b, p, d)
        && detail::sign(d.value, d.error, result);
}

template <typename Certificate, bool AI, bool BI, bool PI>
inline bool try_dot(coordinates const& a, coordinates const& b, coordinates const& p, int& result)
{
    if (!detail::in_measure_range(a) || !detail::in_measure_range(b)
        || !detail::in_measure_range(p)) return false;
    auto const d = linear_formula::direction(detail::bilinear_ops<Certificate>{},
        detail::certified_point<AI>{a}, detail::certified_point<BI>{b}, detail::certified_point<PI>{p});
    return detail::sign(d.value, d.error, result);
}

template <typename Certificate, bool AI, bool BI>
inline bool try_squared_distance(coordinates const& a, coordinates const& b,
                                 scalar_approximation& result)
{
    if (!detail::in_measure_range(a) || !detail::in_measure_range(b)) return false;
    result = linear_formula::squared_distance(detail::bilinear_ops<Certificate>{},
        detail::certified_point<AI>{a}, detail::certified_point<BI>{b});
    return true;
}

}}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian::ulp_filter

BOOST_GEOMETRY_DETAIL_PRECISE_FP_END

#endif
