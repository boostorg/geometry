// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_DETAIL_SEGMENT_EVENTS_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_DETAIL_SEGMENT_EVENTS_HPP

#include <array>
#include <type_traits>
#include <utility>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/policies/relate/intersection_policy.hpp>
#include <boost/geometry/strategies/intersection_result.hpp>
#include <boost/geometry/strategies/side.hpp>
#include <boost/geometry/strategies/side_info.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace cartesian_segment_events
{

template <typename Policy, typename Position, typename Copy>
struct endpoint_policy : Policy {};

template <typename ReturnType, typename Position, typename Copy>
struct endpoint_policy<policies::relate::segments_intersection_points<ReturnType>, Position, Copy>
    : policies::relate::segments_intersection_points<ReturnType>
{
    using return_type = ReturnType;

    template <typename Info, typename Segment1, typename Segment2>
    static return_type segments_crosses(side_info const&, Info const& info,
                                        Segment1 const& a, Segment2 const& b)
    {
        return_type result;
        result.count = 1;
        info.calculate(result.intersections[0], a, b);
        return result;
    }

    template <typename Info, typename Point>
    static return_type segments_share_common_point(side_info const&, Info const&, Point const& point)
    {
        return_type result;
        result.count = 1;
        Copy::apply(point, result.intersections[0]);
        return result;
    }

    template <typename Segment1, typename Segment2, typename Token>
    static return_type segments_collinear(Segment1 const& a, Segment2 const& b, bool,
        int a1, int a2, int b1, int b2, Token const&, Token const&, Token const&, Token const&)
    {
        return_type result;
        std::array<unsigned, 2> selected;
        auto append = [&](unsigned endpoint) { selected[result.count++] = endpoint; };
        if (a1 >= segment_start && a1 <= segment_end && result.count < 2) append(0);
        if (b1 == segment_interior && result.count < 2) append(2);
        if (a2 >= segment_start && a2 <= segment_end && result.count < 2) append(1);
        if (b2 == segment_interior && result.count < 2) append(3);
        auto visit = [&](unsigned endpoint, auto&& apply)
        {
            switch (endpoint)
            {
            case 0: return apply(a.first);
            case 1: return apply(a.second);
            case 2: return apply(b.first);
            default: return apply(b.second);
            }
        };
        // Order the exact endpoints before any rounding to the output type.
        if (result.count == 2 && visit(selected[1], [&](auto const& second)
            { return visit(selected[0], [&](auto const& first)
                { return Position::apply(a.first, a.second, second, first); }); }) < 0)
            std::swap(selected[0], selected[1]);
        for (std::size_t i = 0; i < result.count; ++i)
            visit(selected[i], [&](auto const& point)
                { Copy::apply(point, result.intersections[i]); });
        return result;
    }

    template <typename Segment>
    static return_type degenerate(Segment const& segment, bool)
    {
        return_type result;
        result.count = 1;
        Copy::apply(segment.first, result.intersections[0]);
        return result;
    }

    template <typename Segment, typename Token>
    static return_type one_degenerate(Segment const& segment, Token const&, bool a_degenerate)
    { return degenerate(segment, a_degenerate); }
};

template <typename ReturnType, typename Position, typename Copy>
struct endpoint_policy<policies::relate::segments_intersection_policy<ReturnType>, Position, Copy>
    : policies::relate::segments_intersection_policy<ReturnType>
{
    using original_policy = policies::relate::segments_intersection_policy<ReturnType>;
    using return_type = typename original_policy::return_type;
    using points_policy = endpoint_policy<policies::relate::segments_intersection_points<ReturnType>, Position, Copy>;
    using direction_policy = policies::relate::segments_direction;

    template <typename Info, typename Segment1, typename Segment2>
    static return_type segments_crosses(side_info const& sides, Info const& info,
                                        Segment1 const& a, Segment2 const& b)
    {
        return {points_policy::segments_crosses(sides, info, a, b),
                direction_policy::segments_crosses(sides, info, a, b)};
    }

    template <typename Info, typename Point>
    static return_type segments_share_common_point(side_info const& sides, Info const& info, Point const& point)
    {
        return {points_policy::segments_share_common_point(sides, info, point),
                direction_policy::segments_share_common_point(sides, info, point)};
    }

    template <typename Segment1, typename Segment2, typename Token>
    static return_type segments_collinear(Segment1 const& a, Segment2 const& b, bool opposite,
        int a1, int a2, int b1, int b2, Token const& t1, Token const& t2, Token const& t3, Token const& t4)
    {
        return {points_policy::segments_collinear(a, b, opposite, a1, a2, b1, b2, t1, t2, t3, t4),
                direction_policy::segments_collinear(a, b, opposite, a1, a2, b1, b2, t1, t2, t3, t4)};
    }

    template <typename Segment>
    static return_type degenerate(Segment const& segment, bool a_degenerate)
    {
        return {points_policy::degenerate(segment, a_degenerate),
                direction_policy::degenerate(segment, a_degenerate)};
    }

    template <typename Segment, typename Token>
    static return_type one_degenerate(Segment const& segment, Token const& token, bool a_degenerate)
    {
        return {points_policy::one_degenerate(segment, token, a_degenerate),
                direction_policy::one_degenerate(segment, token, a_degenerate)};
    }
};

// The position strategy supplies ordinal locations, not segment fractions.
template <typename Selected, typename Position, typename Token, typename A, typename B, typename Equal>
typename Selected::return_type collinear(A const& a, B const& b, Equal const& equal)
{
    auto const& a0 = a.first;
    auto const& a1 = a.second;
    auto const& b0 = b.first;
    auto const& b1 = b.second;
    bool const a_point = equal(a0, a1), b_point = equal(b0, b1);
    Token const token;
    if (a_point && b_point) return equal(a0, b0) ? Selected::degenerate(a, true) : Selected::disjoint();
    if (a_point || b_point)
    {
        int const location = a_point ? classify_collinear_position<Position>(b0, b1, a0)
                                     : classify_collinear_position<Position>(a0, a1, b0);
        if (location < segment_start || location > segment_end) return Selected::disjoint();
        return a_point ? Selected::one_degenerate(a, token, true)
                       : Selected::one_degenerate(b, token, false);
    }
    int const a_from = classify_collinear_position<Position>(b0, b1, a0);
    int const a_to = classify_collinear_position<Position>(b0, b1, a1);
    if ((a_from == segment_before && a_to == segment_before)
        || (a_from == segment_after && a_to == segment_after)) return Selected::disjoint();
    int const b_from = classify_collinear_position<Position>(a0, a1, b0);
    int const b_to = classify_collinear_position<Position>(a0, a1, b1);
    return Selected::segments_collinear(a, b, Position::apply(a0, a1, b0, b1) > 0,
                                       a_from, a_to, b_from, b_to, token, token, token, token);
}

struct use_input_endpoint
{
    template <typename Point>
    Point const& operator()(Point const& point, int) const { return point; }
};

struct accept_crossing
{
    template <typename A, typename B>
    bool operator()(A const&, B const&) const { return true; }
};

template <bool AssumeSimplicity, typename Selected, typename P, typename Q,
          typename Side, typename Equal, typename Collinear, typename Construction,
          typename Endpoint = use_input_endpoint, typename Accept = accept_crossing>
typename Selected::return_type classify(P const& p0, P const& p1, Q const& q0, Q const& q1,
    Side const& side, Equal const& equal, Collinear const& collinear,
    Construction const& construction, Endpoint const& endpoint = {}, Accept const& accept = {})
{
    model::referring_segment<P const> const a(p0, p1);
    model::referring_segment<Q const> const b(q0, q1);
    side_info sides;
    if constexpr (AssumeSimplicity)
    {
        using strategy::side::left;
        using strategy::side::right;
        bool const sa = side(q0, q1, p0) > 0;
        if (sa == (side(q0, q1, p1) > 0)) return Selected::disjoint();
        bool const sb = side(p0, p1, q0) > 0;
        if (sb == (side(p0, p1, q1) > 0)) return Selected::disjoint();
        sides.template set<0>(sa ? left : right, sa ? right : left);
        sides.template set<1>(sb ? left : right, sb ? right : left);
        return Selected::segments_crosses(sides, construction(-1), a, b);
    }
    else
    {
        int const sa0 = side(q0, q1, p0), sa1 = side(q0, q1, p1);
        if (sa0 * sa1 > 0) return Selected::disjoint();
        int const sb0 = side(p0, p1, q0), sb1 = side(p0, p1, q1);
        if (sb0 * sb1 > 0) return Selected::disjoint();
        if (sa0 == 0 && sa1 == 0 && sb0 == 0 && sb1 == 0) return collinear(a, b);
        int const contact = sa0 == 0 ? 0 : sa1 == 0 ? 1 : sb0 == 0 ? 2 : sb1 == 0 ? 3 : -1;
        if (contact < 0 && !accept(a, b)) return Selected::disjoint();

        auto const info = construction(contact);
        sides.template set<0>(sa0, sa1);
        sides.template set<1>(sb0, sb1);
        if (sa0 == 0 && (equal(p0, q0) || equal(p0, q1)))
            return Selected::segments_share_common_point(sides, info, endpoint(p0, 0));
        if (sa1 == 0 && (equal(p1, q0) || equal(p1, q1)))
            return Selected::segments_share_common_point(sides, info, endpoint(p1, 1));
        return Selected::segments_crosses(sides, info, a, b);
    }
}

}}}}} // namespace boost::geometry::strategies::detail::cartesian_segment_events

#endif
