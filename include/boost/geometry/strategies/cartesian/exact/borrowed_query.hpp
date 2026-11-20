// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_QUERY_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_QUERY_HPP

#include <array>
#include <cfenv>
#include <cmath>
#include <cstddef>
#include <exception>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <boost/assert.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{
template <typename Root> class query_point;
}}}}}

#include <boost/geometry/strategies/cartesian/exact/borrowed_records.hpp>
#include <boost/geometry/strategies/cartesian/exact/forward_error.hpp>
#include <boost/geometry/util/error_free_transforms.hpp>
#include <boost/geometry/strategies/cartesian/exact/point_precision_cache.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

namespace borrowed_query_detail
{
template <typename Root, typename Point, typename Tag = geometry::tag_t<std::decay_t<Point>>>
struct geometry_operand : std::false_type {};
template <typename Root, typename Point>
struct geometry_operand<Root, Point, point_tag> : std::integral_constant<bool,
    std::is_same<std::decay_t<Point>, borrowed_point<Root>>::value
    || (fixed_native_supported<geometry::coordinate_type_t<std::decay_t<Point>>>::value
        && lossless_coordinate_conversion<geometry::coordinate_type_t<std::decay_t<Point>>,
            typename Root::coordinate_type>::value)>
{};

template <typename Root, typename Point, typename Apply>
inline decltype(auto) with_geometry(Point const& point, Apply&& apply)
{
    static_assert(geometry_operand<Root, Point>::value, "One-level queries require geometry operands");
    static_assert(geometry::dimension<Point>::value == 2
        && std::is_same<geometry::coordinate_system_t<Point>, cs::cartesian>::value,
        "Exact Cartesian queries require two-dimensional Cartesian points");
    if constexpr (std::is_same<std::decay_t<Point>, borrowed_point<Root>>::value)
        return apply(point.homogeneous());
    else return apply(fixed_input_homogeneous(geometry::get<0>(point), geometry::get<1>(point)));
}

template <typename Root, typename Point>
inline bool try_native_coordinate(Point const& point, std::size_t axis, double& output)
{
    double error;
    if constexpr (std::is_same<std::decay_t<Point>, borrowed_point<Root>>::value)
    {
        typename Root::coordinate_type value;
        if (!point.native_coordinate(axis, value)) return false;
        return forward_error::try_native_coordinate(value, output, error) && error == 0;
    }
    else
        return forward_error::try_native_coordinate(axis == 0 ? geometry::get<0>(point)
            : geometry::get<1>(point), output, error) && error == 0;
}

template <typename Root, typename Point>
inline bool copy_compact_geometry(Point const& point, fixed_homogeneous<128, 128>& output)
{
    if constexpr (std::is_same<std::decay_t<Point>, borrowed_point<Root>>::value)
        return point.copy_compact_homogeneous(output);
    else return try_fixed_input_homogeneous(geometry::get<0>(point), geometry::get<1>(point), output);
}

inline bool try_midpoint(double a, double b, double& output)
{
    using namespace arithmetic_detail;
    double const limit = (std::numeric_limits<double>::max)() / 2;
    if (std::abs(a) <= limit && std::abs(b) <= limit)
    {
        auto const [sum, sum_tail] = geometry::detail::precise_math::two_sum(a, b);
        if (sum_tail != 0) return false;
        double const half = rounded_mul(sum, 0.5);
        // Doubling is exact here; this also rejects lost subnormal half-bits.
        if (rounded_add(half, half) != sum) return false;
        output = half;
        return true;
    }
    // Avoid an overflowing sum: scale each operand only when that scaling is
    // exact, then require an error-free scaled sum.
    double const half_a = rounded_mul(a, 0.5), half_b = rounded_mul(b, 0.5);
    if (rounded_add(half_a, half_a) != a || rounded_add(half_b, half_b) != b) return false;
    auto const [scaled, scaled_tail] = geometry::detail::precise_math::two_sum(half_a, half_b);
    if (scaled_tail != 0 || !std::isfinite(scaled)) return false;
    output = scaled;
    return true;
}
} // namespace borrowed_query_detail

// A query is not a geometry vertex or an original-edge support. Its factories
// accept only geometry points; no query can become a recursive construction leaf.
template <typename Root>
class query_point
{
public:
    using root_type = Root;
    using coordinate_type = typename Root::coordinate_type;
    using homogeneous_type = auxiliary_point_type<coordinate_type>;

    template <typename A, typename B, std::enable_if_t<
        borrowed_query_detail::geometry_operand<Root, A>::value
        && borrowed_query_detail::geometry_operand<Root, B>::value, int> = 0>
    static query_point midpoint(A const& a, B const& b)
    {
#ifndef NDEBUG
        BOOST_ASSERT(std::fegetround() == FE_TONEAREST);
#endif
        std::array<double, 2> first, second, result;
        if (borrowed_query_detail::try_native_coordinate<Root>(a, 0, first[0])
            && borrowed_query_detail::try_native_coordinate<Root>(a, 1, first[1])
            && borrowed_query_detail::try_native_coordinate<Root>(b, 0, second[0])
            && borrowed_query_detail::try_native_coordinate<Root>(b, 1, second[1])
            && borrowed_query_detail::try_midpoint(first[0], second[0], result[0])
            && borrowed_query_detail::try_midpoint(first[1], second[1], result[1]))
            return query_point(handle_type{}, result);
        fixed_homogeneous<128, 128> compact_first, compact_second;
        if (borrowed_query_detail::copy_compact_geometry<Root>(a, compact_first)
            && borrowed_query_detail::copy_compact_geometry<Root>(b, compact_second))
            return create<construction_family::midpoint_query>(fixed_midpoint(compact_first, compact_second));
        return borrowed_query_detail::with_geometry<Root>(a, [&](auto const& first)
        {
            return borrowed_query_detail::with_geometry<Root>(b, [&](auto const& second)
            { return create<construction_family::midpoint_query>(fixed_midpoint(first, second)); });
        });
    }

