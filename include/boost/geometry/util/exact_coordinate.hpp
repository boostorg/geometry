// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_UTIL_EXACT_COORDINATE_HPP
#define BOOST_GEOMETRY_UTIL_EXACT_COORDINATE_HPP

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

#include <boost/geometry/strategies/cartesian/exact/rational_rounding.hpp>
#include <boost/geometry/util/coordinate_cast.hpp>

namespace boost { namespace geometry
{

namespace traits
{

// Specialize for an owning, copyable, nothrow-swappable Scalar with value
// semantics. zero() returns a Scalar representing zero. to_rational() must
// deterministically return the exact cpp_rational value (by value or const
// reference), or throw for invalid input. It must not round through binary64.
template <typename Scalar>
struct exact_coordinate_conversion;

template <>
struct exact_coordinate_conversion<boost::multiprecision::cpp_rational>
{
    using rational = boost::multiprecision::cpp_rational;

    static rational zero() { return 0; }
    static rational const& to_rational(rational const& value) { return value; }
};

// Exact ASCII decimal text: [+-]?(digits[.digits?]?|.digits)([eE][+-]?digits)?
// Whitespace, NaN, infinity, and base prefixes are not accepted. Both the
// number of fractional digits and the magnitude of the exponent are limited
// to 10000 to bound exponent-driven allocations. Integer digits are unbounded.
template <>
struct exact_coordinate_conversion<std::string>
{
    static std::string zero() { return "0"; }

    static boost::multiprecision::cpp_rational to_rational(std::string const& text)
    {
        using boost::multiprecision::cpp_int;
        using boost::multiprecision::cpp_rational;
        constexpr int limit = 10000;

        std::size_t pos = 0;
        bool const negative = read_sign(text, pos);
        cpp_int coefficient = 0;
        bool has_digits = false, fraction = false;
        int fractional_digits = 0;
        for (; pos < text.size(); ++pos)
        {
            char const ch = text[pos];
            if (ch == '.' && !fraction)
            {
                fraction = true;
            }
            else if (is_digit(ch))
            {
                if (fraction && ++fractional_digits > limit)
                {
                    throw std::out_of_range("Exact coordinate has too many fractional digits");
                }
                coefficient *= 10;
                coefficient += ch - '0';
                has_digits = true;
            }
            else { break; }
        }
        if (!has_digits) { throw std::invalid_argument("Invalid exact coordinate decimal"); }

        int exponent = 0;
        if (pos < text.size() && (text[pos] == 'e' || text[pos] == 'E'))
        {
            ++pos;
            bool const negative_exponent = read_sign(text, pos);
            std::size_t const first = pos;
            for (; pos < text.size() && is_digit(text[pos]); ++pos)
            {
                int const digit = text[pos] - '0';
                if (exponent > (limit - digit) / 10)
                {
                    throw std::out_of_range("Exact coordinate decimal exponent is out of range");
                }
                exponent = 10 * exponent + digit;
            }
            if (pos == first) { throw std::invalid_argument("Invalid exact coordinate decimal exponent"); }
            if (negative_exponent) { exponent = -exponent; }
        }
        if (pos != text.size()) { throw std::invalid_argument("Invalid exact coordinate decimal"); }
        if (coefficient == 0) { return 0; }
        if (negative) { coefficient = -coefficient; }

        int const scale = exponent - fractional_digits;
        cpp_int const power = boost::multiprecision::pow(cpp_int(10), unsigned(scale < 0 ? -scale : scale));
        if (scale < 0) { return cpp_rational(coefficient, power); }
        coefficient *= power;
        return cpp_rational(coefficient);
    }

private:
    static bool is_digit(char ch) { return ch >= '0' && ch <= '9'; }

    static bool read_sign(std::string const& text, std::size_t& pos)
    {
        if (pos == text.size() || (text[pos] != '-' && text[pos] != '+')) { return false; }
        return text[pos++] == '-';
    }
};

} // namespace traits

namespace detail { namespace exact_coordinate_detail
{

using std::swap;

template <typename Scalar>
void swap_scalar(Scalar& a, Scalar& b) noexcept(noexcept(swap(a, b)))
{
    swap(a, b);
}

inline void swap_scalar(boost::multiprecision::cpp_rational& a,
                        boost::multiprecision::cpp_rational& b) noexcept
{
    // rational_adaptor's swap is not annotated noexcept, but just swaps its
    // two cpp_int backends; neither swap allocates or throws.
    a.swap(b);
}

}} // namespace detail::exact_coordinate_detail

// Owns the authoritative scalar. binary64() is a cached round-to-nearest,
// ties-to-even view, not a replacement for the scalar. Exact zero rounds to
// +0; nonzero underflow preserves sign and overflow can produce infinity.
// This is a storage wrapper, not an arithmetic type or a strategy selection.
template <typename Scalar, typename Conversion = traits::exact_coordinate_conversion<Scalar>>
class exact_coordinate
{
    static_assert(noexcept(detail::exact_coordinate_detail::swap_scalar(
                      std::declval<Scalar&>(), std::declval<Scalar&>())),
                  "Exact coordinate scalars must support nothrow swap");
    static_assert(std::numeric_limits<double>::is_iec559 && std::numeric_limits<double>::radix == 2
                  && std::numeric_limits<double>::digits == 53
                  && std::numeric_limits<double>::min_exponent == -1021
                  && std::numeric_limits<double>::max_exponent == 1024,
                  "Exact coordinate views require IEEE binary64");
    using rounding = strategies::detail::exact_cartesian::rational_rounding::rounded_value<double>;

public:
    exact_coordinate() : exact_coordinate(Conversion::zero()) {}

    explicit exact_coordinate(Scalar value)
        : m_scalar(std::move(value))
        , m_view(strategies::detail::exact_cartesian::rational_rounding::round<double>(rational()))
    {}

    // Rvalues also copy: moving out just the scalar would invalidate the
    // source's view (and could make a decimal source impossible to parse).
    exact_coordinate(exact_coordinate const&) = default;

    exact_coordinate& operator=(exact_coordinate other) noexcept
    {
        swap(other);
        return *this;
    }

    void assign(Scalar value) { exact_coordinate(std::move(value)).swap(*this); }

    Scalar const& scalar() const noexcept { return m_scalar; }
    decltype(auto) rational() const { return Conversion::to_rational(m_scalar); }
    double binary64() const noexcept { return m_view.value; }
    bool binary64_is_exact() const noexcept { return m_view.exact; }

    friend bool operator==(exact_coordinate const& a, exact_coordinate const& b)
    {
        if (a.binary64() != b.binary64()) { return false; }
        // Equal heads alone cannot distinguish decimal values in the same rounding cell.
        if (a.binary64_is_exact() || b.binary64_is_exact())
            return a.binary64_is_exact() && b.binary64_is_exact();
        return a.rational() == b.rational();
    }

    friend bool operator!=(exact_coordinate const& a, exact_coordinate const& b) { return !(a == b); }

private:
    void swap(exact_coordinate& other) noexcept
    {
        detail::exact_coordinate_detail::swap_scalar(m_scalar, other.m_scalar);
        std::swap(m_view, other.m_view);
    }

    Scalar m_scalar;
    rounding m_view;
};

namespace detail
{

// Use the original token when reading decimal coordinates from WKT.
template <typename Conversion>
struct coordinate_cast<exact_coordinate<std::string, Conversion>>
{
    static exact_coordinate<std::string, Conversion> apply(std::string const& source)
    {
        return exact_coordinate<std::string, Conversion>(source);
    }
};

} // namespace detail

}} // namespace boost::geometry

#endif
