// Boost.Geometry
// Copyright (c) 2026
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BOUNDED_RADICAL_FIELD_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BOUNDED_RADICAL_FIELD_HPP

#include <array>
#include <cstddef>
#include <stdexcept>

#include <boost/geometry/strategies/cartesian/exact/arithmetic.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian { namespace round_buffer
{

// A local field Q(sqrt(d0), ..., sqrt(dk)), with independent rational square
// classes. Bit masks index products of the positive generators. Values must
// not outlive their field. Appending a generator preserves old values.
class bounded_radical_field
{
public:
    static constexpr std::size_t max_rank = 4;
    static constexpr std::size_t max_terms = std::size_t(1) << max_rank;

private:
    using coefficients = std::array<exact_number, max_terms>;

public:
    class value
    {
        friend class bounded_radical_field;
        bounded_radical_field const* m_field = nullptr;
        coefficients m_terms{};
    };

    bounded_radical_field() { m_products[0] = 1; }
    bounded_radical_field(bounded_radical_field const&) = delete;
    bounded_radical_field& operator=(bounded_radical_field const&) = delete;

    std::size_t rank() const { return m_rank; }

    value rational(exact_number const& n) const
    {
        value v = zero();
        v.m_terms[0] = n;
        return v;
    }

    // Return false without changing the field or out if another independent
    // generator would exceed the rank bound. No factorization is needed.
    bool try_sqrt(exact_number const& d, value& out)
    {
        if (d < 0) { throw std::domain_error("Radical field requires nonnegative radicands"); }
        if (d == 0) { out = zero(); return true; }
        std::size_t const width = term_count();
        exact_number factor;
        for (std::size_t i = 0; i < width; ++i)
        {
            if (square_root_if_rational(d / m_products[i], factor))
            {
                auto v = zero();
                v.m_terms[i] = factor;
                out = v;
                return true;
            }
        }
        if (m_rank == max_rank) { return false; }
        for (std::size_t i = 0; i < width; ++i) { m_products[i + width] = m_products[i] * d; }
        ++m_rank;
        auto v = zero();
        v.m_terms[width] = 1;
        out = v;
        return true;
    }

    value add(value const& a, value const& b) const
    {
        check(a); check(b);
        auto out = zero();
        for (std::size_t i = 0; i < term_count(); ++i) { out.m_terms[i] = a.m_terms[i] + b.m_terms[i]; }
        return out;
    }

    value subtract(value const& a, value const& b) const
    {
        check(a); check(b);
        auto out = zero();
        for (std::size_t i = 0; i < term_count(); ++i) { out.m_terms[i] = a.m_terms[i] - b.m_terms[i]; }
        return out;
    }

    value scale(value const& a, exact_number const& q) const
    {
        check(a);
        auto out = zero();
        for (std::size_t i = 0; i < term_count(); ++i) { out.m_terms[i] = a.m_terms[i] * q; }
        return out;
    }

    value multiply(value const& a, value const& b) const
    {
        check(a); check(b);
        auto out = zero();
        out.m_terms = product(a.m_terms, b.m_terms, term_count());
        return out;
    }

    int sign(value const& a) const
    {
        check(a);
        return sign(a.m_terms, term_count());
    }

    // Certify exact rational representability of a quotient without full
    // field division. On an irrational quotient leave out unchanged.
    bool try_rational_quotient(value const& n, value const& d, exact_number& out) const
    {
        check(n); check(d);
        std::size_t i = 0;
        while (i < term_count() && d.m_terms[i] == 0) { ++i; }
        if (i == term_count()) { throw std::domain_error("Zero radical-field denominator"); }
        exact_number const q = n.m_terms[i] / d.m_terms[i];
        for (std::size_t j = 0; j < term_count(); ++j)
        {
            if (n.m_terms[j] != q * d.m_terms[j]) { return false; }
        }
        out = q;
        return true;
    }

private:
    std::size_t term_count() const { return std::size_t(1) << m_rank; }

    value zero() const
    {
        value v;
        v.m_field = this;
        return v;
    }

    void check(value const& v) const
    {
        // A default value is a field-independent zero.
        if (v.m_field != nullptr && v.m_field != this)
        {
            throw std::invalid_argument("Values belong to different radical fields");
        }
    }

    static bool square_root_if_rational(exact_number const& a, exact_number& root)
    {
        using boost::multiprecision::cpp_int;
        cpp_int const d = boost::multiprecision::denominator(a);
        cpp_int remainder;
        // sqrt(n/d) = sqrt(n*d)/d. a is positive here.
        cpp_int const integer_root = boost::multiprecision::sqrt(boost::multiprecision::numerator(a) * d, remainder);
        if (remainder != 0) { return false; }
        root = exact_number(integer_root) / d;
        return true;
    }

    coefficients product(coefficients const& a, coefficients const& b, std::size_t width) const
    {
        coefficients out{};
        for (std::size_t i = 0; i < width; ++i)
        {
            if (a[i] == 0) { continue; }
            for (std::size_t j = 0; j < width; ++j)
            {
                if (b[j] != 0) { out[i ^ j] += a[i] * b[j] * m_products[i & j]; }
            }
        }
        return out;
    }

    int sign(coefficients const& a, std::size_t width) const
    {
        if (width == 1) { return a[0] > 0 ? 1 : a[0] < 0 ? -1 : 0; }
        width /= 2;
        coefficients b{};
        for (std::size_t i = 0; i < width; ++i) { b[i] = a[i + width]; }
        int const sa = sign(a, width), sb = sign(b, width);
        if (sa == 0) { return sb; }
        if (sb == 0 || sa == sb) { return sa; }
        // Opposite signs: compare magnitudes in the lower field. Squaring
        // without this sign check would get some predicates backwards.
        auto norm = product(a, a, width);
        auto const b2 = product(b, b, width);
        for (std::size_t i = 0; i < width; ++i) { norm[i] -= m_products[width] * b2[i]; }
        return sa * sign(norm, width);
    }

    std::size_t m_rank = 0;
    coefficients m_products{};
};

}}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian::round_buffer

#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BOUNDED_RADICAL_FIELD_HPP
