// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_SIMPLE_PREDICATES_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_SIMPLE_PREDICATES_HPP

#include <type_traits>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/coordinate_dimension.hpp>
#include <boost/geometry/core/coordinate_system.hpp>
#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/strategies/cartesian/detail/linear_formula.hpp>
#include <boost/geometry/strategies/compare.hpp>
#include <boost/geometry/strategies/intersection_result.hpp>
#include <boost/geometry/strategies/side.hpp>
#include <boost/geometry/util/select_calculation_type.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace simple_cartesian
{

template <typename CalculationType, typename... Geometries>
using selected_t = typename geometry::select_calculation_type_alt<CalculationType, Geometries...>::type;

// Integral division cannot represent a proper crossing. Use the same
// promoted type for predicates and constructions throughout the operation.
template <typename CalculationType, typename... Geometries>
using calculation_t = std::conditional_t<std::is_integral<selected_t<CalculationType, Geometries...>>::value,
    double, selected_t<CalculationType, Geometries...>>;

template <typename Geometry>
inline void check_geometry()
{
    static_assert(geometry::dimension<Geometry>::value == 2
        && std::is_same<geometry::coordinate_system_t<Geometry>, cs::cartesian>::value,
        "simple_cartesian requires two-dimensional Cartesian geometries");
}

template <std::size_t D, typename T, typename Point>
inline T coordinate(Point const& point)
{
    check_geometry<Point>();
    return static_cast<T>(geometry::get<D>(point));
}

template <typename T>
inline int sign(T const& value) { return value > 0 ? 1 : value < 0 ? -1 : 0; }

template <typename T, typename Point>
inline linear_formula::xy<T> ordinates(Point const& p)
{ return {{coordinate<0, T>(p), coordinate<1, T>(p)}}; }

template <std::size_t D, typename CalculationType, typename A, typename B>
inline int compare_coordinate(A const& a, B const& b)
{
    using T = calculation_t<CalculationType, A, B>;
    T const x = coordinate<D, T>(a), y = coordinate<D, T>(b);
    return x < y ? -1 : x > y ? 1 : 0;
}

template <typename CalculationType, typename A, typename B, typename P>
inline auto determinant(A const& a, B const& b, P const& p)
{
    using T = calculation_t<CalculationType, A, B, P>;
    return linear_formula::orientation::evaluate(linear_formula::arithmetic_ops<T>{},
        ordinates<T>(a), ordinates<T>(b), ordinates<T>(p));
}

template <typename CalculationType>
struct point_equal
{
    using cs_tag = cartesian_tag;
    template <typename A, typename B>
    static bool apply(A const& a, B const& b)
    {
        using T = calculation_t<CalculationType, A, B>;
        return coordinate<0, T>(a) == coordinate<0, T>(b)
            && coordinate<1, T>(a) == coordinate<1, T>(b);
    }
};

template <typename CalculationType>
struct side
{
    using cs_tag = cartesian_tag;
    template <typename A, typename B, typename P>
    static int apply(A const& a, B const& b, P const& p) { return sign(determinant<CalculationType>(a, b, p)); }
    template <typename A, typename B, typename P>
    static int direction_code(A const& a, B const& b, P const& p)
    {
        using T = calculation_t<CalculationType, A, B, P>;
        auto const v = ordinates<T>(a), w = ordinates<T>(b), q = ordinates<T>(p);
        return sign((w[0] - v[0]) * (q[0] - w[0]) + (w[1] - v[1]) * (q[1] - w[1]));
    }
};

template <typename ComparePolicy, typename CalculationType>
struct point_compare
{
    template <typename A, typename B>
    static bool apply(A const& a, B const& b)
    {
        int const x = compare_coordinate<0, CalculationType>(a, b);
        int const order = x == 0 ? compare_coordinate<1, CalculationType>(a, b) : x;
        return std::is_same<ComparePolicy, strategy::compare::equal_to>::value
            ? order == 0 : ComparePolicy::apply(order, 0);
    }
};

template <typename CalculationType, std::size_t Axis = 1>
struct winding
{
    static_assert(Axis < 2, "Cartesian winding requires a planar scan axis");
    using cs_tag = cartesian_tag;
    static constexpr int buffer_scan_axis = Axis;
    struct state_type { int count = 0; bool boundary = false; };

    template <typename P, typename Box>
    static bool section_may_contribute(P const& point, Box const& box)
    {
        using T = calculation_t<CalculationType, P, Box>;
        T const value = coordinate<buffer_scan_axis, T>(point);
        return value >= T(geometry::get<min_corner, buffer_scan_axis>(box))
            && value <= T(geometry::get<max_corner, buffer_scan_axis>(box));
    }

    template <typename P, typename A, typename B = A>
    static bool apply(P const& point, A const& a, B const& b, state_type& state)
    {
        int const first = compare_coordinate<Axis, CalculationType>(a, point);
        int const second = compare_coordinate<Axis, CalculationType>(b, point);
        if (first * second > 0) return true;
        int const turn = (Axis == 1 ? 1 : -1) * side<CalculationType>::apply(a, b, point);
        if (turn == strategy::side::collinear && compare_coordinate<1 - Axis, CalculationType>(a, point)
                      * compare_coordinate<1 - Axis, CalculationType>(b, point) <= 0)
        { state.boundary = true; return false; }
        if (first <= 0 && second > 0 && turn > 0) ++state.count;
        else if (second <= 0 && first > 0 && turn < 0) --state.count;
        return true;
    }
    static int result(state_type const& state) { return state.boundary ? 0 : state.count == 0 ? -1 : 1; }
};

template <typename CalculationType>
struct squared_distance
{
    template <typename A, typename B>
    static auto apply(A const& a, B const& b)
    {
        using T = calculation_t<CalculationType, A, B>;
        T const dx = coordinate<0, T>(a) - coordinate<0, T>(b);
        T const dy = coordinate<1, T>(a) - coordinate<1, T>(b);
        return T(dx * dx + dy * dy);
    }
};

template <typename CalculationType>
struct topology
{
    static constexpr bool uses_cluster_tolerance = false;

    template <std::size_t D, typename A, typename B>
    static int compare_coordinate(A const& a, B const& b)
    { return simple_cartesian::compare_coordinate<D, CalculationType>(a, b); }
    template <typename A, typename B>
    static bool cluster_equal(A const& a, B const& b) { return point_equal<CalculationType>::apply(a, b); }
    template <typename A, typename B>
    static int compare_y(A const& a, B const& b) { return compare_coordinate<1>(a, b); }
    template <typename A, typename B>
    static auto squared_distance(A const& a, B const& b)
    { return simple_cartesian::squared_distance<CalculationType>::apply(a, b); }
    template <typename T>
    static bool distance_is_zero(T const& value) { return value == 0; }
    template <typename A, typename B, typename P>
    static int side_measure_sign(A const& a, B const& b, P const& p)
    {
        auto const value = determinant<CalculationType>(a, b, p);
        return static_cast<int>(value > 0) - static_cast<int>(value < 0);
    }
};

template <typename CalculationType>
struct segment_position
{
    template <typename A, typename B, typename P, typename Q>
    static int apply(A const& a, B const& b, P const& p, Q const& q)
    {
        using T = calculation_t<CalculationType, A, B, P, Q>;
        int const x = compare_coordinate<0, T>(a, b);
        int const order = x == 0 ? compare_coordinate<1, T>(p, q) : compare_coordinate<0, T>(p, q);
        int const direction = x == 0 ? compare_coordinate<1, T>(a, b) : x;
        return direction > 0 ? -order : order;
    }
    template <typename A, typename B, typename P>
    static segment_location classify(A const& a, B const& b, P const& p)
    {
        // No-fraction relate helpers also distinguish degenerate/off-line points.
        using T = calculation_t<CalculationType, A, B, P>;
        using equal = point_equal<T>;
        if (equal::apply(a, b)) return equal::apply(a, p) ? segment_degenerate_on : segment_degenerate_off;
        if (equal::apply(a, p)) return segment_start;
        if (equal::apply(b, p)) return segment_end;
        return side<T>::apply(a, b, p) == strategy::side::collinear
            ? classify_collinear_position<segment_position>(a, b, p) : segment_off_line;
    }
};

template <typename CalculationType>
struct position_tag
{
    using no_fraction = void;
    template <typename A, typename B, typename P, typename Q>
    static int compare(A const& a, B const& b, P const& p, Q const& q)
    { return segment_position<CalculationType>::apply(a, b, p, q); }
    template <typename A, typename B>
    static bool equal(A const& a, B const& b) { return point_equal<CalculationType>::apply(a, b); }
};

}}}}}

#endif
