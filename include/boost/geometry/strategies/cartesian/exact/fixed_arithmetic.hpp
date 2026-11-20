// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_FIXED_ARITHMETIC_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_FIXED_ARITHMETIC_HPP

#include <algorithm>
#include <array>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <boost/core/bit.hpp>
#include <boost/multiprecision/cpp_int.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{
namespace exact_cartesian
{

template <std::size_t Bits>
using fixed_integer = boost::multiprecision::number
    <boost::multiprecision::cpp_int_backend<Bits, Bits,
        boost::multiprecision::signed_magnitude, boost::multiprecision::checked, void>,
     boost::multiprecision::et_off>;

template <std::size_t Bits, typename Integer>
inline bool fixed_fits(Integer const& value)
{
    auto const& backend = value.backend();
    constexpr std::size_t limb_bits = sizeof(*backend.limbs()) * CHAR_BIT;
    constexpr std::size_t whole = Bits / limb_bits;
    constexpr std::size_t remainder = Bits % limb_bits;
    if (backend.size() <= whole) { return true; }
    return remainder != 0 && backend.size() == whole + 1
        && (backend.limbs()[backend.size() - 1] >> remainder) == 0;
}

template <std::size_t NumeratorBits, std::size_t DenominatorBits = NumeratorBits>
struct fixed_homogeneous
{
    using numerator_type = fixed_integer<NumeratorBits>;
    using denominator_type = fixed_integer<DenominatorBits>;
    numerator_type X{}, Y{};
    denominator_type W{1};
};

// Inspect active limbs before copying: converting a small value through its
// worst-case point format first would still copy/clear that entire format.
template <std::size_t N, std::size_t D, std::size_t AN, std::size_t AD>
inline bool try_fixed_narrow(fixed_homogeneous<AN, AD> const& input, fixed_homogeneous<N, D>& output)
{
    if (!fixed_fits<N>(input.X) || !fixed_fits<N>(input.Y) || !fixed_fits<D>(input.W)) { return false; }
    output.X.backend() = input.X.backend();
    output.Y.backend() = input.Y.backend();
    output.W.backend() = input.W.backend();
    return true;
}

template <std::size_t LinearBits, std::size_t ConstantBits>
struct fixed_line
{
    fixed_integer<LinearBits> A{}, B{};
    fixed_integer<ConstantBits> C{};
};

template <typename Coordinate>
struct fixed_native_supported : std::integral_constant<bool,
    (std::is_integral<Coordinate>::value && std::numeric_limits<Coordinate>::digits <= 64)
    || std::is_same<Coordinate, float>::value || std::is_same<Coordinate, double>::value>
{};

namespace fixed_detail
{

constexpr unsigned ceiling_log2(unsigned value)
{
    unsigned result = 0;
    for (--value; value; value >>= 1) { ++result; }
    return result;
}

constexpr std::size_t maximum(std::size_t a, std::size_t b) { return a < b ? b : a; }

template <typename Coordinate, bool Integral = std::is_integral<Coordinate>::value>
struct parameters
{
    enum : unsigned
    {
        scale_bits = std::numeric_limits<Coordinate>::digits - std::numeric_limits<Coordinate>::min_exponent,
        coordinate_bits = std::numeric_limits<Coordinate>::max_exponent + scale_bits
    };
};

template <typename Coordinate>
struct parameters<Coordinate, true>
{
    enum : unsigned
    {
        scale_bits = 0,
        coordinate_bits = std::numeric_limits<Coordinate>::digits + std::numeric_limits<Coordinate>::is_signed
    };
};

template <typename Coordinate> struct binary_format;

template <> struct binary_format<double>
{
    using word = std::uint64_t;
    enum : unsigned { fraction_bits = 52, exponent_bits = 11 };
};

template <> struct binary_format<float>
{
    using word = std::uint32_t;
    enum : unsigned { fraction_bits = 23, exponent_bits = 8 };
};

struct dyadic_component
{
    std::uint64_t significand;
    int exponent;
    bool negative;
};

template <typename Coordinate>
inline dyadic_component decompose(Coordinate value, std::true_type)
{
    bool const negative = value < 0;
    auto const magnitude = static_cast<std::uint64_t>(value);
    return {negative ? std::uint64_t(0) - magnitude : magnitude, 0, negative};
}

template <typename Coordinate>
inline dyadic_component decompose(Coordinate value, std::false_type)
{
    using format = binary_format<Coordinate>;
    using word = typename format::word;
    static_assert(std::numeric_limits<Coordinate>::is_iec559
        && std::numeric_limits<Coordinate>::radix == 2
        && std::numeric_limits<Coordinate>::digits == format::fraction_bits + 1
        && sizeof(Coordinate) == sizeof(word), "IEEE binary32/binary64 input required");
    word bits;
    std::memcpy(&bits, &value, sizeof(bits));
    word const mask = (word(1) << format::exponent_bits) - 1;
    word const exponent = (bits >> format::fraction_bits) & mask;
    if (exponent == mask) { throw std::domain_error("Fixed Cartesian input must be finite"); }
    std::uint64_t significand = bits & ((word(1) << format::fraction_bits) - 1);
    if (exponent != 0) { significand |= word(1) << format::fraction_bits; }
    if (significand == 0) { return {0, 0, false}; }
    int const zeros = boost::core::countr_zero(significand);
    int const bias = (1 << (format::exponent_bits - 1)) - 1;
    return {significand >> zeros,
        int(exponent == 0 ? 1 : exponent) - bias - int(format::fraction_bits) + zeros,
        (bits >> (format::fraction_bits + format::exponent_bits)) != 0};
}

} // namespace fixed_detail

template <std::size_t N, std::size_t D, typename Coordinate>
inline bool try_fixed_input_homogeneous(Coordinate x, Coordinate y, fixed_homogeneous<N, D>& output)
{
    auto const a = fixed_detail::decompose(x, std::is_integral<Coordinate>());
    auto const b = fixed_detail::decompose(y, std::is_integral<Coordinate>());
    int const exponent = (std::min)(0, (std::min)(a.exponent, b.exponent));
    if (std::size_t(-exponent) >= D) { return false; }
    auto fits = [&](fixed_detail::dyadic_component const& value)
    {
        return value.significand == 0 || std::size_t(value.exponent - exponent)
            + 64 - boost::core::countl_zero(value.significand) <= N;
    };
    if (!fits(a) || !fits(b)) { return false; }
    auto assign = [&](fixed_integer<N>& target, fixed_detail::dyadic_component const& value)
    {
        target = value.significand;
        if (value.significand != 0) { target <<= value.exponent - exponent; }
        if (value.negative) { target = -target; }
    };
    assign(output.X, a);
    assign(output.Y, b);
    output.W = 1;
    output.W <<= -exponent;
    return true;
}

// No specialization for arbitrary rationals, long double, or wide integers.
// Callers must retain a separate slow representation for those values, and for
// recursive constructions not covered by a proved fixed polynomial capacity.
template <typename Coordinate, typename = void>
struct native_fixed_traits;

template <typename Coordinate>
struct native_fixed_traits<Coordinate,
    typename std::enable_if<fixed_native_supported<Coordinate>::value>::type>
    : fixed_detail::parameters<Coordinate>
{
    static constexpr unsigned coordinate_bits = fixed_detail::parameters<Coordinate>::coordinate_bits;
    static constexpr unsigned scale_bits = fixed_detail::parameters<Coordinate>::scale_bits;
    static constexpr unsigned line_linear_bits = coordinate_bits + 1;
    static constexpr unsigned line_constant_bits = 2 * coordinate_bits + 1;
    static constexpr unsigned intersection_numerator_bits = 3 * coordinate_bits + 3;
    static constexpr unsigned intersection_denominator_bits = 2 * coordinate_bits + scale_bits + 3;

    using coordinate_integer = fixed_integer<coordinate_bits>;
    using input_type = fixed_homogeneous<coordinate_bits, scale_bits + 1>;
    using line_type = fixed_line<line_linear_bits, line_constant_bits>;
    using intersection_type = fixed_homogeneous<intersection_numerator_bits, intersection_denominator_bits>;
};

template <typename Coordinate>
constexpr unsigned native_fixed_traits<Coordinate,
    typename std::enable_if<fixed_native_supported<Coordinate>::value>::type>::coordinate_bits;
template <typename Coordinate>
constexpr unsigned native_fixed_traits<Coordinate,
    typename std::enable_if<fixed_native_supported<Coordinate>::value>::type>::scale_bits;
template <typename Coordinate>
constexpr unsigned native_fixed_traits<Coordinate,
    typename std::enable_if<fixed_native_supported<Coordinate>::value>::type>::line_linear_bits;
template <typename Coordinate>
constexpr unsigned native_fixed_traits<Coordinate,
    typename std::enable_if<fixed_native_supported<Coordinate>::value>::type>::line_constant_bits;
template <typename Coordinate>
constexpr unsigned native_fixed_traits<Coordinate,
    typename std::enable_if<fixed_native_supported<Coordinate>::value>::type>::intersection_numerator_bits;
template <typename Coordinate>
constexpr unsigned native_fixed_traits<Coordinate,
    typename std::enable_if<fixed_native_supported<Coordinate>::value>::type>::intersection_denominator_bits;

// Unit-coefficient monomials, counted with multiplicity: |P| < Terms*2^(b*Degree).
// Constants such as the grid denominator also have magnitude <2^b. Every
// intermediate must obey this term budget; arithmetic is checked, not modular.
template <typename Coordinate, unsigned Degree, unsigned Terms>
struct native_fixed_polynomial
{
    static_assert(Degree >= 1 && Degree <= 8, "Prove a new capacity for higher degrees");
    static_assert(Terms >= 1, "A polynomial needs a positive term budget");
    static constexpr unsigned bits = Degree * native_fixed_traits<Coordinate>::coordinate_bits
                                   + fixed_detail::ceiling_log2(Terms);
    using type = fixed_integer<bits>;
};

template <typename Coordinate, unsigned Degree, unsigned Terms>
constexpr unsigned native_fixed_polynomial<Coordinate, Degree, Terms>::bits;

namespace fixed_detail
{

template <typename Coordinate>
inline typename native_fixed_traits<Coordinate>::coordinate_integer
value(Coordinate input, std::true_type)
{
    return typename native_fixed_traits<Coordinate>::coordinate_integer(input);
}

template <typename Coordinate>
inline typename native_fixed_traits<Coordinate>::coordinate_integer
value(Coordinate input, std::false_type)
{
    using format = binary_format<Coordinate>;
    using word = typename format::word;
    using integer = typename native_fixed_traits<Coordinate>::coordinate_integer;
    static_assert(std::numeric_limits<Coordinate>::is_iec559
        && std::numeric_limits<Coordinate>::radix == 2
        && std::numeric_limits<Coordinate>::digits == format::fraction_bits + 1
        && sizeof(Coordinate) == sizeof(word), "IEEE binary32/binary64 input required");
    word bits;
    std::memcpy(&bits, &input, sizeof(bits));
    word const exponent_mask = (word(1) << format::exponent_bits) - 1;
    word const exponent = (bits >> format::fraction_bits) & exponent_mask;
    if (exponent == exponent_mask) { throw std::domain_error("Fixed Cartesian input must be finite"); }
    word significand = bits & ((word(1) << format::fraction_bits) - 1);
    if (exponent != 0) { significand |= word(1) << format::fraction_bits; }
    integer result(significand);
    // Scaling by 2^149/2^1074 changes the normal exponent shift to e-1;
    // subnormal significands are already integers on this common grid.
    if (exponent != 0) { result <<= unsigned(exponent - 1); }
    if ((bits >> (format::fraction_bits + format::exponent_bits)) != 0) { result = -result; }
    return result;
}

} // namespace fixed_detail

// Returns N such that input == N / 2^native_fixed_traits<Coordinate>::scale_bits.
template <typename Coordinate>
inline typename native_fixed_traits<Coordinate>::coordinate_integer fixed_value(Coordinate input)
{
    return fixed_detail::value(input, std::is_integral<Coordinate>());
}

template <typename Integer>
inline Integer fixed_gcd(Integer a, Integer b)
{
    if (a < 0) { a = -a; }
    if (b < 0) { b = -b; }
    while (b != 0)
    {
        Integer remainder = a % b;
        a = std::move(b);
        b = std::move(remainder);
    }
    return a;
}

// Boost's wide fixed-by-fixed multiplication may allocate Karatsuba scratch.
// Multiplication by a native 32-bit digit uses its allocator-free limb kernel.
// All partial sums are nonnegative and <=|a*b|, so checked capacity is sufficient
// whenever the final product fits. Apply the sign only after shifting.
template <std::size_t Bits, std::size_t A, std::size_t B>
inline fixed_integer<Bits> fixed_multiply(fixed_integer<A> const& a, fixed_integer<B> const& b)
{
    if (a == 0 || b == 0) { return fixed_integer<Bits>(0); }
    bool const negative = (a < 0) != (b < 0);
    fixed_integer<Bits> left(a);
    fixed_integer<B> right(b);
    if (left < 0) { left = -left; }
    if (right < 0) { right = -right; }
    fixed_integer<Bits> result(0);
    std::size_t shift = 0;
    while (right != 0)
    {
        auto const digit = static_cast<std::uint32_t>(right.backend().limbs()[0] & 0xffffffffu);
        if (digit != 0)
        {
            fixed_integer<Bits> term = left * digit;
            term <<= shift;
            result += term;
        }
        right >>= 32;
        shift += 32;
    }
    return negative ? fixed_integer<Bits>(-result) : result;
}

template <std::size_t N, std::size_t D>
inline void fixed_normalize(fixed_homogeneous<N, D>& point)
{
    if (point.W == 0) { throw std::domain_error("Intersection has no unique finite Cartesian point"); }
    if (point.W < 0) { point.X = -point.X; point.Y = -point.Y; point.W = -point.W; }
    using integer = fixed_integer<fixed_detail::maximum(N, D)>;
    integer const common = fixed_gcd(fixed_gcd(integer(point.X), integer(point.Y)), integer(point.W));
    if (common != 1)
    {
        point.X = typename fixed_homogeneous<N, D>::numerator_type(integer(point.X) / common);
        point.Y = typename fixed_homogeneous<N, D>::numerator_type(integer(point.Y) / common);
        point.W = typename fixed_homogeneous<N, D>::denominator_type(integer(point.W) / common);
    }
}

template <std::size_t Bits, std::size_t Size>
inline unsigned remove_binary_scale(std::array<fixed_integer<Bits>, Size>& values, unsigned scale)
{
    if (scale == 0) return 0;
    unsigned shift = scale;
    for (auto const& value : values)
    {
        if (value == 0) continue;
        auto const magnitude = value < 0 ? fixed_integer<Bits>(-value) : value;
        unsigned const zeros = boost::multiprecision::lsb(magnitude);
        if (zeros < shift) shift = zeros;
    }
    if (shift != 0)
    {
        for (auto& value : values)
        {
            bool const negative = value < 0;
            if (negative) value = -value;
            value >>= shift;
            if (negative) value = -value;
        }
    }
    return shift;
}

template <typename Coordinate>
inline typename native_fixed_traits<Coordinate>::input_type fixed_input_homogeneous(Coordinate x, Coordinate y)
{
    using traits = native_fixed_traits<Coordinate>;
    std::array<typename traits::coordinate_integer, 2> values{{fixed_value(x), fixed_value(y)}};
    unsigned const removed = remove_binary_scale(values, traits::scale_bits);
    typename traits::input_type result;
    result.X = values[0];
    result.Y = values[1];
    result.W <<= traits::scale_bits - removed;
    return result;
}

// The line equation is on the common integer grid, not yet scaled Cartesian X/Y.
template <typename Coordinate>
inline typename native_fixed_traits<Coordinate>::line_type fixed_native_line(std::array<Coordinate, 4> const& values)
{
    using traits = native_fixed_traits<Coordinate>;
    using linear = fixed_integer<traits::line_linear_bits>;
    auto const ax = fixed_value(values[0]), ay = fixed_value(values[1]);
    auto const bx = fixed_value(values[2]), by = fixed_value(values[3]);
    return {linear(ay) - linear(by), linear(bx) - linear(ax),
            fixed_multiply<traits::line_constant_bits>(ax, by)
              - fixed_multiply<traits::line_constant_bits>(ay, bx)};
}

template <typename Coordinate>
inline typename native_fixed_traits<Coordinate>::intersection_type
direct_native_homogeneous(std::array<Coordinate, 8> const& values)
{
    using traits = native_fixed_traits<Coordinate>;
    std::array<typename traits::coordinate_integer, 8> grid;
    for (std::size_t i = 0; i < grid.size(); ++i) grid[i] = fixed_value(values[i]);
    // Cancel the common dyadic factor before forming products, not by a large
    // gcd afterwards. Ordinary integer-valued doubles then use small operands.
    unsigned const removed = remove_binary_scale(grid, traits::scale_bits);
    auto line = [&](std::size_t i)
    {
        using linear = fixed_integer<traits::line_linear_bits>;
        return typename traits::line_type{linear(grid[i + 1]) - linear(grid[i + 3]),
            linear(grid[i + 2]) - linear(grid[i]),
            fixed_multiply<traits::line_constant_bits>(grid[i], grid[i + 3])
              - fixed_multiply<traits::line_constant_bits>(grid[i + 1], grid[i + 2])};
    };
    auto const a = line(0), b = line(4);
    // |A|,|B|<2^(b+1), |C|<2^(2b+1). The two-term crosses therefore
    // need 3b+3 numerator bits and 2b+3 grid-denominator bits. Restoring
    // the Cartesian grid denominator adds s bits only to W.
    typename traits::intersection_type result;
    result.X = fixed_multiply<traits::intersection_numerator_bits>(a.B, b.C)
             - fixed_multiply<traits::intersection_numerator_bits>(a.C, b.B);
    result.Y = fixed_multiply<traits::intersection_numerator_bits>(a.C, b.A)
             - fixed_multiply<traits::intersection_numerator_bits>(a.A, b.C);
    result.W = fixed_multiply<traits::intersection_denominator_bits>(a.A, b.B)
             - fixed_multiply<traits::intersection_denominator_bits>(a.B, b.A);
    if (result.W < 0) { result.X = -result.X; result.Y = -result.Y; result.W = -result.W; }
    result.W <<= traits::scale_bits - removed;
    fixed_normalize(result);
    return result;
}

template <typename Coordinate>
inline int fixed_orientation(std::array<Coordinate, 6> const& values)
{
    using polynomial = native_fixed_polynomial<Coordinate, 2, 8>;
    using integer = typename polynomial::type;
    integer const ax(fixed_value(values[0])), ay(fixed_value(values[1]));
    integer const bx(fixed_value(values[2])), by(fixed_value(values[3]));
    integer const cx(fixed_value(values[4])), cy(fixed_value(values[5]));
    integer const determinant = fixed_multiply<polynomial::bits>(bx - ax, cy - ay)
                              - fixed_multiply<polynomial::bits>(by - ay, cx - ax);
    return determinant.sign();
}

template <typename Coordinate>
inline int fixed_product_difference(std::array<Coordinate, 4> const& values)
{
    using polynomial = native_fixed_polynomial<Coordinate, 2, 2>;
    using integer = typename polynomial::type;
    integer const result = fixed_multiply<polynomial::bits>(fixed_value(values[0]), fixed_value(values[1]))
                         - fixed_multiply<polynomial::bits>(fixed_value(values[2]), fixed_value(values[3]));
    return result.sign();
}

template <std::size_t AN, std::size_t AD, std::size_t BN, std::size_t BD>
inline fixed_homogeneous<fixed_detail::maximum(AN + BD, BN + AD), AD + BD>
fixed_splice(fixed_homogeneous<AN, AD> const& a, std::size_t a_axis,
             fixed_homogeneous<BN, BD> const& b, std::size_t b_axis)
{
    constexpr std::size_t nbits = fixed_detail::maximum(AN + BD, BN + AD);
    fixed_homogeneous<nbits, AD + BD> result;
    result.X = fixed_multiply<nbits>(a_axis == 0 ? a.X : a.Y, b.W);
    result.Y = fixed_multiply<nbits>(b_axis == 0 ? b.X : b.Y, a.W);
    result.W = fixed_multiply<AD + BD>(a.W, b.W);
    return result;
}

template <std::size_t AN, std::size_t AD, std::size_t BN, std::size_t BD>
inline fixed_homogeneous<fixed_detail::maximum(AN + BD, BN + AD) + 1, AD + BD + 1>
fixed_midpoint(fixed_homogeneous<AN, AD> const& a, fixed_homogeneous<BN, BD> const& b)
{
    constexpr std::size_t nbits = fixed_detail::maximum(AN + BD, BN + AD) + 1;
    fixed_homogeneous<nbits, AD + BD + 1> result;
    result.X = fixed_multiply<nbits>(a.X, b.W) + fixed_multiply<nbits>(b.X, a.W);
    result.Y = fixed_multiply<nbits>(a.Y, b.W) + fixed_multiply<nbits>(b.Y, a.W);
    result.W = fixed_multiply<AD + BD + 1>(a.W, b.W);
    result.W <<= 1;
    return result;
}

template <std::size_t AN, std::size_t AD, std::size_t BN, std::size_t BD>
inline int fixed_coordinate_compare(fixed_homogeneous<AN, AD> const& a,
                                     fixed_homogeneous<BN, BD> const& b, std::size_t dimension)
{
    if (dimension > 1) { throw std::out_of_range("Fixed Cartesian coordinate dimension"); }
    if (a.W <= 0 || b.W <= 0) { throw std::domain_error("Normalized finite homogeneous points required"); }
    constexpr std::size_t bits = fixed_detail::maximum(AN + BD, BN + AD) + 1;
    using integer = fixed_integer<bits>;
    integer const result = fixed_multiply<bits>(dimension == 0 ? a.X : a.Y, b.W)
                         - fixed_multiply<bits>(dimension == 0 ? b.X : b.Y, a.W);
    return result.sign();
}

template <std::size_t AN, std::size_t AD, std::size_t BN, std::size_t BD, std::size_t CN, std::size_t CD>
inline int fixed_orientation(fixed_homogeneous<AN, AD> const& a,
                              fixed_homogeneous<BN, BD> const& b,
                              fixed_homogeneous<CN, CD> const& c)
{
    if (a.W <= 0 || b.W <= 0 || c.W <= 0)
    {
        throw std::domain_error("Normalized finite homogeneous points required");
    }
    // Six products: two numerator fields and one denominator field each.
    constexpr std::size_t bits = fixed_detail::maximum(AN + BN + CD,
        fixed_detail::maximum(AN + BD + CN, AD + BN + CN)) + 3;
    using integer = fixed_integer<bits>;
    integer result = fixed_multiply<bits>(fixed_multiply<AN + BN>(a.X, b.Y), c.W);
    result -= fixed_multiply<bits>(fixed_multiply<AN + BN>(a.Y, b.X), c.W);
    result += fixed_multiply<bits>(fixed_multiply<AN + BD>(a.Y, b.W), c.X);
    result -= fixed_multiply<bits>(fixed_multiply<AN + BD>(a.X, b.W), c.Y);
    result += fixed_multiply<bits>(fixed_multiply<AD + BN>(a.W, b.X), c.Y);
    result -= fixed_multiply<bits>(fixed_multiply<AD + BN>(a.W, b.Y), c.X);
    return result.sign();
}

} // namespace exact_cartesian
}}}} // namespace boost::geometry::strategies::detail

#endif
