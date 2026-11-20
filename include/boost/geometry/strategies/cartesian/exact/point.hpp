// Boost.Geometry
// Copyright (c) 2026
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_POINT_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_POINT_HPP

// Interval and expansion arithmetic enforce their own rounding boundaries.
#if defined(__FAST_MATH__)
#error Exact Cartesian predicates do not support fast-math
#endif

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/coordinate_system.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/cs.hpp>
#include <boost/geometry/core/tag.hpp>
#include <boost/geometry/algorithms/detail/convert_point_to_point.hpp>
#include <boost/geometry/strategies/cartesian/exact/arithmetic.hpp>
#include <boost/geometry/strategies/cartesian/exact/construction_filter.hpp>
#include <boost/geometry/strategies/cartesian/exact/point_cache.hpp>
#include <boost/geometry/strategies/cartesian/exact/point_precision_cache.hpp>
#include <boost/geometry/strategies/cartesian/exact/forward_error.hpp>
#include <boost/geometry/strategies/cartesian/exact/fixed_rounding.hpp>
#include <boost/geometry/strategies/cartesian/exact/native_traits.hpp>
#include <boost/geometry/strategies/cartesian/exact/output_policy.hpp>
#include <boost/geometry/strategies/cartesian/exact/rational_rounding.hpp>
#include <boost/geometry/strategy/cartesian/side_robust.hpp>
#include <boost/geometry/util/numeric_cast.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{
namespace exact_cartesian
{

namespace point_detail
{

template <typename Number>
struct triple
{
    Number X, Y, W;
};

template <typename Number>
inline triple<Number> cross(triple<Number> const& a, triple<Number> const& b)
{
    return {a.Y * b.W - a.W * b.Y,
            a.W * b.X - a.X * b.W,
            a.X * b.Y - a.Y * b.X};
}

inline interval unknown_interval()
{
    return {0.0, (std::numeric_limits<double>::infinity)()};
}

inline filter_result compare_approximations(interval const& a, interval const& b)
{
    int result;
    if (forward_error::try_compare_scalars(a.approximate, a.error, b.approximate, b.error, result))
    {
        return {result, true};
    }
    return {0, false};
}

template <typename Coordinate>
inline interval input_interval(Coordinate value, std::false_type)
{
    double const approximate = static_cast<double>(value);
    if (!std::isfinite(approximate))
    {
        throw std::domain_error("An exact Cartesian input must be finite");
    }
    return {approximate, 0.0};
}

template <typename Coordinate>
inline interval input_interval(Coordinate value, std::true_type)
{
    double const approximate = static_cast<double>(value);
    // A full ULP encloses integral-to-double conversion, including INT64_MAX.
    // Small integers and types with at most 53 value bits need no widening.
    double const limit = 4503599627370496.0;
    if (std::numeric_limits<Coordinate>::digits <= std::numeric_limits<double>::digits
        || (approximate >= -limit && approximate <= limit))
    {
        return {approximate, 0.0};
    }
    double const up = std::nextafter(approximate,
                                    (std::numeric_limits<double>::infinity)());
    double const down = std::nextafter(approximate,
                                      -(std::numeric_limits<double>::infinity)());
    return {approximate, (std::max)(up - approximate, approximate - down)};
}

template <typename Coordinate>
inline bool represented_by_double(Coordinate value, double approximate, std::true_type)
{
    bool const negative = std::is_signed<Coordinate>::value && value < 0;
    auto const magnitude = negative ? std::uint64_t(0) - std::uint64_t(value) : std::uint64_t(value);
    double const absolute = std::abs(approximate);
    return absolute < std::ldexp(1.0, 64)
        && static_cast<std::uint64_t>(absolute) == magnitude
        && (value == 0 || std::signbit(approximate) == negative);
}

template <typename Coordinate>
inline bool represented_by_double(Coordinate value, double approximate, std::false_type)
{
    return double(value) == approximate;
}

inline interval coordinate_interval(exact_number const& value, double)
{
    precision_homogeneous_point<double> point;
    point.X = point_precision_detail::numerator<double>(value);
    point.W = point_precision_detail::denominator<double>(value);
    return point_precision_detail::bounds(point, 0);
}

inline interval literal_interval(exact_number const& value)
{
    return coordinate_interval(value, 0);
}

inline interval refined_interval(exact_number const& value);

inline bool filtered_sign(interval const& value, int& result)
{
    double const lower = value.lower();
    double const upper = value.upper();
    if (lower > 0)
    {
        result = 1;
        return true;
    }
    if (upper < 0)
    {
        result = -1;
        return true;
    }
    if (lower == 0 && upper == 0)
    {
        result = 0;
        return true;
    }
    return false;
}

template <typename Number>
inline int sign(Number const& value)
{
    return value > 0 ? 1 : value < 0 ? -1 : 0;
}

} // namespace point_detail

using homogeneous_coordinates = point_detail::triple<boost::multiprecision::cpp_int>;

// Native producers own endpoint tuples. Auxiliary values refer to their
// enclosing operation's typed precision entries.
template <typename Coordinate>
class exact_point
{
    static_assert(std::is_integral<Coordinate>::value
                  || std::is_same<Coordinate, float>::value
                  || std::is_same<Coordinate, double>::value,
                  "Exact Cartesian inputs support integral, float and double, not long double");

    using bounds_type = std::array<interval, 2>;

    template <typename Input>
    static exact_point mixed_input_impl(Input x, Input y, std::true_type)
    {
        static_assert(lossless_coordinate_conversion<Input, Coordinate>::value,
                      "Exact Cartesian promotion must preserve the complete native input type");
        return exact_point(static_cast<Coordinate>(x), static_cast<Coordinate>(y));
    }

