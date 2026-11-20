// Boost.Geometry
// Copyright (c) 2026
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_LINESTRING_ARRANGEMENT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_LINESTRING_ARRANGEMENT_HPP

#include <algorithm>
#include <map>
#include <optional>
#include <set>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <boost/range/value_type.hpp>

#include <boost/geometry/algorithms/detail/buffer/certified/linestring_star.hpp>
#include <boost/geometry/algorithms/detail/buffer/certified/polygon.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace certified_star_buffer
{

namespace rational_arr = rational_chord_arrangement;
namespace rational_line = certified_linestring_buffer;

// The same exact producers and local predicate field as the star sweep,
// without a common witness. Polygon source rings can also supply facets.
class algebraic_chord_union
{
    static constexpr id max_pieces = 16;
    static constexpr id max_edges = 384;
    static constexpr id max_pair_checks = 8192;
    static constexpr id max_atoms = 4096;
    static constexpr id max_cycles = 16;

    struct rectangle
    {
        double lx, ly, ux, uy;

        explicit rectangle(box_point const& p)
            : lx(p.x.lower()), ly(p.y.lower()), ux(p.x.upper()), uy(p.y.upper()) {}

        void extend(rectangle const& r)
        {
            lx = (std::min)(lx, r.lx); ly = (std::min)(ly, r.ly);
            ux = (std::max)(ux, r.ux); uy = (std::max)(uy, r.uy);
        }

        bool overlaps(rectangle const& r) const
        { return lx <= r.ux && r.lx <= ux && ly <= r.uy && r.ly <= uy; }
    };

    struct piece_range { id first, end; rectangle box; };
    struct edge_cuts
    {
        id owner;
        rectangle box;
        bool ascending;
        std::vector<knot> cuts;
    };

    static constexpr id source_owner = max_pieces;
    enum class operation { band, dilation, erosion };
    struct native_rings
    {
        std::vector<rational_arr::ring> rings;
        std::vector<id> parent;
    };

public:
    void append(capsule::rational_segment const& source, exact_number const& radius, exact_number const& sagitta)
    {
        if (m_pieces.size() == max_pieces || m_edges.size() == max_edges) { throw uncertified{}; }
        auto const span = m_geometry.append_capsule(source, radius, sagitta, max_edges - m_edges.size());
        m_pieces.push_back(record(span.first, span.second, m_pieces.size()));
    }

    void append_disk(rational_xy const& center, exact_number const& radius, exact_number const& sagitta)
    {
        if (m_pieces.size() == max_pieces || m_edges.size() == max_edges) { throw uncertified{}; }
        rational_line::piece vertices;
        auto emit = [&](rational_xy const& u)
        {
            if (vertices.size() == max_edges - m_edges.size()) { return false; }
            vertices.push_back({center.x + radius * u.x, center.y + radius * u.y});
            return true;
        };
        for (unsigned q = 0; q < 4; ++q)
        {
            rb::circle_direction const a(rb::rational_arc_sample(q, 0)), b(rb::rational_arc_sample(q, 1));
            if (!common::subdivide(q, 0, a, 1, b, radius, sagitta, 0, emit)) { throw uncertified{}; }
        }
        m_pieces.push_back(record_rational(vertices, m_pieces.size()));
    }

    void append_polygon(std::vector<rational_arr::ring> const& source, rational_xy const& anchor,
                        exact_number const& radius, exact_number const& sagitta)
    {
        id const start = m_source.size();
        for (auto const& r : source)
        {
            id const n = r.points.size();
            if (n > max_edges - m_edges.size()) { throw uncertified{}; }
            rational_line::piece vertices;
            for (id i = 0; i < n; ++i)
            {
                vertices.push_back(difference(r.points[r.area2 > 0 ? i : n - 1 - i], anchor));
            }
            m_source.push_back(record_rational(vertices, source_owner));
            for (auto const& e : r.edges)
            {
                append(capsule::rational_segment(difference(e.from, anchor), difference(e.to, anchor)), radius, sagitta);
            }
        }
        m_source_components.emplace_back(start, m_source.size());
    }

    template <typename Polygon>
    bool build(rational_xy const& anchor, exact_number const& displacement, Polygon& output)
    {
        node();
        auto const rings = cycles(operation::band);
        using coordinate = coordinate_type_t<point_type_t<Polygon>>;
        native_rings stored;
        if (!certify_native<coordinate>(rings, anchor, displacement, false, stored)) { return false; }
        id shell = stored.rings.size();
        for (id i = 0; i < stored.rings.size(); ++i)
        {
            if (stored.rings[i].area2 < 0) { continue; }
            if (shell != stored.rings.size()) { return false; }
            shell = i;
        }
        if (shell == stored.rings.size()) { return false; }
        Polygon candidate;
        if (!write_component(stored, shell, candidate)) { return false; }
        output = std::move(candidate);
        return true;
    }

    template <typename MultiPolygon>
    bool build_polygon(rational_xy const& anchor, exact_number const& displacement, bool erosion,
                       MultiPolygon& output)
    {
        return build_multi(erosion ? operation::erosion : operation::dilation, anchor, displacement, output);
    }

    template <typename MultiPolygon>
    bool build_band(rational_xy const& anchor, exact_number const& displacement, MultiPolygon& output)
    {
        return build_multi(operation::band, anchor, displacement, output);
    }

private:
    template <typename MultiPolygon>
    bool build_multi(operation op, rational_xy const& anchor, exact_number const& displacement, MultiPolygon& output)
    {
        node();
        auto const rings = cycles(op);
        bool const erosion = op == operation::erosion;
        if (rings.empty() && !erosion && !m_pieces.empty()) { return false; }
        using polygon = typename boost::range_value<MultiPolygon>::type;
        using coordinate = coordinate_type_t<point_type_t<polygon>>;
        native_rings stored;
        if (!certify_native<coordinate>(rings, anchor, displacement, erosion, stored)) { return false; }
        MultiPolygon candidate;
        auto const count = std::count_if(stored.rings.begin(), stored.rings.end(),
            [](rational_arr::ring const& r) { return r.area2 > 0; });
        range::resize(candidate, static_cast<id>(count));
        auto component = boost::begin(candidate);
        for (id i = 0; i < stored.rings.size(); ++i)
        {
            if (stored.rings[i].area2 < 0) { continue; }
            if (component == boost::end(candidate) || !write_component(stored, i, *component)) { return false; }
            ++component;
        }
        if (component != boost::end(candidate)) { return false; }
        output = std::move(candidate);
        return true;
    }

    knot vertex(id i) const { return {knot_kind::vertex, i, 0, m_geometry.m_vertices[i].bounds, 0, false}; }

    piece_range record_rational(rational_line::piece const& vertices, id owner)
    {
        if (!m_rational_root)
        {
            m_rational_root = m_geometry.m_roots.size();
            m_geometry.m_roots.emplace_back(1);
        }
        auto const& scale = m_geometry.m_roots[*m_rational_root];
        id const vertex_first = m_geometry.m_vertices.size(), edge_first = m_geometry.m_edges.size();
        for (id i = 0; i < vertices.size(); ++i)
        {
            auto const& p = vertices[i];
            m_geometry.m_vertices.push_back({p, {0, 0}, *m_rational_root,
                {scale.coordinate(p.x, 0), scale.coordinate(p.y, 0)}});
            m_geometry.m_edges.push_back({vertex_first + i, vertex_first + (i + 1) % vertices.size()});
        }
        return record(edge_first, m_geometry.m_edges.size(), owner);
    }

    int compare_axis(knot const& a, knot const& b, bool y)
    {
        if (a.kind == b.kind && a.a == b.a && a.b == b.b) { return 0; }
        auto const s = ec::sign(y ? a.bounds.y - b.bounds.y : a.bounds.x - b.bounds.x);
        if (s.certain) { return s.value; }
        m_geometry.fallback();
        envelope::local f(m_geometry);
        auto const p = f.evaluate(a), q = f.evaluate(b);
        return rn::compare(f.f, y ? p.y : p.x, y ? q.y : q.x);
    }

    int compare(knot const& a, knot const& b)
    {
        int const x = compare_axis(a, b, false);
        return x != 0 ? x : compare_axis(a, b, true);
    }

    piece_range record(id first, id end, id owner)
    {
        rectangle box(vertex(m_geometry.m_edges[first].from).bounds);
        for (id i = first; i < end; ++i)
        {
            auto const& e = m_geometry.m_edges[i];
            auto const a = vertex(e.from), b = vertex(e.to);
            rectangle bounds(a.bounds);
            bounds.extend(rectangle(b.bounds));
            box.extend(bounds);
            m_edges.push_back({owner, bounds, compare(a, b) < 0, {a, b}});
        }
        return {first, end, box};
    }

    void node()
    {
        id checks = 0;
        for (id i = 0; i < m_edges.size(); ++i)
        {
            for (id j = i + 1; j < m_edges.size(); ++j)
            {
                auto& a = m_edges[i];
                auto& b = m_edges[j];
                if (a.owner == b.owner || !a.box.overlaps(b.box)) { continue; }
                if (++checks > max_pair_checks) { throw uncertified{}; }
                envelope::local f(m_geometry);
                auto const s = f.segment(i), t = f.segment(j);
                auto const hit = rn::intersect(f.f, s, t);
                if (hit.kind == rn::intersection_kind::disjoint) { continue; }
                if (hit.kind == rn::intersection_kind::degenerate) { throw uncertified{}; }
                auto const insert = [&](rn::parameter const& u, rn::parameter const& v)
                {
                    // An overlap end is an original endpoint. Retain it as
                    // such; only an interior-interior hit needs two producers.
                    auto const& ab = m_geometry.m_edges[i];
                    auto const& cd = m_geometry.m_edges[j];
                    knot p = f.f.sign(u.n) == 0 ? vertex(ab.from)
                        : f.f.sign(f.f.subtract(u.d, u.n)) == 0 ? vertex(ab.to)
                        : f.f.sign(v.n) == 0 ? vertex(cd.from)
                        : f.f.sign(f.f.subtract(v.d, v.n)) == 0 ? vertex(cd.to)
                        : knot{knot_kind::crossing, i, j, m_geometry.crossing_bounds(i, j), 0, false};
                    a.cuts.push_back(p);
                    b.cuts.push_back(p);
                };
                insert(hit.t0, hit.u0);
                if (hit.kind == rn::intersection_kind::overlap) { insert(hit.t1, hit.u1); }
            }
        }
    }

    static rn::parameter midpoint_coordinate(rn::field const& f, rn::parameter const& a, rn::parameter const& b)
    {
        return {f.add(f.multiply(a.n, b.d), f.multiply(b.n, a.d)), f.scale(f.multiply(a.d, b.d), 2)};
    }

    int midpoint_side(id e, knot const& a, knot const& b, box_point const& mid)
    {
        auto const& edge = m_geometry.m_edges[e];
        auto const& p = m_geometry.m_vertices[edge.from].bounds;
        auto const& q = m_geometry.m_vertices[edge.to].bounds;
        auto const s = ec::sign(det(q - p, mid - p));
        if (s.certain) { return s.value; }
        m_geometry.fallback();

        // Both nodes are on the same producer: they use at most three
        // roots together, plus the tested facet's root. Keep the midpoint
        // homogeneous; all denominators are positive.
        envelope::local f(m_geometry);
        auto const u = f.evaluate(a), v = f.evaluate(b);
        auto const edge_exact = f.segment(e);
        auto const offset = [&](rn::parameter const& x, rn::parameter const& y, rn::value const& base)
        {
            auto const m = midpoint_coordinate(f.f, x, y);
            return f.f.subtract(m.n, f.f.multiply(m.d, base));
        };
        return f.f.sign(rn::cross(f.f, rn::subtract(f.f, edge_exact.to, edge_exact.from),
            {offset(u.x, v.x, edge_exact.from.x), offset(u.y, v.y, edge_exact.from.y)}));
    }

    bool exposed(id e, knot const& a, knot const& b, box_point const& mid)
    {
        rectangle const bounds(mid);
        for (id k = 0; k < m_pieces.size(); ++k)
        {
            auto const& p = m_pieces[k];
            if (k == m_edges[e].owner || !p.box.overlaps(bounds)) { continue; }
            bool covers_right = true;
            for (id i = p.first; i < p.end; ++i)
            {
                int const s = midpoint_side(i, a, b, mid);
                // Exact noding excludes a transverse boundary through the
                // midpoint. A zero incident facet only contains the right
                // neighborhood if it runs opposite to this atom.
                if (s < 0 || (s == 0 && m_edges[i].ascending == m_edges[e].ascending))
                {
                    covers_right = false;
                    break;
                }
            }
            if (covers_right) { return false; }
        }
        return true;
    }

    bool above_midpoint(id v, knot const& a, knot const& b, box_point const& mid)
    {
        auto const& p = m_geometry.m_vertices[v];
        auto const s = ec::sign(p.bounds.y - mid.y);
        if (s.certain) { return s.value > 0; }
        m_geometry.fallback();
        envelope::local f(m_geometry);
        auto const u = f.evaluate(a), w = f.evaluate(b);
        auto const y = midpoint_coordinate(f.f, u.y, w.y);
        return f.f.sign(f.f.subtract(f.f.scale(y.d, p.base.y), y.n)) > 0;
    }

    bool source_ring_contains(piece_range const& ring, knot const& a, knot const& b, box_point const& mid)
    {
        if (!ring.box.overlaps(rectangle(mid))) { return false; }
        bool inside = false;
        bool high = above_midpoint(m_geometry.m_edges[ring.first].from, a, b, mid);
        for (id i = ring.first; i < ring.end; ++i)
        {
            bool const next = above_midpoint(m_geometry.m_edges[i].to, a, b, mid);
            if (high != next)
            {
                int const side = midpoint_side(i, a, b, mid);
                if (side == 0) { throw uncertified{}; }
                if ((side > 0) == next) { inside = !inside; }
            }
            high = next;
        }
        return inside;
    }

    bool source_contains(knot const& a, knot const& b, box_point const& mid)
    {
        for (auto const& c : m_source_components)
        {
            if (!source_ring_contains(m_source[c.first], a, b, mid)) { continue; }
            bool in_hole = false;
            for (id i = c.first + 1; i < c.second; ++i)
            {
                if (source_ring_contains(m_source[i], a, b, mid)) { in_hole = true; break; }
            }
            if (!in_hole) { return true; }
        }
        return false;
    }

    struct link { knot to; id edge; };

    std::vector<contour> cycles(operation op)
    {
        auto const less = [&](knot const& a, knot const& b) { return compare(a, b) < 0; };
        std::map<knot, link, decltype(less)> next(less);
        std::set<knot, decltype(less)> incoming(less);
        id atoms = 0;
        for (id i = 0; i < m_edges.size(); ++i)
        {
            // Each source-boundary point is interior to the band, which
            // contains its positive (r-e)-neighborhood. Neither P union T
            // nor regularized P minus T can change side at a source atom.
            if (m_edges[i].owner == source_owner) { continue; }
            auto& c = m_edges[i].cuts;
            std::sort(c.begin(), c.end(), less);
            c.erase(std::unique(c.begin(), c.end(), [&](knot const& a, knot const& b) { return compare(a, b) == 0; }),
                    c.end());
            for (id j = 1; j < c.size(); ++j)
            {
                if (++atoms > max_atoms) { throw uncertified{}; }
                auto const& a = m_edges[i].ascending ? c[j - 1] : c[j];
                auto const& b = m_edges[i].ascending ? c[j] : c[j - 1];
                ec::interval const half{0.5, 0};
                box_point const mid{(a.bounds.x + b.bounds.x) * half, (a.bounds.y + b.bounds.y) * half};
                if (!exposed(i, a, b, mid)) { continue; }
                // Exposed atoms are away from the source rings, so source
                // material is constant on both sides. Dilation selects an
                // outside band atom; erosion selects and reverses an inside one.
                bool const reverse = op == operation::erosion;
                if (op != operation::band && source_contains(a, b, mid) != reverse) { continue; }
                auto const& from = reverse ? b : a;
                auto const& to = reverse ? a : b;
                auto const added = next.emplace(from, link{to, i});
                if (added.second ? !incoming.insert(to).second : compare(added.first->second.to, to) != 0)
                {
                    throw uncertified{}; // Touching or branching boundary.
                }
            }
        }

        std::vector<contour> out;
        while (!next.empty())
        {
            if (out.size() == max_cycles) { throw uncertified{}; }
            contour ring;
            auto const first = next.begin()->first;
            auto cur = first;
            do
            {
                auto const it = next.find(cur);
                if (it == next.end()) { throw uncertified{}; }
                ring.push_back({it->first, it->second.edge});
                cur = it->second.to;
                next.erase(it);
            }
            while (compare(cur, first) != 0);
            if (ring.size() < 3) { throw uncertified{}; }
            out.push_back(std::move(ring));
        }
        return out;
    }

    int winding_order(contour const& ring)
    {
        // The lexicographically least vertex is a strict extreme turn of
        // a simple noded ring. The original directions suffice here; no
        // area sum involving all roots is required. In erosion both
        // directions are reversed, which leaves their cross product alone.
        envelope::local f(m_geometry);
        auto const a = f.segment(ring.back().edge), b = f.segment(ring.front().edge);
        return f.f.sign(rn::cross(f.f, rn::subtract(f.f, a.to, a.from), rn::subtract(f.f, b.to, b.from)));
    }

    bool inside(contour const& ring, knot const& p, bool reversed)
    {
        bool result = false;
        for (id i = 0; i < ring.size(); ++i)
        {
            auto const& a = ring[i];
            auto const& b = ring[(i + 1) % ring.size()];
            bool const high_a = compare_axis(a.at, p, true) > 0, high_b = compare_axis(b.at, p, true) > 0;
            if (high_a == high_b) { continue; }
            int const side = m_geometry.side(a.edge, p) * (reversed ? -1 : 1);
            if (side == 0) { throw uncertified{}; }
            if ((side > 0) == high_b) { result = !result; }
        }
        return result;
    }

    using containment = std::vector<std::vector<bool>>;

    static bool same_native_topology(std::vector<rational_arr::ring> const& rings, containment const& contains)
    {
        for (id i = 0; i < rings.size(); ++i)
        {
            auto const& a = rings[i];
            for (id j = 0; j < a.edges.size(); ++j)
            {
                for (id k = j + 1; k < a.edges.size(); ++k)
                {
                    rational_arr::intersection hit;
                    if (!rational_arr::intersect(a.edges[j], a.edges[k], hit)) { continue; }
                    bool const adjacent = k == j + 1 || (j == 0 && k + 1 == a.edges.size());
                    if (!adjacent || hit.t0 != hit.t1) { return false; }
                }
            }
            for (id j = 0; j < i; ++j)
            {
                auto const& b = rings[j];
                for (auto const& u : a.edges)
                {
                    for (auto const& v : b.edges)
                    {
                        rational_arr::intersection hit;
                        if (rational_arr::intersect(u, v, hit)) { return false; }
                    }
                }
                if ((rational_arr::locate(a, b.points.front()) == 1) != contains[i][j]
                 || (rational_arr::locate(b, a.points.front()) == 1) != contains[j][i]) { return false; }
            }
        }
        return true;
    }

    template <typename Coordinate>
    bool certify_native(std::vector<contour> const& ideal, rational_xy const& anchor,
                        exact_number const& displacement, bool reversed, native_rings& stored)
    {
        static_assert(std::is_integral<Coordinate>::value || std::is_same<Coordinate, float>::value
                   || std::is_same<Coordinate, double>::value, "Chord arrangement export requires native coordinates");
        id const n = ideal.size();
        containment contains(n, std::vector<bool>(n));
        std::vector<bool> positive;
        for (id i = 0; i < n; ++i)
        {
            int const turn = winding_order(ideal[i]);
            if (turn == 0) { return false; }
            positive.push_back(turn > 0);
            for (id j = 0; j < n; ++j)
            {
                if (i != j) { contains[i][j] = inside(ideal[i], ideal[j].front().at, reversed); }
            }
        }

        // Disjoint cycles are either nested or disjoint. Find the immediate
        // container; orientation must alternate along this forest.
        for (id i = 0; i < n; ++i)
        {
            id parent = n;
            for (id j = 0; j < n; ++j)
            {
                if (contains[j][i] && (parent == n || contains[parent][j])) { parent = j; }
            }
            if (parent == n ? !positive[i] : positive[i] == positive[parent]) { return false; }
            stored.parent.push_back(parent);
        }

        for (id i = 0; i < n; ++i)
        {
            rational_line::piece vertices;
            for (auto const& m : ideal[i])
            {
                Coordinate x, y;
                if (!m_geometry.native_euclidean(m.at, anchor, displacement, x, y)) { return false; }
                rational_xy const p{common::rational_value(x), common::rational_value(y)};
                if (!vertices.empty() && capsule::same_point(vertices.back(), p)) { return false; }
                vertices.push_back(p);
            }
            if (capsule::same_point(vertices.back(), vertices.front())) { return false; }
            stored.rings.emplace_back(std::move(vertices));
            auto const& area = stored.rings.back().area2;
            if (area == 0 || (area > 0) != positive[i]) { return false; }
        }
        return same_native_topology(stored.rings, contains);
    }

    template <typename Polygon>
    static bool write_component(native_rings const& stored, id shell, Polygon& output)
    {
        bool constexpr cw = geometry::point_order<Polygon>::value == clockwise;
        bool constexpr closed = closure<Polygon>::value == geometry::closed;
        if (!rational_line::write_ring_lossless(stored.rings[shell].points, exterior_ring(output), cw, closed))
        { return false; }
        auto& holes = interior_rings(output);
        range::resize(holes, static_cast<id>(std::count(stored.parent.begin(), stored.parent.end(), shell)));
        auto h = boost::begin(holes);
        for (id i = 0; i < stored.rings.size(); ++i)
        {
            if (stored.parent[i] != shell) { continue; }
            if (h == boost::end(holes) || !rational_line::write_ring_lossless(stored.rings[i].points, *h, cw, closed))
            { return false; }
            ++h;
        }
        return h == boost::end(holes);
    }

    envelope m_geometry;
    std::vector<piece_range> m_pieces;
    std::vector<piece_range> m_source;
    std::vector<std::pair<id, id>> m_source_components;
    std::optional<id> m_rational_root;
    std::vector<edge_cuts> m_edges;
};

}} // namespace detail::certified_star_buffer
#endif // DOXYGEN_NO_DETAIL

