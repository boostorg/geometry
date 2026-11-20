// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Licensed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_FORWARD_ERROR_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_FORWARD_ERROR_HPP

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#include <boost/geometry/strategies/cartesian/exact/arithmetic.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{
namespace exact_cartesian { namespace forward_error
{

// Absolute forward errors: |exact_coordinate - value[d]| <= error[d].
// Errors include any preceding native-integer conversion. No interval
// arithmetic, expression ownership, allocation, or exact fallback is hidden here.
struct point_approximation
{
    std::array<double, 2> value{};
    std::array<double, 2> error{};

    bool valid() const
    {
        return std::isfinite(value[0]) && std::isfinite(value[1])
            && std::isfinite(error[0]) && std::isfinite(error[1])
            && error[0] >= 0 && error[1] >= 0;
    }
};

struct scalar_approximation
{
    double value = 0;
    double error = 0;

    bool valid() const
    { return std::isfinite(value) && std::isfinite(error) && error >= 0; }
};

namespace detail
{
namespace arithmetic = arithmetic_detail;
constexpr double u = std::numeric_limits<double>::epsilon() / 2;
constexpr double rho = 2 * u;

constexpr bool binary64 = std::numeric_limits<double>::is_iec559
    && std::numeric_limits<double>::radix == 2
    && std::numeric_limits<double>::digits == 53;

// FP overflow/underflow (including in residual/error evaluation) is outside
// this first-stage contract. All formulas use fl(z)=z(1+delta), |delta|<=u.
// Posterior single-operation error is <=u/(1-u)*|fl(z)| < rho*|fl(z)|.
// Each fixed nonnegative error polynomial below has a path of <=16 rounded
// operations. Its computed value is >=(1-u)^16 times the real polynomial.
// The final multiplication is also rounded: (1+64u)(1-u)^17 > 1 for u=2^-53.
// Hence this STATIC inflation covers positive-bound evaluation roundoff too.
inline double inflate(double value)
{ return arithmetic::rounded_mul(1 + 64 * u, value); }

inline bool sign(scalar_approximation const& a, int& result)
{
    if (!a.valid()) return false;
    if (a.value > a.error) { result = 1; return true; }
    if (a.value < -a.error) { result = -1; return true; }
    if (a.value == 0 && a.error == 0) { result = 0; return true; }
    return false;
}

template <typename T>
struct native_coordinate : std::integral_constant<bool,
    std::is_same<T, double>::value || std::is_same<T, float>::value
    || (std::is_integral<T>::value && std::numeric_limits<T>::digits <= 64)> {};

template <typename T>
inline bool native(T value, double& approximate, double& error, std::true_type)
{
    double const converted = static_cast<double>(value);
    if (!std::isfinite(converted)) return false;
    bool const negative = std::is_signed<T>::value && value < 0;
    std::uint64_t const magnitude = negative ? std::uint64_t(0) - std::uint64_t(value)
                                             : std::uint64_t(value);
    double const absolute = std::abs(converted);
    // Guard the backconversion, including uint64_max -> double(2^64).
    bool const exact = absolute < std::ldexp(1.0, 64)
        && static_cast<std::uint64_t>(absolute) == magnitude;
    approximate = converted;
    // An integer-to-double conversion selects an adjacent binary64 value.
    // Its full adjacent-ULP error is <=2u*|converted|. This power-of-two
    // scaling is exact under the no-under/overflow contract, even if the
    // implementation selects either neighbour instead of nearest-even.
    error = exact ? 0 : arithmetic::rounded_mul(rho, absolute);
    return std::isfinite(error);
}

template <typename T>
inline bool native(T value, double& approximate, double& error, std::false_type)
{
    if (std::numeric_limits<T>::radix != 2 || std::numeric_limits<T>::digits > 53) return false;
    double const converted = static_cast<double>(value);
    if (!std::isfinite(converted)) return false;
    approximate = converted;
    error = 0;
    return true;
}

template <typename T>
inline typename std::enable_if<native_coordinate<T>::value, bool>::type
extract(T value, double& approximate, double& error)
{ return native(value, approximate, error, std::is_integral<T>()); }

template <typename T>
inline typename std::enable_if<!native_coordinate<T>::value, bool>::type
extract(T const&, double&, double&)
{ return false; }

inline bool exact_coordinates(point_approximation const& point)
{ return point.error[0] == 0 && point.error[1] == 0; }

inline std::array<double, 2> difference_values(point_approximation const& a, point_approximation const& b)
{
    return {{arithmetic::rounded_sub(a.value[0], b.value[0]),
             arithmetic::rounded_sub(a.value[1], b.value[1])}};
}

// Exact-converted endpoints give each product at most three roundings (two
// differences and one product), including repeated differences in a square.
// Its reverse relative error is <=(1-u)^-3-1 <4u. The final sum/difference
// adds <=u*M, M=|p|+|q|. Computed M >=(1-u)*M, so 8*(1-u)>5 proves that
// ONE final 8u coefficient covers the error and bound evaluation. Scaling by
// this power of two is exact under the first-stage no-under/overflow contract.
inline double native_bilinear_error(double p, double q)
{ return arithmetic::rounded_mul(8 * u, arithmetic::rounded_add(std::abs(p), std::abs(q))); }

inline scalar_approximation native_bilinear(std::array<double, 2> const& a,
                                            std::array<double, 2> const& b, bool determinant)
{
    double const p = arithmetic::rounded_mul(a[0], b[determinant ? 1 : 0]);
    double const q = arithmetic::rounded_mul(a[1], b[determinant ? 0 : 1]);
    return {determinant ? arithmetic::rounded_sub(p, q) : arithmetic::rounded_add(p, q), native_bilinear_error(p, q)};
}

// |(A-B)-fl(a-b)| <= Ea+Eb+rho*|fl(a-b)|.
inline double difference_error(double value, double ea, double eb)
{
    return inflate(arithmetic::rounded_add(arithmetic::rounded_add(ea, eb),
        arithmetic::rounded_mul(rho, std::abs(value))));
}

inline point_approximation difference(point_approximation const& a, point_approximation const& b)
{
    point_approximation result;
    for (std::size_t d = 0; d < 2; ++d)
    {
        result.value[d] = arithmetic::rounded_sub(a.value[d], b.value[d]);
        result.error[d] = difference_error(result.value[d], a.error[d], b.error[d]);
    }
    return result;
}

// Determinant and dot product share this nonnegative polynomial, with either
// (ax,by),(ay,bx) or (ax,bx),(ay,by) as the two products. Input perturbation of
// x*y is bounded by |x|Ey+|y|Ex+Ex*Ey (the second-order term is not dropped).
// For computed products p,q and their computed sum/difference s, arithmetic
// error is <=rho*(|p|+|q|+|s|), by the posterior single-operation bound above.
inline double bilinear_error(double ax, double ay, double bx, double by,
                              double eax, double eay, double ebx, double eby,
                              double p, double q, double s)
{
    double error = arithmetic::rounded_mul(rho, arithmetic::rounded_add(
        arithmetic::rounded_add(std::abs(p), std::abs(q)), std::abs(s)));
    error = arithmetic::rounded_add(error, arithmetic::rounded_mul(std::abs(ax), ebx));
    error = arithmetic::rounded_add(error, arithmetic::rounded_mul(std::abs(bx), eax));
    error = arithmetic::rounded_add(error, arithmetic::rounded_mul(eax, ebx));
    error = arithmetic::rounded_add(error, arithmetic::rounded_mul(std::abs(ay), eby));
    error = arithmetic::rounded_add(error, arithmetic::rounded_mul(std::abs(by), eay));
    error = arithmetic::rounded_add(error, arithmetic::rounded_mul(eay, eby));
    return inflate(error);
}

inline scalar_approximation determinant(point_approximation const& a, point_approximation const& b)
{
    if (exact_coordinates(a) && exact_coordinates(b)) { return native_bilinear(a.value, b.value, true); }
    double const p = arithmetic::rounded_mul(a.value[0], b.value[1]);
    double const q = arithmetic::rounded_mul(a.value[1], b.value[0]);
    double const value = arithmetic::rounded_sub(p, q);
    return {value, bilinear_error(a.value[0], a.value[1], b.value[1], b.value[0],
        a.error[0], a.error[1], b.error[1], b.error[0], p, q, value)};
}

inline scalar_approximation dot(point_approximation const& a, point_approximation const& b)
{
    if (exact_coordinates(a) && exact_coordinates(b)) { return native_bilinear(a.value, b.value, false); }
    double const p = arithmetic::rounded_mul(a.value[0], b.value[0]);
    double const q = arithmetic::rounded_mul(a.value[1], b.value[1]);
    double const value = arithmetic::rounded_add(p, q);
    return {value, bilinear_error(a.value[0], a.value[1], b.value[0], b.value[1],
        a.error[0], a.error[1], b.error[0], b.error[1], p, q, value)};
}

struct native_intersection_heads
{
    double ux, uy, numerator, denominator, mt, md;
};

inline native_intersection_heads intersection_heads(std::array<double, 8> const& p)
{
    double const ux = arithmetic::rounded_sub(p[2], p[0]);
    double const uy = arithmetic::rounded_sub(p[3], p[1]);
    double const vx = arithmetic::rounded_sub(p[6], p[4]);
    double const vy = arithmetic::rounded_sub(p[7], p[5]);
    double const wx = arithmetic::rounded_sub(p[4], p[0]);
    double const wy = arithmetic::rounded_sub(p[5], p[1]);
    double const dp = arithmetic::rounded_mul(ux, vy), dq = arithmetic::rounded_mul(uy, vx);
    double const tp = arithmetic::rounded_mul(wx, vy);
    double const tq = arithmetic::rounded_mul(wy, vx);
    double const denominator = arithmetic::rounded_sub(dp, dq);
    double const numerator = arithmetic::rounded_sub(tp, tq);
    return {ux, uy, numerator, denominator,
            arithmetic::rounded_add(std::abs(tp), std::abs(tq)),
            arithmetic::rounded_add(std::abs(dp), std::abs(dq))};
}

template <typename Coordinate>
inline bool native_intersection(std::array<Coordinate, 8> const& endpoints,
                                point_approximation* output, scalar_approximation* parameter)
{
    if (!binary64) return false;
    std::array<double, 8> p;
    bool exact = true;
    for (std::size_t i = 0; i < p.size(); ++i)
    {
        double error;
        if (!extract(endpoints[i], p[i], error)) return false;
        exact = exact && error == 0;
    }
    auto const heads = intersection_heads(p);
    double const ux = heads.ux, uy = heads.uy;
    double const numerator = heads.numerator, denominator = heads.denominator;
    double sx = std::abs(ux), sy = std::abs(uy);
    double md = heads.md, mt = heads.mt;
    if (!exact)
    {
        // Rounded-integer endpoint errors are <=2u*|p[i]|. A computed
        // difference then has error <=3u*(|endpoint1|+|endpoint2|).
        sx = arithmetic::rounded_add(std::abs(p[2]), std::abs(p[0]));
        sy = arithmetic::rounded_add(std::abs(p[3]), std::abs(p[1]));
        double const svx = arithmetic::rounded_add(std::abs(p[6]), std::abs(p[4]));
        double const svy = arithmetic::rounded_add(std::abs(p[7]), std::abs(p[5]));
        double const swx = arithmetic::rounded_add(std::abs(p[4]), std::abs(p[0]));
        double const swy = arithmetic::rounded_add(std::abs(p[5]), std::abs(p[1]));
        md = arithmetic::rounded_add(arithmetic::rounded_mul(sx, svy), arithmetic::rounded_mul(sy, svx));
        mt = arithmetic::rounded_add(arithmetic::rounded_mul(swx, svy),
            arithmetic::rounded_mul(swy, svx));
    }
    // Exact-converted native data: every product has three rounded operations
    // (two differences, one product). For M=|product1|+|product2|, determinant
    // error <=[gamma3+u*(1+u)/(1-u)]*M <5u*M. The computed M may understate by
    // (1-u), so K=8 bounds both D and T, with power-of-two error scaling exact.
    // Rounded integers: writing S=|endpoint1|+|endpoint2|, a product's error is
    // <=(7u+17u^2+u^3)*S1*S2. The final subtraction makes the determinant error
    // <9u*(S1*S2+S3*S4). Its computed magnitude has paths of <=4 operations;
    // K=16 is safe since 16*(1-u)^4>9. No generic per-operation error objects.
    double const k = exact ? 8 : 16;
    double const denominator_error = arithmetic::rounded_mul(k * u, md);
    if (!std::isfinite(denominator) || !std::isfinite(numerator)
        || !std::isfinite(md) || !std::isfinite(mt)
        || !(std::abs(denominator) > denominator_error)) return false;
    double const lower = arithmetic::rounded_mul(1 - 2 * u,
        arithmetic::rounded_sub(std::abs(denominator), denominator_error));
    if (!(lower > 0) || !std::isfinite(lower)) return false;
    double const t = arithmetic::rounded_div(numerator, denominator);
    if (!std::isfinite(t)) return false;
    double const absolute_t = std::abs(t);
    double const quotient_budget = arithmetic::rounded_add(arithmetic::rounded_add(mt,
        arithmetic::rounded_mul(absolute_t, md)), std::abs(numerator));
    if (parameter)
    {
        // |exact_t-t| <=u*(K*MT+K*|t|MD+|T|)/L. As K>=1, the
        // computed (MT+|t|MD+|T|)/L needs only the final factor 2K*u:
        // its paths have <=4 operations, and 2*(1-u)^4>1.
        scalar_approximation const result{t, arithmetic::rounded_mul(2 * k * u,
            arithmetic::rounded_div(quotient_budget, lower))};
        if (!result.valid()) return false;
        *parameter = result;
        return true;
    }
    std::array<double, 2> const scale{{sx, sy}}, direction{{ux, uy}};
    point_approximation result;
    for (std::size_t d = 0; d < 2; ++d)
    {
        double const coordinate = std::fma(t, direction[d], p[d]);
        result.value[d] = coordinate;
        double base = arithmetic::rounded_add(std::abs(coordinate), arithmetic::rounded_mul(absolute_t, scale[d]));
        if (!exact) base = arithmetic::rounded_add(base, std::abs(p[d]));
        double const budget = arithmetic::rounded_add(base,
            arithmetic::rounded_mul(arithmetic::rounded_div(scale[d], lower), quotient_budget));
        // With L<=|exact D|, |exact t-t| <=u*(K*MT+K*|t|MD+|T|)/L.
        // Endpoint error is <=0 (exact) or 2u*|a| (rounded). Difference error
        // is <=2u*S or 4u*S, respectively, and |exact U|<=(1+6u)*S.
        // One FMA contributes <=2u*|coordinate|. Therefore the real coordinate
        // error is bounded by K*u*(1+6u)*B, with
        // B=|coordinate|+|t|S+[rounded:|a|]+(S/L)*(MT+|t|MD+|T|).
        // B's evaluation paths have <=8 operations. ONE final coefficient
        // 2K*u suffices: 2*(1-u)^8 >1+6u at u=2^-53. Thus C=16 (exact-
        // converted) or C=32 (rounded integer), including bound-evaluation
        // rounding. This is not an empirically selected epsilon.
        result.error[d] = arithmetic::rounded_mul(2 * k * u, budget);
    }
    if (!result.valid()) return false;
    *output = result;
    return true;
}

} // namespace detail

// Every public API leaves output unchanged on false. A successful approximation
// is an enclosure, not a rounded exact answer; a sign API certifies only signs.
template <typename Coordinate>
inline bool try_native_coordinate(Coordinate value, double& approximate, double& error)
{
    if (!detail::binary64) return false;
    double a, e;
    if (!detail::extract(value, a, e)) return false;
    approximate = a; error = e;
    return true;
}

template <typename X, typename Y>
inline bool try_native_point(X x, Y y, point_approximation& output)
{
    if (!detail::binary64) return false;
    point_approximation result;
    if (!detail::extract(x, result.value[0], result.error[0])
        || !detail::extract(y, result.value[1], result.error[1])) return false;
    output = result;
    return true;
}

template <typename Coordinate>
inline bool try_intersection(std::array<Coordinate, 8> const& endpoints, point_approximation& output)
{
    return detail::native_intersection(endpoints, &output, nullptr);
}

template <typename Coordinate>
inline bool try_segment_parameter(std::array<Coordinate, 8> const& endpoints, scalar_approximation& output)
{
    return detail::native_intersection(endpoints, nullptr, &output);
}

inline bool try_signed_determinant(point_approximation const& a, point_approximation const& b,
                                    point_approximation const& p, scalar_approximation& output)
{
    if (!detail::binary64 || !a.valid() || !b.valid() || !p.valid()) return false;
    auto const result = detail::exact_coordinates(a) && detail::exact_coordinates(b) && detail::exact_coordinates(p)
        ? detail::native_bilinear(detail::difference_values(b, a), detail::difference_values(p, a), true)
        : detail::determinant(detail::difference(b, a), detail::difference(p, a));
    if (!result.valid()) return false;
    output = result;
    return true;
}

inline bool try_orientation(point_approximation const& a, point_approximation const& b,
                             point_approximation const& c, int& output)
{
    scalar_approximation determinant;
    return try_signed_determinant(a, b, c, determinant) && detail::sign(determinant, output);
}

// Sign of (b-a).(p-b), as used by the Cartesian direction code.
inline bool try_direction(point_approximation const& a, point_approximation const& b,
                           point_approximation const& p, int& output)
{
    if (!detail::binary64 || !a.valid() || !b.valid() || !p.valid()) return false;
    if (detail::exact_coordinates(a) && detail::exact_coordinates(b) && detail::exact_coordinates(p))
    {
        return detail::sign(detail::native_bilinear(detail::difference_values(b, a),
            detail::difference_values(p, b), false), output);
    }
    return detail::sign(detail::dot(detail::difference(b, a), detail::difference(p, b)), output);
}

inline bool try_squared_distance(point_approximation const& a, point_approximation const& b,
                                  scalar_approximation& output)
{
    if (!detail::binary64 || !a.valid() || !b.valid()) return false;
    if (detail::exact_coordinates(a) && detail::exact_coordinates(b))
    {
        auto const d = detail::difference_values(b, a);
        auto const result = detail::native_bilinear(d, d, false);
        if (!result.valid()) return false;
        output = result;
        return true;
    }
    auto const d = detail::difference(b, a);
    auto const result = detail::dot(d, d);
    if (!result.valid()) return false;
    output = result;
    return true;
}

inline bool try_compare_scalars(scalar_approximation const& a, scalar_approximation const& b, int& output)
{
    if (!detail::binary64 || !a.valid() || !b.valid()) return false;
    if (a.error == 0 && b.error == 0)
    {
        output = (a.value > b.value) - (a.value < b.value);
        return true;
    }
    double const value = detail::arithmetic::rounded_sub(a.value, b.value);
    // RN is monotone and symmetric. RN(|a-b|) > RN(ea+eb) implies
    // |a-b| > ea+eb, even when both rounded quantities round down.
    double const error = detail::arithmetic::rounded_add(a.error, b.error);
    if (!(std::abs(value) > error)) return false;
    output = value > 0 ? 1 : -1;
    return true;
}

inline bool try_compare_scalars(double a, double ea, double b, double eb, int& output)
{ return try_compare_scalars(scalar_approximation{a, ea}, scalar_approximation{b, eb}, output); }

} // namespace forward_error
} // namespace exact_cartesian
}}}} // namespace boost::geometry::strategies::detail

#endif