    template <typename Input>
    static exact_point mixed_input_impl(Input const& x, Input const& y, std::false_type)
    {
        return materialized(point_precision_detail::input<Coordinate>(x, y));
    }

    template <std::size_t N, std::size_t D>
    static exact_point materialized(exact_cartesian::fixed_homogeneous<N, D> const& value)
    {
        exact_point result;
        result.m_precision = point_precision_detail::point_handle<Coordinate>::create(value);
        result.m_bounds = {{point_precision_detail::bounds(value, 0),
                            point_precision_detail::bounds(value, 1)}};
        return result;
    }

    using native_line = std::array<Coordinate, 4>;

    native_line supporting_line(std::size_t index) const
    {
        return {{m_values[4 * index], m_values[4 * index + 1],
                 m_values[4 * index + 2], m_values[4 * index + 3]}};
    }

    static bool line_contains(Coordinate const* line, Coordinate x, Coordinate y)
    {
        if (line[0] == line[2] && line[1] == line[3]) { return false; }
        if ((x == line[0] && y == line[1]) || (x == line[2] && y == line[3])) { return true; }
        if constexpr (std::is_integral<Coordinate>::value)
        {
            std::array<Coordinate, 6> const input{{line[0], line[1], line[2], line[3], x, y}};
            if constexpr (std::numeric_limits<Coordinate>::digits
                          + std::numeric_limits<Coordinate>::is_signed <= 16)
            {
                return arithmetic_detail::integer_orientation(line[0], line[1], line[2], line[3],
                                                               x, y, std::true_type{}) == 0;
            }
#if defined(BOOST_HAS_INT128)
            using wide = boost::int128_type;
            wide const dx1 = wide(line[2]) - wide(line[0]), dy1 = wide(line[3]) - wide(line[1]);
            wide const dx2 = wide(x) - wide(line[0]), dy2 = wide(y) - wide(line[1]);
            wide const bound = (std::numeric_limits<std::int64_t>::max)();
            if (dx1 >= -bound && dx1 <= bound && dy1 >= -bound && dy1 <= bound
                && dx2 >= -bound && dx2 <= bound && dy2 >= -bound && dy2 <= bound)
            {
                return dx1 * dy2 == dy1 * dx2;
            }
#else
            if (arithmetic_detail::fits_int64(line[0]) && arithmetic_detail::fits_int64(line[1])
                && arithmetic_detail::fits_int64(line[2]) && arithmetic_detail::fits_int64(line[3])
                && arithmetic_detail::fits_int64(x) && arithmetic_detail::fits_int64(y))
            {
                std::int64_t dx1, dy1, dx2, dy2;
                std::int64_t const bound = 2147483647;
                if (arithmetic_detail::subtract_int64(std::int64_t(line[2]), std::int64_t(line[0]), dx1)
                    && arithmetic_detail::subtract_int64(std::int64_t(line[3]), std::int64_t(line[1]), dy1)
                    && arithmetic_detail::subtract_int64(std::int64_t(x), std::int64_t(line[0]), dx2)
                    && arithmetic_detail::subtract_int64(std::int64_t(y), std::int64_t(line[1]), dy2)
                    && dx1 >= -bound && dx1 <= bound && dy1 >= -bound && dy1 <= bound
                    && dx2 >= -bound && dx2 <= bound && dy2 >= -bound && dy2 <= bound)
                {
                    return dx1 * dy2 == dy1 * dx2;
                }
            }
#endif
            return fixed_orientation<Coordinate>(input) == 0;
        }
        else
        {
            using vec2d = geometry::detail::precise_math::vec2d<double>;
            using side = strategy::side::side_robust<double, strategy::side::fp_equals_policy, 3>;
            side::epsilon_policy<double> policy;
            return geometry::detail::precise_math::orient2d<double, 3>
                (vec2d{double(line[0]), double(line[1])}, vec2d{double(line[2]), double(line[3])},
                 vec2d{double(x), double(y)}, policy) == 0;
        }
    }

    static bool line_contains(native_line const& line, Coordinate x, Coordinate y)
    {
        return line_contains(line.data(), x, y);
    }

    static bool same_line(native_line const& first, native_line const& second)
    {
        return line_contains(first, second[0], second[1])
            && line_contains(first, second[2], second[3])
            && (second[0] != second[2] || second[1] != second[3]);
    }

    bool known_line_member(native_line const& line) const
    {
        if (m_precision) { return false; }
        if (!m_intersection) { return line_contains(line, m_values[0], m_values[1]); }
        return same_line(line, supporting_line(0)) || same_line(line, supporting_line(1));
    }

    static bool get_supporting_line(exact_point const& a, exact_point const& b, native_line& result)
    {
        if (a.m_precision || b.m_precision) { return false; }
        if (!a.m_intersection && !b.m_intersection)
        {
            result = {{a.m_values[0], a.m_values[1], b.m_values[0], b.m_values[1]}};
            return true;
        }
        for (auto const* point : {&a, &b})
        {
            if (!point->m_intersection) { continue; }
            for (std::size_t i = 0; i != 2; ++i)
            {
                auto const line = point->supporting_line(i);
                if (a.known_line_member(line) && b.known_line_member(line))
                {
                    result = line;
                    return true;
                }
            }
        }
        return false;
    }

    static bool represented_producers(std::array<Coordinate, 8> const& input,
                                      std::array<double, 8>& output)
    {
        for (std::size_t i = 0; i != input.size(); ++i)
        {
            double const value = static_cast<double>(input[i]);
            if (!point_detail::represented_by_double(input[i], value, std::is_integral<Coordinate>()))
            {
                return false;
            }
            output[i] = value;
        }
        return true;
    }

