// Boost.Geometry
// Copyright (c) 2026
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_CONVEX_DILATION_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_CONVEX_DILATION_HPP

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

#include <boost/geometry/algorithms/detail/buffer/certified/convex_erosion.hpp>

namespace boost { namespace geometry { namespace detail { namespace certified_convex_dilation
{

namespace source = certified_convex_erosion;
namespace common = certified_point_buffer;
namespace rb = common::rb;
using common::cross;
using common::dot;
using common::exact_number;
using common::rational_xy;
using source::status;

constexpr std::size_t max_vertices = 4096;
constexpr unsigned max_depth = 16;

inline rb::circle_direction direction(rational_xy const& n)
{
    using boost::multiprecision::abs;
    exact_number const scale = abs(n.x) + abs(n.y);
    rational_xy const unit_l1{n.x / scale, n.y / scale};
    if (!source::bounded(unit_l1.x) || !source::bounded(unit_l1.y))
    { throw std::overflow_error("Convex buffer direction exceeds the exact work bound"); }
    return rb::circle_direction(unit_l1);
}

template <typename Emit>
bool subdivide(rb::circle_direction const& a, rb::circle_direction const& b,
               exact_number const& r, exact_number const& e, unsigned depth, Emit const& emit)
{
    if (rb::minor_arc_radial_error_within(a, b, r, e)) { return emit(a); }
    if (depth == max_depth) { return false; }

    // A positive sum of the two L1-normalized directions stays strictly
    // inside the minor sweep. Only the exact sagitta test accepts a chord.
    auto const& u = a.numerator();
    auto const& v = b.numerator();
    auto const mid = direction({u.x + v.x, u.y + v.y});
    return subdivide(a, mid, r, e, depth + 1, emit)
        && subdivide(mid, b, r, e, depth + 1, emit);
}

struct vertex
{
    std::size_t corner;
    rb::circle_direction normal;
};

inline bool build(std::vector<rational_xy> const& ring, exact_number const& r,
                  exact_number const& e, std::vector<vertex>& boundary)
{
    std::vector<rb::circle_direction> normals;
    normals.reserve(ring.size());
    for (std::size_t i = 0; i < ring.size(); ++i)
    {
        auto const edge = common::difference(ring[(i + 1) % ring.size()], ring[i]);
        normals.push_back(direction({edge.y, -edge.x})); // outward of CCW
    }
    for (std::size_t i = 0; i < ring.size(); ++i)
    {
        auto const& a = normals[i == 0 ? ring.size() - 1 : i - 1];
        auto const& b = normals[i];
        auto const emit = [&](rb::circle_direction const& n)
        {
            if (boundary.size() == max_vertices) { return false; }
            boundary.push_back({i, n});
            return true;
        };
        // The source guard excludes reversals. Co-directed supports meet
        // at one exact point; keep that point, but do not emit it twice.
        if (cross(a.numerator(), b.numerator()) != 0 && !subdivide(a, b, r, e, 0, emit))
        { return false; }
        if (!emit(b)) { return false; }
    }
    // Every translated source edge lies between its two exact offset
    // endpoints. Between tangents the directed chords sweep strictly
    // in order, so the closed boundary is convex with those same supports.
    return true;
}

inline bool within(rational_xy const& p, rational_xy const& base, rb::circle_direction const& n,
                   exact_number const& r, exact_number const& r2, exact_number const& limit2)
{
    auto const delta = common::difference(base, p);
    // Square the Euclidean error to base + r*n/sqrt(n.n). Clearing the
    // positive root leaves one sign-aware comparison, with no root field.
    return rb::kernel_detail::compare_radical(2 * r * dot(delta, n.numerator()),
                 limit2 - dot(delta, delta) - r2, n.squared_length()) <= 0;
}

template <typename Polygon>
bool export_ring(std::vector<rational_xy> const& ring, std::vector<vertex> const& boundary,
                 Polygon& candidate, exact_number const& r, exact_number const& d)
{
    using point = point_type_t<Polygon>;
    using rounder = common::ec::point_detail::output_rounder<coordinate_type_t<point>>;
    constexpr bool cw = geometry::point_order<Polygon>::value == clockwise;
    constexpr bool closed_output = closure<Polygon>::value == closed;
    auto& exterior = exterior_ring(candidate);
    auto const at = [&](std::size_t i) -> vertex const&
    { return boundary[cw ? boundary.size() - 1 - i : i]; };
    for (std::size_t i = 0; i < boundary.size(); ++i)
    {
        auto const& v = at(i);
        auto const& base = ring[v.corner];
        auto const& n = v.normal;
        auto const root = source::sqrt_enclosure(n.squared_length());
        // The enclosure only proposes coordinates; the exact one-root
        // displacement predicate checks what the output actually stored.
        exact_number const scale = r / ((root.low + root.high) / 2);
        point p;
        geometry::set<0>(p, rounder::apply(base.x + scale * n.numerator().x));
        geometry::set<1>(p, rounder::apply(base.y + scale * n.numerator().y));
        range::push_back(exterior, std::move(p));
    }
    if constexpr (closed_output)
    {
        if (boost::empty(exterior)) { return false; }
        range::push_back(exterior, point(*boost::begin(exterior)));
    }

    std::vector<rational_xy> stored;
    stored.reserve(boundary.size());
    exact_number const r2 = r * r, d2 = d * d;
    auto it = boost::begin(exterior);
    auto const end = boost::end(exterior);
    for (std::size_t i = 0; i < boundary.size(); ++i)
    {
        if (it == end) { return false; }
        auto p = common::read_point(*it++);
        auto const& v = at(i);
        if (!within(p, ring[v.corner], v.normal, r, r2, d2)) { return false; }
        stored.push_back(std::move(p));
    }
    if constexpr (closed_output)
    {
        if (it == end || !source::same(common::read_point(*it++), stored.front())) { return false; }
    }
    if (cw) { std::reverse(stored.begin(), stored.end()); }
    std::vector<rational_xy> corners;
    return it == end && boost::empty(interior_rings(candidate))
        && source::convex_corners(stored, corners);
}

template <typename Input, typename Polygon>
status apply(Input const& input, Polygon& candidate, exact_number const& r,
             exact_number const& sagitta, exact_number const& displacement)
{
    std::vector<rational_xy> ring;
    try
    {
        auto const result = source::read_source<true>(input, ring);
        if (result != status::success) { return result; }
    }
    catch (std::domain_error const&) { return status::invalid_input; }

    exact_number const e = sagitta < r ? sagitta : exact_number(r / 2);
    exact_number const inner = r - e;
    exact_number const d = displacement < inner ? displacement : exact_number(inner / 2);
    std::vector<vertex> boundary;
    try
    {
        return build(ring, r, e, boundary) && export_ring(ring, boundary, candidate, r, d)
            ? status::success : status::cannot_certify;
    }
    catch (std::overflow_error const&) { return status::cannot_certify; }
    catch (std::domain_error const&) { return status::cannot_certify; }
}

}}}} // namespace boost::geometry::detail::certified_convex_dilation

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_CONVEX_DILATION_HPP
