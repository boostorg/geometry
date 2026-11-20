// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_POINT_PRECISION_CACHE_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_POINT_PRECISION_CACHE_HPP

#include <climits>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <boost/assert.hpp>
#include <boost/geometry/strategies/cartesian/exact/arithmetic.hpp>
#include <boost/geometry/strategies/cartesian/exact/operation_context.hpp>
#include <boost/geometry/strategies/cartesian/exact/fixed_rounding.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

template <typename C>
using auxiliary_point_type = typename operation_detail::storage_for<C>::auxiliary_type;

template <typename C>
using precision_homogeneous_point = auxiliary_point_type<C>;
template <typename C>
constexpr std::size_t precision_numerator_bits = native_fixed_traits<C>::intersection_numerator_bits
    + native_fixed_traits<C>::intersection_denominator_bits + 1;
template <typename C>
constexpr std::size_t precision_denominator_bits = 2 * native_fixed_traits<C>::intersection_denominator_bits + 1;

namespace point_precision_detail
{

template <std::size_t Bits, typename Backend>
inline void check_component_capacity(Backend const& value)
{
    constexpr std::size_t limb_bits = sizeof(*value.limbs()) * CHAR_BIT;
    constexpr std::size_t full = Bits / limb_bits, remaining = Bits % limb_bits;
    if (value.size() <= full) return;
    if (remaining != 0 && value.size() == full + 1
        && (value.limbs()[value.size() - 1] >> remaining) == 0) return;
    throw std::overflow_error("Exact auxiliary point exceeds its derived coordinate/expression capacity");
}

template <typename C, std::size_t N, std::size_t D>
inline precision_homogeneous_point<C> promote(fixed_homogeneous<N, D> const& value)
{
    precision_homogeneous_point<C> output;
    if (!try_fixed_narrow(value, output))
        throw std::overflow_error("Exact auxiliary point exceeds the supported native expression capacity");
    return output;
}

template <typename C>
class point_handle
{
public:
    point_handle() = default;

    template <std::size_t N, std::size_t D>
    static point_handle create(fixed_homogeneous<N, D> const& value)
    {
        auto& cache = operation_detail::require<C>().storage();
        auto point = promote<C>(value);
        cache.auxiliary.push_back(std::move(point));
        point_handle output;
        output.m_index = cache.auxiliary.size() - 1;
        return output;
    }

    explicit operator bool() const { return m_index != (std::numeric_limits<std::size_t>::max)(); }
    bool same(point_handle const& other) const
    {
        return m_index == other.m_index;
    }
    auxiliary_point_type<C> get() const
    {
        return entry();
    }
    template <std::size_t N, std::size_t D>
    bool copy_compact(fixed_homogeneous<N, D>& output) const
    {
        return try_fixed_narrow(entry(), output);
    }
private:
    auxiliary_point_type<C> const& entry() const
    {
        auto const& cache = operation_detail::require<C>().storage();
        BOOST_ASSERT(m_index < cache.auxiliary.size());
        return cache.auxiliary[m_index];
    }