    static bounds_type native_intersection_bounds(std::array<Coordinate, 8> const& values,
                                                 point_cache_detail::native_cache_handle& cache)
    {
        forward_error::point_approximation approximation;
        if (forward_error::try_intersection(values, approximation))
        {
            return {{{approximation.value[0], approximation.error[0]},
                     {approximation.value[1], approximation.error[1]}}};
        }
        bounds_type refined;
        std::array<bool, 2> certified{{false, false}};
        std::array<double, 8> represented;
        if (represented_producers(values, represented))
        {
            for (std::size_t dimension = 0; dimension != 2; ++dimension)
            {
                certified[dimension] = point_cache_detail::native_cache<Coordinate>::refine
                    (cache, represented, dimension, refined[dimension]);
            }
            if (certified[0] && certified[1]) { return refined; }
        }
        auto const exact = point_cache_detail::native_cache<Coordinate>::get(cache, true, values);
        for (std::size_t dimension = 0; dimension != 2; ++dimension)
        {
            if (!certified[dimension]) refined[dimension] = point_precision_detail::bounds(exact, dimension);
        }
        return refined;
    }

public:
    using input_coordinate_type = Coordinate;

    using fixed_point_type = typename native_fixed_traits<Coordinate>::intersection_type;
    using precision_point_type = precision_homogeneous_point<Coordinate>;

    exact_point() = default;

    exact_point(Coordinate x, Coordinate y)
        : m_values{{x, y}}
        , m_bounds{{point_detail::input_interval(x, std::is_integral<Coordinate>()),
                    point_detail::input_interval(y, std::is_integral<Coordinate>())}}
    {}

    interval approximation(std::size_t dimension) const
    {
        interval result = m_bounds.at(dimension);
        if (!m_precision && m_intersection)
        {
            point_cache_detail::native_cache<Coordinate>::cached_bounds(m_cache, dimension, result);
        }
        return result;
    }

    interval refined_approximation(std::size_t dimension) const
    {
        return refine_point(*this, dimension);
    }

    double output_approximation(std::size_t dimension) const
    {
        return m_bounds.at(dimension).approximate;
    }

    fixed_point_type fixed_homogeneous() const
    {
        if (!has_fixed_representation()) { throw std::logic_error("An auxiliary point requires precision_homogeneous()"); }
        return point_cache_detail::native_cache<Coordinate>::get(m_cache, m_intersection, m_values);
    }

    template <std::size_t N, std::size_t D>
    bool copy_compact_homogeneous(exact_cartesian::fixed_homogeneous<N, D>& output) const
    {
        return m_precision ? m_precision.copy_compact(output)
            : point_cache_detail::native_cache<Coordinate>::copy_compact(m_cache, m_intersection, m_values, output);
    }

    precision_point_type precision_homogeneous() const
    {
        auto const result = m_precision ? m_precision.get()
            : point_precision_detail::promote<Coordinate>(fixed_homogeneous());
        if (result.W <= 0) { throw std::domain_error("Intersection has no unique finite Cartesian point"); }
        return result;
    }

    static BOOST_NOINLINE bool try_fixed_orientation(exact_point const& a, exact_point const& b,
                                       exact_point const& c, int& result)
    {
        if (!a.has_fixed_representation() || !b.has_fixed_representation() || !c.has_fixed_representation())
        {
            exact_cartesian::fixed_homogeneous<128> x, y, z;
            if (a.copy_compact_homogeneous(x) && b.copy_compact_homogeneous(y) && c.copy_compact_homogeneous(z))
            {
                result = exact_cartesian::fixed_orientation(x, y, z);
                return true;
            }
            result = exact_cartesian::fixed_orientation
                (a.precision_homogeneous(), b.precision_homogeneous(), c.precision_homogeneous());
        }
        else
        {
            result = exact_cartesian::fixed_orientation
                (a.fixed_homogeneous(), b.fixed_homogeneous(), c.fixed_homogeneous());
        }
        return true;
    }

    // These two methods are explicit rational output boundaries, never used by
    // internal point predicates or constructions.
    homogeneous_coordinates homogeneous() const
    {
        auto convert = [](auto const& value)
        {
            return homogeneous_coordinates{boost::multiprecision::cpp_int(value.X),
                boost::multiprecision::cpp_int(value.Y), boost::multiprecision::cpp_int(value.W)};
        };
        return has_fixed_representation() ? convert(fixed_homogeneous()) : convert(precision_homogeneous());
    }

    exact_number exact_coordinate(std::size_t dimension) const
    {
        if (dimension > 1) { throw std::out_of_range("Exact Cartesian coordinate dimension"); }
        if (is_input()) { return exact_value(m_values[dimension]); }
        auto convert = [dimension](auto const& value)
        {
            exact_number result(boost::multiprecision::cpp_int(dimension == 0 ? value.X : value.Y));
            result /= exact_number(boost::multiprecision::cpp_int(value.W));
            return result;
        };
        return has_fixed_representation() ? convert(fixed_homogeneous()) : convert(precision_homogeneous());
    }

    bool is_input() const { return !m_precision && !m_intersection; }

    bool has_fixed_representation() const { return !m_precision; }

    bool native_coordinate(std::size_t dimension, Coordinate& value) const
    {
        if (dimension > 1) { throw std::out_of_range("Exact Cartesian coordinate dimension"); }
        if (m_precision) { return false; }
        if (!m_intersection) { value = m_values[dimension]; return true; }
        for (std::size_t i = 0; i != 8; i += 4)
        {
            if (m_values[i + dimension] == m_values[i + 2 + dimension])
            {
                value = m_values[i + dimension];
                return true;
            }
        }
        return false;
    }

