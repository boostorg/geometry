// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Licensed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_CONSTRUCTION_FILTER_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_CONSTRUCTION_FILTER_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#include <boost/geometry/strategies/cartesian/exact/arithmetic.hpp>
#include <boost/geometry/util/error_free_transforms.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{
namespace exact_cartesian { namespace construction_detail
{

// Grow-Expansion with zero elimination, following Shewchuk's error-free
// transforms (https://www.cs.cmu.edu/~quake/robust.html). Every write is checked:
// losing a component invalidates the filter instead of truncating its value.
template <std::size_t Capacity>
struct expansion
{
    std::array<double, Capacity> components;
    std::size_t size = 0;
    bool valid = true;

    // Unused components have no value; every read is bounded by size.
    expansion() noexcept {}
    expansion(expansion const& other) noexcept : size(other.size), valid(other.valid)
    {
        std::copy_n(other.components.begin(), size, components.begin());
    }
    expansion& operator=(expansion const& other) noexcept
    {
        if (this != &other)
        {
            std::copy_n(other.components.begin(), other.size, components.begin());
            size = other.size;
            valid = other.valid;
        }
        return *this;
    }
    expansion(expansion&& other) noexcept : expansion(static_cast<expansion const&>(other)) {}
    expansion& operator=(expansion&& other) noexcept
    { return *this = static_cast<expansion const&>(other); }

    bool add(double value)
    {
        if (!valid || !std::isfinite(value)) { valid = false; return false; }
        std::array<double, Capacity> next;
        std::size_t count = 0;
        double sum = value;
        for (std::size_t i = 0; i < size; ++i)
        {
            auto const [head, tail] = geometry::detail::precise_math::two_sum(sum, components[i]);
            if (!std::isfinite(tail) || !std::isfinite(head))
            {
                valid = false;
                return false;
            }
            if (tail != 0)
            {
                if (count == Capacity) { valid = false; return false; }
                next[count++] = tail;
            }
            sum = head;
        }
        if (sum != 0)
        {
            if (count == Capacity) { valid = false; return false; }
            next[count++] = sum;
        }
        std::copy_n(next.begin(), count, components.begin());
        size = count;
        return true;
    }

    bool product(double a, double b, bool subtract = false)
    {
        double const head = arithmetic_detail::rounded_mul(a, b);
        double const tail = std::fma(a, b, -head);
        return add(subtract ? -tail : tail) && add(subtract ? -head : head);
    }

    int sign() const
    {
        return size == 0 ? 0 : components[size - 1] > 0 ? 1 : -1;
    }

    double estimate() const
    {
        double result = 0;
        for (std::size_t i = 0; i < size; ++i)
            result = arithmetic_detail::rounded_add(result, components[i]);
        return result;
    }

