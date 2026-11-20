// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_STATIC_PREDICATES_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_STATIC_PREDICATES_HPP

#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <boost/geometry/strategies/cartesian/exact/point.hpp>
#include <boost/geometry/strategies/cartesian/exact/fixed_measures.hpp>
#include <boost/geometry/strategies/cartesian/exact/native_predicates.hpp>
#include <boost/geometry/util/select_most_precise.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{
namespace exact_cartesian
{

template <typename Root> class borrowed_point;
template <typename Root, typename A, typename B, typename P> int root_orientation(A const&, B const&, P const&);
template <std::size_t D, typename Root, typename A, typename B> int root_compare_coordinate(A const&, B const&);
template <typename Root, typename A, typename B> bool root_equal(A const&, B const&);
template <typename Root, typename A, typename B, typename P> int root_direction(A const&, B const&, P const&);

namespace static_predicates_detail
{

template <typename Point, typename = void>
struct root_of { using type = void; };
template <typename Point>
struct root_of<Point, std::void_t<typename std::decay_t<Point>::root_type>>
{ using type = typename std::decay_t<Point>::root_type; };
template <typename... Points> struct common_root;
template <> struct common_root<> { using type = void; };
template <typename Point, typename... Points>
struct common_root<Point, Points...>
{
    using first = typename root_of<Point>::type;
    using rest = typename common_root<Points...>::type;
    static_assert(std::is_void<first>::value || std::is_void<rest>::value
        || std::is_same<first, rest>::value, "Exact predicate operands must share their Root");
    using type = std::conditional_t<std::is_void<first>::value, rest, first>;
};

template <typename C>
inline exact_point<C> own_point(exact_point<C> const& point)
{
    return point;
}

template <typename Root>
inline borrowed_point<Root> own_point(borrowed_point<Root> const& point)
{ return point; }

template <typename Point>
inline auto own_point(Point const& point) -> exact_point<typename geometry::coordinate_type<Point>::type>
{
    using coordinate = typename geometry::coordinate_type<Point>::type;
    static_assert(fixed_native_supported<coordinate>::value, "Static predicates require native finite coordinates");
    return make_point<coordinate>(point);
}

template <typename Point> struct exact_input : std::false_type {};
template <typename C> struct exact_input<exact_point<C>> : std::true_type {};
template <typename Root> struct exact_input<borrowed_point<Root>> : std::true_type {};

template <typename Coordinate, typename Point>
inline auto predicate_point(Point const& point, std::true_type) { return own_point(point); }

template <typename Coordinate, typename Point>
inline auto predicate_point(Point const& point, std::false_type) { return make_point<Coordinate>(point); }

// An explicit strategy coordinate admits retained rational input fixtures via
// the point precision cache. Native and existing exact points never take that
// conversion, so mixed native searches remain lossless and cache-free.
template <typename Coordinate, typename Point>
inline auto predicate_point(Point const& point)
{
    using retain = std::integral_constant<bool, std::is_void<Coordinate>::value
        || fixed_native_supported<typename geometry::coordinate_type<Point>::type>::value
        || exact_input<typename std::decay<Point>::type>::value>;
    return predicate_point<Coordinate>(point, retain{});
}

template <typename Coordinate, typename Native>
inline exact_point<Native> const& predicate_point(exact_point<Native> const& point)
{ return point; }

template <typename Coordinate, typename Point>
inline decltype(auto) predicate_operand(Point const& point)
{
    if constexpr (!std::is_void<typename root_of<Point>::type>::value) return (point);
    else if constexpr (fixed_native_supported<geometry::coordinate_type_t<Point>>::value) return (point);
    else return predicate_point<Coordinate>(point);
}

template <typename C>
inline forward_error::point_approximation approximation(exact_point<C> const& point, bool refine = false)
{
    auto const x = refine ? point.refined_approximation(0) : point.approximation(0);
    auto const y = refine ? point.refined_approximation(1) : point.approximation(1);
    return {{{x.approximate, y.approximate}}, {{x.error, y.error}}};
}

template <typename Point>
inline std::enable_if_t<fixed_native_supported<geometry::coordinate_type_t<Point>>::value,
                       forward_error::point_approximation>
approximation(Point const& point, bool = false)
{
    forward_error::point_approximation result;
    forward_error::try_native_point(geometry::get<0>(point), geometry::get<1>(point), result);
    return result;
}

template <typename Point, typename Function>
inline decltype(auto) with_homogeneous(Point const& point, Function&& function)
{
    if constexpr (!std::is_void<typename root_of<Point>::type>::value)
        return function(point.homogeneous());
    else if constexpr (fixed_native_supported<geometry::coordinate_type_t<Point>>::value)
        return function(fixed_input_homogeneous(geometry::get<0>(point), geometry::get<1>(point)));
    else
    {
        if (point.has_fixed_representation()) return function(point.fixed_homogeneous());
        return function(point.precision_homogeneous());
    }
}

template <typename A, typename B, typename P>
BOOST_NOINLINE int exact_orientation(A const& a, B const& b, P const& p)
{
    return with_homogeneous(a, [&](auto const& first)
    {
        return with_homogeneous(b, [&](auto const& second)
        {
            return with_homogeneous(p, [&](auto const& third)
            { return fixed_orientation(first, second, third); });
        });
    });
}

template <typename A, typename B, typename P>
inline int orientation(A const& a, B const& b, P const& p)
{
    int result;
    if (forward_error::try_orientation(approximation(a), approximation(b), approximation(p), result)) return result;
    // A proper intersection lies on each producing line. Borrowed native
    // endpoints can certify this zero without constructing homogeneous points.
    if constexpr (fixed_native_supported<geometry::coordinate_type_t<A>>::value
                  && fixed_native_supported<geometry::coordinate_type_t<B>>::value
                  && exact_input<P>::value)
    {
        if (p.native_points_on_producing_line(a, b)) return 0;
    }
    else if constexpr (fixed_native_supported<geometry::coordinate_type_t<A>>::value
                       && exact_input<B>::value
                       && fixed_native_supported<geometry::coordinate_type_t<P>>::value)
    {
        if (b.native_points_on_producing_line(a, p)) return 0;
    }
    else if constexpr (exact_input<A>::value
                       && fixed_native_supported<geometry::coordinate_type_t<B>>::value
                       && fixed_native_supported<geometry::coordinate_type_t<P>>::value)
    {
        if (a.native_points_on_producing_line(b, p)) return 0;
    }
    if (forward_error::try_orientation(approximation(a, true), approximation(b, true), approximation(p, true), result))
        return result;
    return exact_orientation(a, b, p);
}

template <typename C>
inline interval coordinate_bounds(exact_point<C> const& point, std::size_t axis, bool refine)
{
    return refine ? point.refined_approximation(axis) : point.approximation(axis);
}

template <typename A, typename B>
inline int coordinate_compare(exact_point<A> const& a, exact_point<B> const& b, std::size_t axis)
{
    for (bool refine : {false, true})
    {
        auto const result = point_detail::compare_approximations(coordinate_bounds(a, axis, refine),
                                                                coordinate_bounds(b, axis, refine));
        if (result.certain) { return result.value; }
    }
    return a.has_fixed_representation() && b.has_fixed_representation()
        ? fixed_coordinate_compare(a.fixed_homogeneous(), b.fixed_homogeneous(), axis)
        : fixed_coordinate_compare(a.precision_homogeneous(), b.precision_homogeneous(), axis);
}

template <typename C>
inline int coordinate_compare(exact_point<C> const& a, exact_point<C> const& b, std::size_t axis)
{
    return point_detail::compare(a, b, axis);
}

template <typename A, typename B, typename P>
inline int orientation(exact_point<A> const& a, exact_point<B> const& b, exact_point<P> const& p)
{
    int result;
    for (bool refine : {false, true})
    {
        if (forward_error::try_orientation(approximation(a, refine), approximation(b, refine),
                                            approximation(p, refine), result)) { return result; }
    }
    return a.has_fixed_representation() && b.has_fixed_representation() && p.has_fixed_representation()
        ? fixed_orientation(a.fixed_homogeneous(), b.fixed_homogeneous(), p.fixed_homogeneous())
        : fixed_orientation(a.precision_homogeneous(), b.precision_homogeneous(), p.precision_homogeneous());
}

template <typename C>
inline int orientation(exact_point<C> const& a, exact_point<C> const& b, exact_point<C> const& p)
{
    return exact_cartesian::orientation(a, b, p);
}

template <std::size_t AN, std::size_t AD, std::size_t BN, std::size_t BD,
          std::size_t PN, std::size_t PD>
inline int fixed_direction(fixed_homogeneous<AN, AD> const& a, fixed_homogeneous<BN, BD> const& b,
                            fixed_homogeneous<PN, PD> const& p)
{
    constexpr std::size_t first_bits = fixed_detail::maximum(BN + AD, AN + BD) + 1;
    constexpr std::size_t second_bits = fixed_detail::maximum(PN + BD, BN + PD) + 1;
    constexpr std::size_t bits = first_bits + second_bits + 1;
    auto const ax = fixed_multiply<first_bits>(b.X, a.W) - fixed_multiply<first_bits>(a.X, b.W);
    auto const ay = fixed_multiply<first_bits>(b.Y, a.W) - fixed_multiply<first_bits>(a.Y, b.W);
    auto const px = fixed_multiply<second_bits>(p.X, b.W) - fixed_multiply<second_bits>(b.X, p.W);
    auto const py = fixed_multiply<second_bits>(p.Y, b.W) - fixed_multiply<second_bits>(b.Y, p.W);
    // All omitted denominator factors are positive; two products need one carry.
    fixed_integer<bits> const value = fixed_multiply<bits>(ax, px) + fixed_multiply<bits>(ay, py);
    return value.sign();
}

template <typename A, typename B, typename P>
BOOST_NOINLINE int exact_direction(A const& a, B const& b, P const& p)
{
    return with_homogeneous(a, [&](auto const& first)
    {
        return with_homogeneous(b, [&](auto const& second)
        {
            return with_homogeneous(p, [&](auto const& third)
            { return fixed_direction(first, second, third); });
        });
    });
}

inline void require_zero(int value)
{
    if (value != 0) { throw std::invalid_argument("A static measure accepts only the zero scalar"); }
}

template <std::size_t AN, std::size_t AD, std::size_t BN, std::size_t BD>
inline int compare_axes(fixed_homogeneous<AN, AD> const& a, std::size_t a_axis,
                         fixed_homogeneous<BN, BD> b, std::size_t b_axis)
{
    if (a_axis != b_axis) { std::swap(b.X, b.Y); }
    return fixed_coordinate_compare(a, b, a_axis);
}

template <typename C, typename Native>
inline int compare_native(exact_point<C> const& point, std::size_t axis, Native value)
{
    C native;
    if (std::is_same<C, Native>::value && point.native_coordinate(axis, native))
    { return (native > value) - (native < value); }
    auto const target = point_detail::input_interval(value, std::is_integral<Native>());
    for (bool refine : {false, true})
    {
        auto const filtered = point_detail::compare_approximations(coordinate_bounds(point, axis, refine), target);
        if (filtered.certain) { return filtered.value; }
    }
    auto const fixed = fixed_input_homogeneous(value, value);
    return point.has_fixed_representation() ? fixed_coordinate_compare(point.fixed_homogeneous(), fixed, axis)
        : fixed_coordinate_compare(point.precision_homogeneous(), fixed, axis);
}

template <typename Point>
struct native_point : fixed_native_supported<typename geometry::coordinate_type<Point>::type> {};

template <typename C>
inline bool small_integer_coordinate(exact_point<C> const& point, std::size_t axis, std::int64_t& output)
{
    C native;
    if (!point.native_coordinate(axis, native)) { return false; }
    double const value = static_cast<double>(native);
    constexpr double limit = 1073741824.0; // 2^30; integers in this range convert exactly.
    if (!(value >= -limit && value <= limit)) { return false; }
    auto const integer = static_cast<std::int64_t>(value);
    if (static_cast<C>(integer) != native) { return false; }
    output = integer;
    return true;
}

template <typename A, typename B>
inline bool small_integer_squared_distance(exact_point<A> const& a, exact_point<B> const& b,
                                           std::uint64_t& output)
{
    std::int64_t ax, ay, bx, by;
    if (!small_integer_coordinate(a, 0, ax) || !small_integer_coordinate(a, 1, ay)
        || !small_integer_coordinate(b, 0, bx) || !small_integer_coordinate(b, 1, by)) { return false; }
    auto const dx = bx - ax, dy = by - ay;
    auto const x = static_cast<std::uint64_t>(dx < 0 ? -dx : dx);
    auto const y = static_cast<std::uint64_t>(dy < 0 ? -dy : dy);
    // |dx|,|dy| <= 2^31, so their sum of squares is <= 2^63.
    output = x * x + y * y;
    return true;
}

template <std::size_t Dim, typename Coordinate, typename A, typename B>
inline int coordinate_dispatch(A const& a, B const& b, std::integral_constant<int, 3>)
{
    using first = typename geometry::coordinate_type<A>::type;
    using second = typename geometry::coordinate_type<B>::type;
    auto const x = geometry::get<Dim>(a);
    auto const y = geometry::get<Dim>(b);
    if (std::is_same<first, second>::value) { return (x > y) - (x < y); }
    double ax, ex, ay, ey;
    int result;
    if (forward_error::try_native_coordinate(x, ax, ex) && forward_error::try_native_coordinate(y, ay, ey)
        && forward_error::try_compare_scalars(ax, ex, ay, ey, result)) { return result; }
    return fixed_coordinate_compare(fixed_input_homogeneous(x, x), fixed_input_homogeneous(y, y), 0);
}

template <std::size_t Dim, typename Coordinate, typename A, typename B>
inline int coordinate_dispatch(A const& a, B const& b, std::integral_constant<int, 1>)
{ return -compare_native(predicate_point<Coordinate>(b), Dim, geometry::get<Dim>(a)); }

template <std::size_t Dim, typename Coordinate, typename A, typename B>
inline int coordinate_dispatch(A const& a, B const& b, std::integral_constant<int, 2>)
{ return compare_native(predicate_point<Coordinate>(a), Dim, geometry::get<Dim>(b)); }

template <std::size_t Dim, typename Coordinate, typename A, typename B>
inline int coordinate_dispatch(A const& a, B const& b, std::integral_constant<int, 0>)
{ return coordinate_compare(predicate_point<Coordinate>(a), predicate_point<Coordinate>(b), Dim); }

template <std::size_t Dim, typename Coordinate, typename A, typename B>
inline int compare_coordinate(A const& a, B const& b)
{
    static_assert(Dim < 2 && geometry::dimension<A>::value == 2 && geometry::dimension<B>::value == 2,
                  "Exact Cartesian coordinates are two-dimensional");
    using root = typename common_root<A, B>::type;
    if constexpr (!std::is_void<root>::value) return root_compare_coordinate<Dim, root>(a, b);
    else return coordinate_dispatch<Dim, Coordinate>(a, b,
        std::integral_constant<int, (native_point<A>::value ? 1 : 0) + (native_point<B>::value ? 2 : 0)>{});
}

template <typename Coordinate, typename Point>
inline auto measure_point(Point const& point, std::true_type) { return own_point(point); }

template <typename Coordinate, typename Point>
inline auto measure_point(Point const& point, std::false_type) { return make_point<Coordinate>(point); }

} // namespace static_predicates_detail

template <std::size_t Dim, typename Coordinate = void, typename A, typename B>
inline int static_compare_coordinate(A const& a, B const& b)
{ return static_predicates_detail::compare_coordinate<Dim, Coordinate>(a, b); }

template <typename Coordinate = void, typename A, typename B>
inline int static_compare_coordinate(A const& a, B const& b, std::size_t axis)
{
    if (axis > 1) { throw std::out_of_range("Exact Cartesian coordinate dimension"); }
    return axis == 0 ? static_predicates_detail::compare_coordinate<0, Coordinate>(a, b)
                     : static_predicates_detail::compare_coordinate<1, Coordinate>(a, b);
}

template <typename Coordinate = void, typename A, typename B>
inline bool static_equal_points(A const& a, B const& b)
{
    using root = typename static_predicates_detail::common_root<A, B>::type;
    if constexpr (!std::is_void<root>::value) return root_equal<root>(a, b);
    else return static_compare_coordinate<0, Coordinate>(a, b) == 0
        && static_compare_coordinate<1, Coordinate>(a, b) == 0;
}

template <typename Coordinate = void, typename A, typename B, typename P>
inline int static_orientation(A const& a, B const& b, P const& p)
{
    using root = typename static_predicates_detail::common_root<A, B, P>::type;
    if constexpr (!std::is_void<root>::value) return root_orientation<root>(a, b, p);
    else if constexpr (static_predicates_detail::native_point<A>::value
                  && static_predicates_detail::native_point<B>::value
                  && static_predicates_detail::native_point<P>::value)
    {
        using selected = std::conditional_t<std::is_void<Coordinate>::value,
            typename geometry::select_most_precise<geometry::coordinate_type_t<A>,
                geometry::coordinate_type_t<B>, geometry::coordinate_type_t<P>>::type, Coordinate>;
        if constexpr (lossless_coordinate_conversion<geometry::coordinate_type_t<A>, selected>::value
                      && lossless_coordinate_conversion<geometry::coordinate_type_t<B>, selected>::value
                      && lossless_coordinate_conversion<geometry::coordinate_type_t<P>, selected>::value)
        {
            return native_orientation<selected>(a, b, p);
        }
        else
        {
            static_assert(std::is_void<Coordinate>::value,
                          "Exact strategy inputs must admit lossless coordinate promotion");
            auto const first = static_predicates_detail::approximation(a);
            auto const second = static_predicates_detail::approximation(b);
            auto const third = static_predicates_detail::approximation(p);
            int sign;
            if (forward_error::try_orientation(first, second, third, sign)) return sign;
            // Internal search bounds may have a different native format. Once
            // every actual value converts exactly, orient2d remains applicable.
            if (forward_error::detail::exact_coordinates(first)
                && forward_error::detail::exact_coordinates(second)
                && forward_error::detail::exact_coordinates(third))
                return strategy::side::side_robust<double, strategy::side::fp_equals_policy, 3>
                    ::apply(a, b, p);
            return fixed_orientation(fixed_input_homogeneous(geometry::get<0>(a), geometry::get<1>(a)),
                fixed_input_homogeneous(geometry::get<0>(b), geometry::get<1>(b)),
                fixed_input_homogeneous(geometry::get<0>(p), geometry::get<1>(p)));
        }
    }
    else
    {
        return static_predicates_detail::orientation(static_predicates_detail::predicate_operand<Coordinate>(a),
            static_predicates_detail::predicate_operand<Coordinate>(b), static_predicates_detail::predicate_operand<Coordinate>(p));
    }
}

template <typename Coordinate = void, typename A, typename B, typename P>
inline int static_direction(A const& a, B const& b, P const& p)
{
    using root = typename static_predicates_detail::common_root<A, B, P>::type;
    if constexpr (!std::is_void<root>::value) return root_direction<root>(a, b, p);
    else
    {
        auto const& x = static_predicates_detail::predicate_operand<Coordinate>(a);
        auto const& y = static_predicates_detail::predicate_operand<Coordinate>(b);
        auto const& z = static_predicates_detail::predicate_operand<Coordinate>(p);
        int result;
        if (forward_error::try_direction(static_predicates_detail::approximation(x),
                static_predicates_detail::approximation(y), static_predicates_detail::approximation(z), result))
        { return result; }
        if (static_orientation<Coordinate>(x, y, z) == 0)
        {
            int const dx = static_compare_coordinate<Coordinate>(y, x, 0);
            return dx != 0 ? dx * static_compare_coordinate<Coordinate>(z, y, 0)
                : static_compare_coordinate<Coordinate>(y, x, 1)
                    * static_compare_coordinate<Coordinate>(z, y, 1);
        }
        if (forward_error::try_direction(static_predicates_detail::approximation(x, true),
                static_predicates_detail::approximation(y, true), static_predicates_detail::approximation(z, true), result))
            return result;
        return static_predicates_detail::exact_direction(x, y, z);
    }
}

template <typename Coordinate = void, typename A, typename B, typename P>
inline int static_direction_code(A const& a, B const& b, P const& p)
{ return static_direction<Coordinate>(a, b, p); }

// Coordinate values own only their existing point and axis; no scalar arithmetic.
template <typename C>
struct static_coordinate_view
{
    exact_point<C> point;
    std::size_t axis;

