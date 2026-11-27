// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_RATIONAL_TOPOLOGY_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_RATIONAL_TOPOLOGY_HPP

#include <boost/geometry/strategies/cartesian/exact/lazy_decimal_geometry.hpp>
#include <boost/geometry/strategies/cartesian/exact/ring_topology_strategy.hpp>
#include <boost/geometry/strategies/detail/topology.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

namespace lazy_decimal_detail
{

template <std::size_t D, typename Point>
inline scalar coordinate(Point const& point) { return geometry::get<D>(point); }

template <std::size_t D, typename Box>
inline scalar coordinate(source_geometry_detail::indexed_vertex<Box> const& point)
{ return source_geometry_detail::coordinate<D>(point); }

struct point_order
{
    template <std::size_t D, typename A, typename B>
    static int compare(A const& a, B const& b)
    { return scalar::compare(coordinate<D>(a), coordinate<D>(b)); }

    template <typename A, typename B>
    static bool less(A const& a, B const& b)
    {
        int const x = compare<0>(a, b);
        return x < 0 || (x == 0 && compare<1>(a, b) < 0);
    }

    template <typename A, typename B>
    static bool equal(A const& a, B const& b)
    { return compare<0>(a, b) == 0 && compare<1>(a, b) == 0; }

    template <typename A, typename B, typename P>
    static int side(A const& a, B const& b, P const& p)
    {
        auto const x = coordinate<0>(a), y = coordinate<1>(a);
        return scalar::compare((coordinate<0>(b) - x) * (coordinate<1>(p) - y),
                               (coordinate<1>(b) - y) * (coordinate<0>(p) - x));
    }
};

struct segment_position : point_order
{
    using no_fraction = void;

    template <typename A, typename B, typename P, typename Q>
    static int apply(A const& a, B const& b, P const& p, Q const& q)
    { return compare(a, b, p, q); }

    template <typename A, typename B, typename P, typename Q>
    static int compare(A const& a, B const& b, P const& p, Q const& q)
    {
        int const x = point_order::compare<0>(b, a);
        return x != 0 ? x * point_order::compare<0>(p, q)
                      : point_order::compare<1>(b, a) * point_order::compare<1>(p, q);
    }

    template <typename A, typename B, typename P>
    static segment_location classify(A const& a, B const& b, P const& p)
    {
        if (equal(a, b)) return equal(a, p) ? segment_degenerate_on : segment_degenerate_off;
        if (equal(a, p)) return segment_start;
        if (equal(b, p)) return segment_end;
        if (side(a, b, p) != strategy::side::collinear) return segment_off_line;
        if (compare(a, b, p, a) < 0) return segment_before;
        return compare(a, b, p, b) > 0 ? segment_after : segment_interior;
    }
};

template <typename Strategy>
struct topology : strategies::detail::tolerance_based_topology<Strategy>
{
    static constexpr bool uses_cluster_tolerance = false;

    using strategies::detail::tolerance_based_topology<Strategy>::tolerance_based_topology;

    template <std::size_t D, typename A, typename B>
    static int compare_coordinate(A const& a, B const& b)
    { return point_order::compare<D>(a, b); }

    template <typename A, typename B>
    static int compare_y(A const& a, B const& b)
    { return compare_coordinate<1>(a, b); }

    template <typename A, typename B>
    static bool cluster_equal(A const& a, B const& b) { return point_order::equal(a, b); }

};

} // namespace lazy_decimal_detail

template <>
struct ring_topology_detail::predicates<lazy_decimal_scalar> : lazy_decimal_detail::point_order
{
    using scalar = lazy_decimal_scalar;

    template <typename Source, typename Point>
    static void assign(Source const& source, Point& point)
    {
        geometry::set<0>(point, lazy_decimal_detail::coordinate<0>(source));
        geometry::set<1>(point, lazy_decimal_detail::coordinate<1>(source));
    }

    template <typename A, typename B>
    static lazy_decimal_detail::point midpoint(A const& a, B const& b)
    {
        using lazy_decimal_detail::coordinate;
        return {(coordinate<0>(a) + coordinate<0>(b)) / 2,
                (coordinate<1>(a) + coordinate<1>(b)) / 2};
    }
};

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

#endif