/*!
\brief Bounded opt-in native round buffer of a 2D Cartesian Linestring.
\details No common interior witness is required. Nodes the exact inscribed
    chord capsules before conversion. Requires one simple shell with disjoint
    (non-touching) holes, and certifies the same ring topology and corresponding
    boundary displacement at most max_displacement on the native grid.
    The vertex bound is Euclidean, checked against the exact algebraic contour.
    A zero displacement requires lossless export; nonzero displacement does not
    promise nearest-rounded vertices.
    Requires 0 < sagitta < radius and 0 <= max_displacement < radius-sagitta.
    The stored region contains the radius-(r-sagitta-max_displacement)
    neighborhood and is contained in the radius-(r+max_displacement)
    neighborhood. Limits apply. This does not certify smooth-buffer topology.
\return success, invalid_input, or cannot_certify. Failure leaves output unchanged.
*/
template <typename Linestring, typename Polygon, typename Radius, typename Error, typename Displacement>
[[nodiscard]] inline certified_buffer_status buffer_linestring_arrangement_certified(
    Linestring const& input, Polygon& output, Radius const& radius,
    Error const& sagitta, Displacement const& max_displacement)
{
    namespace impl = detail::certified_star_buffer;
    using status = certified_buffer_status;
    impl::capsule::check_types<linestring_tag, Linestring, Polygon>();
    auto it = boost::begin(input);
    auto const end = boost::end(input);
    if (it == end) { return status::invalid_input; }

    impl::rational_xy anchor, previous;
    impl::exact_number r, e, displacement;
    try
    {
        anchor = impl::common::read_point(*it++);
        r = impl::common::rational_value(radius);
        e = impl::common::rational_value(sagitta);
        displacement = impl::common::rational_value(max_displacement);
        if (r <= 0 || e < 0 || e >= r || displacement < 0 || displacement >= r - e)
        { return status::invalid_input; }
        if (e == 0) { return status::cannot_certify; }
    }
    catch (std::domain_error const&) { return status::invalid_input; }

    impl::algebraic_chord_union arrangement;
    for (; it != end; ++it)
    {
        impl::rational_xy current;
        try { current = impl::difference(impl::common::read_point(*it), anchor); }
        catch (std::domain_error const&) { return status::invalid_input; }
        if (impl::capsule::same_point(previous, current)) { continue; }
        try { arrangement.append(impl::capsule::rational_segment(previous, current), r, e); }
        catch (impl::uncertified const&) { return status::cannot_certify; }
        previous = std::move(current);
    }
    try { return arrangement.build(anchor, displacement, output) ? status::success : status::cannot_certify; }
    catch (impl::uncertified const&) { return status::cannot_certify; }
    catch (std::overflow_error const&) { return status::cannot_certify; }
    catch (std::domain_error const&) { return status::cannot_certify; }
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_LINESTRING_ARRANGEMENT_HPP
