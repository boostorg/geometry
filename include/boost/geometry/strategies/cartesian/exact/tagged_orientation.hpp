// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_TAGGED_ORIENTATION_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_TAGGED_ORIENTATION_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>

#include <boost/assert.hpp>
#include <boost/config.hpp>
#include <boost/geometry/strategies/cartesian/exact/fixed_arithmetic.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian { namespace tagged_orientation
{

constexpr int digit_bits = 21;
constexpr std::uint64_t radix = std::uint64_t{1} << digit_bits;
constexpr double inverse_radix = 1.0 / radix;
constexpr int block_of(int exponent)
{
    return exponent < 0 ? -((-exponent + digit_bits - 1) / digit_bits)
                        : exponent / digit_bits;
}

// Every digit d[k] represents d[k] * radix^k, regardless of its sign.
// Degree tracks the lowest possible exponent, and Monomials bounds the
// weighted L1 norm, including partial sums before and after carries.
template <int Degree, unsigned Monomials>
class number
{
public:
    static constexpr int degree = Degree;
    static constexpr unsigned monomials = Monomials;
    static constexpr int min_block = Degree * block_of(-1074);
    static constexpr int end_block =
        (Degree * 1024 + int(fixed_detail::ceiling_log2(Monomials)) + digit_bits - 1) / digit_bits;
    static constexpr unsigned capacity = end_block - min_block;

    number() noexcept {} // Inactive digits are neither read nor initialized.
    number(number const& other) noexcept : m_first(other.m_first), m_last(other.m_last)
    {
        std::copy_n(other.data(m_first), size(), data(m_first));
    }
    number& operator=(number const& other) noexcept
    {
        if (this != &other)
        {
            m_first = other.m_first;
            m_last = other.m_last;
            std::copy_n(other.data(m_first), size(), data(m_first));
        }
        return *this;
    }

    bool empty() const { return m_first == m_last; }
    int first() const { return m_first; }
    int last() const { return m_last; }
    int size() const { return m_last - m_first; }
    double const* data(int k) const
    {
        BOOST_ASSERT(min_block <= k && k <= end_block);
        return m_digits.data() + (k - min_block);
    }
    int sign() const { return empty() ? 0 : *data(m_last - 1) < 0 ? -1 : 1; }

    static number input(double x)
    {
        static_assert(Degree == 1 && Monomials == 1, "Decode only native inputs");
        auto const value = fixed_detail::decompose(x, std::false_type{});
        number result;
        if (value.significand == 0) return result;
        result.m_first = result.m_last = block_of(value.exponent);
        int offset = value.exponent - digit_bits * result.m_first;
        auto bits = value.significand;
        do
        {
            int const width = digit_bits - offset;
            double const digit = double((bits & ((std::uint64_t{1} << width) - 1)) << offset);
            result.append(value.negative ? -digit : digit);
            bits >>= width;
            offset = 0;
        }
        while (bits != 0);
        return result;
    }

    template <typename First, typename... Rest>
    static number sum(First const& first, Rest const&... rest)
    {
        static_assert(First::degree == Degree && ((Rest::degree == Degree) && ...),
                      "Only the same polynomial degree can be added");
        static_assert(First::monomials + (0u + ... + Rest::monomials) <= Monomials,
                      "Insufficient weighted L1 budget");
        constexpr auto fan_in = First::fan_in + (0u + ... + Rest::fan_in);
        static_assert(fan_in <= (std::uint64_t{1} << (53 - 2 * digit_bits)),
                      "Raw coefficient and its incoming carry must fit exactly in binary64");
        int begin = end_block, end = min_block;
        auto include = [&](auto const& term)
        {
            if (term.empty()) return;
            begin = (std::min)(begin, term.first());
            end = (std::max)(end, term.last());
        };
        include(first);
        (include(rest), ...);
        number result;
        if (begin >= end) return result;

        BOOST_ASSERT(min_block <= begin && end <= end_block);
        result.m_first = begin;
        result.m_last = end;
        double* const digits = result.data(begin);
        std::fill_n(digits, result.size(), 0.0);
        first.accumulate(digits, begin);
        (rest.accumulate(digits, begin), ...);
        result.carry();
        return result;
    }

private:
    double* data(int k)
    {
        BOOST_ASSERT(min_block <= k && k <= end_block);
        return m_digits.data() + (k - min_block);
    }

    void append(double digit)
    {
        BOOST_ASSERT(m_last < end_block);
        *data(m_last++) = digit;
    }

    void carry()
    {
        double carry = 0;
        auto split = [&](double value)
        {
            // The scaling is exact. Truncate toward zero so that the new
            // digit and the carried part have the sign of value.
            carry = std::trunc(value * inverse_radix);
            return value - carry * double(radix);
        };
        for (auto* digit = data(m_first); digit != data(m_last); ++digit)
            *digit = split(*digit + carry);
        while (carry != 0)
            append(split(carry));
        while (m_last > m_first && *data(m_last - 1) == 0) --m_last;
        while (m_first < m_last && *data(m_first) == 0) ++m_first;
    }

    std::array<double, capacity> m_digits;
    int m_first = 0;
    int m_last = 0;
};

template <typename A>
struct term
{
    static constexpr int degree = A::degree;
    static constexpr unsigned monomials = A::monomials;
    static constexpr unsigned fan_in = 1;
    A const& a;
    bool negate;
    bool empty() const { return a.empty(); }
    int first() const { return a.first(); }
    int last() const { return a.last(); }
    void accumulate(double* digits, int begin) const
    {
        if (empty()) return;
        auto const* src = a.data(first());
        auto* dst = digits + (first() - begin);
        for (int i = 0; i != a.size(); ++i)
            dst[i] += negate ? -src[i] : src[i];
    }
};

template <typename A, typename B>
struct product
{
    static constexpr int degree = A::degree + B::degree;
    static constexpr unsigned monomials = A::monomials * B::monomials;
    static constexpr unsigned fan_in = A::capacity < B::capacity ? A::capacity : B::capacity;
    A const& a;
    B const& b;
    bool negate;
    bool empty() const { return a.empty() || b.empty(); }
    int first() const { return a.first() + b.first(); }
    int last() const { return a.last() + b.last() - 1; }

    void accumulate(double* digits, int begin) const
    {
        if (empty()) return;
        auto const* x = a.data(a.first());
        auto const* y = b.data(b.first());
        int n = a.size(), m = b.size();
        if (n > m) { std::swap(x, y); std::swap(n, m); }
        auto* dst = digits + (first() - begin);
        for (int i = 0; i != n; ++i)
        {
            double const digit = negate ? -x[i] : x[i];
            if (digit == 0) continue;
            for (int j = 0; j != m; ++j)
                dst[i + j] += digit * y[j];
        }
    }
};

template <typename A> term(A const&, bool) -> term<A>;
template <typename A, typename B> product(A const&, B const&, bool) -> product<A, B>;

template <typename A, typename B, typename C, typename D>
auto det2(A const& a, B const& b, C const& c, D const& d)
{
    using result = number<A::degree + B::degree, A::monomials * B::monomials + C::monomials * D::monomials>;
    return result::sum(product{a, b, false}, product{c, d, true});
}

using native = number<1, 1>;
using producers = std::array<double, 8>;

struct line
{
    number<1, 2> a, b;
    number<2, 2> c;
};

inline line make_line(double x0, double y0, double x1, double y1)
{
    auto const x = native::input(x0), y = native::input(y0);
    auto const u = native::input(x1), v = native::input(y1);
    return {number<1, 2>::sum(term{y, false}, term{v, true}),
            number<1, 2>::sum(term{u, false}, term{x, true}),
            det2(x, v, u, y)};
}

struct intersection
{
    number<3, 8> x, y;
    number<2, 8> w;
};

inline intersection make_intersection(producers const& p)
{
    auto const a = make_line(p[0], p[1], p[2], p[3]);
    auto const b = make_line(p[4], p[5], p[6], p[7]);
    auto const w = det2(a.a, b.b, a.b, b.a);
    if (w.empty()) throw std::domain_error("Tagged orientation requires unique line intersections");
    return {det2(a.b, b.c, a.c, b.b), det2(a.c, b.a, a.a, b.c), w};
}

using minor_xy = number<5, 128>;
using minor_w = number<6, 128>;
using determinant = number<8, 3072>;

inline int orientation(intersection const& a, intersection const& b, intersection const& c)
{
    int const denominator_sign = a.w.sign() * b.w.sign() * c.w.sign();
    if (denominator_sign == 0) throw std::domain_error("Tagged orientation requires finite intersections");
    auto const x = det2(a.y, b.w, a.w, b.y);
    auto const y = det2(a.w, b.x, a.x, b.w);
    auto const w = det2(a.x, b.y, a.y, b.x);
    auto const det = determinant::sum(product{x, c.x, false}, product{y, c.y, false}, product{w, c.w, false});
    return det.sign() * denominator_sign;
}

// Keep this uncommon scratch frame out of the regular predicate.
BOOST_NOINLINE inline int from_producers(producers const& a, producers const& b, producers const& c)
{
    return orientation(make_intersection(a), make_intersection(b), make_intersection(c));
}

}}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian::tagged_orientation

#endif