    static interval refine_point(exact_point const& point, std::size_t dimension)
    {
        if (point.is_input()) { return point.m_bounds.at(dimension); }
        if (dimension > 1) { throw std::out_of_range("Exact Cartesian coordinate dimension"); }
        std::array<double, 8> values;
        interval result;
        if (point.native_producers(values)
            && point_cache_detail::native_cache<Coordinate>::refine(point.m_cache, values, dimension, result))
        {
            point.m_bounds[dimension] = result;
            return result;
        }
        // A filter must not materialize the exact value merely to refine an
        // unsupported construction. Leave that to the final predicate stage.
        return point.approximation(dimension);
    }

    static filter_result compare_native(exact_point const& point, std::size_t dimension, double value)
    {
        auto const refined = point_detail::compare_approximations
            (refine_point(point, dimension), {value, 0});
        if (refined.certain) { return refined; }
        std::array<double, 8> values;
        int result;
        if (point.native_producers(values)
            && point_cache_detail::native_cache<Coordinate>::compare
                    (point.m_cache, values, dimension, value, result))
        {
            return {result, true};
        }
        if (std::isfinite(value))
        {
            auto const target = fixed_input_homogeneous(value, value);
            return {point.has_fixed_representation() ? fixed_coordinate_compare(point.fixed_homogeneous(), target, dimension)
                : fixed_coordinate_compare(point.precision_homogeneous(), target, dimension), true};
        }
        return {0, false};
    }

    static filter_result compare_points(exact_point const& a, exact_point const& b, std::size_t dimension)
    {
        if (a.shares_node(b) || a.shares_construction(b)) { return {0, true}; }
        auto const refined = point_detail::compare_approximations
            (refine_point(a, dimension), refine_point(b, dimension));
        if (refined.certain) { return refined; }
        std::array<double, 8> first_values, second_values;
        int result;
        if (a.native_producers(first_values) && b.native_producers(second_values)
            && point_cache_detail::native_cache<Coordinate>::compare
                    (a.m_cache, first_values, b.m_cache, second_values, dimension, result))
        {
            return {result, true};
        }
        Coordinate native;
        if (a.native_coordinate(dimension, native))
        {
            double const value = static_cast<double>(native);
            if (point_detail::represented_by_double(native, value, std::is_integral<Coordinate>()))
            {
                auto const comparison = compare_native(b, dimension, value);
                if (comparison.certain) { return {-comparison.value, true}; }
            }
        }
        if (b.native_coordinate(dimension, native))
        {
            double const value = static_cast<double>(native);
            if (point_detail::represented_by_double(native, value, std::is_integral<Coordinate>()))
            {
                auto const comparison = compare_native(a, dimension, value);
                if (comparison.certain) { return comparison; }
            }
        }
        if (a.has_fixed_representation() && b.has_fixed_representation())
        {
            return {fixed_coordinate_compare(a.fixed_homogeneous(), b.fixed_homogeneous(), dimension), true};
        }
        return {fixed_coordinate_compare(a.precision_homogeneous(), b.precision_homogeneous(), dimension), true};
    }

    bool shares_node(exact_point const& other) const
    {
        if (m_precision || other.m_precision)
        {
            return m_precision && other.m_precision && m_precision.same(other.m_precision);
        }
        return m_intersection == other.m_intersection && m_values == other.m_values;
    }

    bool shares_construction(exact_point const& other) const
    {
        if (m_precision || other.m_precision) { return false; }
        return (m_intersection && other.known_line_member(supporting_line(0))
                               && other.known_line_member(supporting_line(1)))
            || (other.m_intersection && known_line_member(other.supporting_line(0))
                                     && known_line_member(other.supporting_line(1)));
    }

    bool has_producing_line(exact_point const& a, exact_point const& b) const
    {
        if (has_fixed_representation())
        {
            return m_intersection
                && ((a.known_line_member(supporting_line(0)) && b.known_line_member(supporting_line(0)))
                    || (a.known_line_member(supporting_line(1)) && b.known_line_member(supporting_line(1))));
        }
        return false;
    }

    template <typename A, typename B>
    bool native_points_on_producing_line(A const& a, B const& b) const
    {
        if constexpr (lossless_coordinate_conversion<geometry::coordinate_type_t<A>, Coordinate>::value
                      && lossless_coordinate_conversion<geometry::coordinate_type_t<B>, Coordinate>::value)
        {
            if (m_precision || !m_intersection) return false;
            for (std::size_t i = 0; i != 2; ++i)
            {
                auto const* line = m_values.data() + 4 * i;
                if (line_contains(line, geometry::get<0>(a), geometry::get<1>(a))
                    && line_contains(line, geometry::get<0>(b), geometry::get<1>(b)))
                    return true;
            }
        }
        return false;
    }

    bool native_producers(std::array<double, 8>& values) const
    {
        if (m_precision) { return false; }
        if (!m_intersection) { return false; }
        return represented_producers(m_values, values);
    }

    bool native_producers(std::array<Coordinate, 8>& values, std::true_type) const
    {
        if (m_precision || !m_intersection) { return false; }
        values = m_values;
        return true;
    }

    Coordinate input_coordinate(std::size_t dimension) const
    {
        if (!is_input())
        {
            throw std::logic_error("An intersection is not an input leaf");
        }
        if (dimension > 1) { throw std::out_of_range("Exact Cartesian coordinate dimension"); }
        return m_values[dimension];
    }

