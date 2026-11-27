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

#include <boost/geometry/strategies/cartesian/detail/linear_formula.hpp>
#include <boost/geometry/strategies/cartesian/exact/arithmetic.hpp>

#include <boost/geometry/util/detail/precise_fp.hpp>

BOOST_GEOMETRY_DETAIL_PRECISE_FP_BEGIN

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

namespace detail
{
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
{ return (1 + 64 * u) * value; }

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
    || (std::is_integral<T>::value
        && std::numeric_limits<T>::digits <= std::numeric_limits<std::uint64_t>::digits)> {};

template <typename T>
inline bool native(T value, double& approximate, double& error, std::true_type)
{
    double const converted = static_cast<double>(value);
    if (!std::isfinite(converted)) return false;
    bool const negative = std::is_signed<T>::value && value < 0;
    std::uint64_t const magnitude = negative ? std::uint64_t(0) - std::uint64_t(value)
                                             : std::uint64_t(value);
    double const absolute = std::abs(converted);
    // The maximum uint64_t can round to the first out-of-range double.
    constexpr auto bits = std::numeric_limits<std::uint64_t>::digits;
    constexpr double limit = 2.0 * (std::uint64_t{1} << (bits - 1));
    bool const exact = absolute < limit
        && static_cast<std::uint64_t>(absolute) == magnitude;
    approximate = converted;
    // An integer-to-double conversion selects an adjacent binary64 value.
    // Its full adjacent-ULP error is <=2u*|converted|. This power-of-two
    // scaling is exact under the no-under/overflow contract, even if the
    // implementation selects either neighbour instead of nearest-even.
    error = exact ? 0 : (rho * absolute);
    return std::isfinite(error);
}

template <typename T>
inline bool native(T value, double& approximate, double& error, std::false_type)
{
    if (std::numeric_limits<T>::radix != 2
        || std::numeric_limits<T>::digits > std::numeric_limits<double>::digits) return false;
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

// Exact-converted inputs need only a product-magnitude bound. The 8u
// coefficient encloses three product roundings and the final sum/difference,
// including rounding of the magnitude itself (no underflow on this path).
constexpr double native_determinant_units = 8;
using enclosed_point = linear_formula::xy<scalar_approximation>;

inline enclosed_point ordinates(point_approximation const& p)
{ return {{{p.value[0], p.error[0]}, {p.value[1], p.error[1]}}}; }

struct enclosed_bilinear_ops
{
    struct delta
    {
        double value, ea, eb;
        bool exact() const { return ea == 0 && eb == 0; }
        double error() const
        {
            return inflate((ea + eb) + rho * std::abs(value));
        }
    };

    using vector = linear_formula::xy<delta>;

    static vector difference(enclosed_point const& a, enclosed_point const& b)
    {
        return linear_formula::difference(a, b, [](auto const& x, auto const& y)
        { return delta{x.value - y.value, x.error, y.error}; });
    }

    template <bool Dot>
    static scalar_approximation evaluate(vector const& a, vector const& b)
    {
        constexpr std::size_t i = Dot ? 0 : 1;
        auto const& w = b[i];
        auto const& z = b[1 - i];
        double const p = a[0].value * w.value;
        double const q = a[1].value * z.value;
        double const value = Dot ? p + q : p - q;
        double const magnitude = std::abs(p) + std::abs(q);
        if (a[0].exact() && a[1].exact() && w.exact() && z.exact())
            return {value, (native_determinant_units * u) * magnitude};

        // Posterior error of the two products and the final operation.
        double error = rho * (magnitude + std::abs(value));
        auto add_perturbation = [&](delta const& x, delta const& y)
        {
            // |(x+Ex)(y+Ey)-xy| <= |x|Ey+|y|Ex+ExEy.
            double const ex = x.error(), ey = y.error();
            error += std::abs(x.value) * ey;
            error += std::abs(y.value) * ex;
            error += ex * ey;
        };
        add_perturbation(a[0], w);
        add_perturbation(a[1], z);
        return {value, inflate(error)};
    }

    static auto cross(vector const& a, vector const& b) { return evaluate<false>(a, b); }

    static auto dot(vector const& a, vector const& b) { return evaluate<true>(a, b); }
};

struct native_intersection_heads
{
    double ux, uy, numerator, denominator, mt, md;
};

inline double denominator_lower_bound(double magnitude, double error)
{
    // (1-2u)*(1+u)^2 < 1 covers rounding both operations.
    return (1 - 2 * u) * (magnitude - error);
}

struct rounded_planar_ops
{
    using point = linear_formula::xy<double>;
    static double subtract(double a, double b) { return a - b; }
    static double product_difference(double a, double b, double c, double d)
    {
        return a * b - c * d;
    }
    static point difference(point const& a, point const& b)
    { return linear_formula::difference(a, b, subtract); }
};

struct intersection_ops : rounded_planar_ops
{
    struct determinant_head { double value, magnitude; };
    static determinant_head cross(linear_formula::xy<double> const& a,
                                  linear_formula::xy<double> const& b)
    {
        auto const products = linear_formula::bilinear_products(a, b, [](double x, double y) { return x * y; });
        return {products[0] - products[1],
                std::abs(products[0]) + std::abs(products[1])};
    }
};

// For rounded integer inputs, lift the same recipe to magnitude space:
// bound each endpoint difference by the sum of endpoint magnitudes.
struct intersection_magnitude_ops
{
    using point = linear_formula::xy<double>;

    static point difference(point const& a, point const& b)
    {
        return linear_formula::difference(a, b, [](double x, double y)
        { return std::abs(x) + std::abs(y); });
    }

    static double cross(point const& a, point const& b)
    {
        auto const terms = linear_formula::bilinear_products(a, b, [](double x, double y) { return x * y; });
        return terms[0] + terms[1];
    }
};

template <typename Ops>
inline auto intersection_terms(Ops const& ops, std::array<double, 8> const& p)
{
    using point = linear_formula::xy<double>;
    return linear_formula::affine_intersection(ops,
        point{{p[0], p[1]}}, point{{p[2], p[3]}}, point{{p[4], p[5]}}, point{{p[6], p[7]}});
}

inline auto intersection_candidate(std::array<double, 8> const& p,
                                   native_intersection_heads const& heads, double parameter)
{
    return linear_formula::affine_point(linear_formula::xy<double>{{p[0], p[1]}}, parameter,
        linear_formula::xy<double>{{heads.ux, heads.uy}},
        [](double t, double u, double a) { return std::fma(t, u, a); });
}

inline native_intersection_heads intersection_heads(std::array<double, 8> const& p)
{
    auto const terms = intersection_terms(intersection_ops{}, p);
    return {terms.direction[0], terms.direction[1], terms.numerator.value, terms.denominator.value,
            terms.numerator.magnitude, terms.denominator.magnitude};
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
        auto const bounds = intersection_terms(intersection_magnitude_ops{}, p);
        sx = bounds.direction[0];
        sy = bounds.direction[1];
        md = bounds.denominator;
        mt = bounds.numerator;
    }
    // Exact-converted native data: every product has three rounded operations
    // (two differences, one product). For M=|product1|+|product2|, determinant
    // error <=[gamma3+u*(1+u)/(1-u)]*M <5u*M. The computed M may understate by
    // (1-u), so K=8 bounds both D and T, with power-of-two error scaling exact.
    // Rounded integers: writing S=|endpoint1|+|endpoint2|, a product's error is
    // <=(7u+17u^2+u^3)*S1*S2. The final subtraction makes the determinant error
    // <9u*(S1*S2+S3*S4). Its computed magnitude has paths of <=4 operations;
    // K=16 is safe since 16*(1-u)^4>9. No generic per-operation error objects.
    double const k = exact ? native_determinant_units : 16;
    double const denominator_error = k * u * md;
    if (!std::isfinite(denominator) || !std::isfinite(numerator)
        || !std::isfinite(md) || !std::isfinite(mt)
        || !(std::abs(denominator) > denominator_error)) return false;
    double const lower = denominator_lower_bound(std::abs(denominator), denominator_error);
    if (!(lower > 0) || !std::isfinite(lower)) return false;
    double const t = numerator / denominator;
    if (!std::isfinite(t)) return false;
    double const absolute_t = std::abs(t);
    double const quotient_budget = mt + absolute_t * md + std::abs(numerator);
    if (parameter)
    {
        // |exact_t-t| <=u*(K*MT+K*|t|MD+|T|)/L. As K>=1, the
        // computed (MT+|t|MD+|T|)/L needs only the final factor 2K*u:
        // its paths have <=4 operations, and 2*(1-u)^4>1.
        scalar_approximation const result{t, (2 * k * u) * (quotient_budget / lower)};
        if (!result.valid()) return false;
        *parameter = result;
        return true;
    }
    std::array<double, 2> const scale{{sx, sy}};
    point_approximation result;
    result.value = intersection_candidate(p, heads, t);
    for (std::size_t d = 0; d < 2; ++d)
    {
        double const coordinate = result.value[d];
        double base = std::abs(coordinate) + absolute_t * scale[d];
        if (!exact) base += std::abs(p[d]);
        double const budget = base + (scale[d] / lower) * quotient_budget;
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
        result.error[d] = (2 * k * u) * budget;
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
    auto const result = linear_formula::orientation::evaluate(detail::enclosed_bilinear_ops{},
        detail::ordinates(a), detail::ordinates(b), detail::ordinates(p));
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
    return detail::sign(linear_formula::direction(detail::enclosed_bilinear_ops{},
        detail::ordinates(a), detail::ordinates(b), detail::ordinates(p)), output);
}

inline bool try_squared_distance(point_approximation const& a, point_approximation const& b,
                                  scalar_approximation& output)
{
    if (!detail::binary64 || !a.valid() || !b.valid()) return false;
    auto const result = linear_formula::squared_distance(detail::enclosed_bilinear_ops{},
        detail::ordinates(a), detail::ordinates(b));
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
    double const value = a.value - b.value;
    // RN is monotone and symmetric. RN(|a-b|) > RN(ea+eb) implies
    // |a-b| > ea+eb, even when both rounded quantities round down.
    double const error = a.error + b.error;
    if (!(std::abs(value) > error)) return false;
    output = value > 0 ? 1 : -1;
    return true;
}

} // namespace forward_error
} // namespace exact_cartesian
}}}} // namespace boost::geometry::strategies::detail

BOOST_GEOMETRY_DETAIL_PRECISE_FP_END

#endif
