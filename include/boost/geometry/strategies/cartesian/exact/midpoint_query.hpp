// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_MIDPOINT_QUERY_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_MIDPOINT_QUERY_HPP

#include <array>
#include <cfenv>
#include <cmath>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <boost/assert.hpp>

#include <boost/geometry/util/detail/precise_fp.hpp>

BOOST_GEOMETRY_DETAIL_PRECISE_FP_BEGIN

#include <boost/geometry/strategies/cartesian/exact/operation_geometry.hpp>
#include <boost/geometry/strategies/cartesian/exact/forward_error.hpp>
#include <boost/geometry/util/error_free_transforms.hpp>
#include <boost/geometry/strategies/cartesian/exact/point_precision_cache.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

// A query is not a geometry vertex or an original-edge support. Its factories
// accept only geometry points; no query can become a recursive construction leaf.
template <typename Root>
class midpoint_query
{
    template <typename Point, typename Tag = geometry::tag_t<std::decay_t<Point>>>
    struct geometry_operand : std::false_type {};
    template <typename Point>
    struct geometry_operand<Point, point_tag> : std::integral_constant<bool,
        std::is_same<std::decay_t<Point>, point_ref<Root>>::value
        || (fixed_native_supported<geometry::coordinate_type_t<std::decay_t<Point>>>::value
            && lossless_coordinate_conversion<geometry::coordinate_type_t<std::decay_t<Point>>,
                typename Root::coordinate_type>::value)>
    {};

    template <typename Point, typename Apply>
    static decltype(auto) with_geometry(Point const& point, Apply&& apply)
    {
        static_assert(geometry_operand<Point>::value, "One-level queries require geometry operands");
        static_assert(geometry::dimension<Point>::value == 2
            && std::is_same<geometry::coordinate_system_t<Point>, cs::cartesian>::value,
            "Exact Cartesian queries require two-dimensional Cartesian points");
        if constexpr (std::is_same<std::decay_t<Point>, point_ref<Root>>::value)
            return apply(point.homogeneous());
        else return apply(fixed_input_homogeneous(geometry::get<0>(point), geometry::get<1>(point)));
    }

    template <typename Point>
    static bool try_native_coordinate(Point const& point, std::size_t axis, double& output)
    {
        double error;
        if constexpr (std::is_same<std::decay_t<Point>, point_ref<Root>>::value)
        {
            typename Root::coordinate_type value;
            if (!point.native_coordinate(axis, value)) return false;
            return forward_error::try_native_coordinate(value, output, error) && error == 0;
        }
        else
            return forward_error::try_native_coordinate(axis == 0 ? geometry::get<0>(point)
                : geometry::get<1>(point), output, error) && error == 0;
    }

    template <typename Point>
    static bool copy_compact_geometry(Point const& point, fixed_homogeneous<128, 128>& output)
    {
        if constexpr (std::is_same<std::decay_t<Point>, point_ref<Root>>::value)
            return point.copy_compact_homogeneous(output);
        else return try_fixed_input_homogeneous(geometry::get<0>(point), geometry::get<1>(point), output);
    }

    static bool try_midpoint(double a, double b, double& output)
    {
        double const limit = (std::numeric_limits<double>::max)() / 2;
        if (std::abs(a) <= limit && std::abs(b) <= limit)
        {
            auto const [sum, sum_tail] = geometry::detail::precise_math::two_sum(a, b);
            if (sum_tail != 0) return false;
            double const half = sum * 0.5;
            // Doubling is exact here; this also rejects lost subnormal half-bits.
            if ((half + half) != sum) return false;
            output = half;
            return true;
        }
        // Avoid an overflowing sum: scale each operand only when that scaling is
        // exact, then require an error-free scaled sum.
        double const half_a = a * 0.5, half_b = b * 0.5;
        if ((half_a + half_a) != a || (half_b + half_b) != b) return false;
        auto const [scaled, scaled_tail] = geometry::detail::precise_math::two_sum(half_a, half_b);
        if (scaled_tail != 0 || !std::isfinite(scaled)) return false;
        output = scaled;
        return true;
    }

public:
    using root_type = Root;
    using coordinate_type = typename Root::coordinate_type;
    using homogeneous_type = auxiliary_point_type<coordinate_type>;

    template <typename A, typename B, std::enable_if_t<
        geometry_operand<A>::value
        && geometry_operand<B>::value, int> = 0>
    static midpoint_query midpoint(A const& a, B const& b)
    {
#ifndef NDEBUG
        BOOST_ASSERT(std::fegetround() == FE_TONEAREST);
#endif
        std::array<double, 2> first, second, result;
        if (try_native_coordinate(a, 0, first[0])
            && try_native_coordinate(a, 1, first[1])
            && try_native_coordinate(b, 0, second[0])
            && try_native_coordinate(b, 1, second[1])
            && try_midpoint(first[0], second[0], result[0])
            && try_midpoint(first[1], second[1], result[1]))
            return midpoint_query(handle_type{}, result);
        fixed_homogeneous<128, 128> compact_first, compact_second;
        if (copy_compact_geometry(a, compact_first)
            && copy_compact_geometry(b, compact_second))
            return from_midpoint(fixed_midpoint(compact_first, compact_second));
        return with_geometry(a, [&](auto const& first)
        {
            return with_geometry(b, [&](auto const& second)
            { return from_midpoint(fixed_midpoint(first, second)); });
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
    scalar_approximation approximation(std::size_t dimension) const
    {
        double const value = approximate(dimension);
        return {value, m_precision ? arithmetic_detail::max_adjacent_spacing(value) : 0};
    }
    scalar_approximation refine(std::size_t dimension) const { return approximation(dimension); }

private:
    using handle_type = point_precision_detail::point_handle<coordinate_type>;
    // Empty handles are reserved for the exact-native certificates above.
    midpoint_query(handle_type precision, std::array<double, 2> approximate)
        : m_precision(precision), m_approximate(approximate) {}

    template <std::size_t N, std::size_t D>
    static midpoint_query from_midpoint(fixed_homogeneous<N, D> value)
    {
        fixed_normalize(value);
        std::array<double, 2> approximate;
        for (std::size_t dimension = 0; dimension != 2; ++dimension)
        {
            if (!try_fixed_round(value, dimension, approximate[dimension]) || !std::isfinite(approximate[dimension]))
                throw std::domain_error("Exact query has no finite Cartesian approximation");
        }
        return midpoint_query(handle_type::create(value), approximate);
    }

    handle_type m_precision;
    std::array<double, 2> m_approximate;
};

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

BOOST_GEOMETRY_DETAIL_PRECISE_FP_END

#endif
