// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_RELATE_SIMPLE_CARTESIAN_HPP
#define BOOST_GEOMETRY_STRATEGIES_RELATE_SIMPLE_CARTESIAN_HPP

#include <algorithm>
#include <cmath>
#include <type_traits>
#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/strategies/cartesian/clip.hpp>
#include <boost/geometry/strategies/cartesian/simple/intersection.hpp>
#include <boost/geometry/strategies/detail/section.hpp>
#include <boost/geometry/strategies/relate/cartesian.hpp>
#include <boost/geometry/strategies/relate/intersection_types.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace simple_cartesian
{

struct segment_box
{
    using cs_tag = cartesian_tag;
    static strategy::covered_by::cartesian_point_box get_disjoint_point_box_strategy() { return {}; }
    template <typename Segment, typename Box>
    static bool apply(Segment const& segment, Box const& box)
    {
        check_geometry<Segment>(); check_geometry<Box>();
        double const ax = geometry::get<0, 0>(segment), ay = geometry::get<0, 1>(segment);
        double const bx = geometry::get<1, 0>(segment), by = geometry::get<1, 1>(segment);
        double const lx = geometry::get<min_corner, 0>(box), ly = geometry::get<min_corner, 1>(box);
        double const hx = geometry::get<max_corner, 0>(box), hy = geometry::get<max_corner, 1>(box);
        if (lx > hx || ly > hy || (std::max)(ax, bx) < lx || (std::min)(ax, bx) > hx
            || (std::max)(ay, by) < ly || (std::min)(ay, by) > hy) return true;
        auto turn = [&](double x, double y) { return sign((bx - ax) * (y - ay) - (by - ay) * (x - ax)); };
        int const first = turn(lx, ly);
        return first != 0 && turn(lx, hy) == first && turn(hx, ly) == first && turn(hx, hy) == first;
    }
};

template <typename Box, typename Point>
struct clipping : strategy::intersection::liang_barsky<Box, Point>
{
    using equals_point_point_strategy_type = point_equal;
    static point_equal get_equals_point_point_strategy() { return {}; }
};

struct ring_topology
{
    template <typename Geometry>
    static double signed_measure(Geometry const& geometry)
    {
        check_geometry<Geometry>();
        return geometry::area(geometry, strategies::relate::cartesian<double>());
    }
    static double absolute_measure(double value) { return std::abs(value); }
    static bool measure_less(double a, double b) { return a < b; }
    static int measure_sign(double value) { return sign(value); }
    static bool measure_is_positive(double value) { return value > 0; }
    static bool measure_is_negative(double value) { return value < 0; }
    static bool measure_is_zero(double value) { return value == 0; }
    template <typename Box> static void expand_envelope(Box&) {}
};

}}}}}

namespace boost { namespace geometry
{
template <typename Point>
struct segment_intersection_points<Point, strategies::detail::simple_cartesian::position_tag>
    : strategies::detail::simple_cartesian::intersection_result<Point> {};
}}

namespace boost { namespace geometry { namespace strategies { namespace relate
{

// Plain double arithmetic, with no tolerance or certified-predicate fallback.
// Requires finite 2D double input and finite arithmetic intermediates. Ring
// parenting uses stock signed area, not the exact strategy's boundary topology.
// Point equality and coordinate ordering use ordinary double comparisons.
class simple_cartesian : public cartesian<double>
{
    using base = cartesian<double>;
    using primitives = strategies::detail::simple_cartesian::point_equal;
public:
    static constexpr bool exact_predicates = false;
    using segment_position_type = strategies::detail::simple_cartesian::position_tag;
    template <typename Point> using intersection_result_type = strategies::detail::simple_cartesian::intersection_result<Point>;
    template <typename Point> using comparable_distance_type = double;
    template <typename Geometry> using ring_measure_type = double;
    template <typename ComparePolicy, typename EqualsPolicy>
    using compare_type = strategies::detail::simple_cartesian::point_compare<ComparePolicy>;
    using exact_cluster_compare = strategies::detail::simple_cartesian::point_compare<strategy::compare::less>;

    template <typename Geometry>
    static auto area(Geometry const& geometry)
    { strategies::detail::simple_cartesian::check_geometry<Geometry>(); return base::area(geometry); }
    template <typename Geometry, typename Box>
    static auto envelope(Geometry const& geometry, Box const& box)
    {
        strategies::detail::simple_cartesian::check_geometry<Geometry>();
        strategies::detail::simple_cartesian::check_geometry<Box>();
        return base::envelope(geometry, box);
    }
    template <typename Box, typename Geometry>
    static auto expand(Box const& box, Geometry const& geometry)
    {
        strategies::detail::simple_cartesian::check_geometry<Box>();
        strategies::detail::simple_cartesian::check_geometry<Geometry>();
        return base::expand(box, geometry);
    }
    template <typename A, typename B>
    static auto within(A const& a, B const& b,
        std::enable_if_t<(util::is_pointlike<A>::value || util::is_box<A>::value) && util::is_box<B>::value>* = nullptr)
    {
        strategies::detail::simple_cartesian::check_geometry<A>();
        strategies::detail::simple_cartesian::check_geometry<B>();
        return base::within(a, b);
    }
    template <typename A, typename B>
    static auto covered_by(A const& a, B const& b,
        std::enable_if_t<(util::is_pointlike<A>::value || util::is_box<A>::value) && util::is_box<B>::value>* = nullptr)
    {
        strategies::detail::simple_cartesian::check_geometry<A>();
        strategies::detail::simple_cartesian::check_geometry<B>();
        return base::covered_by(a, b);
    }
    template <typename A, typename B>
    static auto disjoint(A const& a, B const& b,
        std::enable_if_t<util::is_box<A>::value && util::is_box<B>::value>* = nullptr)
    {
        strategies::detail::simple_cartesian::check_geometry<A>();
        strategies::detail::simple_cartesian::check_geometry<B>();
        return base::disjoint(a, b);
    }
    template <typename A, typename B>
    static auto disjoint(A const&, B const&,
        std::enable_if_t<util::is_segment<A>::value && util::is_box<B>::value>* = nullptr)
    { return strategies::detail::simple_cartesian::segment_box(); }
    static auto side() { return strategies::detail::simple_cartesian::side(); }
    static auto topology() { return strategies::detail::simple_cartesian::topology(); }
    static auto ring_topology() { return strategies::detail::simple_cartesian::ring_topology(); }
    static auto section() { return strategies::detail::strict_cartesian_section(); }
    static auto segment_position() { return strategies::detail::simple_cartesian::segment_position(); }
    template <typename A, typename B>
    static auto comparable_distance(A const&, B const&) { return strategies::detail::simple_cartesian::squared_distance(); }
    template <typename Point, typename Box>
    static auto clipping(Box const&) { return strategies::detail::simple_cartesian::clipping<Box, Point>(); }
    static auto relate() { return strategies::detail::simple_cartesian::segments(); }
    template <typename A, typename B>
    static auto relate(A const&, B const&,
        std::enable_if_t<util::is_pointlike<A>::value && util::is_pointlike<B>::value>* = nullptr)
    { return primitives(); }
    template <typename A, typename B>
    static auto relate(A const&, B const&,
        std::enable_if_t<util::is_pointlike<A>::value && (util::is_linear<B>::value || util::is_polygonal<B>::value)>* = nullptr)
    { return strategies::detail::simple_cartesian::winding(); }
};

}}}}

#endif
