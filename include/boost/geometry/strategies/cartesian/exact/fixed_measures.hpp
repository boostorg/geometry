// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_FIXED_MEASURES_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_FIXED_MEASURES_HPP

#include <cstddef>
#include <stdexcept>
#include <type_traits>

#include <boost/geometry/strategies/cartesian/exact/fixed_arithmetic.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{
namespace exact_cartesian
{

// Predicate pairs need no gcd reduction. All producers below retain D>0.
template <std::size_t NumeratorBits, std::size_t DenominatorBits>
struct fixed_rational_pair
{
    using numerator_type = fixed_integer<NumeratorBits>;
    using denominator_type = fixed_integer<DenominatorBits>;
    numerator_type N{};
    denominator_type D{1};
};

template <std::size_t N, std::size_t D>
struct fixed_measure_traits
{
    using pair_type = fixed_rational_pair<2 * N + 2 * D + 3, 4 * D>;
    using word_type = fixed_integer<2 * N + 6 * D + 8>;
};

namespace fixed_measure_detail
{

template <std::size_t AN, std::size_t AD, std::size_t BN, std::size_t BD>
struct distance_capacity
{
    static constexpr std::size_t difference_bits = fixed_detail::maximum(AN + BD, BN + AD) + 1;
    using type = fixed_rational_pair<2 * difference_bits + 1, 2 * (AD + BD)>;
};

template <std::size_t AN, std::size_t AD, std::size_t BN, std::size_t BD,
          std::size_t PN, std::size_t PD>
struct determinant_capacity
{
    static constexpr std::size_t first_bits = fixed_detail::maximum(AN + BD, BN + AD) + 1;
    static constexpr std::size_t second_bits = fixed_detail::maximum(AN + PD, PN + AD) + 1;
    using type = fixed_rational_pair<first_bits + second_bits + 1, 2 * AD + BD + PD>;
};

} // namespace fixed_measure_detail

template <std::size_t AN, std::size_t AD, std::size_t BN, std::size_t BD>
inline typename fixed_measure_detail::distance_capacity<AN, AD, BN, BD>::type
fixed_squared_distance(fixed_homogeneous<AN, AD> const& a, fixed_homogeneous<BN, BD> const& b)
{
    if (a.W <= 0 || b.W <= 0) { throw std::domain_error("Normalized finite homogeneous points required"); }
    using capacity = fixed_measure_detail::distance_capacity<AN, AD, BN, BD>;
    using difference = fixed_integer<capacity::difference_bits>;
    using result_type = typename capacity::type;
    difference const dx = fixed_multiply<capacity::difference_bits>(a.X, b.W)
                        - fixed_multiply<capacity::difference_bits>(b.X, a.W);
    difference const dy = fixed_multiply<capacity::difference_bits>(a.Y, b.W)
                        - fixed_multiply<capacity::difference_bits>(b.Y, a.W);
    auto const denominator = fixed_multiply<AD + BD>(a.W, b.W);
    // |dx|,|dy|<2^difference_bits, so the sum of squares needs 2*bits+1.
    result_type result;
    result.N = fixed_multiply<2 * capacity::difference_bits + 1>(dx, dx)
             + fixed_multiply<2 * capacity::difference_bits + 1>(dy, dy);
    result.D = fixed_multiply<2 * (AD + BD)>(denominator, denominator);
    return result;
}

template <std::size_t AN, std::size_t AD, std::size_t BN, std::size_t BD,
          std::size_t PN, std::size_t PD>
inline typename fixed_measure_detail::determinant_capacity<AN, AD, BN, BD, PN, PD>::type
fixed_signed_determinant(fixed_homogeneous<AN, AD> const& a,
                         fixed_homogeneous<BN, BD> const& b, fixed_homogeneous<PN, PD> const& point)
{
    if (a.W <= 0 || b.W <= 0 || point.W <= 0)
    {
        throw std::domain_error("Normalized finite homogeneous points required");
    }
    using capacity = fixed_measure_detail::determinant_capacity<AN, AD, BN, BD, PN, PD>;
    using first = fixed_integer<capacity::first_bits>;
    using second = fixed_integer<capacity::second_bits>;
    using result_type = typename capacity::type;
    constexpr std::size_t numerator_bits = capacity::first_bits + capacity::second_bits + 1;
    first const dx = fixed_multiply<capacity::first_bits>(b.X, a.W)
                   - fixed_multiply<capacity::first_bits>(a.X, b.W);
    first const dy = fixed_multiply<capacity::first_bits>(b.Y, a.W)
                   - fixed_multiply<capacity::first_bits>(a.Y, b.W);
    second const px = fixed_multiply<capacity::second_bits>(point.X, a.W)
                    - fixed_multiply<capacity::second_bits>(a.X, point.W);
    second const py = fixed_multiply<capacity::second_bits>(point.Y, a.W)
                    - fixed_multiply<capacity::second_bits>(a.Y, point.W);
    result_type result;
    result.N = fixed_multiply<numerator_bits>(dx, py) - fixed_multiply<numerator_bits>(dy, px);
    // The two affine differences have denominators a.W*b.W and a.W*point.W.
    result.D = fixed_multiply<2 * AD + BD + PD>(fixed_multiply<AD + BD>(a.W, b.W),
                                               fixed_multiply<AD + PD>(a.W, point.W));
    return result;
}

template <std::size_t AN, std::size_t AD, std::size_t BN, std::size_t BD>
inline int fixed_pair_compare(fixed_rational_pair<AN, AD> const& a, fixed_rational_pair<BN, BD> const& b)
{
    if (a.D <= 0 || b.D <= 0) { throw std::domain_error("Positive fixed rational denominators required"); }
    // One carry bit suffices for subtraction; four spare bits give the common
    // point-format word budget 2N+6D+8 without assuming reduced fractions.
    constexpr std::size_t bits = fixed_detail::maximum(AN + BD, BN + AD) + 5;
    fixed_integer<bits> const difference = fixed_multiply<bits>(a.N, b.D) - fixed_multiply<bits>(b.N, a.D);
    return difference.sign();
}

template <std::size_t N, std::size_t D, typename Coordinate>
inline typename std::enable_if<fixed_native_supported<Coordinate>::value, int>::type
fixed_pair_compare(fixed_rational_pair<N, D> const& a, Coordinate value)
{
    using traits = native_fixed_traits<Coordinate>;
    fixed_rational_pair<traits::coordinate_bits, traits::scale_bits + 1> native;
    native.N = fixed_value(value);
    native.D <<= traits::scale_bits;
    // Derive both cross-product widths, including the native grid denominator.
    return fixed_pair_compare(a, native);
}

} // namespace exact_cartesian
}}}} // namespace boost::geometry::strategies::detail

#endif
