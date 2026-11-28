// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Licensed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_NATIVE_PREDICATES_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_NATIVE_PREDICATES_HPP

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

#include <boost/geometry/strategies/cartesian/exact/fixed_arithmetic.hpp>
#include <boost/geometry/strategies/cartesian/exact/native_traits.hpp>
#include <boost/geometry/strategies/cartesian/exact/arithmetic.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/strategy/cartesian/side_robust.hpp>

#include <boost/geometry/util/detail/precise_fp.hpp>

BOOST_GEOMETRY_DETAIL_PRECISE_FP_BEGIN

namespace boost { namespace geometry { namespace strategies { namespace detail
{
namespace exact_cartesian
{

namespace native_predicates_detail
{

template <typename Coordinate, typename... Points>
struct native_inputs : std::true_type {};

template <typename Coordinate, typename Point, typename... Points>
struct native_inputs<Coordinate, Point, Points...>
    : std::integral_constant<bool,
        fixed_native_supported<Coordinate>::value
        && fixed_native_supported<typename geometry::coordinate_type<Point>::type>::value
        && lossless_coordinate_conversion<typename geometry::coordinate_type<Point>::type,
                                          Coordinate>::value
        && native_inputs<Coordinate, Points...>::value>
{};

} // namespace native_predicates_detail

// Inputs remain immutable and alive for the predicate call. In particular,
// implicit points must never be read through a native accessor.
template <typename Coordinate, typename Point>
class native_point_view
{
public:
    using point_type = Point;

    explicit native_point_view(Point const& point)
        : m_point(point)
    {
        static_assert(geometry::dimension<Point>::value == 2,
                      "Exact Cartesian points are two-dimensional");
        static_assert(std::is_same<typename geometry::coordinate_system<Point>::type,
                                   cs::cartesian>::value, "Exact Cartesian input required");
        static_assert(native_predicates_detail::native_inputs<Coordinate, Point>::value,
                      "Native predicates require lossless native coordinate promotion");
    }

    native_point_view(Point&&) = delete;
    native_point_view(Point const&&) = delete;

    Coordinate coordinate(std::size_t dimension) const
    {
        return dimension == 0 ? static_cast<Coordinate>(geometry::get<0>(m_point))
                              : static_cast<Coordinate>(geometry::get<1>(m_point));
    }

    Point const& point() const { return m_point; }

private:
    Point const& m_point;
};

namespace native_predicates_detail
{

template <typename Coordinate, typename A, typename B, typename P>
inline int orientation(native_point_view<Coordinate, A> const& a,
                       native_point_view<Coordinate, B> const& b,
                       native_point_view<Coordinate, P> const& c, std::true_type)
{
    std::array<Coordinate, 6> const values{{a.coordinate(0), a.coordinate(1),
                                          b.coordinate(0), b.coordinate(1),
                                          c.coordinate(0), c.coordinate(1)}};
    if constexpr (std::numeric_limits<Coordinate>::digits + std::numeric_limits<Coordinate>::is_signed <= 16)
    {
        return arithmetic_detail::integer_orientation(values[0], values[1], values[2], values[3],
                                                       values[4], values[5], std::true_type{});
    }
#if defined(BOOST_HAS_INT128)
    using wide = boost::int128_type;
    // Cast before subtracting: all <=64-bit native inputs and their differences
    // fit. Guard products so they can be compared without overflow.
    wide const dx1 = wide(values[2]) - wide(values[0]), dy1 = wide(values[3]) - wide(values[1]);
    wide const dx2 = wide(values[4]) - wide(values[0]), dy2 = wide(values[5]) - wide(values[1]);
    wide const bound = (std::numeric_limits<std::int64_t>::max)();
    if (dx1 >= -bound && dx1 <= bound && dy1 >= -bound && dy1 <= bound
        && dx2 >= -bound && dx2 <= bound && dy2 >= -bound && dy2 <= bound)
    {
        wide const left = dx1 * dy2, right = dy1 * dx2;
        return (left > right) - (left < right);
    }
#else
    if (arithmetic_detail::fits_int64(values[0]) && arithmetic_detail::fits_int64(values[1])
        && arithmetic_detail::fits_int64(values[2]) && arithmetic_detail::fits_int64(values[3])
        && arithmetic_detail::fits_int64(values[4]) && arithmetic_detail::fits_int64(values[5]))
    {
        std::int64_t dx1, dy1, dx2, dy2;
        std::int64_t const bound = 2147483647;
        if (arithmetic_detail::subtract_int64(std::int64_t(values[2]), std::int64_t(values[0]), dx1)
            && arithmetic_detail::subtract_int64(std::int64_t(values[3]), std::int64_t(values[1]), dy1)
            && arithmetic_detail::subtract_int64(std::int64_t(values[4]), std::int64_t(values[0]), dx2)
            && arithmetic_detail::subtract_int64(std::int64_t(values[5]), std::int64_t(values[1]), dy2)
            && dx1 >= -bound && dx1 <= bound && dy1 >= -bound && dy1 <= bound
            && dx2 >= -bound && dx2 <= bound && dy2 >= -bound && dy2 <= bound)
        {
            std::int64_t const left = dx1 * dy2, right = dy1 * dx2;
            return (left > right) - (left < right);
        }
    }
#endif
    return fixed_orientation(values);
}

// orient2d also serves epsilon-based callers. Its exact sign has no use
// for the scale of those comparisons, including on the adaptive path.
struct no_epsilon_scale
{
    no_epsilon_scale() = default;
    no_epsilon_scale(double, double, double, double) {}
};

template <typename Coordinate, typename A, typename B, typename P>
inline int orientation(native_point_view<Coordinate, A> const& a,
                       native_point_view<Coordinate, B> const& b,
                       native_point_view<Coordinate, P> const& c, std::false_type)
{
    no_epsilon_scale unused;
    auto const det = strategy::side::side_robust<double, strategy::side::fp_equals_policy, 3>
        ::side_value<double>(a.point(), b.point(), c.point(), unused);
    return (det > 0) - (det < 0);
}

} // namespace native_predicates_detail

template <typename Coordinate, typename A, typename B, typename P>
inline int orientation(native_point_view<Coordinate, A> const& a,
                       native_point_view<Coordinate, B> const& b,
                       native_point_view<Coordinate, P> const& c)
{
    return native_predicates_detail::orientation(a, b, c, std::is_integral<Coordinate>());
}

// Public within exact_cartesian for native-input callers such as segment
// rejection. Implicit points retain their separate exact representation.
template <typename Coordinate, typename A, typename B, typename P>
inline int native_orientation(A const& a, B const& b, P const& p)
{
    return orientation(native_point_view<Coordinate, A>(a), native_point_view<Coordinate, B>(b),
                       native_point_view<Coordinate, P>(p));
}

} // namespace exact_cartesian
}}}} // namespace boost::geometry::strategies::detail

BOOST_GEOMETRY_DETAIL_PRECISE_FP_END

#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_NATIVE_PREDICATES_HPP