    // Compatibility for trait conversions, not the strategy's input path.
    void set_exact_coordinate(std::size_t dimension, exact_number const& value)
    {
        if (dimension > 1)
        {
            throw std::out_of_range("Exact Cartesian coordinate dimension");
        }
        auto const replaced = point_precision_detail::replace<Coordinate>(precision_homogeneous(), dimension, value);
        if (is_input())
        {
            auto const low = fixed_input_homogeneous((std::numeric_limits<Coordinate>::lowest)(),
                                                     (std::numeric_limits<Coordinate>::lowest)());
            auto const high = fixed_input_homogeneous((std::numeric_limits<Coordinate>::max)(),
                                                      (std::numeric_limits<Coordinate>::max)());
            Coordinate native;
            if (fixed_coordinate_compare(replaced, low, dimension) >= 0
                && fixed_coordinate_compare(replaced, high, dimension) <= 0
                && try_fixed_round(replaced, dimension, native)
                && fixed_coordinate_compare(replaced, fixed_input_homogeneous(native, native), dimension) == 0)
            {
                m_values[dimension] = native;
                m_bounds[dimension] = point_detail::input_interval(native, std::is_integral<Coordinate>());
                return;
            }
        }
        *this = materialized(replaced);
    }

    template <typename Input>
    static exact_point mixed_input(Input const& x, Input const& y)
    {
        return mixed_input_impl(x, y, fixed_native_supported<Input>());
    }

    static exact_point from_precision(precision_point_type const& value)
    {
        return materialized(value);
    }

    template <typename Other>
    static exact_point from_point(exact_point<Other> const& input)
    {
        static_assert(std::is_same<Coordinate, Other>::value,
                      "Constructed exact points use one operation coordinate type");
        if (input.is_input()) return mixed_input(input.input_coordinate(0), input.input_coordinate(1));
        return materialized(input.precision_homogeneous());
    }

    template <std::size_t N, std::size_t D>
    static exact_point from_homogeneous(exact_cartesian::fixed_homogeneous<N, D> value)
    {
        if (value.W != 0) fixed_normalize(value);
        return materialized(value);
    }

    static exact_point splice_coordinates(exact_point const& x_owner, std::size_t x_axis,
                                           exact_point const& y_owner, std::size_t y_axis)
    {
        if (x_axis > 1 || y_axis > 1) { throw std::out_of_range("Exact Cartesian coordinate dimension"); }
        if (x_axis == 0 && y_axis == 1 && x_owner.shares_node(y_owner)) { return x_owner; }
        Coordinate x, y;
        if (x_owner.native_coordinate(x_axis, x) && y_owner.native_coordinate(y_axis, y))
        {
            return exact_point(x, y);
        }
        if (x_owner.has_fixed_representation() && y_owner.has_fixed_representation())
            return from_homogeneous(fixed_splice(x_owner.fixed_homogeneous(), x_axis,
                                                 y_owner.fixed_homogeneous(), y_axis));
        return from_homogeneous(fixed_splice(x_owner.precision_homogeneous(), x_axis,
                                             y_owner.precision_homogeneous(), y_axis));
    }

    static exact_point intersection(exact_point const& a, exact_point const& b,
                                    exact_point const& c, exact_point const& d)
    {
        native_line first, second;
        if (get_supporting_line(a, b, first) && get_supporting_line(c, d, second))
        {
            exact_point result;
            result.m_intersection = true;
            std::copy(first.begin(), first.end(), result.m_values.begin());
            std::copy(second.begin(), second.end(), result.m_values.begin() + 4);
            result.m_bounds = native_intersection_bounds(result.m_values, result.m_cache);
            return result;
        }
        return materialized(point_precision_detail::intersection<Coordinate>(a.precision_homogeneous(), b.precision_homogeneous(),
            c.precision_homogeneous(), d.precision_homogeneous()));
    }