    bool compress()
    {
        if (!valid || size < 2) { return valid; }
        std::array<double, Capacity> work;
        double q = components[size - 1];
        std::size_t bottom = size;
        for (std::size_t i = size - 1; i != 0; --i)
        {
            auto const [head, tail] = geometry::detail::precise_math::two_sum(q, components[i - 1]);
            if (tail != 0) { work[--bottom] = head; q = tail; }
            else { q = head; }
        }
        work[--bottom] = q;
        std::size_t top = 0;
        for (std::size_t i = bottom + 1; i < size; ++i)
        {
            auto const [head, tail] = geometry::detail::precise_math::two_sum(work[i], q);
            if (tail != 0) { components[top++] = tail; }
            q = head;
        }
        if (q != 0) { components[top++] = q; }
        size = top;
        return true;
    }
};

template <std::size_t Out, std::size_t A, std::size_t B>
inline bool add_product(expansion<Out>& result, expansion<A> const& a,
                        expansion<B> const& b, bool subtract = false)
{
    if (!a.valid || !b.valid) { result.valid = false; return false; }
    for (std::size_t i = 0; i < a.size; ++i)
        for (std::size_t j = 0; j < b.size; ++j)
            if (!result.product(a.components[i], b.components[j], subtract)) return false;
    return result.valid;
}

inline expansion<2> difference(double a, double b)
{
    auto const [head, tail] = geometry::detail::precise_math::two_diff(a, b);
    expansion<2> result;
    result.add(tail);
    result.add(head);
    return result;
}

inline bool supported_value(double value, int exponent)
{
    double const magnitude = std::abs(value);
    return value == 0 || (magnitude >= std::ldexp(1.0, -exponent)
                          && magnitude <= std::ldexp(1.0, exponent));
}

inline bool supported(std::array<double, 8> const& values, int exponent = 150)
{
    if (!std::numeric_limits<double>::is_iec559
        || std::numeric_limits<double>::radix != 2
        || std::numeric_limits<double>::digits != 53) return false;
    for (double value : values)
    {
        if (!supported_value(value, exponent)) return false;
    }
    return true;
}

// Input: {ax,ay,bx,by,cx,cy,dx,dy}. Cross the two homogeneous line equations.
// Differences have <=2 components, line constants <=4, W <=16 and X/Y <=32.
inline bool homogeneous_coordinate(std::array<double, 8> const& v, std::size_t dimension,
                                    expansion<32>& numerator, expansion<16>& denominator)
{
    auto const ax = difference(v[2], v[0]);
    auto const ay = difference(v[1], v[3]);
    auto const bx = difference(v[6], v[4]);
    auto const by = difference(v[5], v[7]);
    expansion<4> a, b;
    if (!a.product(v[0], v[3]) || !a.product(v[1], v[2], true)
        || !b.product(v[4], v[7]) || !b.product(v[5], v[6], true)
        || !add_product(denominator, ay, bx)
        || !add_product(denominator, ax, by, true)) return false;
    return dimension == 0
        ? add_product(numerator, ax, b) && add_product(numerator, a, bx, true)
        : add_product(numerator, a, by) && add_product(numerator, ay, b, true);
}

// With guarded inputs every value is a multiple of 2^-202. Consequently W is
// a multiple of 2^-404 and X/Y of 2^-606. Candidate neighbours are multiples of
// 2^-203 or larger; their exact sum times W is a multiple of 2^-607. All sums
// and products used here have magnitude <2^460. Thus every nonzero EFT/FMA
// residual is normal: no underflow/overflow or dependence on subnormal modes.
// Two N contributes <=32 components; a two-term midpoint sum times W <=64.
// The 96-component bound fits 128 slots; checked capacity is still mandatory.
inline bool compare_boundary(expansion<32> const& numerator,
                             expansion<16> const& denominator,
                             double first, double second, int& result)
{
    expansion<2> sum;
    if (!sum.add(first) || !sum.add(second)) return false;
    expansion<128> difference;
    for (std::size_t i = 0; i < numerator.size; ++i)
        if (!difference.add(arithmetic_detail::rounded_mul(2.0, numerator.components[i]))) return false;
    if (!add_product(difference, sum, denominator, true)) return false;
    result = difference.sign() * denominator.sign();
    return true;
}

template <typename Output>
struct supported_output : std::integral_constant<bool,
    std::is_same<Output, double>::value || std::is_same<Output, float>::value
    || (std::is_integral<Output>::value && !std::is_same<Output, bool>::value
        && std::numeric_limits<Output>::digits <= 64)> {};

template <typename Output, bool Integral = std::is_integral<Output>::value>
struct rounding_cell;

template <typename Output>
struct rounding_cell<Output, false>
{
    static bool allowed(double value)
    {
        // Keep float neighbours normal as well as all double EFT operands.
        double const minimum = (std::max)(std::ldexp(1.0, -150),
            2.0 * double((std::numeric_limits<Output>::min)()));
        return std::numeric_limits<Output>::is_iec559
            && std::numeric_limits<Output>::radix == 2
            && std::numeric_limits<Output>::digits <= 53
            && std::isfinite(value) && std::abs(value) >= minimum
            && std::abs(value) <= std::ldexp(1.0, 150)
            && std::abs(value) <= double((std::numeric_limits<Output>::max)());
    }

    static bool initial(double estimate, Output& result)
    {
        if (!allowed(estimate)) return false;
        Output const rounded = static_cast<Output>(estimate);
        if (!allowed(double(rounded))) return false;
        result = rounded;
        return true;
    }

    static double previous(Output value)
    { return double(std::nextafter(value, -(std::numeric_limits<Output>::infinity)())); }
    static double next(Output value)
    { return double(std::nextafter(value, (std::numeric_limits<Output>::infinity)())); }

    static bool adjacent(Output value, int direction, Output& result)
    {
        double const candidate = direction < 0 ? previous(value) : next(value);
        if (!allowed(candidate)) return false;
        result = static_cast<Output>(candidate);
        return true;
    }

    static bool even(Output value)
    {
        int exponent;
        std::frexp(double(value), &exponent);
        double const significand = std::ldexp(std::abs(double(value)),
            std::numeric_limits<Output>::digits - exponent);
        return (static_cast<std::uint64_t>(significand) & 1) == 0;
    }
};

template <typename Output>
struct rounding_cell<Output, true>
{
    static bool allowed(double value)
    {
        // This subset has exactly representable adjacent integers. For wider
        // native integers the rational converter remains the fallback. Range
        // endpoints with >53 value bits are far outside this subset, so their
        // double conversion cannot admit an out-of-range integral cast.
        return std::isfinite(value) && std::abs(value) <= std::ldexp(1.0, 52)
            && value >= double((std::numeric_limits<Output>::min)())
            && value <= double((std::numeric_limits<Output>::max)());
    }

