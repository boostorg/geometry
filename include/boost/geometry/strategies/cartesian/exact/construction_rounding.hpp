// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_CONSTRUCTION_ROUNDING_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_CONSTRUCTION_ROUNDING_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <type_traits>

#include <boost/config.hpp>
#include <boost/geometry/strategies/cartesian/exact/coordinate_certificate.hpp>
#include <boost/geometry/strategies/cartesian/exact/forward_error.hpp>
#include <boost/geometry/strategies/cartesian/exact/output_policy.hpp>
#include <boost/geometry/strategies/cartesian/exact/point_cache.hpp>
#include <boost/geometry/util/numeric_cast.hpp>
#include <boost/geometry/util/error_free_transforms.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian { namespace construction_rounding
{

// C++17, IEEE binary64, nearest/ties-even, -ffp-contract=off, no fast-math.
// FP overflow/underflow in first-stage arithmetic is outside the contract.
// Producers are {ax,ay,bx,by,cx,cy,dx,dy}, supplied anew by the owner on every
// arithmetic call. A state never retains coordinates, pointers or callbacks.
// Copies and their plain cache indices belong to the enclosing operation;
// they must not outlive its point_cache_scope or be reused with other producers.
template <typename Policy> struct policy_traits;

template <> struct policy_traits<relate::approximate_output>
{
    using arithmetic_certificate = void;
    static constexpr bool store_errors = true;
    static constexpr bool nearest_output = false;
};

template <> struct policy_traits<relate::nearest_output>
{
    using arithmetic_certificate = ulp_filter::nearest_certificate;
    static constexpr std::size_t order_steps = 0;
    static constexpr bool store_errors = false;
    static constexpr bool nearest_output = true;
};

template <std::size_t N>
struct policy_traits<relate::bounded_ulp_output<N>>
{
    using arithmetic_certificate = ulp_filter::producer_certificate<N>;
    static constexpr std::size_t order_steps = N;
    static constexpr bool nearest_output = false;
    static constexpr bool store_errors = false;
};

namespace detail
{
namespace arithmetic = arithmetic_detail;
namespace fe = forward_error;

inline bool even(double value)
{
    std::uint64_t bits;
    static_assert(sizeof(bits) == sizeof(value), "Binary64 required");
    std::memcpy(&bits, &value, sizeof(bits));
    return (bits & 1) == 0;
}

inline double spacing(double value)
{
    double const infinity = (std::numeric_limits<double>::infinity)();
    return (std::max)(arithmetic::rounded_sub(std::nextafter(value, infinity), value),
                      arithmetic::rounded_sub(value, std::nextafter(value, -infinity)));
}

// Symmetric forward bounds must fit BOTH adjacent spacings. At a binade they
// differ by two, so epsilon*abs(value) is not an adequate certificate. The
// spacings (and normal half-spacings) are exact powers of two. Nearest cells
// include both midpoint endpoints iff the retained significand is even.
inline bool certify_nearest(fe::scalar_approximation const& a)
{
    if (!a.valid()) return false;
    if (a.error == 0) return true;
    if (a.value == 0) return false;
    double const infinity = (std::numeric_limits<double>::infinity)();
    double const previous = std::nextafter(a.value, -infinity);
    double const next = std::nextafter(a.value, infinity);
    if (!std::isfinite(previous) || !std::isfinite(next)) return false;
    double const spacing = (std::min)(arithmetic::rounded_sub(a.value, previous),
                                      arithmetic::rounded_sub(next, a.value));
    double const half = arithmetic::rounded_mul(0.5, spacing);
    return a.error < half || (a.error == half && even(a.value));
}

// Use a stricter certificate for very large policies. For K <= 2^24,
// Bernoulli gives (1-2u)^(K+1)*(1-u)^4/(1+u) >= 1-(2K+7)u.
// The parenthesized dyadic below is exact; the extra slack covers rounding
// its multiplication by K. No runtime power, division or spacing lookup.
template <std::size_t N>
constexpr double bounded_threshold_factor()
{
    constexpr auto k = ulp_filter::effective_steps<N>;
    return double(k) * (1 - (2 * double(k) + 16) * fe::detail::u);
}

inline double scaled_division_residual(double t, double d, double n)
{
    // For n != 0, ilogb(t)+ilogb(d) >= ilogb(n)-1, so the grid of t*d
    // is no finer than 2^(ilogb(n)-105).
    // The residual lies on that grid and |n-t*d| <= u*|n|, so it fits
    // in 53 bits. The admitted exponent band also covers the scaling.
    return std::abs(std::fma(-t, d, n)) * 0x1p53;
}

inline bool bounded_axis_passes(double direction, double z, double height, double threshold)
{
    return arithmetic::rounded_mul(std::abs(direction), height)
         < arithmetic::rounded_mul(std::abs(z), threshold);
}

// Budget in units of u for the ORIGINAL head k = RN(p-q). Requires all
// four endpoint differences to be exact. The three EFT tails ep, eq, ek
// give det-k = ek+ep-eq exactly. For s=RN(ep-eq), r=RN(ek+s), the
// posterior rounding bound gives |det-k| <= (|r|+u*|s|)/(1-u).
// The factor 2 covers this denominator and the rounded nonnegative sum:
// 2*(1-u)^2 > 1.
inline double determinant_budget(double ax, double ay, double bx, double by, double k)
{
    using geometry::detail::precise_math::two_diff_tail;
    using geometry::detail::precise_math::two_product_tail;
    double const p = arithmetic::rounded_mul(ax, by), q = arithmetic::rounded_mul(ay, bx);
    double const s = arithmetic::rounded_sub(two_product_tail(ax, by, p), two_product_tail(ay, bx, q));
    double const r = arithmetic::rounded_add(two_diff_tail(p, q, k), s);
    return 2 * std::fma(0x1p53, std::abs(r), std::abs(s));
}

// Tighten the ORIGINAL head, not a compensated replacement. The signed
// linear terms may cancel each other or the exact product/subtraction tails.
// Let M be RN(|p|+|q|). In the admitted exponent band, the sums of the exact
// and rounded product magnitudes are each <= 2*M. The three product/head
// tails total <= 4*u*M and the four linear terms total <= 5*u*M. Therefore
// for M > 0 each of the six rounded intermediates has magnitude < 16*u*M
// (even after accumulating earlier rounding errors). The two omitted
// quadratic terms total <= 3*u*u*M because |tail| <= u*|head|/(1-u).
// 128*u*M covers those errors in units of u; (1+4*u)*(1-u)^2 > 1
// covers evaluation of the final nonnegative bound. All terms stay on
// the existing determinant grid before exact scaling by 1/u.
inline double cross_term_budget(double ax, double ay, double bx, double by, double k,
                                double axt, double ayt, double bxt, double byt, double magnitude)
{
    using geometry::detail::precise_math::two_diff_tail;
    using geometry::detail::precise_math::two_product_tail;
    constexpr double u = fe::detail::u;
    double const p = arithmetic::rounded_mul(ax, by), q = arithmetic::rounded_mul(ay, bx);
    double r = arithmetic::rounded_add(two_diff_tail(p, q, k),
        arithmetic::rounded_sub(two_product_tail(ax, by, p), two_product_tail(ay, bx, q)));
    r = std::fma(ax, byt, r);
    r = std::fma(axt, by, r);
    r = std::fma(-ay, bxt, r);
    r = std::fma(-ayt, bx, r);
    return (1 + 4 * u) * std::fma(128 * u, magnitude, 0x1p53 * std::abs(r));
}

// Keep the original quotient, candidate and certified lower bound. This
// slow path is reached only if removing the determinant and quotient errors
// could help at least one axis. Keep it out of the initial filter.
BOOST_NOINLINE inline double retry_determinants(std::array<double, 8> const& p,
    fe::detail::native_intersection_heads const& heads, double t, double lower, double threshold,
    std::array<double, 2> const& z, std::array<bool, 2> const& accepted)
{
    double const at = std::abs(t);
    double const residual = scaled_division_residual(t, heads.denominator, heads.numerator);
    auto height = [&](double et, double ed)
    { return std::fma(at, lower, arithmetic::rounded_add(residual, std::fma(at, ed, et))); };
    double et = 8 * heads.mt, ed = 8 * heads.md;
    double h = height(et, ed);
    auto can_improve = [&](double optimistic)
    {
        auto axis = [&](std::size_t i, double dir)
        {
            return !accepted[i] && bounded_axis_passes(dir, z[i], optimistic, threshold)
                && !bounded_axis_passes(dir, z[i], h, threshold);
        };
        return axis(0, heads.ux) || axis(1, heads.uy);
    };
    // With t and L fixed, the three nonnegative rounded operations in
    // height are monotone in each budget. If even perfect determinants
    // cannot rescue another axis, don't recover any tails.
    if (!can_improve(height(0, 0))) return h;

    using geometry::detail::precise_math::two_diff_tail;
    double const vx = arithmetic::rounded_sub(p[6], p[4]), vy = arithmetic::rounded_sub(p[7], p[5]);
    double const vxt = two_diff_tail(p[6], p[4], vx);
    double const vyt = two_diff_tail(p[7], p[5], vy);
    auto tighten = [&](bool denominator)
    {
        std::size_t const i = denominator ? 2 : 4;
        double const x = denominator ? heads.ux : arithmetic::rounded_sub(p[4], p[0]);
        double const y = denominator ? heads.uy : arithmetic::rounded_sub(p[5], p[1]);
        double const xt = two_diff_tail(p[i], p[0], x);
        double const yt = two_diff_tail(p[i + 1], p[1], y);
        double const k = denominator ? heads.denominator : heads.numerator;
        double const tightened = xt == 0 && yt == 0 && vxt == 0 && vyt == 0
            ? determinant_budget(x, y, vx, vy, k)
            : cross_term_budget(x, y, vx, vy, k, xt, yt, vxt, vyt,
                                denominator ? heads.md : heads.mt);
        auto& budget = denominator ? ed : et;
        budget = (std::min)(budget, tightened);
        h = height(et, ed);
    };
    bool const denominator_first = at * ed > et;
    tighten(denominator_first);
    // Don't tighten the other determinant unless erasing its entire
    // contribution would certify an axis which still fails.
    if (can_improve(denominator_first ? height(0, ed) : height(et, 0)))
        tighten(!denominator_first);
    return h;
}

// Enter only in bounded_intersection's exponent band. Exact endpoint
// differences permit three error-free tails for the rebuilt determinant.
inline fe::scalar_approximation rebuild_determinant(double ax, double ay, double bx, double by,
                                                    double original)
{
    using geometry::detail::precise_math::two_diff_tail;
    double const p = arithmetic::rounded_mul(ax, by), q = arithmetic::rounded_mul(ay, bx);
    double const ep = std::fma(ax, by, -p), eq = std::fma(ay, bx, -q);
    double const r = arithmetic::rounded_sub(ep, eq);
    double const a = std::abs(two_diff_tail(p, q, original));
    double const b = std::abs(two_diff_tail(ep, eq, r));
    auto const sum = geometry::detail::precise_math::two_sum(original, r);
    return {sum[0], 2 * arithmetic::rounded_add(arithmetic::rounded_add(a, b), std::abs(sum[1]))};
}

template <std::size_t N>
BOOST_NOINLINE std::array<bool, 2> rescue_bounded_denominator(std::array<double, 8> const& p,
    fe::detail::native_intersection_heads const& heads, std::array<double, 2>& output)
{
    using geometry::detail::precise_math::two_diff_tail;
    constexpr double u = fe::detail::u;
    constexpr double c = bounded_threshold_factor<N>();
    std::array<bool, 2> accepted{};
    double const vx = arithmetic::rounded_sub(p[6], p[4]), vy = arithmetic::rounded_sub(p[7], p[5]);
    if (two_diff_tail(p[6], p[4], vx) != 0 || two_diff_tail(p[7], p[5], vy) != 0
        || two_diff_tail(p[2], p[0], heads.ux) != 0 || two_diff_tail(p[3], p[1], heads.uy) != 0)
        return accepted;
    auto const d = rebuild_determinant(heads.ux, heads.uy, vx, vy, heads.denominator);
    if (!(std::abs(d.value) > d.error)) return accepted;
    double const lower = arithmetic::rounded_mul(1 - 2 * u,
        arithmetic::rounded_sub(std::abs(d.value), d.error));
    fe::scalar_approximation n{heads.numerator, 8 * u * heads.mt};
    double const wx = arithmetic::rounded_sub(p[4], p[0]), wy = arithmetic::rounded_sub(p[5], p[1]);
    if (two_diff_tail(p[4], p[0], wx) == 0 && two_diff_tail(p[5], p[1], wy) == 0)
    {
        auto const rebuilt = rebuild_determinant(wx, wy, vx, vy, n.value);
        if (rebuilt.error <= n.error) n = rebuilt;
    }
    // New denominator and possibly new numerator: recompute everything
    // dependent on them, including the division residual and both candidates.
    double const t = arithmetic::rounded_div(n.value, d.value), at = std::abs(t);
    double const residual = scaled_division_residual(t, d.value, n.value);
    double const budget = std::fma(at, d.error * 0x1p53, n.error * 0x1p53);
    double const height = std::fma(at, lower, arithmetic::rounded_add(residual, budget));
    double const threshold = arithmetic::rounded_mul(c, lower);
    std::array<double, 2> const direction{{heads.ux, heads.uy}};
    for (std::size_t axis = 0; axis != 2; ++axis)
    {
        double const z = std::fma(t, direction[axis], p[axis]);
        if (!bounded_axis_passes(direction[axis], z, height, threshold)) continue;
        output[axis] = z;
        accepted[axis] = true;
    }
    return accepted;
}

template <std::size_t N>
inline std::array<bool, 2> bounded_intersection(std::array<double, 8> const& p,
                                                std::array<double, 2>& output)
{
    static_assert(N >= 1, "A rounding cell must intervene");
    constexpr double u = fe::detail::u;
    constexpr double c = bounded_threshold_factor<N>();
    // In this band differences are multiples of 2^-116 and <= 2^65.
    // Determinants and magnitudes are multiples of 2^-232 and <= 2^131,
    // so nonzero |t| is in [2^-363, 2^363], and positive L >= 2^-283.
    // Even the FMA residual and comparisons stay within normal range:
    // t's grid is >= 2^-415; t*h and t*d use grids >= 2^-531 and 2^-647.
    // The smallest nonzero comparison product exceeds 2^-820, the
    // largest is below 2^600. The intervening cells are normal and finite.
    for (double x : p)
        if (x != 0 && !(std::abs(x) >= 0x1p-64 && std::abs(x) <= 0x1p64))
            return {{false, false}};

    auto const heads = fe::detail::intersection_heads(p);
    // Same 8u determinant bounds as forward_error::detail::native_intersection.
    // (1-2u)*(1+u)^2 < 1 accounts for rounding both lower-bound operations.
    double const d = std::abs(heads.denominator);
    double const ed = arithmetic::rounded_mul(8 * u, heads.md);
    if (!(d > ed)) return rescue_bounded_denominator<N>(p, heads, output);
    double const lower = arithmetic::rounded_mul(1 - 2 * u, arithmetic::rounded_sub(d, ed));
    double const t = arithmetic::rounded_div(heads.numerator, heads.denominator);
    double const at = std::abs(t);
    double const v = std::fma(at, heads.md, heads.mt);
    double const b = arithmetic::rounded_mul(c, lower);
    double const h = std::fma(at, lower, std::fma(8, v, std::abs(heads.numerator)));
    std::array<double, 2> const direction{{heads.ux, heads.uy}};
    std::array<bool, 2> accepted{};
    // |T-tD| <= u*(|n| + 8*(MT + |t|*MD)). Divide the mathematical
    // bound by L, use |U| <= |h_axis|/(1-u) and add |t|*|U-h_axis|.
    // The three nonnegative FMAs can lose at most another (1-u)^3,
    // yielding |Z-y| <= u*|h_axis|*H/(L*(1-u)^4), y=a+t*h_axis.
    auto certify_axis = [&](std::size_t axis, double z, double bound)
    {
        // Strict rounded order implies strict real order of the products.
        // Passing keeps RN(Z) within k steps of z=RN(y): k intervening
        // rounding cells span at least k*u*(1-2u)^(k+1)*|z|.
        if (!bounded_axis_passes(direction[axis], z, bound, b)) return;
        output[axis] = z;
        accepted[axis] = true;
    };
    std::array<double, 2> const z{{std::fma(t, direction[0], p[0]),
                                  std::fma(t, direction[1], p[1])}};
    for (std::size_t axis = 0; axis < 2; ++axis)
        certify_axis(axis, z[axis], h);

    if (!accepted[0] || !accepted[1])
    {
        double const floor = arithmetic::rounded_mul(at, lower);
        if ((!accepted[0] && bounded_axis_passes(heads.ux, z[0], floor, b))
            || (!accepted[1] && bounded_axis_passes(heads.uy, z[1], floor, b)))
        {
            double const tightened = retry_determinants(p, heads, t, lower, b, z, accepted);
            for (std::size_t axis = 0; axis < 2; ++axis)
                if (!accepted[axis]) certify_axis(axis, z[axis], tightened);
        }
    }
    return accepted;
}

// If t encloses the exact segment parameter T, exact X=A+T*(B-A).
// two_diff(b,a) gives U+tail exactly, hence EU=EA+EB+abs(tail).
// two_sum(fl(t*U),a), together with the FMA product residual and two_diff
// against the final FMA result c, encloses a+t*U-c without a relative bound
// on c (which would fail at cancellation/zero). Therefore
//   |X-c| <= |a+t*U-c| + EA + |t|EU + (|U|+EU)Et.
// Each nonnegative bound below has <=16 rounded operations on any path;
// forward_error::inflate covers evaluation roundoff as well. Unlike the
// coarse a-priori construction bound, exact EFT residuals can vanish.
template <typename C>
inline bool forward_bound(std::array<C, 8> const& producers, fe::point_approximation& output)
{
    fe::scalar_approximation t;
    if (!fe::try_segment_parameter(producers, t)) return false;
    fe::point_approximation result;
    for (std::size_t d = 0; d < 2; ++d)
    {
        double a, ea, b, eb;
        if (!fe::try_native_coordinate(producers[d], a, ea)
            || !fe::try_native_coordinate(producers[2 + d], b, eb)) return false;
        auto const [u, u_tail] = geometry::detail::precise_math::two_diff(b, a);
        double const eu = fe::detail::inflate(arithmetic::rounded_add(
            arithmetic::rounded_add(ea, eb), std::abs(u_tail)));
        double const product = arithmetic::rounded_mul(t.value, u);
        double const product_tail = std::fma(t.value, u, -product);
        auto const [sum, sum_tail] = geometry::detail::precise_math::two_sum(product, a);
        double const coordinate = std::fma(t.value, u, a);
        auto const [delta, delta_tail] = geometry::detail::precise_math::two_diff(sum, coordinate);
        auto const [residual, residual_tail] = geometry::detail::precise_math::two_sum(sum_tail, product_tail);
        auto const [combined, combined_tail] = geometry::detail::precise_math::two_sum(delta, residual);
        double const rounding = fe::detail::inflate(arithmetic::rounded_add(
            arithmetic::rounded_add(std::abs(combined), std::abs(combined_tail)),
            arithmetic::rounded_add(std::abs(residual_tail), std::abs(delta_tail))));
        result.value[d] = coordinate;
        result.error[d] = fe::detail::inflate(arithmetic::rounded_add(
            arithmetic::rounded_add(rounding, ea), arithmetic::rounded_add(
                arithmetic::rounded_mul(std::abs(t.value), eu),
                arithmetic::rounded_mul(arithmetic::rounded_add(std::abs(u), eu), t.error))));
        // Match fixed_rounding's canonical zero for an exactly zero quotient.
        if (result.value[d] == 0 && result.error[d] == 0) result.value[d] = 0;
    }
    if (!result.valid()) return false;
    output = result;
    return true;
}

template <typename C>
inline bool represented(std::array<C, 8> const& input, std::array<double, 8>& output)
{
    for (std::size_t i = 0; i < input.size(); ++i)
    {
        double error;
        if (!fe::try_native_coordinate(input[i], output[i], error) || error != 0) return false;
    }
    return true;
}

// Native b=16 capacity theorem, not a floating-value heuristic. For integral
// |p[i]|<2^16, line U/V have magnitude <2^17 and C <2^33. Consequently every
// product in X/Y is <2^50, X/Y <2^51, and W <2^35. Each written product and
// difference is therefore an EXACT binary64 integer. For W!=0, one hardware
// division per coordinate is nearest/ties-even; nonzero quotients lie between
// 2^-35 and 2^51, so neither overflow nor underflow is possible on this path.
// Complete <=16-bit native formats need no per-value guard. Wider signed
// formats compare directly (never abs/negate MIN); unsigned formats have only
// the upper check. Producer conversion/storage remains call-local.
template <typename C>
inline std::enable_if_t<std::is_integral<C>::value, bool>
small_integer(std::array<C, 8> const& producers, std::array<double, 2>& output)
{
    if constexpr (std::numeric_limits<C>::digits + std::is_signed<C>::value > 16)
    {
        for (C value : producers)
        {
            if constexpr (std::is_signed<C>::value)
            {
                if (value <= -65536 || value >= 65536) return false;
            }
            else if (value >= 65536u) return false;
        }
    }
    std::array<double, 8> p;
    for (std::size_t i = 0; i < p.size(); ++i) p[i] = double(producers[i]);
    double const u = p[1] - p[3], v = p[2] - p[0], c = p[0] * p[3] - p[1] * p[2];
    double const s = p[5] - p[7], t = p[6] - p[4], f = p[4] * p[7] - p[5] * p[6];
    double const w = u * t - v * s;
    if (w == 0) return false;
    double const x = v * f - c * t, y = c * s - u * f;
    output = {{x == 0 ? 0 : arithmetic::rounded_div(x, w),
               y == 0 ? 0 : arithmetic::rounded_div(y, w)}};
    return true;
}

template <bool Errors> struct storage;

template <> struct storage<false>
{
    std::array<double, 2> approximate{};
    point_cache_detail::native_cache_handle precision;
};

template <> struct storage<true>
{
    std::array<double, 2> approximate{};
    std::array<double, 2> error{};
    point_cache_detail::native_cache_handle precision;
};
} // namespace detail

inline bool try_certify_nearest(forward_error::scalar_approximation const& approximation)
{ return detail::certify_nearest(approximation); }

template <typename C, typename Policy = relate::approximate_output>
class point_state
{
    static_assert(fixed_native_supported<C>::value, "Native construction coordinates required");
    static_assert(std::numeric_limits<double>::is_iec559
        && std::numeric_limits<double>::radix == 2 && std::numeric_limits<double>::digits == 53,
        "Construction rounding requires IEEE binary64");
    using traits = policy_traits<Policy>;
    using cache = point_cache_detail::native_cache<C>;
    static constexpr bool bounded_output = is_bounded_ulp_output<Policy>::value;

public:
    using coordinate_type = C;
    using policy_type = Policy;
    using producers_type = std::array<C, 8>;
    using storage_type = detail::storage<traits::store_errors>;
    static constexpr bool stores_errors = traits::store_errors;

    double approximate(std::size_t dimension) const
    { return m_state.approximate.at(dimension); }

    // Search boxes require an enclosure even when no per-axis error is stored.
    interval approximation(std::size_t dimension) const
    {
        double const value = m_state.approximate.at(dimension);
        if constexpr (traits::store_errors) return {value, m_state.error[dimension]};
        else if constexpr (bounded_output)
        {
            // Search envelopes need an absolute enclosure, not a predicate
            // sign. The accepted construction proves the bound below; a
            // refined binary64 is nearest. The floor covers rounded zero
            // and subnormal coordinates on the latter path.
            constexpr double coefficient = traits::arithmetic_certificate::rho * forward_error::detail::u;
            return {value, detail::arithmetic::rounded_mul(coefficient,
                (std::max)(std::abs(value), (std::numeric_limits<double>::min)()))};
        }
        else return {value, detail::spacing(value)};
    }

    interval bounds(std::size_t dimension) const { return approximation(dimension); }

    point_cache_detail::native_cache_handle& cache_handle() const
    { return m_state.precision; }

    void init(producers_type const& producers)
    {
        m_state = storage_type{};
        if constexpr (bounded_output)
        {
            std::array<bool, 2> accepted{};
            if constexpr (std::is_same<C, double>::value)
                accepted = detail::bounded_intersection<Policy::steps>(producers, m_state.approximate);
            for (std::size_t d = 0; d < 2; ++d)
                if (!accepted[d]) refine_value(producers, d);
        }
        else
        {
            if constexpr (traits::nearest_output && std::is_integral<C>::value)
                if (detail::small_integer(producers, m_state.approximate)) return;
            forward_error::point_approximation first;
            bool ready;
            // Approximate output needs only an enclosure. Nearest uses the tighter
            // EFT residuals to certify each coordinate before falling back.
            if constexpr (traits::store_errors)
                ready = forward_error::try_intersection(producers, first);
            else
                ready = detail::forward_bound(producers, first);
            for (std::size_t d = 0; d < 2; ++d)
            {
                if (ready)
                {
                    m_state.approximate[d] = first.value[d];
                    if (first.value[d] == 0 && first.error[d] == 0) m_state.approximate[d] = 0;
                    if constexpr (traits::store_errors)
                    {
                        m_state.error[d] = first.error[d];
                        continue;
                    }
                    else if (try_certify_nearest({first.value[d], first.error[d]})) continue;
                }
                refine_value(producers, d);
            }
        }
    }

    // Cache refinement returns nearest binary64. No cache reference survives
    // another indexed lookup.
    // Without an enclosing scope, native_cache uses stack temporaries instead.
    interval refine(producers_type const& producers, std::size_t dimension) const
    {
        refine_value(producers, dimension);
        return approximation(dimension);
    }

private:
    void refine_value(producers_type const& producers, std::size_t dimension) const
    {
        check_dimension(dimension);
        std::array<double, 8> represented;
        interval refined;
        if (cache::cached_bounds(m_state.precision, dimension, refined)
            || (detail::represented(producers, represented)
                && cache::refine(m_state.precision, represented, dimension, refined)))
        {
            update(dimension, refined.approximate, refined.error == 0);
        }
        else
        {
            auto const exact = cache::get(m_state.precision, true, producers);
            double rounded;
            if (!try_fixed_round(exact, dimension, rounded) || !std::isfinite(rounded))
                throw std::domain_error("Intersection has no finite Cartesian approximation");
            update(dimension, rounded, (dimension == 0 ? exact.X : exact.Y) == 0);
        }
    }

public:
    template <typename Output>
    Output emit(std::size_t dimension, producers_type const& producers) const
    {
        check_dimension(dimension);
        // BG integral conversion deliberately observes the CURRENT value,
        // without nearest refinement or nearest-integer rational rounding.
        if constexpr ((!traits::nearest_output && !bounded_output) || std::is_integral<Output>::value)
        {
            return geometry::util::numeric_cast<Output>(approximate(dimension));
        }
        else if constexpr (std::is_same<Output, double>::value)
        {
            return approximate(dimension);
        }
        else
        {
            static_assert(std::is_same<Output, float>::value,
                          "Nearest construction output supports float and double");
            // Rounding nearest-double again to float can double-round at a
            // float midpoint. Round the cached exact quotient to float itself.
            auto const exact = cache::get(m_state.precision, true, producers);
            Output rounded;
            if (!try_fixed_round(exact, dimension, rounded))
                throw std::domain_error("Intersection has no finite Cartesian approximation");
            if constexpr (bounded_output)
            {
                // Preserve bounded output's refinement of its stored approximation.
                double current;
                if (!try_fixed_round(exact, dimension, current) || !std::isfinite(current))
                    throw std::domain_error("Intersection has no finite Cartesian approximation");
                update(dimension, current, (dimension == 0 ? exact.X : exact.Y) == 0);
            }
            return rounded;
        }
    }

private:
    static void check_dimension(std::size_t dimension)
    {
        if (dimension > 1) throw std::out_of_range("Exact Cartesian coordinate dimension");
    }

    void update(std::size_t dimension, double nearest, bool zero) const
    {
        m_state.approximate[dimension] = nearest;
        if constexpr (traits::store_errors)
            m_state.error[dimension] = zero ? 0 : detail::arithmetic::rounded_mul(0.5, detail::spacing(nearest));
    }

    mutable storage_type m_state;
};

}}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian::construction_rounding

#endif