    interval bounds(bool refine = false) const
    { return static_predicates_detail::coordinate_bounds(point, axis, refine); }
};

template <std::size_t Dimension, typename Coordinate = void, typename Point>
inline auto static_coordinate(Point const& point)
{
    static_assert(Dimension < 2, "Exact Cartesian coordinate dimension");
    auto owned = static_predicates_detail::predicate_point<Coordinate>(point);
    return static_coordinate_view<typename decltype(owned)::input_coordinate_type>{std::move(owned), Dimension};
}

template <typename A, typename B>
inline int compare(static_coordinate_view<A> const& a, static_coordinate_view<B> const& b)
{
    if (a.axis == b.axis) { return static_predicates_detail::coordinate_compare(a.point, b.point, a.axis); }
    for (bool refine : {false, true})
    {
        auto const filtered = point_detail::compare_approximations(a.bounds(refine), b.bounds(refine));
        if (filtered.certain) { return filtered.value; }
    }
    return a.point.has_fixed_representation() && b.point.has_fixed_representation()
        ? static_predicates_detail::compare_axes(a.point.fixed_homogeneous(), a.axis, b.point.fixed_homogeneous(), b.axis)
        : static_predicates_detail::compare_axes(a.point.precision_homogeneous(), a.axis, b.point.precision_homogeneous(), b.axis);
}

template <typename A, typename B = A>
struct static_squared_distance_value
{
    using compact_point_type = fixed_homogeneous<128>;
    using compact_pair_type = typename fixed_measure_detail::distance_capacity<128, 128, 128, 128>::type;
    exact_point<A> a;
    exact_point<B> b;
    double approximate = 0, error = 0;
    bool valid = true;
    bool unset = false;

