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

#include <boost/geometry/util/detail/precise_fp.hpp>

BOOST_GEOMETRY_DETAIL_PRECISE_FP_BEGIN

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian { namespace construction_rounding
{

// IEEE binary64, nearest/ties-even, no implicit contraction or fast-math.
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
namespace fe = forward_error;
constexpr double inverse_unit_roundoff = 1 / fe::detail::u;

// Symmetric forward bounds must fit BOTH adjacent spacings. At a binade they
// differ by two, so epsilon*abs(value) is not an adequate certificate. The
// spacings (and normal half-spacings) are exact powers of two. Nearest cells
// include both midpoint endpoints iff the retained significand is even.
inline bool certify_nearest(scalar_approximation const& a)
{
    if (!a.valid()) return false;
    if (a.error == 0) return true;
    if (a.value == 0) return false;
    double const infinity = (std::numeric_limits<double>::infinity)();
    double const previous = std::nextafter(a.value, -infinity);
    double const next = std::nextafter(a.value, infinity);
    if (!std::isfinite(previous) || !std::isfinite(next)) return false;
    double const spacing = (std::min)(a.value - previous,
                                      next - a.value);
    double const half = 0.5 * spacing;
    return a.error < half || (a.error == half && arithmetic_detail::binary64_even(a.value));
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
    // With p significand bits, ilogb(t)+ilogb(d) >= ilogb(n)-1 for n != 0,
    // so the grid of t*d is no finer than 2^(ilogb(n)-(2*p-1)).
    // The residual lies on that grid and |n-t*d| <= u*|n|, so it fits
    // in p bits. The admitted exponent band also covers the scaling.
    return std::abs(std::fma(-t, d, n)) * inverse_unit_roundoff;
}

inline bool bounded_axis_passes(double direction, double z, double height, double threshold)
{
    return std::abs(direction) * height < std::abs(z) * threshold;
}

inline auto determinant_tails(double ax, double ay, double bx, double by, double head)
{
    using namespace geometry::detail::precise_math;
    auto const p = two_product(ax, by), q = two_product(ay, bx);
    return std::array<double, 3>{{two_diff_tail(p[0], q[0], head), p[1], q[1]}};
}

inline double residual_height(double at, double lower, double residual, double et, double ed)
{
    return std::fma(at, lower, residual + std::fma(at, ed, et));
}

inline void certify_axes(std::array<double, 2> const& direction, std::array<double, 2> const& candidate,
                         double height, double threshold, std::array<double, 2>& output,
                         std::array<bool, 2>& accepted)
{
    for (std::size_t axis = 0; axis < 2; ++axis)
    {
        if (accepted[axis] || !bounded_axis_passes(direction[axis], candidate[axis], height, threshold)) continue;
        output[axis] = candidate[axis];
        accepted[axis] = true;
    }
}

// Budget in units of u for the ORIGINAL head k = RN(p-q). Requires all
// four endpoint differences to be exact. The three EFT tails ep, eq, ek
// give det-k = ek+ep-eq exactly. For s=RN(ep-eq), r=RN(ek+s), the
// posterior rounding bound gives |det-k| <= (|r|+u*|s|)/(1-u).
// The factor 2 covers this denominator and the rounded nonnegative sum:
// 2*(1-u)^2 > 1.
inline double determinant_budget(double ax, double ay, double bx, double by, double k)
{
    auto const tails = determinant_tails(ax, ay, bx, by, k);
    double const s = tails[1] - tails[2];
    double const r = tails[0] + s;
    return 2 * std::fma(inverse_unit_roundoff, std::abs(r), std::abs(s));
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
    constexpr double u = fe::detail::u;
    auto const tails = determinant_tails(ax, ay, bx, by, k);
    double r = tails[0] + (tails[1] - tails[2]);
    r = std::fma(ax, byt, r);
    r = std::fma(axt, by, r);
    r = std::fma(-ay, bxt, r);
    r = std::fma(-ayt, bx, r);
    return (1 + 4 * u) * std::fma(128 * u, magnitude, inverse_unit_roundoff * std::abs(r));
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
    { return residual_height(at, lower, residual, et, ed); };
    constexpr double units = fe::detail::native_determinant_units;
    double et = units * heads.mt, ed = units * heads.md;
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
    double const vx = p[6] - p[4], vy = p[7] - p[5];
    double const vxt = two_diff_tail(p[6], p[4], vx);
    double const vyt = two_diff_tail(p[7], p[5], vy);
    auto tighten = [&](bool denominator)
    {
        std::size_t const i = denominator ? 2 : 4;
        double const x = denominator ? heads.ux : p[4] - p[0];
        double const y = denominator ? heads.uy : p[5] - p[1];
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
inline scalar_approximation rebuild_determinant(double ax, double ay, double bx, double by,
                                                    double original)
{
    using geometry::detail::precise_math::two_diff_tail;
    auto const tails = determinant_tails(ax, ay, bx, by, original);
    double const r = tails[1] - tails[2];
    double const a = std::abs(tails[0]);
    double const b = std::abs(two_diff_tail(tails[1], tails[2], r));
    auto const sum = geometry::detail::precise_math::two_sum(original, r);
    return {sum[0], 2 * (a + b + std::abs(sum[1]))};
}

template <std::size_t N>
BOOST_NOINLINE std::array<bool, 2> rescue_bounded_denominator(std::array<double, 8> const& p,
    fe::detail::native_intersection_heads const& heads, std::array<double, 2>& output,
    std::array<bool, 2> accepted)
{
    using geometry::detail::precise_math::two_diff_tail;
    constexpr double u = fe::detail::u;
    constexpr double c = bounded_threshold_factor<N>();
    double const vx = p[6] - p[4], vy = p[7] - p[5];
    if (two_diff_tail(p[6], p[4], vx) != 0 || two_diff_tail(p[7], p[5], vy) != 0
        || two_diff_tail(p[2], p[0], heads.ux) != 0 || two_diff_tail(p[3], p[1], heads.uy) != 0)
        return accepted;
    auto const d = rebuild_determinant(heads.ux, heads.uy, vx, vy, heads.denominator);
    if (!(std::abs(d.value) > d.error)) return accepted;
    double const lower = fe::detail::denominator_lower_bound(std::abs(d.value), d.error);
    scalar_approximation n{heads.numerator, fe::detail::native_determinant_units * u * heads.mt};
    double const wx = p[4] - p[0], wy = p[5] - p[1];
    if (two_diff_tail(p[4], p[0], wx) == 0 && two_diff_tail(p[5], p[1], wy) == 0)
    {
        auto const rebuilt = rebuild_determinant(wx, wy, vx, vy, n.value);
        if (rebuilt.error <= n.error) n = rebuilt;
    }
    // New denominator and possibly new numerator: recompute everything
    // dependent on them, including the division residual and both candidates.
    double const t = n.value / d.value, at = std::abs(t);
    double const residual = scaled_division_residual(t, d.value, n.value);
    double const height = residual_height(at, lower, residual,
        n.error * inverse_unit_roundoff, d.error * inverse_unit_roundoff);
    double const threshold = c * lower;
    std::array<double, 2> const direction{{heads.ux, heads.uy}};
    certify_axes(direction, fe::detail::intersection_candidate(p, heads, t), height, threshold, output, accepted);
    return accepted;
}

constexpr int bounded_intersection_band()
{
    using namespace exponent_range;
    constexpr int p = significand_bits, f = fraction_bits;
    // With B = 2^E and grid g = 2^-(E+f), determinant heads and
    // rebuilt heads are multiples of g^2 and have magnitude <= 16B^2.
    // Hence a nonzero t has magnitude in [g^2/(16B^2), 16B^2/g^2]
    // and grid exponent at least -(4E + quotient_grid_bits).
    constexpr int determinant_bits = 4;
    constexpr int quotient_magnitude_bits = 2 * f + determinant_bits;
    constexpr int quotient_grid_bits = quotient_magnitude_bits + f;

    // L and the retry's budgets use at most two u factors below the
    // determinant grid. The threshold c*L adds one more. The final RHS
    // multiplies it by z = fma(t, direction, a), on grid g times t's grid.
    // This also covers t*d in the residual and direction*height on the LHS.
    constexpr int comparison_grid_bits = quotient_grid_bits + 3 * (f + p);

    // Upper bounds apart from powers of B: height uses at most one t,
    // a determinant budget scaled by 1/u, and sums of the residual and
    // retry terms. Eight bits cover these sums and their coefficients.
    constexpr int height_bits = determinant_bits + p + 8;
    constexpr int left_bits = quotient_magnitude_bits + height_bits + 1;
    constexpr int right_bits = quotient_magnitude_bits + 2 + determinant_bits
                            + fixed_detail::ceiling_log2(ulp_filter::max_steps);
    return (std::min)((-min_normal - comparison_grid_bits) / 7,
                      (max_normal - (std::max)(left_bits, right_bits)) / 7);
}

template <std::size_t N>
inline std::array<bool, 2> bounded_intersection(std::array<double, 8> const& producers,
                                                std::array<double, 2>& output)
{
    static_assert(N >= 1, "A rounding cell must intervene");
    constexpr double u = fe::detail::u;
    constexpr double c = bounded_threshold_factor<N>();
    std::array<bool, 2> accepted{};
    for (std::size_t axis = 0; axis < 2; ++axis)
    {
        // A constant ordinate on either support is exact, even at zero.
        auto const from = producers[axis] == producers[axis + 2] ? axis : axis + 4;
        if (producers[from] == producers[from + 2])
        {
            output[axis] = producers[from] == 0 ? 0.0 : producers[from];
            accepted[axis] = true;
        }
    }
    if (accepted[0] && accepted[1]) return accepted;
    for (double x : producers)
        if (!exponent_range::contains<bounded_intersection_band()>(x))
            return accepted;

    // Either support gives the same exact point. A much shorter support
    // reduces the displacement whose error the certificate has to bound.
    auto const span = [&](std::size_t i)
    {
        return (std::max)(std::abs(producers[i + 2] - producers[i]),
                          std::abs(producers[i + 3] - producers[i + 1]));
    };
    bool const swap_supports = span(4) < 0.5 * span(0);
    std::array<double, 8> swapped;
    if (swap_supports)
        swapped = {{producers[4], producers[5], producers[6], producers[7],
                    producers[0], producers[1], producers[2], producers[3]}};
    auto const& p = swap_supports ? swapped : producers;

    auto const heads = fe::detail::intersection_heads(p);
    double const d = std::abs(heads.denominator);
    constexpr double k = fe::detail::native_determinant_units;
    double const ed = k * u * heads.md;
    if (!(d > ed)) return rescue_bounded_denominator<N>(p, heads, output, accepted);
    double const lower = fe::detail::denominator_lower_bound(d, ed);
    double const t = heads.numerator / heads.denominator;
    double const at = std::abs(t);
    double const v = std::fma(at, heads.md, heads.mt);
    double const b = c * lower;
    double const h = std::fma(at, lower, std::fma(k, v, std::abs(heads.numerator)));
    std::array<double, 2> const direction{{heads.ux, heads.uy}};
    // |T-tD| <= u*(|n| + 8*(MT + |t|*MD)). Divide the mathematical
    // bound by L, use |U| <= |h_axis|/(1-u) and add |t|*|U-h_axis|.
    // The three nonnegative FMAs can lose at most another (1-u)^3,
    // yielding |Z-y| <= u*|h_axis|*H/(L*(1-u)^4), y=a+t*h_axis.
    // Strict rounded order implies strict real order of the products.
    // Passing keeps RN(Z) within N steps of z=RN(y): N intervening
    // rounding cells span at least N*u*(1-2u)^(N+1)*|z|.
    auto const z = fe::detail::intersection_candidate(p, heads, t);
    certify_axes(direction, z, h, b, output, accepted);

    if (!accepted[0] || !accepted[1])
    {
        double const floor = at * lower;
        if ((!accepted[0] && bounded_axis_passes(heads.ux, z[0], floor, b))
            || (!accepted[1] && bounded_axis_passes(heads.uy, z[1], floor, b)))
        {
            double const tightened = retry_determinants(p, heads, t, lower, b, z, accepted);
            certify_axes(direction, z, tightened, b, output, accepted);
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
    scalar_approximation t;
    if (!fe::try_segment_parameter(producers, t)) return false;
    fe::point_approximation result;
    for (std::size_t d = 0; d < 2; ++d)
    {
        double a, ea, b, eb;
        if (!fe::try_native_coordinate(producers[d], a, ea)
            || !fe::try_native_coordinate(producers[2 + d], b, eb)) return false;
        auto const [u, u_tail] = geometry::detail::precise_math::two_diff(b, a);
        double const eu = fe::detail::inflate(ea + eb + std::abs(u_tail));
        double const product = t.value * u;
        double const product_tail = std::fma(t.value, u, -product);
        auto const [sum, sum_tail] = geometry::detail::precise_math::two_sum(product, a);
        double const coordinate = std::fma(t.value, u, a);
        auto const [delta, delta_tail] = geometry::detail::precise_math::two_diff(sum, coordinate);
        auto const [residual, residual_tail] = geometry::detail::precise_math::two_sum(sum_tail, product_tail);
        auto const [combined, combined_tail] = geometry::detail::precise_math::two_sum(delta, residual);
        double const rounding = fe::detail::inflate(
            (std::abs(combined) + std::abs(combined_tail))
            + (std::abs(residual_tail) + std::abs(delta_tail)));
        result.value[d] = coordinate;
        result.error[d] = fe::detail::inflate((rounding + ea)
            + (std::abs(t.value) * eu + (std::abs(u) + eu) * t.error));
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
    fe::detail::rounded_planar_ops const ops;
    auto const point = linear_formula::meet(ops,
        linear_formula::through(ops, p.data(), p.data() + 2),
        linear_formula::through(ops, p.data() + 4, p.data() + 6));
    if (point.w == 0) return false;
    output = {{point.x == 0 ? 0 : point.x / point.w,
               point.y == 0 ? 0 : point.y / point.w}};
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

    double approximate(std::size_t dimension) const
    { return m_state.approximate.at(dimension); }

    // Search boxes require an enclosure even when no per-axis error is stored.
    scalar_approximation approximation(std::size_t dimension) const
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
            return {value, coefficient * (std::max)(std::abs(value), (std::numeric_limits<double>::min)())};
        }
        else return {value, arithmetic_detail::max_adjacent_spacing(value)};
    }

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
                    else if (detail::certify_nearest({first.value[d], first.error[d]})) continue;
                }
                refine_value(producers, d);
            }
        }
    }

    // Cache refinement returns nearest binary64. No cache reference survives
    // another indexed lookup.
    // Without an enclosing scope, native_cache uses stack temporaries instead.
    scalar_approximation refine(producers_type const& producers, std::size_t dimension) const
    {
        refine_value(producers, dimension);
        return approximation(dimension);
    }

private:
    void refine_value(producers_type const& producers, std::size_t dimension) const
    {
        check_dimension(dimension);
        std::array<double, 8> represented;
        scalar_approximation refined;
        if (cache::cached_approximation(m_state.precision, dimension, refined)
            || (detail::represented(producers, represented)
                && cache::refine(m_state.precision, represented, dimension, refined)))
        {
            update(dimension, refined.value, refined.error == 0);
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
            m_state.error[dimension] = zero ? 0 : 0.5 * arithmetic_detail::max_adjacent_spacing(nearest);
    }

    mutable storage_type m_state;
};

}}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian::construction_rounding

BOOST_GEOMETRY_DETAIL_PRECISE_FP_END

#endif
