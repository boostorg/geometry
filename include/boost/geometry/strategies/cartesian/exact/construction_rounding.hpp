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
    static constexpr bool store_errors = false;
    static constexpr bool nearest_output = true;
};

namespace detail
{
namespace fe = forward_error;

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
        else return {value, arithmetic_detail::max_adjacent_spacing(value)};
    }

    point_cache_detail::native_cache_handle& cache_handle() const
    { return m_state.precision; }

    void init(producers_type const& producers)
    {
        m_state = storage_type{};
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
        if constexpr (!traits::nearest_output || std::is_integral<Output>::value)
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
