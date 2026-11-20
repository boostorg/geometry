// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_NATIVE_TRAITS_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_NATIVE_TRAITS_HPP

#include <limits>
#include <type_traits>
#include <boost/geometry/core/coordinate_type.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

template <typename From, typename To>
struct lossless_coordinate_conversion : std::integral_constant<bool,
    std::numeric_limits<To>::digits >= std::numeric_limits<From>::digits
    && ((std::is_integral<From>::value && std::is_integral<To>::value
         && (std::is_signed<To>::value || !std::is_signed<From>::value))
        || (std::is_floating_point<To>::value
            && (std::is_integral<From>::value
                || (std::is_floating_point<From>::value
                    && std::numeric_limits<To>::max_exponent >= std::numeric_limits<From>::max_exponent
                    && std::numeric_limits<To>::min_exponent <= std::numeric_limits<From>::min_exponent))))>
{};

template <typename Coordinate, typename Geometry>
constexpr void check_coordinate_type()
{
    using source = geometry::coordinate_type_t<Geometry>;
    static_assert(!(std::is_integral<source>::value || std::is_floating_point<source>::value)
                  || lossless_coordinate_conversion<source, Coordinate>::value,
                  "Exact Cartesian inputs must admit lossless coordinate promotion");
}

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

#endif