    std::size_t m_index = (std::numeric_limits<std::size_t>::max)();
};

template <typename C, std::size_t N, std::size_t D>
inline precision_homogeneous_point<C> normalized(fixed_homogeneous<N, D> value)
{
    if (value.W == 0)
    {
        precision_homogeneous_point<C> invalid;
        invalid.W = 0;
        return invalid;
    }
    fixed_normalize(value);
    return promote<C>(value);
}

template <typename C>
inline fixed_integer<precision_numerator_bits<C>> numerator(exact_number const& value)
{
    fixed_integer<precision_numerator_bits<C>> output;
    check_component_capacity<precision_numerator_bits<C>>(value.backend().num());
    output.backend() = value.backend().num();
    return output;
}

template <typename C>
inline fixed_integer<precision_denominator_bits<C>> denominator(exact_number const& value)
{
    fixed_integer<precision_denominator_bits<C>> output;
    check_component_capacity<precision_denominator_bits<C>>(value.backend().denom());
    output.backend() = value.backend().denom();
    return output;
}

template <typename C>
inline precision_homogeneous_point<C> replace(precision_homogeneous_point<C> const& old,
                                           std::size_t dimension, exact_number const& coordinate)
{
    if (dimension > 1) throw std::out_of_range("Exact Cartesian coordinate dimension");
    if (old.W <= 0) throw std::domain_error("A finite exact point is required");
    auto const n = numerator<C>(coordinate);
    auto const d = denominator<C>(coordinate);
    auto const common = fixed_gcd(old.W, d);
    auto const first_scale = d / common, second_scale = old.W / common;
    constexpr std::size_t nbits = precision_numerator_bits<C> + precision_denominator_bits<C>;
    constexpr std::size_t dbits = 2 * precision_denominator_bits<C>;
    fixed_homogeneous<nbits, dbits> output;
    output.X = dimension == 0 ? fixed_multiply<nbits>(n, second_scale) : fixed_multiply<nbits>(old.X, first_scale);
    output.Y = dimension == 1 ? fixed_multiply<nbits>(n, second_scale) : fixed_multiply<nbits>(old.Y, first_scale);
    output.W = fixed_multiply<dbits>(old.W, first_scale);
    return normalized<C>(std::move(output));
}

template <typename C>
inline precision_homogeneous_point<C> literal(exact_number const& x, exact_number const& y)
{
    precision_homogeneous_point<C> zero;
    return replace<C>(replace<C>(zero, 0, x), 1, y);
}

template <typename C, typename Input>
inline precision_homogeneous_point<C> input(Input x, Input y,
    typename std::enable_if<fixed_native_supported<Input>::value>::type* = nullptr)
{
    return promote<C>(fixed_input_homogeneous(x, y));
}

template <typename C>
inline precision_homogeneous_point<C> input(exact_number const& x, exact_number const& y) { return literal<C>(x, y); }

template <typename C>
inline precision_homogeneous_point<C> intersection(precision_homogeneous_point<C> const& a,
    precision_homogeneous_point<C> const& b, precision_homogeneous_point<C> const& c,
    precision_homogeneous_point<C> const& d)
{
    constexpr std::size_t linear = precision_numerator_bits<C> + precision_denominator_bits<C> + 1;
    constexpr std::size_t constant = 2 * precision_numerator_bits<C> + 1;
    constexpr std::size_t nbits = linear + constant + 1, dbits = 2 * linear + 1;
    auto line = [](precision_homogeneous_point<C> const& p, precision_homogeneous_point<C> const& q)
    {
        fixed_line<linear, constant> output;
        output.A = fixed_multiply<linear>(p.Y, q.W) - fixed_multiply<linear>(p.W, q.Y);
        output.B = fixed_multiply<linear>(p.W, q.X) - fixed_multiply<linear>(p.X, q.W);
        output.C = fixed_multiply<constant>(p.X, q.Y) - fixed_multiply<constant>(p.Y, q.X);
        return output;
    };
    auto const first = line(a, b), second = line(c, d);
    fixed_homogeneous<nbits, dbits> output;
    output.X = fixed_multiply<nbits>(first.B, second.C) - fixed_multiply<nbits>(first.C, second.B);
    output.Y = fixed_multiply<nbits>(first.C, second.A) - fixed_multiply<nbits>(first.A, second.C);
    output.W = fixed_multiply<dbits>(first.A, second.B) - fixed_multiply<dbits>(first.B, second.A);
    return normalized<C>(std::move(output));
}

template <std::size_t N, std::size_t D>
inline interval bounds(fixed_homogeneous<N, D> const& point, std::size_t dimension)
{
    double rounded;
    if (point.W <= 0 || !try_fixed_round(point, dimension, rounded) || !std::isfinite(rounded))
        return {0, (std::numeric_limits<double>::infinity)()};
    if (fixed_coordinate_compare(point, fixed_input_homogeneous(rounded, rounded), dimension) == 0)
        return {rounded, 0};
    double const width = (std::max)(
        arithmetic_detail::rounded_sub(arithmetic_detail::upward(rounded), rounded),
        arithmetic_detail::rounded_sub(rounded, arithmetic_detail::downward(rounded)));
    return {rounded, width};
}

} // namespace point_precision_detail
}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

#endif
