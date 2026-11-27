// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_DETAIL_OUTPUT_POINT_KEY_HPP
#define BOOST_GEOMETRY_STRATEGIES_DETAIL_OUTPUT_POINT_KEY_HPP

#include <type_traits>
#include <boost/geometry/algorithms/detail/convert_point_to_point.hpp>
#include <boost/geometry/core/coordinate_promotion.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/geometries/helper_geometry.hpp>
#include <boost/geometry/strategies/compare.hpp>
#include <boost/geometry/strategies/normalize.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{

template <typename Point>
struct output_point_key
{
    static constexpr bool spheroidal = std::is_base_of<spherical_tag, cs_tag_t<Point>>::value;
    using coordinate = coordinate_type_t<Point>;
    // Integral angular coordinates need floating-point spheroidal constants.
    using key_coordinate = std::conditional_t<spheroidal && std::is_integral<coordinate>::value,
        typename promote_floating_point<coordinate, long double>::type, coordinate>;
    // Adapted output points need not be copy constructible.
    using type = typename helper_geometry<Point, key_coordinate>::type;

    static type apply(Point const& point)
    {
        type key;
        geometry::detail::conversion::convert_point_to_point(point, key);
        // Canonicalize the key, not the point emitted to the caller.
        using normalize = std::conditional_t<spheroidal,
            strategy::normalize::spherical_point, strategy::normalize::cartesian_point>;
        normalize::apply(key, key);
        return key;
    }

    bool operator()(type const& a, type const& b) const
    {
        return strategy::compare::cartesian<
            strategy::compare::less, strategy::compare::equals_exact>::apply(a, b);
    }
};

template <typename Point, typename Strategy>
inline auto get_output_point_key(Strategy const& strategy)
{
    auto const hook = [](auto const& s) -> decltype(s.template output_point_key<Point>())
    { return s.template output_point_key<Point>(); };
    if constexpr (std::is_invocable_v<decltype(hook), Strategy const&>)
        return hook(strategy);
    else
        return output_point_key<Point>{};
}

}}}} // namespace boost::geometry::strategies::detail

#endif
