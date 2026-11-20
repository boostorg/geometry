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
#include <boost/geometry/strategies/compare.hpp>
#include <boost/geometry/strategies/detail/distance_measure.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace simple_cartesian
{

template <typename Geometry>
inline void check_geometry()
{
    static_assert(std::is_same<geometry::coordinate_type_t<Geometry>, double>::value,
                  "simple_cartesian requires native double coordinates");
    static_assert(geometry::dimension<Geometry>::value == 2
        && std::is_same<geometry::coordinate_system_t<Geometry>, cs::cartesian>::value,
        "simple_cartesian requires two-dimensional Cartesian geometries");
}

template <std::size_t D, typename Point>
inline double coordinate(Point const& point)
{
    check_geometry<Point>();
    return geometry::get<D>(point);
}

inline int sign(double value) { return value > 0 ? 1 : value < 0 ? -1 : 0; }

template <std::size_t D, typename A, typename B>
inline int compare_coordinate(A const& a, B const& b)
{
    double const x = coordinate<D>(a), y = coordinate<D>(b);
    return x < y ? -1 : x > y ? 1 : 0;
}

template <typename A, typename B, typename P>
inline double determinant(A const& a, B const& b, P const& p)
{
    double const ax = coordinate<0>(a), ay = coordinate<1>(a);
    return (coordinate<0>(b) - ax) * (coordinate<1>(p) - ay)
         - (coordinate<1>(b) - ay) * (coordinate<0>(p) - ax);
}

struct point_equal
{
    using cs_tag = cartesian_tag;
    template <typename A, typename B>
    static bool apply(A const& a, B const& b)
    { return coordinate<0>(a) == coordinate<0>(b) && coordinate<1>(a) == coordinate<1>(b); }
};

struct side
{
    using cs_tag = cartesian_tag;
    template <typename A, typename B, typename P>
    static int apply(A const& a, B const& b, P const& p) { return sign(determinant(a, b, p)); }
    template <typename A, typename B, typename P>
    static int direction_code(A const& a, B const& b, P const& p)
    {
        return sign((coordinate<0>(b) - coordinate<0>(a)) * (coordinate<0>(p) - coordinate<0>(b))
                  + (coordinate<1>(b) - coordinate<1>(a)) * (coordinate<1>(p) - coordinate<1>(b)));
    }
};

template <typename ComparePolicy>
struct point_compare
{
    template <typename A, typename B>
    static bool apply(A const& a, B const& b)
    {
        int const x = compare_coordinate<0>(a, b);
        int const order = x == 0 ? compare_coordinate<1>(a, b) : x;
        return std::is_same<ComparePolicy, strategy::compare::equal_to>::value
            ? order == 0 : ComparePolicy::apply(order, 0);
    }
};

struct winding
{
    using cs_tag = cartesian_tag;
    struct state_type { int count = 0; bool boundary = false; };
    template <typename P, typename A, typename B = A>
    static bool apply(P const& point, A const& a, B const& b, state_type& state)
    {
        int const ay = compare_coordinate<1>(a, point), by = compare_coordinate<1>(b, point);
        if (ay * by > 0) return true;
        int const turn = side::apply(a, b, point);
        if (turn == 0 && compare_coordinate<0>(a, point) * compare_coordinate<0>(b, point) <= 0)
        { state.boundary = true; return false; }
        if (ay <= 0 && by > 0 && turn > 0) ++state.count;
        else if (by <= 0 && ay > 0 && turn < 0) --state.count;
        return true;
    }
    static int result(state_type const& state) { return state.boundary ? 0 : state.count == 0 ? -1 : 1; }
};

struct squared_distance
{
    template <typename A, typename B>
    static double apply(A const& a, B const& b)
    {
        double const dx = coordinate<0>(a) - coordinate<0>(b), dy = coordinate<1>(a) - coordinate<1>(b);
        return dx * dx + dy * dy;
    }
};

struct topology
{
    template <typename A, typename B>
    static bool cluster_equal(A const& a, B const& b) { return point_equal::apply(a, b); }
    template <typename A, typename B>
    static int compare_y(A const& a, B const& b) { return compare_coordinate<1>(a, b); }
    template <typename A, typename B>
    static bool cluster_window_exceeded(A const& upper, B const& lower) { return compare_y(upper, lower) > 0; }
    template <typename Indices, typename Turns>
    static void colocate_cluster(Indices const& indices, Turns& turns)
    {
        auto const point = turns[*indices.begin()].point;
        for (auto const index : indices) turns[index].point = point;
    }
    template <typename A, typename B>
    static bool same_direction_point(A const& a, B const& b) { return point_equal::apply(a, b); }
    template <typename A, typename B>
    static double squared_distance(A const& a, B const& b) { return simple_cartesian::squared_distance::apply(a, b); }
    static bool distance_is_zero(double value) { return value == 0; }
    template <typename A, typename B, typename P>
    static strategies::detail::distance_measure<double> distance_measure(A const& a, B const& b, P const& p)
    {
        strategies::detail::distance_measure<double> result;
        result.measure = determinant(a, b, p);
        return result;
    }
};

struct segment_position
{
    template <typename A, typename B, typename P, typename Q>
    static int apply(A const& a, B const& b, P const& p, Q const& q)
    {
        int const x = compare_coordinate<0>(a, b);
        int const order = x == 0 ? compare_coordinate<1>(p, q) : compare_coordinate<0>(p, q);
        int const direction = x == 0 ? compare_coordinate<1>(a, b) : x;
        return direction > 0 ? -order : order;
    }
    template <typename A, typename B, typename P>
    static int classify_collinear(A const& a, B const& b, P const& p)
    {
        int const from = apply(a, b, p, a);
        if (from == 0) return 1;
        if (from < 0) return 0;
        int const to = apply(a, b, p, b);
        return to < 0 ? 2 : to == 0 ? 3 : 4;
    }
    template <typename A, typename B, typename P>
    static int classify(A const& a, B const& b, P const& p)
    {
        // No-fraction relate helpers also distinguish degenerate/off-line points.
        if (point_equal::apply(a, b)) return point_equal::apply(a, p) ? 5 : 6;
        if (point_equal::apply(a, p)) return 1;
        if (point_equal::apply(b, p)) return 3;
        return side::apply(a, b, p) == 0 ? classify_collinear(a, b, p) : 7;
    }
};

struct position_tag
{
    using no_fraction = void;
    template <typename A, typename B, typename P, typename Q>
    static int compare(A const& a, B const& b, P const& p, Q const& q)
    { return segment_position::apply(a, b, p, q); }
    template <typename A, typename B>
    static bool equal(A const& a, B const& b) { return point_equal::apply(a, b); }
};

}}}}}

#endif
