// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_PREDICATE_DISPATCH_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_PREDICATE_DISPATCH_HPP

#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <boost/geometry/strategies/cartesian/exact/fixed_measures.hpp>
#include <boost/geometry/strategies/cartesian/exact/forward_error.hpp>
#include <boost/geometry/strategies/cartesian/exact/native_predicates.hpp>
#include <boost/geometry/strategies/cartesian/exact/predicate_point.hpp>
#include <boost/geometry/strategies/intersection_result.hpp>
#include <boost/geometry/strategies/side.hpp>
#include <boost/geometry/util/select_most_precise.hpp>

#include <boost/geometry/util/detail/precise_fp.hpp>

BOOST_GEOMETRY_DETAIL_PRECISE_FP_BEGIN

namespace boost { namespace geometry { namespace strategies { namespace detail
{
namespace exact_cartesian
{

template <typename Root> class point_ref;
template <typename Root, typename A, typename B, typename P> int root_orientation(A const&, B const&, P const&);
template <std::size_t D, typename Root, typename A, typename B> int root_compare_coordinate(A const&, B const&);
template <typename Root, typename A, typename B> bool root_equal(A const&, B const&);
template <typename Root, typename A, typename B, typename P> int root_direction(A const&, B const&, P const&);

namespace static_predicates_detail
{

using exact_cartesian::root_of;
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

template <typename Point>
inline forward_error::point_approximation approximation(Point const& point)
{
    forward_error::point_approximation result;
    forward_error::try_native_point(geometry::get<0>(point), geometry::get<1>(point), result);
    return result;
}

template <typename Point>
using native_point = fixed_native_supported<geometry::coordinate_type_t<Point>>;

template <bool Dot, typename A, typename B, typename P>
int rational_sign(A const& a, B const& b, P const& p)
{
    auto diff = [](auto axis, auto const& x, auto const& y) -> exact_number
    { return exact_value(geometry::get<axis>(x)) - exact_value(geometry::get<axis>(y)); };
    std::integral_constant<std::size_t, 0> x;
    std::integral_constant<std::size_t, 1> y;
    exact_number const dx = diff(x, b, a), dy = diff(y, b, a);
    if constexpr (Dot)
    {
        exact_number const value = dx * diff(x, p, b) + dy * diff(y, p, b);
        return value.sign();
    }
    else
    {
        exact_number const value = dx * diff(y, p, a) - dy * diff(x, p, a);
        return value.sign();
    }
}

template <std::size_t Dim, typename Coordinate, typename A, typename B>
inline int coordinate_dispatch(A const& a, B const& b)
{
    using first = typename geometry::coordinate_type<A>::type;
    using second = typename geometry::coordinate_type<B>::type;
    auto const x = geometry::get<Dim>(a);
    auto const y = geometry::get<Dim>(b);
    if constexpr (std::is_same<first, second>::value) return (x > y) - (x < y);
    else if constexpr (native_point<A>::value && native_point<B>::value)
    {
        double ax, ex, ay, ey;
        int result;
        if (forward_error::try_native_coordinate(x, ax, ex) && forward_error::try_native_coordinate(y, ay, ey)
            && forward_error::try_compare_scalars({ax, ex}, {ay, ey}, result)) { return result; }
        return fixed_coordinate_compare(fixed_input_homogeneous(x, x), fixed_input_homogeneous(y, y), 0);
    }
    else return exact_value(x).compare(exact_value(y));
}

} // namespace static_predicates_detail

template <std::size_t D, typename Coordinate = void, typename A, typename B>
inline int static_compare_coordinate(A const& a, B const& b)
{
    using root = typename static_predicates_detail::common_root<A, B>::type;
    if constexpr (!std::is_void<root>::value) return root_compare_coordinate<D, root>(a, b);
    else return static_predicates_detail::coordinate_dispatch<D, Coordinate>(a, b);
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
    else return static_predicates_detail::rational_sign<false>(a, b, p);
}

template <typename Coordinate = void, typename A, typename B, typename P>
inline int static_direction(A const& a, B const& b, P const& p)
{
    using root = typename static_predicates_detail::common_root<A, B, P>::type;
    if constexpr (!std::is_void<root>::value) return root_direction<root>(a, b, p);
    else if constexpr (static_predicates_detail::native_point<A>::value
                    && static_predicates_detail::native_point<B>::value
                    && static_predicates_detail::native_point<P>::value)
    {
        int result;
        if (forward_error::try_direction(static_predicates_detail::approximation(a),
            static_predicates_detail::approximation(b), static_predicates_detail::approximation(p), result))
            return result;
        return with_homogeneous(a, b, p, [](auto const& x, auto const& y, auto const& z)
        { return fixed_direction(x, y, z); });
    }
    else return static_predicates_detail::rational_sign<true>(a, b, p);
}

// Projection ordering is not an incidence test. The caller retains its
// topological checks; reversing A->B reverses the selected coordinate order.
// Ratio-free ordering supplies a Root and treats a zero-length segment as a tie.
template <typename Coordinate = void, typename Root = void,
          typename A, typename B, typename P, typename Q>
inline int compare_along_segment(A const& a, B const& b, P const& p, Q const& q)
{
    auto const compare = [](auto axis, auto const& first, auto const& second)
    {
        constexpr std::size_t dim = decltype(axis)::value;
        if constexpr (std::is_void<Root>::value)
            return static_compare_coordinate<dim, Coordinate>(first, second);
        else
            return root_compare_coordinate<dim, Root>(first, second);
    };
    std::integral_constant<std::size_t, 0> const x;
    std::integral_constant<std::size_t, 1> const y;
    int const dx = compare(x, b, a);
    if (dx != 0) return dx * compare(x, p, q);
    int const dy = compare(y, b, a);
    if constexpr (std::is_void<Root>::value)
    {
        if (dy == 0)
        {
            if (static_equal_points<Coordinate>(a, p) && static_equal_points<Coordinate>(a, q)) return 0;
            throw std::domain_error("A degenerate segment has no projection direction");
        }
    }
    return dy * compare(y, p, q);
}

template <typename Coordinate = void, typename A, typename B, typename P>
inline segment_location classify_point_on_segment(A const& a, B const& b, P const& p)
{
    if (static_equal_points<Coordinate>(a, b)) { return static_equal_points<Coordinate>(a, p) ? segment_degenerate_on : segment_degenerate_off; }
    if (static_equal_points<Coordinate>(a, p)) { return segment_start; }
    if (static_equal_points<Coordinate>(b, p)) { return segment_end; }
    if (static_orientation<Coordinate>(a, b, p) != strategy::side::collinear) { return segment_off_line; }
    if (compare_along_segment<Coordinate>(a, b, p, a) < 0) { return segment_before; }
    return compare_along_segment<Coordinate>(a, b, p, b) > 0 ? segment_after : segment_interior;
}

} // namespace exact_cartesian
}}}} // namespace boost::geometry::strategies::detail

// Records reach this header through segment_position. Complete static lookup
// declarations before loading the Root predicate definitions.
#include <boost/geometry/strategies/cartesian/exact/root_predicates.hpp>

BOOST_GEOMETRY_DETAIL_PRECISE_FP_END

#endif
