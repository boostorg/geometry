// Boost.Geometry
// Copyright (c) 2026
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_LINESTRING_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_LINESTRING_HPP

#include <algorithm>
#include <cstddef>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

#include <boost/geometry/algorithms/detail/buffer/certified/segment.hpp>
#include <boost/geometry/geometries/polygon.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace certified_linestring_buffer
{

namespace capsule = certified_segment_buffer;
using capsule::cross;
using capsule::difference;
using capsule::dot;
using capsule::exact_number;
using capsule::rational_value;
using capsule::rational_xy;
using capsule::read_point;
using capsule::same_point;
using status = certified_buffer_status;

using piece = std::vector<rational_xy>;
using exact_point = model::point<exact_number, 2, cs::cartesian>;
using exact_polygon = model::polygon<exact_point, false, false>;

constexpr std::size_t max_pieces = 32;
constexpr std::size_t max_piece_edges = 512;
constexpr std::size_t max_boundary_edges = 4096;

struct interval_on_edge
{
    exact_number lo = 0, hi = 1;
    bool covers_right = true;
};

// Clip a + t*d to a closed convex CCW piece by intersecting its halfplanes.
// A positive-length interval covers the right side of the open atom, unless
// it lies on a co-directed facet (the piece is then entirely to the left).
inline bool clip(rational_xy const& a, rational_xy const& d, piece const& p,
                 interval_on_edge& result)
{
    auto previous = p.end() - 1;
    for (auto current = p.begin(); current != p.end(); previous = current++)
    {
        auto const facet = difference(*current, *previous);
        exact_number const intercept = cross(facet, difference(a, *previous));
        exact_number const slope = cross(facet, d);
        if (slope == 0)
        {
            if (intercept < 0) { return false; }
            if (intercept == 0 && dot(facet, d) > 0) { result.covers_right = false; }
        }
        else
        {
            exact_number const t = -intercept / slope;
            if (slope > 0 && t > result.lo) { result.lo = t; }
            if (slope < 0 && t < result.hi) { result.hi = t; }
            if (result.lo > result.hi) { return false; }
        }
    }
    return true;
}

struct less_xy
{
    bool operator()(rational_xy const& a, rational_xy const& b) const
    { return a.x < b.x || (a.x == b.x && a.y < b.y); }
};

using boundary_edges = std::map<rational_xy, rational_xy, less_xy>;

inline bool add_boundary_atoms(rational_xy const& from, rational_xy const& to,
                               std::vector<piece> const& pieces, boundary_edges& edges)
{
    auto const d = difference(to, from);
    std::vector<exact_number> cuts{0, 1};
    std::vector<interval_on_edge> covering;
    for (auto const& p : pieces)
    {
        interval_on_edge span;
        if (!clip(from, d, p, span)) { continue; }
        // Include tangencies and same-side overlaps as nodes, even if they
        // hide no open atom. This also aligns duplicate boundary atoms.
        cuts.push_back(span.lo);
        cuts.push_back(span.hi);
        if (span.covers_right && span.lo < span.hi) { covering.push_back(std::move(span)); }
    }
    std::sort(cuts.begin(), cuts.end());
    cuts.erase(std::unique(cuts.begin(), cuts.end()), cuts.end());
    auto const at = [&](exact_number const& t) -> rational_xy
    {
        return {from.x + t * d.x, from.y + t * d.y};
    };
    auto before = cuts.begin();
    for (auto after = before + 1; after != cuts.end(); before = after++)
    {
        exact_number const mid = (*before + *after) / 2;
        auto const hides = [&](interval_on_edge const& span) { return span.lo < mid && mid < span.hi; };
        if (std::any_of(covering.begin(), covering.end(), hides)) { continue; }
        auto const endpoint = at(*after);
        auto const insertion = edges.emplace(at(*before), endpoint);
        if (!same_point(insertion.first->second, endpoint) || edges.size() > max_boundary_edges)
        {
            return false;
        }
    }
    return true;
}

// Pieces are the strictly convex CCW chord polygons produced by the kernel.
// In particular there are no intermediate vertices on a supporting facet.
inline bool union_one_ring(std::vector<piece> const& pieces, piece& ring)
{
    boundary_edges edges;
    for (auto const& p : pieces)
    {
        auto previous = p.end() - 1;
        for (auto current = p.begin(); current != p.end(); previous = current++)
        {
            if (!add_boundary_atoms(*previous, *current, pieces, edges)) { return false; }
        }
    }
    if (edges.size() < 3) { return false; }

    // Boundary atoms are already noded. A single cycle consuming every
    // atom rejects holes, touching branches and disconnected boundaries.
    auto const first = edges.begin();
    auto it = first;
    exact_number area2 = 0;
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        if (it == edges.end() || (i != 0 && it == first)) { return false; }
        ring.push_back(it->first);
        area2 += cross(it->first, it->second);
        it = edges.find(it->second);
    }
    return it == first && area2 > 0;
}

