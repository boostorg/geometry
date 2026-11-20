// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_COORDINATES_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_COORDINATES_HPP

#include <stdexcept>
#include <string>
#include <type_traits>

#include <boost/geometry/strategies/cartesian/exact/point.hpp>
#include <boost/geometry/util/exact_coordinate.hpp>

namespace boost { namespace geometry { namespace detail { namespace certified_point_buffer
{

namespace ec = strategies::detail::exact_cartesian;
using ec::exact_number;
using decimal_coordinate = exact_coordinate<std::string>;

inline bool bounded(exact_number const& q)
{
    constexpr unsigned max_scalar_bits = 4096;
    using namespace boost::multiprecision;
    cpp_int n = numerator(q);
    if (n < 0) { n = -n; }
    return (n == 0 || msb(n) < max_scalar_bits) && msb(denominator(q)) < max_scalar_bits;
}

template <typename Coordinate>
struct use_rational_backend : std::is_same<Coordinate, exact_number>
{};

template <>
struct use_rational_backend<decimal_coordinate> : std::true_type
{};

template <typename Coordinate>
struct coordinate_writer : ec::point_detail::output_rounder<Coordinate>
{};

template <>
struct coordinate_writer<decimal_coordinate>
{
    static decimal_coordinate apply(exact_number const& value)
    {
        using boost::multiprecision::cpp_int;
        cpp_int d = denominator(value);
        unsigned const twos = boost::multiprecision::lsb(d);
        d >>= twos;
        unsigned fives = 0;
        // Match the exponent limit of the authoritative text parser.
        unsigned const limit = 10000;
        while (fives < limit && d % 5 == 0)
        {
            d /= 5;
            ++fives;
        }
        if (twos > limit || d != 1)
        {
            throw std::domain_error("Buffer vertex has no supported finite decimal representation");
        }

        // n / (2^a 5^b) = coefficient * 10^-max(a,b).
        cpp_int coefficient = numerator(value);
        if (twos > fives)
        {
            coefficient *= boost::multiprecision::pow(cpp_int(5), twos - fives);
        }
        else
        {
            coefficient *= cpp_int(1) << (fives - twos);
        }
        unsigned const scale = twos > fives ? twos : fives;
        return decimal_coordinate(scale == 0 ? coefficient.str()
            : coefficient.str() + "e-" + std::to_string(scale));
    }
};

template <typename Scalar, typename Conversion>
inline exact_number rational_value(exact_coordinate<Scalar, Conversion> const& value)
{
    return value.rational();
}

}}}} // namespace boost::geometry::detail::certified_point_buffer

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_COORDINATES_HPP
