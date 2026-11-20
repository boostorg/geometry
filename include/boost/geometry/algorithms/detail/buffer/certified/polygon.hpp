// Boost.Geometry
// Copyright (c) 2026
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_POLYGON_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_POLYGON_HPP

#include <cstddef>
#include <set>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <boost/range/value_type.hpp>

#include <boost/geometry/algorithms/detail/buffer/certified/rational_chord_arrangement.hpp>

namespace boost { namespace geometry
{

enum class certified_polygon_buffer_mode { dilation, erosion, boundary_band };

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace certified_polygon_buffer
{

namespace arrangement = rational_chord_arrangement;
namespace line = certified_linestring_buffer;
using arrangement::exact_number;
using arrangement::piece;
using arrangement::rational_xy;
using arrangement::ring;
using arrangement::same_point;
using arrangement::sides;
using status = certified_buffer_status;
using mode = certified_polygon_buffer_mode;

template <typename Ring>
inline status read_ring(Ring const& input, bool closed, bool ccw,
                        std::vector<ring>& source, std::size_t& edge_count)
{
    piece vertices;
    std::size_t const limit = line::max_pieces - edge_count + (closed ? 1 : 0);
    for (auto const& p : input)
    {
        if (vertices.size() == limit) { return status::cannot_certify; }
        auto q = line::read_point(p);
        if (! certified_point_buffer::bounded(q.x) || ! certified_point_buffer::bounded(q.y))
        {
            return status::cannot_certify;
        }
        vertices.push_back(std::move(q));
    }
    if (vertices.size() < 3u + (closed ? 1 : 0)) { return status::invalid_input; }
    if (closed)
    {
        if (!same_point(vertices.front(), vertices.back())) { return status::invalid_input; }
        vertices.pop_back();
    }
    auto previous = vertices.end() - 1;
    for (auto current = vertices.begin(); current != vertices.end(); previous = current++)
    {
        if (same_point(*previous, *current)) { return status::invalid_input; }
    }
    edge_count += vertices.size();
    source.emplace_back(std::move(vertices));
    exact_number const& area = source.back().area2;
    return area != 0 && (area > 0) == ccw ? status::success : status::invalid_input;
}

// The chord arrangement requires simple rings with pairwise disjoint boundaries.
// Even a single inter-ring contact is outside this input contract.
inline bool valid_source(std::vector<ring> const& rings)
{
    for (std::size_t k = 0; k < rings.size(); ++k)
    {
        auto const& r = rings[k];
        for (std::size_t i = 0; i < r.edges.size(); ++i)
        {
            for (std::size_t j = i + 1; j < r.edges.size(); ++j)
            {
                arrangement::intersection x;
                if (!arrangement::intersect(r.edges[i], r.edges[j], x)) { continue; }
                bool const adjacent = j == i + 1 || (i == 0 && j + 1 == r.edges.size());
                if (!adjacent || x.t0 != x.t1) { return false; }
            }
        }
        if (k == 0) { continue; }
        if (arrangement::locate(rings.front(), r.points.front()) != 1) { return false; }
        for (std::size_t j = 0; j < k; ++j)
        {
            auto const& other = rings[j];
            for (auto const& a : r.edges)
            {
                for (auto const& b : other.edges)
                {
                    arrangement::intersection x;
                    if (arrangement::intersect(a, b, x)) { return false; }
                }
            }
            if (j != 0 && (arrangement::locate(other, r.points.front()) != -1
                        || arrangement::locate(r, other.points.front()) != -1))
            {
                return false;
            }
        }
    }
    return true;
}

inline status make_band(std::vector<ring> const& source, exact_number const& radius,
                        exact_number const& error, std::vector<ring>& band)
{
    std::vector<piece> pieces;
    std::size_t edge_count = 0;
    for (auto const& r : source)
    {
        for (auto const& e : r.edges)
        {
            auto const result = line::append_piece(e.from, e.to, radius, error, pieces, edge_count);
            if (result != status::success) { return result; }
        }
    }
    for (auto& p : pieces) { band.emplace_back(std::move(p)); }
    return status::success;
}

inline sides select(rational_xy const& mid, rational_xy const& direction,
                    std::vector<ring> const& source, std::vector<ring> const& band, mode operation)
{
    sides tube{false, false};
    for (auto const& p : band)
    {
        auto const s = arrangement::incident(p, mid, direction);
        tube = {tube.left || s.left, tube.right || s.right};
        if (tube.left && tube.right) { break; }
    }
    if (operation == mode::boundary_band) { return tube; }
    sides material = arrangement::incident(source.front(), mid, direction);
    for (auto h = source.begin() + 1; h != source.end() && (material.left || material.right); ++h)
    {
        auto const hole = arrangement::incident(*h, mid, direction);
        material = {material.left && !hole.left, material.right && !hole.right};
    }
    return operation == mode::dilation
        ? sides{material.left || tube.left, material.right || tube.right}
        : sides{material.left && !tube.left, material.right && !tube.right};
}

struct component
{
    std::size_t shell;
    std::vector<std::size_t> holes;
};

inline bool group_cycles(std::vector<ring> const& cycles, std::vector<component>& components)
{
    for (std::size_t i = 0; i < cycles.size(); ++i)
    {
        if (cycles[i].area2 > 0) { components.push_back({i, {}}); }
    }
    for (std::size_t i = 0; i < cycles.size(); ++i)
    {
        if (cycles[i].area2 > 0) { continue; }
        // Disjoint boundary cycles are either nested or disjoint. The
        // innermost containing positive cycle owns this hole.
        auto owner = components.end();
        for (auto c = components.begin(); c != components.end(); ++c)
        {
            int const where = arrangement::locate(cycles[c->shell], cycles[i].points.front());
            if (where == 0) { return false; }
            if (where == 1 && (owner == components.end()
                || cycles[c->shell].area2 < cycles[owner->shell].area2))
            {
                owner = c;
            }
        }
        if (owner == components.end()) { return false; }
        owner->holes.push_back(i);
    }
    return true;
}

template <typename MultiPolygon>
inline bool export_lossless(std::vector<ring> const& cycles, MultiPolygon& output)
{
    std::vector<component> components;
    if (!group_cycles(cycles, components)) { return false; }

    using polygon = typename boost::range_value<MultiPolygon>::type;
    bool constexpr cw = geometry::point_order<polygon>::value == clockwise;
    bool constexpr closed = closure<polygon>::value == geometry::closed;
    MultiPolygon candidate;
    range::resize(candidate, components.size());
    auto p = boost::begin(candidate);
    for (auto const& c : components)
    {
        if (p == boost::end(candidate)
            || !line::write_ring_lossless(cycles[c.shell].points, exterior_ring(*p), cw, closed))
        {
            return false;
        }
        auto& holes = interior_rings(*p);
        range::resize(holes, c.holes.size());
        auto h = boost::begin(holes);
        for (auto const i : c.holes)
        {
            if (h == boost::end(holes) || !line::write_ring_lossless(cycles[i].points, *h, cw, closed))
            {
                return false;
            }
            ++h;
        }
        if (h != boost::end(holes)) { return false; }
        ++p;
    }
    if (p != boost::end(candidate)) { return false; }
    output = std::move(candidate);
    return true;
}

}} // namespace detail::certified_polygon_buffer
#endif // DOXYGEN_NO_DETAIL

/*!
\brief Experimental certified chord buffer of a 2D Cartesian Polygon.
\details Requires simple, correctly oriented rings with disjoint boundaries,
    rational source edge lengths, and a MultiPolygon output. Dilation and
    boundary_band have a strict filled-set error bound, not a smooth-boundary
    topology bound. Erosion is exact for a convex shell without holes;
    otherwise it is regularized and has set-inclusion bounds at radii r and
    r-max_error (when positive), not a Hausdorff promise. Empty erosion is
    supported. Export preserves every exact chord vertex.
\return success, invalid_input or cannot_certify. On failure output is
    unchanged. Irrational lengths, capacity limits, point-touching output
    rings, or lossy coordinates are refused.
*/
template <typename Polygon, typename MultiPolygon, typename Radius, typename Error>
[[nodiscard]] inline certified_buffer_status buffer_polygon_certified(
    Polygon const& input, MultiPolygon& output, Radius const& radius, Error const& max_error,
    certified_polygon_buffer_mode operation = certified_polygon_buffer_mode::dilation)
{
    namespace impl = detail::certified_polygon_buffer;
    namespace arr = impl::arrangement;
    using status = certified_buffer_status;
    using mode = certified_polygon_buffer_mode;
    static_assert(std::is_same<tag_t<Polygon>, polygon_tag>::value
               && std::is_same<tag_t<MultiPolygon>, multi_polygon_tag>::value,
                  "Certified polygon buffer requires a Polygon and a MultiPolygon");
    static_assert(dimension<Polygon>::value == 2 && dimension<MultiPolygon>::value == 2
               && std::is_same<cs_tag_t<Polygon>, cartesian_tag>::value
               && std::is_same<cs_tag_t<MultiPolygon>, cartesian_tag>::value,
                  "Certified polygon buffer requires 2D Cartesian geometries");
    concepts::check<Polygon const>();
    concepts::check<MultiPolygon>();

    std::vector<impl::ring> source;
    impl::exact_number r, error;
    try
    {
        r = impl::line::rational_value(radius);
        error = impl::line::rational_value(max_error);
        if (r <= 0 || error < 0 || (operation != mode::dilation && operation != mode::erosion
                               && operation != mode::boundary_band))
        {
            return status::invalid_input;
        }
        if (error == 0) { return status::cannot_certify; }
        if (error >= r) { error = r / 2; }

        bool constexpr ccw = point_order<Polygon>::value == counterclockwise;
        bool constexpr closed = closure<Polygon>::value == geometry::closed;
        std::size_t edge_count = 0;
        auto result = impl::read_ring(exterior_ring(input), closed, ccw, source, edge_count);
        if (result != status::success) { return result; }
        for (auto const& hole : interior_rings(input))
        {
            result = impl::read_ring(hole, closed, !ccw, source, edge_count);
            if (result != status::success) { return result; }
        }
    }
    catch (std::domain_error const&)
    {
        return status::invalid_input;
    }
    if (!impl::valid_source(source)) { return status::invalid_input; }

    std::vector<impl::ring> band;
    auto const result = impl::make_band(source, r, error, band);
    if (result != status::success) { return result; }
    std::vector<arr::edge const*> edges;
    for (auto const* rings : {&source, &band})
    {
        for (auto const& ring : *rings)
        {
            for (auto const& e : ring.edges) { edges.push_back(&e); }
        }
    }
    std::set<arr::atom, arr::less_atom> atoms;
    if (!arr::node(edges, atoms)) { return status::cannot_certify; }
    auto const select = [&](impl::rational_xy const& p, impl::rational_xy const& direction)
    {
        return impl::select(p, direction, source, band, operation);
    };
    std::vector<impl::ring> cycles;
    if (!arr::assemble(atoms, select, cycles)) { return status::cannot_certify; }
    try
    {
        return impl::export_lossless(cycles, output) ? status::success : status::cannot_certify;
    }
    catch (std::overflow_error const&)
    {
        return status::cannot_certify;
    }
    catch (std::domain_error const&)
    {
        return status::cannot_certify;
    }
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_POLYGON_HPP