template <typename Ring>
inline bool write_ring_lossless(piece const& vertices, Ring& ring, bool reverse, bool close)
{
    using point = point_type_t<Ring>;
    using coordinate = coordinate_type_t<point>;
    using rounder = certified_point_buffer::coordinate_writer<coordinate>;
    auto const at = [&](std::size_t i) -> rational_xy const&
    {
        if (i == vertices.size()) { i = 0; }
        return vertices[reverse ? vertices.size() - 1 - i : i];
    };
    std::size_t const size = vertices.size() + (close ? 1 : 0);
    for (std::size_t i = 0; i < size; ++i)
    {
        auto const& v = at(i);
        point p;
        geometry::set<0>(p, rounder::apply(v.x));
        geometry::set<1>(p, rounder::apply(v.y));
        range::push_back(ring, std::move(p));
    }
    auto it = boost::begin(ring);
    auto const end = boost::end(ring);
    for (std::size_t i = 0; i < size; ++i)
    {
        if (it == end || !same_point(read_point(*it++), at(i))) { return false; }
    }
    return it == end;
}

template <typename Polygon>
inline bool export_lossless(piece const& ccw, Polygon& output)
{
    Polygon candidate;
    auto const& holes = interior_rings(candidate);
    if (boost::begin(holes) != boost::end(holes)
        || !write_ring_lossless(ccw, exterior_ring(candidate),
                                geometry::point_order<Polygon>::value == clockwise,
                                closure<Polygon>::value == closed))
    {
        return false;
    }
    output = std::move(candidate);
    return true;
}

inline status append_piece(rational_xy const& a, rational_xy const& b,
                           exact_number const& r, exact_number const& error,
                           std::vector<piece>& pieces, std::size_t& edge_count)
{
    if (pieces.size() == max_pieces) { return status::cannot_certify; }
    exact_polygon p;
    auto const result = capsule::build(a, b, p, r, error, true);
    if (result != status::success) { return result; }
    edge_count += p.outer().size();
    if (edge_count > max_piece_edges) { return status::cannot_certify; }
    pieces.emplace_back();
    for (auto const& v : p.outer()) { pieces.back().push_back(read_point(v)); }
    return status::success;
}

}} // namespace detail::certified_linestring_buffer
#endif // DOXYGEN_NO_DETAIL

/*!
\brief Experimental certified round buffer of a 2D Cartesian Linestring.
\details Each nonzero edge must have rational length. The output is the
    exact union of the inscribed chord capsules, exported without any
    coordinate change. Only one simple exterior ring without holes is
    supported. On success the filled output lies inside the exact radius-r
    buffer and strictly contains the radius-(r-max_error) buffer if positive.
    This bounds filled-set Hausdorff distance strictly, not boundary
    Hausdorff distance or topology of the smooth round buffer.
\return success, invalid_input or cannot_certify. Returned failures leave
    output unchanged. Irrational supports, topology outside one simple ring,
    loss of coordinates on export and resource limits are cannot_certify.
*/
template <typename Linestring, typename Polygon, typename Radius, typename Error>
[[nodiscard]] inline certified_buffer_status buffer_linestring_certified(
    Linestring const& input, Polygon& output, Radius const& radius, Error const& max_error)
{
    namespace impl = detail::certified_linestring_buffer;
    using status = certified_buffer_status;
    impl::capsule::check_types<linestring_tag, Linestring, Polygon>();
    auto it = boost::begin(input);
    auto const end = boost::end(input);
    if (it == end) { return status::invalid_input; }

    impl::exact_number r, error;
    impl::rational_xy previous;
    std::vector<impl::piece> pieces;
    std::size_t edge_count = 0;
    try
    {
        r = impl::rational_value(radius);
        error = impl::rational_value(max_error);
        if (r <= 0 || error < 0) { return status::invalid_input; }
        if (error == 0) { return status::cannot_certify; }
        // Keep a positive inner buffer even for a very loose request.
        if (error >= r) { error = r / 2; }
        previous = impl::read_point(*it++);
        for (; it != end; ++it)
        {
            auto current = impl::read_point(*it);
            if (!impl::same_point(previous, current))
            {
                auto const result = impl::append_piece(previous, current, r, error, pieces, edge_count);
                if (result != status::success) { return result; }
                previous = std::move(current);
            }
        }
        if (pieces.empty())
        {
            auto const result = impl::append_piece(previous, previous, r, error, pieces, edge_count);
            if (result != status::success) { return result; }
        }
    }
    catch (std::domain_error const&)
    {
        return status::invalid_input;
    }

    impl::piece boundary;
    if (!impl::union_one_ring(pieces, boundary)) { return status::cannot_certify; }
    try
    {
        return impl::export_lossless(boundary, output) ? status::success : status::cannot_certify;
    }
    catch (std::overflow_error const&)
    {
        return status::cannot_certify;
    }
    catch (std::domain_error const&)
    {
        // An out-of-range floating coordinate can round to infinity.
        return status::cannot_certify;
    }
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_LINESTRING_HPP
