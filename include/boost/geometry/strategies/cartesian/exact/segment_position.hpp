// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_SEGMENT_POSITION_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_SEGMENT_POSITION_HPP

#include <cstddef>
#include <boost/geometry/strategies/intersection_result.hpp>
#include <boost/geometry/strategies/cartesian/exact/static_predicates.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

// Type-level opt-in only: exact turns store their point, not a second scalar
// parameter or another copy of the point's supporting segments.
struct segment_position_tag
{
    using no_fraction = void;

    template <typename A, typename B, typename C, typename D>
    static int compare(A const& a, B const& b, exact_point<C> const& p, exact_point<D> const& q)
    { return compare_along_segment<C>(a, b, p, q); }

    template <typename C, typename B>
    static bool equal(exact_point<C> const& a, B const& b)
    { return static_equal_points<C>(a, b); }
};

template <typename Point>
struct segment_intersection_result
{
    using point_type = Point;
    std::size_t count = 0;
    Point intersections[2];
};

template <typename Coordinate>
struct segment_position
{
    template <typename A, typename B, typename P, typename Q>
    static int apply(A const& a, B const& b, P const& p, Q const& q)
    {
        return compare_along_segment<Coordinate>(a, b, p, q);
    }

    // Segment direction policies consume these five ordinal locations, but no
    // numerical parameter is formed. Precondition: p lies on the support line.
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
        return classify_point_on_segment<Coordinate>(a, b, p);
    }
};

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

namespace boost { namespace geometry
{
template <typename Point>
struct segment_intersection_points<Point, strategies::detail::exact_cartesian::segment_position_tag>
    : strategies::detail::exact_cartesian::segment_intersection_result<Point>
{};
}}

#endif