    template <typename A, typename B, std::enable_if_t<
        borrowed_query_detail::geometry_operand<Root, A>::value
        && borrowed_query_detail::geometry_operand<Root, B>::value, int> = 0>
    static query_point splice_coordinates(A const& a, std::size_t a_axis, B const& b, std::size_t b_axis)
    {
#ifndef NDEBUG
        BOOST_ASSERT(std::fegetround() == FE_TONEAREST);
#endif
        if (a_axis > 1 || b_axis > 1) throw std::out_of_range("Exact query coordinate dimension");
        std::array<double, 2> result;
        if (borrowed_query_detail::try_native_coordinate<Root>(a, a_axis, result[0])
            && borrowed_query_detail::try_native_coordinate<Root>(b, b_axis, result[1]))
            return query_point(handle_type{}, result);
        fixed_homogeneous<128, 128> compact_first, compact_second;
        if (borrowed_query_detail::copy_compact_geometry<Root>(a, compact_first)
            && borrowed_query_detail::copy_compact_geometry<Root>(b, compact_second))
            return create<construction_family::splice_query>(fixed_splice(compact_first, a_axis, compact_second, b_axis));
        return borrowed_query_detail::with_geometry<Root>(a, [&](auto const& first)
        {
            return borrowed_query_detail::with_geometry<Root>(b, [&](auto const& second)
            { return create<construction_family::splice_query>(fixed_splice(first, a_axis, second, b_axis)); });
        });
    }

    homogeneous_type homogeneous() const
    {
        if (m_precision) return m_precision.get();
        using native = native_fixed_traits<coordinate_type>;
        fixed_homogeneous<native::coordinate_bits + 1, native::scale_bits + 2> value;
        bool const fits = try_fixed_input_homogeneous(m_approximate[0], m_approximate[1], value);
        BOOST_ASSERT_MSG(fits, "Exact native queries fit the native midpoint grid");
        if (!fits) throw std::overflow_error("Exact native query exceeds its derived capacity");
        return point_precision_detail::promote<coordinate_type>(value);
    }
    template <std::size_t N, std::size_t D>
    bool copy_compact_homogeneous(fixed_homogeneous<N, D>& output) const
    {
        if (m_precision) return m_precision.copy_compact(output);
        return try_fixed_input_homogeneous(m_approximate[0], m_approximate[1], output);
    }
    double approximate(std::size_t dimension) const { return m_approximate.at(dimension); }
    interval approximation(std::size_t dimension) const
    {
        double const value = approximate(dimension);
        return {value, m_precision ? construction_rounding::detail::spacing(value) : 0};
    }
    interval refine(std::size_t dimension) const { return approximation(dimension); }

    // Diagnostic Point access only. Predicates use homogeneous/approximation.
    exact_number exact_coordinate(std::size_t dimension) const
    {
        if (dimension > 1) throw std::out_of_range("Exact query coordinate dimension");
        auto const value = homogeneous();
        exact_number result(dimension == 0 ? value.X : value.Y);
        result /= value.W;
        return result;
    }

private:
    using handle_type = point_precision_detail::point_handle<coordinate_type>;
    // Empty handles are reserved for the exact-native certificates above.
    query_point(handle_type precision, std::array<double, 2> approximate)
        : m_precision(precision), m_approximate(approximate) {}

    template <construction_family Family, std::size_t N, std::size_t D>
    static query_point create(fixed_homogeneous<N, D> value)
    {
        fixed_normalize(value);
        using capacity = construction_capacity<coordinate_type, Family>;
        bool const fits = fixed_fits<capacity::numerator_bits>(value.X)
            && fixed_fits<capacity::numerator_bits>(value.Y)
            && fixed_fits<capacity::denominator_bits>(value.W);
        BOOST_ASSERT_MSG(fits, "Normalized one-level query must fit its construction-family capacity");
        if (!fits) throw std::overflow_error("Exact query exceeds its construction-family capacity");
        std::array<double, 2> approximate;
        for (std::size_t dimension = 0; dimension != 2; ++dimension)
        {
            if (!try_fixed_round(value, dimension, approximate[dimension]) || !std::isfinite(approximate[dimension]))
                throw std::domain_error("Exact query has no finite Cartesian approximation");
        }
        return query_point(handle_type::create(value), approximate);
    }

    handle_type m_precision;
    std::array<double, 2> m_approximate;
};

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

namespace boost { namespace geometry { namespace traits
{
template <typename Root> struct tag<strategies::detail::exact_cartesian::query_point<Root>>
{ using type = point_tag; };
template <typename Root> struct coordinate_type<strategies::detail::exact_cartesian::query_point<Root>>
{ using type = strategies::detail::exact_cartesian::exact_number; };
template <typename Root> struct coordinate_system<strategies::detail::exact_cartesian::query_point<Root>>
{ using type = cs::cartesian; };
template <typename Root> struct dimension<strategies::detail::exact_cartesian::query_point<Root>>
    : std::integral_constant<std::size_t, 2> {};
template <typename Root, std::size_t D>
struct access<strategies::detail::exact_cartesian::query_point<Root>, D>
{
    using point = strategies::detail::exact_cartesian::query_point<Root>;
    static auto get(point const& value) { return value.exact_coordinate(D); }
    static void set(point&, strategies::detail::exact_cartesian::exact_number const&)
    { BOOST_ASSERT_MSG(false, "Exact queries are constructed as a whole"); std::terminate(); }
};
}}}

#endif