    static exact_point literal(exact_number const& x, exact_number const& y)
    {
        return materialized(point_precision_detail::literal<Coordinate>(x, y));
    }

private:
    template <typename> friend class exact_point;
    bool m_intersection = false;
    std::array<Coordinate, 8> m_values{};
    mutable bounds_type m_bounds{};
    point_precision_detail::point_handle<Coordinate> m_precision;
    mutable point_cache_detail::native_cache_handle m_cache;
};

namespace point_detail
{

inline exact_number const& input_exact_value(exact_number const& value) { return value; }

template <typename T>
inline auto input_exact_value(T value) -> decltype(exact_value(value)) { return exact_value(value); }

template <typename Coordinate, typename Input>
inline exact_point<Coordinate> input_point(Input x, Input y, std::true_type)
{
    return exact_point<Coordinate>(x, y);
}

template <typename Coordinate, typename Input>
inline exact_point<Coordinate> input_point(Input const& x, Input const& y, std::false_type)
{
    // Mixed input types must not be narrowed to the strategy's native leaf type.
    return exact_point<Coordinate>::mixed_input(x, y);
}

} // namespace point_detail

template <typename Coordinate, typename Point>
inline exact_point<Coordinate> make_point(Point const& input)
{
    static_assert(geometry::dimension<Point>::value == 2, "Exact Cartesian points are two-dimensional");
    static_assert(std::is_same<typename geometry::coordinate_system<Point>::type,
                               cs::cartesian>::value, "Exact Cartesian input required");
    using input_coordinate = typename geometry::coordinate_type<Point>::type;
    return point_detail::input_point<Coordinate>(geometry::get<0>(input), geometry::get<1>(input),
        std::is_same<Coordinate, input_coordinate>());
}

template <typename Point>
inline auto make_point(Point const& input)
{
    return make_point<typename geometry::coordinate_type<Point>::type>(input);
}

template <typename Coordinate>
inline exact_point<Coordinate> make_point(exact_point<Coordinate> const& input)
{
    return input;
}

template <typename Coordinate, typename Other>
inline typename std::enable_if<!std::is_same<Coordinate, Other>::value, exact_point<Coordinate>>::type
make_point(exact_point<Other> const& input)
{
    return exact_point<Coordinate>::from_point(input);
}

// Precondition: the producing lines have a unique finite intersection. A zero
// approximate denominator requires refinement already during construction.
template <typename Coordinate>
inline exact_point<Coordinate> construct_intersection(exact_point<Coordinate> const& a,
                                                      exact_point<Coordinate> const& b,
                                                      exact_point<Coordinate> const& c,
                                                      exact_point<Coordinate> const& d)
{
    return exact_point<Coordinate>::intersection(a, b, c, d);
}

namespace point_detail
{

template <typename Coordinate>
inline forward_error::point_approximation forward_approximation(exact_point<Coordinate> const& point,
                                                                bool refine = false)
{
    auto const x = refine ? point.refined_approximation(0) : point.approximation(0);
    auto const y = refine ? point.refined_approximation(1) : point.approximation(1);
    forward_error::point_approximation result{{{x.approximate, y.approximate}}, {{x.error, y.error}}};
    return result;
}

template <typename Coordinate>
inline int compare(exact_point<Coordinate> const& a, exact_point<Coordinate> const& b,
                   std::size_t dimension)
{
    if (a.is_input() && b.is_input())
    {
        Coordinate const first = a.input_coordinate(dimension), second = b.input_coordinate(dimension);
        return first < second ? -1 : first > second ? 1 : 0;
    }
    if (&a == &b) { return 0; }
    auto const initial = compare_approximations(a.approximation(dimension), b.approximation(dimension));
    if (initial.certain) { return initial.value; }
    if (a.shares_node(b) || a.shares_construction(b)) { return 0; }
    Coordinate first, second;
    if (a.native_coordinate(dimension, first) && b.native_coordinate(dimension, second))
        return (first > second) - (first < second);
    auto const refined = compare_approximations(a.refined_approximation(dimension),
                                                b.refined_approximation(dimension));
    if (refined.certain)
    {
        return refined.value;
    }
    auto const polynomial = exact_point<Coordinate>::compare_points(a, b, dimension);
    return polynomial.value;
}

template <typename Coordinate>
inline bool leaf_orientation(exact_point<Coordinate> const& a,
                             exact_point<Coordinate> const& b,
                             exact_point<Coordinate> const& c, int& result,
                             std::true_type)
{
    if (a.is_input() && b.is_input() && c.is_input())
    {
        std::array<Coordinate, 6> const values{{a.input_coordinate(0), a.input_coordinate(1),
                                               b.input_coordinate(0), b.input_coordinate(1),
                                               c.input_coordinate(0), c.input_coordinate(1)}};
        if (std::numeric_limits<Coordinate>::digits + std::numeric_limits<Coordinate>::is_signed <= 16)
        {
            result = arithmetic_detail::integer_orientation(values[0], values[1], values[2], values[3],
                                                              values[4], values[5], std::true_type());
            return true;
        }
#if defined(BOOST_HAS_INT128)
        using wide = boost::int128_type;
        wide const dx1 = wide(values[2]) - wide(values[0]), dy1 = wide(values[3]) - wide(values[1]);
        wide const dx2 = wide(values[4]) - wide(values[0]), dy2 = wide(values[5]) - wide(values[1]);
        wide const bound = (std::numeric_limits<std::int64_t>::max)();
        if (dx1 >= -bound && dx1 <= bound && dy1 >= -bound && dy1 <= bound
            && dx2 >= -bound && dx2 <= bound && dy2 >= -bound && dy2 <= bound)
        {
            wide const left = dx1 * dy2, right = dy1 * dx2;
            result = (left > right) - (left < right);
            return true;
        }
#endif
        result = fixed_orientation(values);
        return true;
    }
    return false;
}

template <typename Coordinate>
inline bool leaf_orientation(exact_point<Coordinate> const&, exact_point<Coordinate> const&,
                             exact_point<Coordinate> const&, int&, std::false_type)
{
    return false;
}

template <typename Coordinate>
inline bool floating_leaf_orientation(exact_point<Coordinate> const& a,
                                      exact_point<Coordinate> const& b,
                                      exact_point<Coordinate> const& c, int& result,
                                      std::true_type)
{
    if (!a.is_input() || !b.is_input() || !c.is_input())
    {
        return false;
    }
    using vec2d = geometry::detail::precise_math::vec2d<double>;
    using side = strategy::side::side_robust<double, strategy::side::fp_equals_policy, 3>;
    side::epsilon_policy<double> policy;
    double const determinant = geometry::detail::precise_math::orient2d<double, 3>
        (vec2d{double(a.input_coordinate(0)), double(a.input_coordinate(1))},
         vec2d{double(b.input_coordinate(0)), double(b.input_coordinate(1))},
         vec2d{double(c.input_coordinate(0)), double(c.input_coordinate(1))}, policy);
    result = (determinant > 0) - (determinant < 0);
    return true;
}

template <typename Coordinate>
inline bool floating_leaf_orientation(exact_point<Coordinate> const&, exact_point<Coordinate> const&,
                                      exact_point<Coordinate> const&, int&, std::false_type)
{
    return false;
}

} // namespace point_detail

template <typename Coordinate>
inline int compare_x(exact_point<Coordinate> const& a, exact_point<Coordinate> const& b)
{
    return point_detail::compare(a, b, 0);
}

template <typename Coordinate>
inline int compare_y(exact_point<Coordinate> const& a, exact_point<Coordinate> const& b)
{
    return point_detail::compare(a, b, 1);
}

template <typename Coordinate>
inline bool equal_points(exact_point<Coordinate> const& a, exact_point<Coordinate> const& b)
{
    if (a.is_input() && b.is_input())
    {
        return a.input_coordinate(0) == b.input_coordinate(0)
            && a.input_coordinate(1) == b.input_coordinate(1);
    }
    if (&a == &b) { return true; }
    // Either ordinate can disprove equality before the other needs refinement.
    auto const x = point_detail::compare_approximations(a.approximation(0), b.approximation(0));
    if (x.certain && x.value != 0) { return false; }
    auto const y = point_detail::compare_approximations(a.approximation(1), b.approximation(1));
    if (y.certain && y.value != 0) { return false; }
    if (a.shares_node(b) || a.shares_construction(b)) { return true; }
    return compare_x(a, b) == 0 && compare_y(a, b) == 0;
}

template <typename Coordinate>
inline int orientation(exact_point<Coordinate> const& a, exact_point<Coordinate> const& b,
                       exact_point<Coordinate> const& c)
{
    if (&a == &b || &a == &c || &b == &c)
    {
        return 0;
    }
    int result;
    if (point_detail::leaf_orientation(a, b, c, result, std::is_integral<Coordinate>())
        || point_detail::floating_leaf_orientation(a, b, c, result,
                                                   std::is_floating_point<Coordinate>()))
    {
        return result;
    }
    if (forward_error::try_orientation(point_detail::forward_approximation(a),
            point_detail::forward_approximation(b), point_detail::forward_approximation(c), result))
    {
        return result;
    }
    if (a.shares_node(b) || a.shares_node(c) || b.shares_node(c)) { return 0; }
    if (a.shares_construction(b) || a.shares_construction(c) || b.shares_construction(c)) { return 0; }
    if (a.has_producing_line(b, c) || b.has_producing_line(a, c) || c.has_producing_line(a, b)) { return 0; }
    if (!a.is_input() || !b.is_input() || !c.is_input())
    {
        if (forward_error::try_orientation(point_detail::forward_approximation(a, true),
                point_detail::forward_approximation(b, true), point_detail::forward_approximation(c, true), result))
        {
            return result;
        }
    }
    exact_point<Coordinate>::try_fixed_orientation(a, b, c, result);
    return result;
}

template <typename Coordinate>
inline bool on_segment(exact_point<Coordinate> const& a, exact_point<Coordinate> const& b,
                       exact_point<Coordinate> const& point)
{
    return orientation(a, b, point) == 0
        && compare_x(a, point) * compare_x(b, point) <= 0
        && compare_y(a, point) * compare_y(b, point) <= 0;
}

namespace point_detail
{

using output_integer = boost::multiprecision::cpp_int;
using rational_rounding::rounded_quotient;

template <typename Output, typename Enable = void>
struct output_rounder
{
    static Output apply(exact_number const&)
    {
        static_assert(std::is_integral<Output>::value
                      || std::is_same<Output, float>::value
                      || std::is_same<Output, double>::value,
                      "Exact Cartesian output supports integral, float and double, not long double");
        return Output();
    }
};

template <typename Output>
struct output_rounder<Output, typename std::enable_if<std::is_integral<Output>::value>::type>
{
    static Output apply(exact_number const& value)
    {
        output_integer numerator = boost::multiprecision::numerator(value);
        bool const negative = numerator < 0;
        if (negative)
        {
            numerator = -numerator;
        }
        output_integer result = rounded_quotient(numerator, boost::multiprecision::denominator(value));
        if (negative)
        {
            result = -result;
        }
        if (result < output_integer((std::numeric_limits<Output>::min)())
            || result > output_integer((std::numeric_limits<Output>::max)()))
        {
            throw std::overflow_error("Exact Cartesian integral output is out of range");
        }
        return result.template convert_to<Output>();
    }
};

template <>
struct output_rounder<exact_number, void>
{
    static exact_number const& apply(exact_number const& value) { return value; }
};

template <typename Output>
struct output_rounder<Output, typename std::enable_if<std::is_same<Output, float>::value
                                                  || std::is_same<Output, double>::value>::type>
{
    static Output apply(exact_number const& value)
    {
        return rational_rounding::round<Output>(value).value;
    }
};

inline interval refined_interval(exact_number const& value)
{
    return literal_interval(value);
}

template <typename Coordinate, typename OutputPoint>
inline bool copy_input(exact_point<Coordinate> const& point, OutputPoint& output, std::true_type)
{
    if (!point.is_input()) { return false; }
    geometry::set<0>(output, point.input_coordinate(0));
    geometry::set<1>(output, point.input_coordinate(1));
    return true;
}

template <typename Coordinate, typename OutputPoint>
inline bool copy_input(exact_point<Coordinate> const&, OutputPoint&, std::false_type)
{
    return false;
}

template <typename Coordinate, typename Output>
inline void round_fixed_output(exact_point<Coordinate> const& point, Output& x, Output& y,
                               bool& ready_x, bool& ready_y, std::true_type)
{
    if (ready_x && ready_y) { return; }
    auto round = [&](auto const& value)
    {
        if (!ready_x) { ready_x = try_fixed_round(value, 0, x); }
        if (!ready_y) { ready_y = try_fixed_round(value, 1, y); }
    };
    if (point.has_fixed_representation()) { round(point.fixed_homogeneous()); }
    else { round(point.precision_homogeneous()); }
}

template <typename Coordinate, typename Output>
inline void round_fixed_output(exact_point<Coordinate> const&, Output&, Output&,
                               bool&, bool&, std::false_type)
{}

} // namespace point_detail

template <typename Output, typename OutputPolicy = strategies::relate::approximate_output,
          typename Coordinate>
inline Output output_coordinate(exact_point<Coordinate> const& point, std::size_t dimension)
{
    static_assert(std::is_same<OutputPolicy, strategies::relate::approximate_output>::value
        || std::is_same<OutputPolicy, strategies::relate::nearest_output>::value
        || is_bounded_ulp_output<OutputPolicy>::value,
        "Unknown exact Cartesian output policy");
    if constexpr (std::is_same<Output, exact_number>::value)
    {
        return point.exact_coordinate(dimension);
    }
    else
    {
        if (point.is_input()) return geometry::util::numeric_cast<Output>(point.input_coordinate(dimension));
        if constexpr (std::is_same<OutputPolicy, strategies::relate::approximate_output>::value
                      || std::is_integral<Output>::value)
        {
            return geometry::util::numeric_cast<Output>(point.output_approximation(dimension));
        }
        else
        {
            static_assert(std::is_same<Output, float>::value || std::is_same<Output, double>::value,
                          "Nearest floating output requires float or double");
            Output rounded;
            std::array<double, 8> values;
            if (point.native_producers(values) && try_round_intersection(values, dimension, rounded))
            {
                return rounded;
            }
            if (point.has_fixed_representation())
            {
                if (try_fixed_round(point.fixed_homogeneous(), dimension, rounded)) return rounded;
            }
            else if (try_fixed_round(point.precision_homogeneous(), dimension, rounded)) return rounded;
            return point_detail::output_rounder<Output>::apply(point.exact_coordinate(dimension));
        }
    }
}

// Internal destinations preserve implicit definitions, regardless of policy.
template <typename OutputPolicy = strategies::relate::approximate_output, typename Coordinate>
inline void assign_output(exact_point<Coordinate> const& point, exact_point<Coordinate>& output)
{
    output = point;
}

// Only this boundary loses exact coordinates. Integer conversion follows BG's
// numeric_cast, not the origin-relative rounding of native integer construction.
template <typename OutputPolicy = strategies::relate::approximate_output,
          typename Coordinate, typename OutputPoint>
inline void assign_output(exact_point<Coordinate> const& point, OutputPoint& output)
{
    static_assert(geometry::dimension<OutputPoint>::value == 2, "Exact Cartesian output is two-dimensional");
    static_assert(std::is_same<OutputPolicy, strategies::relate::approximate_output>::value
        || std::is_same<OutputPolicy, strategies::relate::nearest_output>::value
        || is_bounded_ulp_output<OutputPolicy>::value,
        "Unknown exact Cartesian output policy");
    using coordinate = typename geometry::coordinate_type<OutputPoint>::type;
    coordinate x, y;
    if constexpr (std::is_same<coordinate, exact_number>::value)
    {
        x = point_detail::output_rounder<coordinate>::apply(point.exact_coordinate(0));
        y = point_detail::output_rounder<coordinate>::apply(point.exact_coordinate(1));
    }
    else
    {
        static_assert(std::is_integral<coordinate>::value || std::is_same<coordinate, float>::value
            || std::is_same<coordinate, double>::value,
            "Exact Cartesian output supports integral, float and double");
        if (point.is_input())
        {
            x = geometry::util::numeric_cast<coordinate>(point.input_coordinate(0));
            y = geometry::util::numeric_cast<coordinate>(point.input_coordinate(1));
        }
        else if constexpr (std::is_same<OutputPolicy, strategies::relate::approximate_output>::value
                           || std::is_integral<coordinate>::value)
        {
            x = geometry::util::numeric_cast<coordinate>(point.output_approximation(0));
            y = geometry::util::numeric_cast<coordinate>(point.output_approximation(1));
        }
        else
        {
            std::array<double, 8> values;
            bool const native = point.native_producers(values);
            bool ready_x = native && try_round_intersection(values, 0, x);
            bool ready_y = native && try_round_intersection(values, 1, y);
            point_detail::round_fixed_output(point, x, y, ready_x, ready_y, std::true_type());
            if (!ready_x) x = point_detail::output_rounder<coordinate>::apply(point.exact_coordinate(0));
            if (!ready_y) y = point_detail::output_rounder<coordinate>::apply(point.exact_coordinate(1));
        }
    }
    geometry::set<0>(output, x);
    geometry::set<1>(output, y);
}

} // namespace exact_cartesian
}}}} // namespace boost::geometry::strategies::detail

