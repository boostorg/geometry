// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_SIMPLE_INTERSECTION_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_SIMPLE_INTERSECTION_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <type_traits>
#include <boost/geometry/algorithms/detail/convert_point_to_point.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/policies/relate/intersection_policy.hpp>
#include <boost/geometry/strategies/cartesian/simple/predicates.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace simple_cartesian
{

template <typename Point>
struct intersection_result
{
    using point_type = Point;
    std::size_t count = 0;
    Point intersections[2];
};

template <typename Source, typename Destination>
inline void copy_point(Source const& source, Destination& destination)
{
    check_geometry<Destination>();
    check_geometry<Source>();
    geometry::detail::conversion::convert_point_to_point(source, destination);
}

template <typename Policy>
struct endpoint_policy : Policy {};

template <typename Result>
struct endpoint_policy<policies::relate::segments_intersection_points<Result>>
    : policies::relate::segments_intersection_points<Result>
{
    using return_type = Result;
    template <typename Info, typename A, typename B>
    static Result segments_crosses(side_info const&, Info const& info, A const& a, B const& b)
    {
        Result result;
        result.count = 1;
        info.calculate(result.intersections[0], a, b);
        return result;
    }
    template <typename Info, typename Point>
    static Result segments_share_common_point(side_info const&, Info const&, Point const& point)
    {
        Result result;
        result.count = 1;
        copy_point(point, result.intersections[0]);
        return result;
    }
    template <typename A, typename B, typename Token>
    static Result segments_collinear(A const& a, B const& b, bool,
        int a1, int a2, int b1, int b2, Token const&, Token const&, Token const&, Token const&)
    {
        Result result;
        std::array<unsigned, 2> selected;
        auto append = [&](unsigned endpoint) { selected[result.count++] = endpoint; };
        if (a1 >= 1 && a1 <= 3 && result.count < 2) append(0);
        if (b1 == 2 && result.count < 2) append(2);
        if (a2 >= 1 && a2 <= 3 && result.count < 2) append(1);
        if (b2 == 2 && result.count < 2) append(3);
        auto visit = [&](unsigned endpoint, auto apply)
        {
            switch (endpoint)
            {
            case 0: return apply(a.first);
            case 1: return apply(a.second);
            case 2: return apply(b.first);
            default: return apply(b.second);
            }
        };
        if (result.count == 2 && visit(selected[1], [&](auto const& second)
            { return visit(selected[0], [&](auto const& first)
                { return segment_position::apply(a.first, a.second, second, first); }); }) < 0)
            std::swap(selected[0], selected[1]);
        for (std::size_t i = 0; i != result.count; ++i)
            visit(selected[i], [&](auto const& point) { copy_point(point, result.intersections[i]); });
        return result;
    }
    template <typename Segment>
    static Result degenerate(Segment const& segment, bool)
    {
        Result result;
        result.count = 1;
        copy_point(segment.first, result.intersections[0]);
        return result;
    }
    template <typename Segment, typename Token>
    static Result one_degenerate(Segment const& segment, Token const&, bool a_degenerate)
    { return degenerate(segment, a_degenerate); }
};

template <typename Result>
struct endpoint_policy<policies::relate::segments_intersection_policy<Result>>
    : policies::relate::segments_intersection_policy<Result>
{
    using original = policies::relate::segments_intersection_policy<Result>;
    using return_type = typename original::return_type;
    using points = endpoint_policy<policies::relate::segments_intersection_points<Result>>;
    using direction = policies::relate::segments_direction;
    template <typename Info, typename A, typename B>
    static return_type segments_crosses(side_info const& sides, Info const& info, A const& a, B const& b)
    { return {points::segments_crosses(sides, info, a, b), direction::segments_crosses(sides, info, a, b)}; }
    template <typename Info, typename Point>
    static return_type segments_share_common_point(side_info const& sides, Info const& info, Point const& point)
    { return {points::segments_share_common_point(sides, info, point), direction::segments_share_common_point(sides, info, point)}; }
    template <typename A, typename B, typename Token>
    static return_type segments_collinear(A const& a, B const& b, bool opposite,
        int a1, int a2, int b1, int b2, Token const& t1, Token const& t2, Token const& t3, Token const& t4)
    {
        return {points::segments_collinear(a, b, opposite, a1, a2, b1, b2, t1, t2, t3, t4),
                direction::segments_collinear(a, b, opposite, a1, a2, b1, b2, t1, t2, t3, t4)};
    }
    template <typename Segment>
    static return_type degenerate(Segment const& segment, bool a_degenerate)
    { return {points::degenerate(segment, a_degenerate), direction::degenerate(segment, a_degenerate)}; }
    template <typename Segment, typename Token>
    static return_type one_degenerate(Segment const& segment, Token const& token, bool a_degenerate)
    { return {points::one_degenerate(segment, token, a_degenerate), direction::one_degenerate(segment, token, a_degenerate)}; }
};

struct segments
{
    using cs_tag = cartesian_tag;
    struct construction_info
    {
        int endpoint;
        template <typename Output, typename A, typename B>
        void calculate(Output& output, A const& a, B const& b) const
        {
            switch (endpoint)
            {
            case 0: copy_point(a.first, output); return;
            case 1: copy_point(a.second, output); return;
            case 2: copy_point(b.first, output); return;
            case 3: copy_point(b.second, output); return;
            }
            double const ax = coordinate<0>(a.first), ay = coordinate<1>(a.first);
            double const dx = coordinate<0>(a.second) - ax, dy = coordinate<1>(a.second) - ay;
            double const ex = coordinate<0>(b.second) - coordinate<0>(b.first);
            double const ey = coordinate<1>(b.second) - coordinate<1>(b.first);
            double const t = ((coordinate<0>(b.first) - ax) * ey - (coordinate<1>(b.first) - ay) * ex)
                           / (dx * ey - dy * ex);
            check_geometry<Output>();
            geometry::set<0>(output, ax + t * dx);
            geometry::set<1>(output, ay + t * dy);
        }
    };

    template <typename Range1, typename Range2, typename Policy>
    static typename Policy::return_type apply(Range1 const& p, Range2 const& q, Policy const&)
    {
        using selected = endpoint_policy<Policy>;
        auto const& p0 = p.at(0); auto const& p1 = p.at(1);
        auto const& q0 = q.at(0); auto const& q1 = q.at(1);
        using P = typename std::decay<decltype(p0)>::type;
        using Q = typename std::decay<decltype(q0)>::type;
        model::referring_segment<P const> const a(p0, p1);
        model::referring_segment<Q const> const b(q0, q1);
        int const sa0 = side::apply(q0, q1, p0), sa1 = side::apply(q0, q1, p1);
        if (sa0 * sa1 > 0) return selected::disjoint();
        int const sb0 = side::apply(p0, p1, q0), sb1 = side::apply(p0, p1, q1);
        if (sb0 * sb1 > 0) return selected::disjoint();
        if (sa0 == 0 && sa1 == 0 && sb0 == 0 && sb1 == 0)
        {
            bool const ap = point_equal::apply(p0, p1), bp = point_equal::apply(q0, q1);
            position_tag const token;
            if (ap && bp) return point_equal::apply(p0, q0) ? selected::degenerate(a, true) : selected::disjoint();
            if (ap || bp)
            {
                int const location = ap ? segment_position::classify_collinear(q0, q1, p0)
                                        : segment_position::classify_collinear(p0, p1, q0);
                if (location < 1 || location > 3) return selected::disjoint();
                return ap ? selected::one_degenerate(a, token, true) : selected::one_degenerate(b, token, false);
            }
            int const a1 = segment_position::classify_collinear(q0, q1, p0);
            int const a2 = segment_position::classify_collinear(q0, q1, p1);
            if ((a1 < 1 && a2 < 1) || (a1 > 3 && a2 > 3)) return selected::disjoint();
            int const b1 = segment_position::classify_collinear(p0, p1, q0);
            int const b2 = segment_position::classify_collinear(p0, p1, q1);
            return selected::segments_collinear(a, b, segment_position::apply(p0, p1, q0, q1) > 0,
                                                a1, a2, b1, b2, token, token, token, token);
        }
        int const endpoint = sa0 == 0 ? 0 : sa1 == 0 ? 1 : sb0 == 0 ? 2 : sb1 == 0 ? 3 : -1;
        if (endpoint < 0)
        {
            double const dx = coordinate<0>(p1) - coordinate<0>(p0), dy = coordinate<1>(p1) - coordinate<1>(p0);
            double const ex = coordinate<0>(q1) - coordinate<0>(q0), ey = coordinate<1>(q1) - coordinate<1>(q0);
            if (dx * ey - dy * ex == 0) return selected::disjoint();
        }
        construction_info const info{endpoint};
        side_info sides;
        sides.template set<0>(sa0, sa1); sides.template set<1>(sb0, sb1);
        if (sa0 == 0 && (point_equal::apply(p0, q0) || point_equal::apply(p0, q1)))
            return selected::segments_share_common_point(sides, info, p0);
        if (sa1 == 0 && (point_equal::apply(p1, q0) || point_equal::apply(p1, q1)))
            return selected::segments_share_common_point(sides, info, p1);
        return selected::segments_crosses(sides, info, a, b);
    }
};

}}}}}

#endif
