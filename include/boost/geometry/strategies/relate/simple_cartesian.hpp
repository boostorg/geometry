// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_RELATE_SIMPLE_CARTESIAN_HPP
#define BOOST_GEOMETRY_STRATEGIES_RELATE_SIMPLE_CARTESIAN_HPP

#include <algorithm>
#include <type_traits>
#include <utility>
#include <boost/geometry/strategies/cartesian/clip.hpp>
#include <boost/geometry/strategies/cartesian/detail/segment_extents.hpp>
#include <boost/geometry/strategies/cartesian/simple/intersection.hpp>
#include <boost/geometry/strategies/detail/section.hpp>
#include <boost/geometry/strategies/distance/cartesian.hpp>
#include <boost/geometry/strategies/relate/intersection_types.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace simple_cartesian
{

template <typename CalculationType>
struct segment_box
{
    using cs_tag = cartesian_tag;
    template <typename Segment, typename Box>
    static bool apply(Segment const& segment, Box const& box)
    {
        check_geometry<Segment>(); check_geometry<Box>();
        using T = calculation_t<CalculationType, Segment, Box>;
        T const ax(geometry::get<0, 0>(segment)), ay(geometry::get<0, 1>(segment));
        T const bx(geometry::get<1, 0>(segment)), by(geometry::get<1, 1>(segment));
        T const lx(geometry::get<min_corner, 0>(box)), ly(geometry::get<min_corner, 1>(box));
        T const hx(geometry::get<max_corner, 0>(box)), hy(geometry::get<max_corner, 1>(box));
        if (lx > hx || ly > hy || (std::max)(ax, bx) < lx || (std::min)(ax, bx) > hx
            || (std::max)(ay, by) < ly || (std::min)(ay, by) > hy) return true;
        auto turn = [&](T const& x, T const& y) { return sign((bx - ax) * (y - ay) - (by - ay) * (x - ax)); };
        int const first = turn(lx, ly);
        return first != 0 && turn(lx, hy) == first && turn(hx, ly) == first && turn(hx, hy) == first;
    }
};

template <typename Box, typename Point, typename CalculationType>
struct clipping : strategy::intersection::liang_barsky<Box, Point, calculation_t<CalculationType, Box, Point>>
{
    using equals_point_point_strategy_type = point_equal<CalculationType>;
    static equals_point_point_strategy_type get_equals_point_point_strategy() { return {}; }
};

}}}}}

namespace boost { namespace geometry
{
template <typename Point, typename CalculationType>
struct segment_intersection_points<Point, strategies::detail::simple_cartesian::position_tag<CalculationType>>
    : strategies::detail::simple_cartesian::intersection_result<Point> {};
}}

namespace boost { namespace geometry { namespace strategies { namespace relate
{

// Ordinary arithmetic in the selected calculation type, without certified
// predicates. Requires finite 2D Cartesian input and finite intermediates. Ring
// parenting uses stock signed area, not the exact strategy's boundary topology.
// Point equality and coordinate ordering use ordinary comparisons.
template <typename CalculationType = void>
class simple_cartesian_strategy : public strategies::distance::cartesian<CalculationType>
{
    using base = cartesian<CalculationType>;
    using primitives = strategies::detail::simple_cartesian::point_equal<CalculationType>;
    template <typename... G>
    using calculation_t = strategies::detail::simple_cartesian::calculation_t<CalculationType, G...>;
public:
    static constexpr bool exact_predicates = false;
    static constexpr bool uses_overlay_tolerance = false;
    using segment_position_type = strategies::detail::simple_cartesian::position_tag<CalculationType>;
    template <typename Point>
    using intersection_point_type = std::conditional_t<
        std::is_same<coordinate_type_t<Point>, calculation_t<Point>>::value, Point,
        model::point<calculation_t<Point>, 2, cs::cartesian>>;
    template <typename Point> using intersection_result_type = strategies::detail::simple_cartesian::intersection_result<Point>;
    template <typename Point> using comparable_distance_type = calculation_t<Point>;
    template <typename Geometry> using ring_measure_type = calculation_t<Geometry>;
    template <typename ComparePolicy, typename EqualsPolicy>
    using compare_type = strategies::detail::simple_cartesian::point_compare<ComparePolicy, CalculationType>;
    using exact_cluster_compare = compare_type<strategy::compare::less, void>;

    template <typename A, typename B, typename Apply>
    decltype(auto) with_operands(A const& a, B const& b, Apply&& apply) const
    {
        using bound = simple_cartesian_strategy<calculation_t<A, B>>;
        return std::forward<Apply>(apply)(a, b, bound{});
    }

    template <typename Geometry>
    static auto area(Geometry const& geometry)
    {
        strategies::detail::simple_cartesian::check_geometry<Geometry>();
        return cartesian<calculation_t<Geometry>>::area(geometry);
    }
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
    { return strategies::detail::simple_cartesian::segment_box<CalculationType>(); }
    static auto side() { return strategies::detail::simple_cartesian::side<CalculationType>(); }
    static auto topology() { return strategies::detail::simple_cartesian::topology<CalculationType>(); }
    template <typename P, typename Q>
    static auto segment_y_disjoint(P const& a, P const& b, Q const& c, Q const& d)
        -> std::enable_if_t<strategies::detail::native_double_pair<P, Q>::value
                            && std::is_same<calculation_t<P, Q>, double>::value, bool>
    {
        return strategies::detail::segment_y_disjoint(a, b, c, d);
    }
    static auto section() { return strategies::detail::strict_cartesian_section(); }
    static auto segment_position() { return strategies::detail::simple_cartesian::segment_position<CalculationType>(); }
    template <typename A, typename B>
    static auto comparable_distance(A const&, B const&)
    { return strategies::detail::simple_cartesian::squared_distance<CalculationType>(); }
    template <typename Point, typename Box>
    static auto clipping(Box const&)
    { return strategies::detail::simple_cartesian::clipping<Box, Point, CalculationType>(); }
    static auto relate() { return strategies::detail::simple_cartesian::segments<CalculationType>(); }
    template <typename A, typename B>
    static auto relate(A const&, B const&,
        std::enable_if_t<util::is_pointlike<A>::value && util::is_pointlike<B>::value>* = nullptr)
    { return primitives(); }
    template <typename A, typename B>
    static auto relate(A const&, B const&,
        std::enable_if_t<util::is_pointlike<A>::value && (util::is_linear<B>::value || util::is_polygonal<B>::value)>* = nullptr)
    { return strategies::detail::simple_cartesian::winding<CalculationType>(); }
};

using simple_cartesian = simple_cartesian_strategy<>;

}}}}

#endif