namespace boost { namespace geometry { namespace traits
{

template <typename Coordinate>
struct tag<strategies::detail::exact_cartesian::exact_point<Coordinate>>
{
    using type = point_tag;
};

template <typename Coordinate>
struct coordinate_type<strategies::detail::exact_cartesian::exact_point<Coordinate>>
{
    using type = strategies::detail::exact_cartesian::exact_number;
};

template <typename Coordinate>
struct coordinate_system<strategies::detail::exact_cartesian::exact_point<Coordinate>>
{
    using type = cs::cartesian;
};

template <typename Coordinate>
struct dimension<strategies::detail::exact_cartesian::exact_point<Coordinate>>
    : std::integral_constant<std::size_t, 2>
{};

template <typename Coordinate, std::size_t Dimension>
struct access<strategies::detail::exact_cartesian::exact_point<Coordinate>, Dimension>
{
    static_assert(Dimension < 2, "Exact Cartesian coordinate dimension");
    using point_type = strategies::detail::exact_cartesian::exact_point<Coordinate>;
    using coordinate = strategies::detail::exact_cartesian::exact_number;

    static coordinate get(point_type const& point)
    {
        return point.exact_coordinate(Dimension);
    }

    static void set(point_type& point, coordinate const& value)
    {
        point.set_exact_coordinate(Dimension, value);
    }
};

}}} // namespace boost::geometry::traits

namespace boost { namespace geometry { namespace detail { namespace conversion
{

// Full-point copies preserve producers and native leaves. Partial-coordinate
// copies intentionally retain the generic setter-based implementation.
template <typename Source, typename Coordinate>
struct point_to_point<Source, strategies::detail::exact_cartesian::exact_point<Coordinate>, 0, 2>
{
    using destination_type = strategies::detail::exact_cartesian::exact_point<Coordinate>;
    static void apply(Source const& source, destination_type& destination)
    {
        destination = strategies::detail::exact_cartesian::make_point<Coordinate>(source);
    }
};

}}}} // namespace boost::geometry::detail::conversion

#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_POINT_HPP