    static bool initial(double estimate, Output& result)
    {
        double const limit = std::ldexp(1.0, 52);
        double const lower = (std::max)(-limit, double((std::numeric_limits<Output>::min)()));
        double const upper = (std::min)(limit, double((std::numeric_limits<Output>::max)()));
        // An estimate may land just beyond the supported subset at a tie.
        // Clamping only chooses a candidate; the exact cell must still pass.
        double const candidate = (std::max)(lower, (std::min)(upper, std::nearbyint(estimate)));
        if (!allowed(candidate)) return false;
        result = static_cast<Output>(candidate);
        return true;
    }
    static double previous(Output value)
    { return arithmetic_detail::rounded_sub(double(value), 1.0); }
    static double next(Output value)
    { return arithmetic_detail::rounded_add(double(value), 1.0); }
    static bool adjacent(Output value, int direction, Output& result)
    {
        double const candidate = direction < 0 ? previous(value) : next(value);
        if (!allowed(candidate)) return false;
        result = static_cast<Output>(candidate);
        return true;
    }
    static bool even(Output value)
    { return (static_cast<std::uint64_t>(std::abs(double(value))) & 1) == 0; }
};

}} // namespace exact_cartesian::construction_detail

namespace exact_cartesian
{

// Consume retained exact expansions from homogeneous_coordinate() with
// supported endpoint values. The same exponent/capacity proof as the original
// intersection filter applies; arbitrary unguarded expansions are not inputs.
template <typename Output>
inline typename std::enable_if<construction_detail::supported_output<Output>::value, bool>::type
try_round_quotient(construction_detail::expansion<32> const& numerator,
                   construction_detail::expansion<16> const& denominator, Output& output)
{
    using namespace construction_detail;
    if (!numerator.valid || !denominator.valid
        || denominator.sign() == 0) return false;
    if (numerator.sign() == 0) { output = Output(0); return true; }

    double const estimate = arithmetic_detail::rounded_div(numerator.estimate(), denominator.estimate());
    if (!std::isfinite(estimate) || std::abs(estimate) < std::ldexp(1.0, -150)
        || std::abs(estimate) > std::ldexp(1.0, 150)) return false;
    using cell = rounding_cell<Output>;
    Output candidate;
    if (!cell::initial(estimate, candidate)) return false;
    for (unsigned correction = 0; correction <= 4; ++correction)
    {
        int lower, upper;
        if (!compare_boundary(numerator, denominator, cell::previous(candidate), double(candidate), lower)
            || !compare_boundary(numerator, denominator, double(candidate), cell::next(candidate), upper)) return false;
        bool const even = cell::even(candidate);
        bool const above_lower = lower > 0 || (lower == 0 && even);
        bool const below_upper = upper < 0 || (upper == 0 && even);
        if (above_lower && below_upper) { output = candidate; return true; }
        if (correction == 4 || !cell::adjacent(candidate, above_lower ? 1 : -1, candidate)) return false;
    }
    return false;
}

template <typename Output>
inline typename std::enable_if<!construction_detail::supported_output<Output>::value, bool>::type
try_round_quotient(construction_detail::expansion<32> const&,
                   construction_detail::expansion<16> const&, Output&)
{ return false; }

// Only exact native binary64 endpoint values are accepted: never pass rounded
// approximations of implicit coordinates or inexact integer-to-double casts.
// Parallel/coincident lines, unsupported types/ranges/modes and failed bounded
// certification return false, leaving output unchanged for the rational path.
template <typename Output>
inline typename std::enable_if<construction_detail::supported_output<Output>::value, bool>::type
try_round_intersection(std::array<double, 8> const& values, std::size_t dimension, Output& output)
{
    using namespace construction_detail;
    if (dimension > 1 || !supported(values)) return false;
    expansion<32> numerator;
    expansion<16> denominator;
    if (!homogeneous_coordinate(values, dimension, numerator, denominator)) return false;
    return try_round_quotient(numerator, denominator, output);
}

template <typename Output>
inline typename std::enable_if<!construction_detail::supported_output<Output>::value, bool>::type
try_round_intersection(std::array<double, 8> const&, std::size_t, Output&)
{ return false; }

} // namespace exact_cartesian
}}}} // namespace boost::geometry::strategies::detail

#endif