    static_squared_distance_value() = default;
    static_squared_distance_value(int value)
    {
        if (value == -1) { unset = true; approximate = -1; }
        else { static_predicates_detail::require_zero(value); }
    }
    static_squared_distance_value(exact_point<A> first, exact_point<B> second)
        : a(std::move(first)), b(std::move(second))
    {
        forward_error::scalar_approximation result;
        valid = forward_error::try_squared_distance(static_predicates_detail::approximation(a),
                                                     static_predicates_detail::approximation(b), result);
        if (valid) { approximate = result.value; error = result.error; }
    }

    interval bounds(bool refine = false) const
    {
        if (unset) { return {-1, 0}; }
        if (!refine && valid) { return {approximate, error}; }
        forward_error::scalar_approximation result;
        if (forward_error::try_squared_distance(static_predicates_detail::approximation(a, refine),
                static_predicates_detail::approximation(b, refine), result))
            return {result.value, result.error};
        return {0, (std::numeric_limits<double>::infinity)()};
    }
    auto fixed_pair() const
    {
        auto result = fixed_squared_distance(a.fixed_homogeneous(), b.fixed_homogeneous());
        if (unset) { result.N = -1; result.D = 1; }
        return result;
    }
    auto precision_pair() const
    {
        auto result = fixed_squared_distance(a.precision_homogeneous(), b.precision_homogeneous());
        if (unset) { result.N = -1; result.D = 1; }
        return result;
    }
    bool compact_pair(compact_pair_type& output) const
    {
        if (unset) { output.N = -1; output.D = 1; return true; }
        compact_point_type first, second;
        if (!a.copy_compact_homogeneous(first) || !b.copy_compact_homogeneous(second)) { return false; }
        output = fixed_squared_distance(first, second);
        return true;
    }
    bool native() const { return a.has_fixed_representation() && b.has_fixed_representation(); }
    bool is_zero() const { return !unset && static_equal_points(a, b); }
    int sign() const { return unset ? -1 : is_zero() ? 0 : 1; }
};

template <typename C, typename B = C>
using squared_distance_value = static_squared_distance_value<C, B>;

template <typename A, typename B = A, typename P = A>
struct static_distance_measure_value
{
    using compact_point_type = fixed_homogeneous<128>;
    using compact_pair_type = typename fixed_measure_detail::determinant_capacity<128, 128, 128, 128, 128, 128>::type;
    exact_point<A> a;
    exact_point<B> b;
    exact_point<P> p;
    double approximate = 0, error = 0;
    bool valid = true;
    bool negated = false;

