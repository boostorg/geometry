// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_FILTERED_SEGMENT_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_FILTERED_SEGMENT_HPP

#include <algorithm>
#include <array>
#include <type_traits>
#include <boost/geometry/algorithms/detail/convert_point_to_point.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/policies/relate/intersection_policy.hpp>
#include <boost/geometry/strategies/cartesian/exact/segment_position.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{
namespace filtered_segment_detail
{
template <typename Policy, typename Coordinate = void>
struct endpoint_policy : Policy {};

template <typename ReturnType, typename Coordinate>
struct endpoint_policy<policies::relate::segments_intersection_points<ReturnType>, Coordinate>
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
        geometry::detail::conversion::convert_point_to_point(point, result.intersections[0]);
        return result;
    }

    template <typename Segment1, typename Segment2, typename Token>
    static return_type segments_collinear(Segment1 const& a, Segment2 const& b, bool,
        int a1, int a2, int b1, int b2, Token const&, Token const&, Token const&, Token const&)
    {
        return_type result;
        std::array<unsigned, 2> selected;
        auto append = [&](unsigned endpoint) { selected[result.count++] = endpoint; };
        if (a1 >= 1 && a1 <= 3 && result.count < 2) append(0);
        if (b1 == 2 && result.count < 2) append(2);
        if (a2 >= 1 && a2 <= 3 && result.count < 2) append(1);
        if (b2 == 2 && result.count < 2) append(3);
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
                { return compare_along_segment<Coordinate>(a.first, a.second, second, first); }); }) < 0)
            std::swap(selected[0], selected[1]);
        for (std::size_t i = 0; i < result.count; ++i)
            visit(selected[i], [&](auto const& point)
                { geometry::detail::conversion::convert_point_to_point(point, result.intersections[i]); });
        return result;
    }

    template <typename Segment>
    static return_type degenerate(Segment const& segment, bool)
    {
        return_type result;
        result.count = 1;
        geometry::detail::conversion::convert_point_to_point(segment.first, result.intersections[0]);
        return result;
    }

    template <typename Segment, typename Token>
    static return_type one_degenerate(Segment const& segment, Token const&, bool a_degenerate)
    { return degenerate(segment, a_degenerate); }
};

template <typename ReturnType, typename Coordinate>
struct endpoint_policy<policies::relate::segments_intersection_policy<ReturnType>, Coordinate>
    : policies::relate::segments_intersection_policy<ReturnType>
{
    using original_policy = policies::relate::segments_intersection_policy<ReturnType>;
    using return_type = typename original_policy::return_type;
    using points_policy = endpoint_policy<policies::relate::segments_intersection_points<ReturnType>, Coordinate>;
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
} // namespace filtered_segment_detail

// Precondition: all four side tests have certified collinearity. Locations
// are ordinal classifications only; no scalar division or parameter is formed.
template <typename Coordinate>
struct filtered_collinear_intersection
{
    template <typename Range1, typename Range2, typename Policy>
    static typename Policy::return_type apply(Range1 const& first, Range2 const& second, Policy const&)
    {
        using selected = filtered_segment_detail::endpoint_policy<Policy, Coordinate>;
        auto const& a0 = first.at(0);
        auto const& a1 = first.at(1);
        auto const& b0 = second.at(0);
        auto const& b1 = second.at(1);
        using first_point = std::remove_cv_t<std::remove_reference_t<decltype(a0)>>;
        using second_point = std::remove_cv_t<std::remove_reference_t<decltype(b0)>>;
        model::referring_segment<first_point const> const a(a0, a1);
        model::referring_segment<second_point const> const b(b0, b1);
        bool const a_point = static_equal_points<Coordinate>(a0, a1);
        bool const b_point = static_equal_points<Coordinate>(b0, b1);
        segment_position_tag const token;
        if (a_point && b_point)
            return static_equal_points<Coordinate>(a0, b0) ? selected::degenerate(a, true) : selected::disjoint();
        if (a_point || b_point)
        {
            int const location = a_point
                ? segment_position<Coordinate>::classify_collinear(b0, b1, a0)
                : segment_position<Coordinate>::classify_collinear(a0, a1, b0);
            if (location < 1 || location > 3) return selected::disjoint();
            return a_point ? selected::one_degenerate(a, token, true)
                           : selected::one_degenerate(b, token, false);
        }
        int const a_from = segment_position<Coordinate>::classify_collinear(b0, b1, a0);
        int const a_to = segment_position<Coordinate>::classify_collinear(b0, b1, a1);
        if ((a_from < 1 && a_to < 1) || (a_from > 3 && a_to > 3)) return selected::disjoint();
        int const b_from = segment_position<Coordinate>::classify_collinear(a0, a1, b0);
        int const b_to = segment_position<Coordinate>::classify_collinear(a0, a1, b1);
        bool const opposite = compare_along_segment<Coordinate>(a0, a1, b0, b1) > 0;
        return selected::segments_collinear(a, b, opposite, a_from, a_to, b_from, b_to,
                                            token, token, token, token);
    }
};

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian
#endif
