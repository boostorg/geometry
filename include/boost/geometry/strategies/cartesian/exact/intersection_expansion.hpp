// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Licensed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_INTERSECTION_EXPANSION_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_INTERSECTION_EXPANSION_HPP

#include <array>
#include <cmath>
#include <cstddef>
#include <limits>

#include <boost/geometry/strategies/cartesian/exact/arithmetic.hpp>
#include <boost/geometry/util/detail/expansion.hpp>

#include <boost/geometry/util/detail/precise_fp.hpp>

BOOST_GEOMETRY_DETAIL_PRECISE_FP_BEGIN

namespace boost { namespace geometry { namespace strategies { namespace detail
{
namespace exact_cartesian { namespace construction_detail
{

using geometry::detail::precise_math::expansion;
using geometry::detail::precise_math::expand_difference;

// If each native ordinate has magnitude <= B, the L1 norms of the
// difference and line-constant expansions are <= 4B and 4B^2. W and N
// each sum two products of those expansions, with raw L1 norm <= 32B^d.
// A factor two covers accumulation and the final estimate. Each EFT's
// head and tail increase the L1 budget by at most 1+2u, and the fixed
// expansion sizes keep the number of such steps far below 1/(4u).
constexpr int homogeneous_sum_bits = 6;

// Adjacent quotient candidates may sit a half grid below a binade.
// Include one more significand of headroom for the midpoint sign filter.
// A midpoint sum has L1 norm <= 8B, then multiplies W and subtracts 2N.
constexpr int boundary_band = exponent_range::polynomial_band(
    3, 1 + exponent_range::significand_bits, homogeneous_sum_bits + 8);

// Two products of N and W. Allow one bit for their sum and four for
// intermediate EFT values and accumulation.
constexpr int pair_comparison_band = exponent_range::polynomial_band(
    5, 0, 2 * homogeneous_sum_bits + 1 + 4);

// Before checking the candidate, divide the estimates of N and W.
// Their smallest nonzero magnitudes are g^3 and g^2, respectively, and
// their largest are 2^homogeneous_sum_bits * B^d, for input grid g.
constexpr int quotient_band = (std::min)({boundary_band,
    (-exponent_range::min_normal - 3 * exponent_range::fraction_bits - homogeneous_sum_bits) / 5,
    (exponent_range::max_normal - 2 * exponent_range::fraction_bits - homogeneous_sum_bits) / 5});

template <int Band>
inline bool supported(std::array<double, 8> const& values)
{
    if (!std::numeric_limits<double>::is_iec559
        || std::numeric_limits<double>::radix != 2
        || std::numeric_limits<double>::digits != 53) return false;
    for (double value : values)
    {
        if (!exponent_range::contains<Band>(value)) return false;
    }
    return true;
}

// Input: {ax,ay,bx,by,cx,cy,dx,dy}. Cross the two homogeneous line equations.
// Differences have <=2 components, line constants <=4, W <=16 and X/Y <=32.
inline bool homogeneous_coordinate(std::array<double, 8> const& v, std::size_t dimension,
                                    expansion<32>& numerator, expansion<16>& denominator)
{
    auto const ax = expand_difference(v[2], v[0]);
    auto const ay = expand_difference(v[1], v[3]);
    auto const bx = expand_difference(v[6], v[4]);
    auto const by = expand_difference(v[5], v[7]);
    expansion<4> a, b;
    if (!a.product(v[0], v[3]) || !a.product(v[1], v[2], true)
        || !b.product(v[4], v[7]) || !b.product(v[5], v[6], true)
        || !add_product(denominator, ay, bx)
        || !add_product(denominator, ax, by, true)) return false;
    return dimension == 0
        ? add_product(numerator, ax, b) && add_product(numerator, a, bx, true)
        : add_product(numerator, a, by) && add_product(numerator, ay, b, true);
}

// boundary_band covers both the midpoint filter and the expansion comparison.
// Two N contributes <=32 components; a two-term midpoint sum times W <=64.
// The 96-component bound fits 128 slots; checked capacity is still mandatory.
inline bool compare_boundary(expansion<32> const& numerator,
                             expansion<16> const& denominator,
                             double first, double second, int& result)
{
    auto const [hi, lo] = geometry::detail::precise_math::two_sum(first, second);
    if (numerator.size && denominator.size)
    {
        auto const tails = [](auto const& e)
        {
            double value = 0, magnitude = 0;
            for (std::size_t i = 0; i + 1 < e.size; ++i)
            {
                value += e.components[i];
                magnitude += std::abs(e.components[i]);
            }
            return std::array<double, 2>{{value, magnitude}};
        };
        auto const n = tails(numerator);
        auto const d = tails(denominator);
        double const head = denominator.components[denominator.size - 1];
        double const r = std::fma(-hi, head, 2 * numerator.components[numerator.size - 1]);
        double const low_product = lo * head;
        double const value = ((r + 2 * n[0]) - hi * d[0]) - low_product - lo * d[0];
        double const magnitude = std::abs(r) + 2 * n[1]
            + std::abs(hi) * d[1] + std::abs(low_product) + std::abs(lo) * d[1];
        // Test 2*N - (hi+lo)*W. Each tail sum has at most 31 terms.
        // Including the FMA, subsequent operations and rounding of the
        // magnitude, 128u covers the forward error in the band above.
        constexpr double bound = 128 * (std::numeric_limits<double>::epsilon() * 0.5);
        if (std::abs(value) > bound * magnitude)
        {
            result = (value > 0 ? 1 : -1) * denominator.sign();
            return true;
        }
    }
    expansion<2> sum;
    if (!sum.add(std::array<double, 2>{{lo, hi}})) return false;
    expansion<128> difference;
    for (std::size_t i = 0; i < numerator.size; ++i)
        if (!difference.add(2.0 * numerator.components[i])) return false;
    if (!add_product(difference, sum, denominator, true)) return false;
    result = difference.sign() * denominator.sign();
    return true;
}

inline bool admissible_candidate(double value)
{
    return value != 0 && exponent_range::contains<boundary_band>(value);
}

}} // namespace exact_cartesian::construction_detail

namespace exact_cartesian
{

// The producers have passed quotient_band before constructing the expansions.
inline bool try_round_quotient(construction_detail::expansion<32> const& numerator,
                               construction_detail::expansion<16> const& denominator,
                               double& output)
{
    using namespace construction_detail;
    if (!numerator.valid || !denominator.valid || denominator.sign() == 0) return false;
    if (numerator.sign() == 0) { output = 0; return true; }

    double const n = numerator.estimate(), d = denominator.estimate();
    if (n == 0 || d == 0) return false;
    double candidate = n / d;
    double const infinity = (std::numeric_limits<double>::infinity)();
    for (unsigned correction = 0; correction <= 4 && admissible_candidate(candidate); ++correction)
    {
        double const previous = std::nextafter(candidate, -infinity);
        double const next = std::nextafter(candidate, infinity);
        int lower, upper;
        if (!compare_boundary(numerator, denominator, previous, candidate, lower)
            || !compare_boundary(numerator, denominator, candidate, next, upper)) return false;
        bool const even = arithmetic_detail::binary64_even(candidate);
        bool const above_lower = lower > 0 || (lower == 0 && even);
        bool const below_upper = upper < 0 || (upper == 0 && even);
        if (above_lower && below_upper) { output = candidate; return true; }
        candidate = above_lower ? next : previous;
    }
    return false;
}

} // namespace exact_cartesian
}}}} // namespace boost::geometry::strategies::detail

BOOST_GEOMETRY_DETAIL_PRECISE_FP_END

#endif