    static_distance_measure_value() = default;
    static_distance_measure_value(int zero) { static_predicates_detail::require_zero(zero); }
    static_distance_measure_value(exact_point<A> first, exact_point<B> second, exact_point<P> point)
        : a(std::move(first)), b(std::move(second)), p(std::move(point))
    {
        forward_error::scalar_approximation result;
        valid = forward_error::try_signed_determinant(static_predicates_detail::approximation(a),
            static_predicates_detail::approximation(b), static_predicates_detail::approximation(p), result);
        if (valid) { approximate = result.value; error = result.error; }
    }

    interval bounds(bool refine = false) const
    {
        if (!refine && valid) { return {approximate, error}; }
        forward_error::scalar_approximation result;
        if (forward_error::try_signed_determinant(static_predicates_detail::approximation(a, refine),
                static_predicates_detail::approximation(b, refine),
                static_predicates_detail::approximation(p, refine), result))
            return {negated ? -result.value : result.value, result.error};
        return {0, (std::numeric_limits<double>::infinity)()};
    }
    auto fixed_pair() const
    {
        auto result = fixed_signed_determinant(a.fixed_homogeneous(), b.fixed_homogeneous(), p.fixed_homogeneous());
        if (negated) { result.N = -result.N; }
        return result;
    }
    auto precision_pair() const
    {
        auto result = fixed_signed_determinant(a.precision_homogeneous(), b.precision_homogeneous(), p.precision_homogeneous());
        if (negated) { result.N = -result.N; }
        return result;
    }
    bool compact_pair(compact_pair_type& output) const
    {
        compact_point_type first, second, point;
        if (!a.copy_compact_homogeneous(first) || !b.copy_compact_homogeneous(second)
            || !p.copy_compact_homogeneous(point)) { return false; }
        output = fixed_signed_determinant(first, second, point);
        if (negated) { output.N = -output.N; }
        return true;
    }
    bool native() const
    { return a.has_fixed_representation() && b.has_fixed_representation() && p.has_fixed_representation(); }
    int sign() const
    {
        int const value = static_predicates_detail::orientation(a, b, p);
        return negated ? -value : value;
    }
    static_distance_measure_value operator-() const
    {
        auto result = *this;
        result.negated = !result.negated;
        result.approximate = -result.approximate;
        return result;
    }
};

template <typename Coordinate = void, typename A, typename B>
inline auto static_squared_distance(A const& a, B const& b)
{
    auto x = static_predicates_detail::measure_point<Coordinate>(a, std::is_void<Coordinate>{});
    auto y = static_predicates_detail::measure_point<Coordinate>(b, std::is_void<Coordinate>{});
    return static_squared_distance_value<typename decltype(x)::input_coordinate_type,
        typename decltype(y)::input_coordinate_type>(std::move(x), std::move(y));
}

template <typename Coordinate = void, typename A, typename B, typename P>
inline auto static_signed_determinant(A const& a, B const& b, P const& p)
{
    auto x = static_predicates_detail::measure_point<Coordinate>(a, std::is_void<Coordinate>{});
    auto y = static_predicates_detail::measure_point<Coordinate>(b, std::is_void<Coordinate>{});
    auto z = static_predicates_detail::measure_point<Coordinate>(p, std::is_void<Coordinate>{});
    return static_distance_measure_value<typename decltype(x)::input_coordinate_type,
        typename decltype(y)::input_coordinate_type, typename decltype(z)::input_coordinate_type>
        (std::move(x), std::move(y), std::move(z));
}

struct orientation_result
{
    int side = 0;
    bool is_zero() const { return side == 0; }
    bool is_positive() const { return side > 0; }
    bool is_negative() const { return side < 0; }
};

template <typename Coordinate = void, typename A, typename B, typename P>
inline orientation_result static_distance_measure(A const& a, B const& b, P const& p)
{ return {static_orientation<Coordinate>(a, b, p)}; }

namespace static_predicates_detail
{
template <typename T> struct is_measure : std::false_type {};
template <typename A, typename B> struct is_measure<static_squared_distance_value<A, B>> : std::true_type {};
template <typename A, typename B, typename P>
struct is_measure<static_distance_measure_value<A, B, P>> : std::true_type {};

template <typename A, typename B>
inline bool compare_small_integer_distances(A const&, B const&, int&) { return false; }

template <typename A, typename B, typename C, typename D>
inline bool compare_small_integer_distances(static_squared_distance_value<A, B> const& a,
                                           static_squared_distance_value<C, D> const& b, int& output)
{
    if (a.unset || b.unset) { return false; }
    std::uint64_t first, second;
    if (!small_integer_squared_distance(a.a, a.b, first)
        || !small_integer_squared_distance(b.a, b.b, second)) { return false; }
    output = (first > second) - (first < second);
    return true;
}
}

template <typename A, typename B>
inline typename std::enable_if<static_predicates_detail::is_measure<A>::value
    && static_predicates_detail::is_measure<B>::value, int>::type
compare_distance(A const& a, B const& b)
{
    auto const filtered = point_detail::compare_approximations(a.bounds(), b.bounds());
    if (filtered.certain) { return filtered.value; }
    int native;
    if (static_predicates_detail::compare_small_integer_distances(a, b, native)) { return native; }
    auto const refined = point_detail::compare_approximations(a.bounds(true), b.bounds(true));
    if (refined.certain) { return refined.value; }
    typename A::compact_pair_type first;
    typename B::compact_pair_type second;
    if (a.compact_pair(first) && b.compact_pair(second)) { return fixed_pair_compare(first, second); }
    return a.native() && b.native() ? fixed_pair_compare(a.fixed_pair(), b.fixed_pair())
                                   : fixed_pair_compare(a.precision_pair(), b.precision_pair());
}

template <typename T>
inline typename std::enable_if<static_predicates_detail::is_measure<T>::value, int>::type
compare_distance(T const& value, int zero)
{
    static_predicates_detail::require_zero(zero);
    return value.sign();
}

template <typename T>
inline typename std::enable_if<static_predicates_detail::is_measure<T>::value, int>::type
compare_distance(int zero, T const& value) { return -compare_distance(value, zero); }

#define BOOST_GEOMETRY_STATIC_MEASURE_COMPARE(OP) \
template <typename A, typename B> \
inline auto operator OP(A const& a, B const& b) -> decltype(compare_distance(a, b) OP 0) \
{ return compare_distance(a, b) OP 0; }

BOOST_GEOMETRY_STATIC_MEASURE_COMPARE(<)
BOOST_GEOMETRY_STATIC_MEASURE_COMPARE(>)
BOOST_GEOMETRY_STATIC_MEASURE_COMPARE(<=)
BOOST_GEOMETRY_STATIC_MEASURE_COMPARE(>=)
BOOST_GEOMETRY_STATIC_MEASURE_COMPARE(==)
BOOST_GEOMETRY_STATIC_MEASURE_COMPARE(!=)
#undef BOOST_GEOMETRY_STATIC_MEASURE_COMPARE

#define BOOST_GEOMETRY_STATIC_COORDINATE_COMPARE(OP) \
template <typename A, typename B> \
inline bool operator OP(static_coordinate_view<A> const& a, static_coordinate_view<B> const& b) \
{ return compare(a, b) OP 0; }

BOOST_GEOMETRY_STATIC_COORDINATE_COMPARE(<)
BOOST_GEOMETRY_STATIC_COORDINATE_COMPARE(>)
BOOST_GEOMETRY_STATIC_COORDINATE_COMPARE(<=)
BOOST_GEOMETRY_STATIC_COORDINATE_COMPARE(>=)
BOOST_GEOMETRY_STATIC_COORDINATE_COMPARE(==)
BOOST_GEOMETRY_STATIC_COORDINATE_COMPARE(!=)
#undef BOOST_GEOMETRY_STATIC_COORDINATE_COMPARE

// Projection ordering is not an incidence test. The caller retains its
// topological checks; reversing A->B reverses the selected coordinate order.
template <typename Coordinate = void, typename A, typename B, typename P, typename Q>
inline int compare_along_segment(A const& a, B const& b, P const& p, Q const& q)
{
    int direction = static_compare_coordinate<Coordinate>(b, a, 0);
    std::size_t axis = 0;
    if (direction == 0) { direction = static_compare_coordinate<Coordinate>(b, a, 1); axis = 1; }
    if (direction == 0)
    {
        if (static_equal_points<Coordinate>(a, p) && static_equal_points<Coordinate>(a, q)) { return 0; }
        throw std::domain_error("A degenerate segment has no projection direction");
    }
    return direction * static_compare_coordinate<Coordinate>(p, q, axis);
}

enum static_segment_position
{
    segment_before = 0, segment_start = 1, segment_interior = 2, segment_end = 3, segment_after = 4,
    segment_degenerate_on = 5, segment_degenerate_off = 6, segment_off_line = 7
};

template <typename Coordinate = void, typename A, typename B, typename P>
inline int classify_point_on_segment(A const& a, B const& b, P const& p)
{
    if (static_equal_points<Coordinate>(a, b)) { return static_equal_points<Coordinate>(a, p) ? segment_degenerate_on : segment_degenerate_off; }
    if (static_equal_points<Coordinate>(a, p)) { return segment_start; }
    if (static_equal_points<Coordinate>(b, p)) { return segment_end; }
    if (static_orientation<Coordinate>(a, b, p) != 0) { return segment_off_line; }
    if (compare_along_segment<Coordinate>(a, b, p, a) < 0) { return segment_before; }
    return compare_along_segment<Coordinate>(a, b, p, b) > 0 ? segment_after : segment_interior;
}

} // namespace exact_cartesian
}}}} // namespace boost::geometry::strategies::detail

// Records reach this header through segment_position. Complete static lookup
// declarations before loading the Root predicate definitions.
#include <boost/geometry/strategies/cartesian/exact/borrowed_predicates.hpp>

#endif
