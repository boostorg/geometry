// Boost.Geometry
// Copyright (c) 2026
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_LINESTRING_STAR_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_LINESTRING_STAR_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <boost/geometry/algorithms/detail/buffer/certified/segment.hpp>
#include <boost/geometry/strategies/cartesian/exact/radical_buffer_noding.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace certified_star_buffer
{

namespace capsule = certified_segment_buffer;
namespace common = certified_point_buffer;
namespace ec = common::ec;
namespace rb = common::rb;
namespace rn = rb::radical_noding;
using common::exact_number;
using common::rational_xy;
using common::difference;
using common::cross;
using status = certified_buffer_status;
using id = std::size_t;

struct uncertified {};
constexpr id max_pieces = 128;
constexpr id max_vertices = 65536;
constexpr id max_marks = 16384;
constexpr id max_fallbacks = 32768;

struct box_point { ec::interval x, y; };

inline box_point operator-(box_point const& a, box_point const& b)
{ return {a.x - b.x, a.y - b.y}; }

inline ec::interval det(box_point const& a, box_point const& b)
{ return a.x * b.y - a.y * b.x; }

struct root_bounds
{
    explicit root_bounds(exact_number const& d) : square(d)
    {
        using boost::multiprecision::cpp_int;
        cpp_int const den = boost::multiprecision::denominator(d);
        cpp_int const scaled = (boost::multiprecision::numerator(d) * den) << 256;
        cpp_int remainder;
        cpp_int const floor = boost::multiprecision::sqrt(scaled, remainder);
        exact_number const divisor(den << 128);
        lo = exact_number(floor) / divisor;
        hi = remainder == 0 ? lo : exact_number(floor + 1) / divisor;
        mid = (lo + hi) / 2;
    }

    ec::interval coordinate(exact_number const& base, exact_number const& coefficient) const
    {
        using rounder = ec::point_detail::output_rounder<double>;
        using namespace ec::arithmetic_detail;
        exact_number const a = base + coefficient * lo, b = base + coefficient * hi;
        if (a == 0 && b == 0) { return {}; }
        double const x = rounder::apply(a), y = rounder::apply(b);
        return enclose(x, downward((std::min)(x, y)), upward((std::max)(x, y)));
    }

    exact_number square, lo, hi, mid;
};

struct producer_vertex
{
    // Relative to the common witness: base + coefficient * sqrt(square).
    rational_xy base, coefficient;
    id root;
    box_point bounds;
};

struct producer_edge { id from, to; };
enum class knot_kind { vertex, east, crossing };

struct knot
{
    knot_kind kind;
    id a, b;
    box_point bounds;
    int y_sign;
    bool second_half;
};

struct mark { knot at; id edge; };
using contour = std::vector<mark>;

class envelope
{
    friend class algebraic_chord_union;

public:
    contour capsule_contour(capsule::rational_segment const& source, exact_number const& radius,
                            exact_number const& sagitta)
    {
        auto const edges = append_capsule(source, radius, sagitta, max_marks - 1);
        contour out;
        for (id e = edges.first; e < edges.second; ++e)
        {
            id const i = m_edges[e].from;
            out.push_back({make_knot(knot_kind::vertex, i, 0, m_vertices[i].bounds), e});
        }
        auto least = std::min_element(out.begin(), out.end(), [&](mark const& a, mark const& b)
        { return angle(a.at, b.at) < 0; });
        std::rotate(out.begin(), least, out.end());
        if (out.front().at.y_sign != 0 || out.front().at.second_half)
        {
            // No vertex at +x. The final producer crosses the sweep seam.
            id const e = out.back().edge;
            auto const& a = m_vertices[m_edges[e].from].bounds;
            auto const d0 = m_vertices[m_edges[e].to].bounds - a;
            out.insert(out.begin(), {{knot_kind::east, e, 0, {a.x - a.y * d0.x / d0.y, {}}, 0, false}, e});
        }
        return out;
    }

private:
    // Construct only primitive algebraic vertices and directed edges here.
    // Angular events are specific to the common-witness sweep.
    std::pair<id, id> append_capsule(capsule::rational_segment const& source, exact_number const& radius,
                                    exact_number const& sagitta, id vertex_limit)
    {
        if (m_roots.size() == max_pieces) { throw uncertified{}; }
        id const root = m_roots.size(), first = m_vertices.size();
        m_roots.emplace_back(source.norm2);
        rational_xy const d = source.direction;
        for (unsigned q = 0; q < 4; ++q)
        {
            auto const& center = q < 2 ? source.to : source.from;
            auto emit = [&](rational_xy const& u)
            {
                if (m_vertices.size() == max_vertices || m_vertices.size() - first == vertex_limit)
                { return false; }
                rational_xy const c{radius * (u.x * d.y + u.y * d.x) / source.norm2,
                                    radius * (-u.x * d.x + u.y * d.y) / source.norm2};
                auto const& r = m_roots[root];
                m_vertices.push_back({center, c, root, {r.coordinate(center.x, c.x), r.coordinate(center.y, c.y)}});
                return true;
            };
            rb::circle_direction const a(rb::rational_arc_sample(q, 0)), b(rb::rational_arc_sample(q, 1));
            if (!common::subdivide(q, 0, a, 1, b, radius, sagitta, 0, emit)
                || (q % 2 == 1 && !emit(b.numerator())))
            {
                throw uncertified{};
            }
        }

        id const first_edge = m_edges.size();
        for (id i = first; i < m_vertices.size(); ++i)
        {
            m_edges.push_back({i, i + 1 < m_vertices.size() ? i + 1 : first});
        }
        return {first_edge, m_edges.size()};
    }

public:
    contour merge(contour const& a, contour const& b)
    {
        contour out;
        id i = 0, j = 0;
        knot const* ka = &a.front().at;
        knot const* kb = &b.front().at;
        int start = -side(b.front().edge, *ka);
        auto emit = [&](id e, knot const* p)
        {
            if (!out.empty() && out.back().edge == e) { return; }
            if (p == nullptr || out.size() == max_marks) { throw uncertified{}; }
            out.push_back({*p, e});
        };
        for (;;)
        {
            id const ea = a[i].edge, eb = b[j].edge;
            bool const end_a = i + 1 == a.size(), end_b = j + 1 == b.size();
            int const order = end_a ? (end_b ? 0 : 1)
                                    : end_b ? -1 : angle(a[i + 1].at, b[j + 1].at);
            knot const* next_a = order <= 0 ? &a[end_a ? 0 : i + 1].at : nullptr;
            knot const* next_b = order >= 0 ? &b[end_b ? 0 : j + 1].at : nullptr;
            int const finish = next_a != nullptr ? -side(eb, *next_a) : side(ea, *next_b);
            // With two active producers, radial order has the sign of a
            // linear functional of the ray. It can change once only.
            int const winner = start != 0 ? start : finish != 0 ? finish : ea < eb ? 1 : -1;
            knot const* at = winner > 0 ? ka : kb;
            if (at == nullptr && start == 0) { at = ka != nullptr ? ka : kb; }
            emit(winner > 0 ? ea : eb, at);
            if (start * finish < 0)
            {
                auto const p = crossing(ea, eb);
                emit(finish > 0 ? ea : eb, &p);
            }
            if (end_a && end_b) { break; }
            if (next_a != nullptr) { ++i; }
            if (next_b != nullptr) { ++j; }
            ka = next_a; kb = next_b; start = finish;
        }
        return out;
    }

    template <typename Coordinate>
    bool native(knot const& p, rational_xy const& witness, exact_number const& axis_budget,
                Coordinate& x, Coordinate& y) const
    {
        if (!propose_native(p, witness, x, y)) { return false; }
        local f(*this);
        auto const exact = f.evaluate(p);
        return f.within(exact.x, common::rational_value(x) - witness.x, axis_budget)
            && f.within(exact.y, common::rational_value(y) - witness.y, axis_budget);
    }

    template <typename Coordinate>
    bool native_euclidean(knot const& p, rational_xy const& anchor, exact_number const& budget,
                          Coordinate& x, Coordinate& y) const
    {
        if (!propose_native(p, anchor, x, y)) { return false; }
        local f(*this);
        auto const exact = f.evaluate(p);
        auto const x_error = f.f.subtract(exact.x.n,
            f.f.scale(exact.x.d, common::rational_value(x) - anchor.x));
        auto const y_error = f.f.subtract(exact.y.n,
            f.f.scale(exact.y.d, common::rational_value(y) - anchor.y));
        // Both coordinates of a vertex, or of a node evaluated by rn::at,
        // have the same denominator. Compare squared distances in the field.
        auto const limit = f.f.scale(exact.x.d, budget);
        auto const length2 = f.f.add(f.f.multiply(x_error, x_error),
                                     f.f.multiply(y_error, y_error));
        return f.f.sign(f.f.subtract(length2, f.f.multiply(limit, limit))) <= 0;
    }

private:
    template <typename Coordinate>
    bool propose_native(knot const& p, rational_xy const& witness,
                        Coordinate& x, Coordinate& y) const
    {
        using rounder = ec::point_detail::output_rounder<Coordinate>;
        // Rational evaluations here only propose storage values. They never
        // decide a turn, cut or incidence, nor do they become producer edges.
        auto const guess = estimate(p);
        x = rounder::apply(witness.x + guess.x);
        y = rounder::apply(witness.y + guess.y);
        if constexpr (std::is_floating_point<Coordinate>::value)
        {
            if (!std::isfinite(x) || !std::isfinite(y)) { return false; }
        }
        return true;
    }

    struct local
    {
        explicit local(envelope const& owner) : owner(owner) {}

        rn::point vertex(id i)
        {
            auto const& p = owner.m_vertices[i];
            if (p.coefficient.x == 0 && p.coefficient.y == 0) { return rn::lift(f, p.base); }
            rn::value root;
            if (!f.try_sqrt(owner.m_roots[p.root].square, root)) { throw uncertified{}; }
            return {f.add(f.rational(p.base.x), f.scale(root, p.coefficient.x)),
                    f.add(f.rational(p.base.y), f.scale(root, p.coefficient.y))};
        }

        rn::segment segment(id i)
        {
            auto const& e = owner.m_edges[i];
            return {vertex(e.from), vertex(e.to)};
        }

        rn::node evaluate(knot const& p)
        {
            if (p.kind == knot_kind::vertex)
            {
                auto const v = vertex(p.a);
                auto const one = f.rational(1);
                return {{v.x, one}, {v.y, one}};
            }
            auto const s = segment(p.a);
            if (p.kind == knot_kind::east)
            {
                auto const t = rn::ratio(f, f.scale(s.from.y, -1), f.subtract(s.to.y, s.from.y));
                return rn::at(f, s, t);
            }
            auto const i = rn::intersect(f, s, segment(p.b));
            if (i.kind != rn::intersection_kind::contact) { throw uncertified{}; }
            return rn::at(f, s, i.t0);
        }

        bool within(rn::parameter const& q, exact_number const& stored, exact_number const& budget) const
        {
            auto const lo = f.scale(q.d, stored - budget), hi = f.scale(q.d, stored + budget);
            return f.sign(f.subtract(q.n, lo)) >= 0 && f.sign(f.subtract(q.n, hi)) <= 0;
        }

        envelope const& owner;
        rn::field f;
    };

    void fallback()
    { if (++m_fallbacks > max_fallbacks) { throw uncertified{}; } }

    knot make_knot(knot_kind kind, id a, id b, box_point const& bounds)
    {
        knot p{kind, a, b, bounds, 0, false};
        auto const y = ec::sign(bounds.y);
        if (y.certain && y.value != 0)
        {
            p.y_sign = y.value;
            p.second_half = y.value < 0;
        }
        else
        {
            fallback();
            local f(*this);
            auto const exact = f.evaluate(p);
            p.y_sign = f.f.sign(exact.y.n);
            p.second_half = p.y_sign < 0 || (p.y_sign == 0 && f.f.sign(exact.x.n) < 0);
        }
        return p;
    }

    int angle(knot const& a, knot const& b)
    {
        if (a.second_half != b.second_half) { return a.second_half ? 1 : -1; }
        if (a.kind == b.kind && a.a == b.a && a.b == b.b) { return 0; }
        auto const sign = ec::sign(det(a.bounds, b.bounds));
        if (sign.certain) { return -sign.value; }
        fallback();
        local f(*this);
        auto const p = f.evaluate(a), q = f.evaluate(b);
        return -f.f.sign(rn::cross(f.f, {p.x.n, p.y.n}, {q.x.n, q.y.n}));
    }

    int side(id i, knot const& p)
    {
        auto const& e = m_edges[i];
        auto const& a = m_vertices[e.from].bounds;
        auto const& b = m_vertices[e.to].bounds;
        auto const sign = ec::sign(det(b - a, p.bounds - a));
        if (sign.certain) { return sign.value; }
        fallback();
        local f(*this);
        auto const s = f.segment(i);
        auto const q = f.evaluate(p);
        rn::point const offset{f.f.subtract(q.x.n, f.f.multiply(s.from.x, q.x.d)),
                              f.f.subtract(q.y.n, f.f.multiply(s.from.y, q.y.d))};
        return f.f.sign(rn::cross(f.f, rn::subtract(f.f, s.to, s.from), offset));
    }

    box_point crossing_bounds(id e, id f) const
    {
        auto const& a = m_vertices[m_edges[e].from].bounds;
        auto const& b = m_vertices[m_edges[f].from].bounds;
        auto const v = m_vertices[m_edges[e].to].bounds - a;
        auto const w = m_vertices[m_edges[f].to].bounds - b;
        auto const t = det(b - a, w) / det(v, w);
        return {a.x + t * v.x, a.y + t * v.y};
    }

    knot crossing(id e, id f)
    {
        return make_knot(knot_kind::crossing, e, f, crossing_bounds(e, f));
    }

    rational_xy estimate_vertex(id i) const
    {
        auto const& p = m_vertices[i];
        auto const& r = m_roots[p.root].mid;
        return {p.base.x + r * p.coefficient.x, p.base.y + r * p.coefficient.y};
    }

    rational_xy estimate(knot const& p) const
    {
        if (p.kind == knot_kind::vertex) { return estimate_vertex(p.a); }
        auto const a = estimate_vertex(m_edges[p.a].from);
        auto const v = difference(estimate_vertex(m_edges[p.a].to), a);
        exact_number t;
        if (p.kind == knot_kind::east)
        {
            if (v.y == 0) { throw uncertified{}; }
            t = -a.y / v.y;
        }
        else
        {
            auto const b = estimate_vertex(m_edges[p.b].from);
            auto const w = difference(estimate_vertex(m_edges[p.b].to), b);
            exact_number const d = cross(v, w);
            if (d == 0) { throw uncertified{}; }
            t = cross(difference(b, a), w) / d;
        }
        return {a.x + t * v.x, a.y + t * v.y};
    }

    id m_fallbacks = 0;
    std::vector<root_bounds> m_roots;
    std::vector<producer_vertex> m_vertices;
    std::vector<producer_edge> m_edges;
};

template <typename Polygon>
inline bool export_contour(envelope const& env, contour const& ring, rational_xy const& witness,
                           exact_number const& displacement, Polygon& output)
{
    using point = point_type_t<Polygon>;
    using coordinate = coordinate_type_t<point>;
    static_assert(std::is_integral<coordinate>::value || std::is_same<coordinate, float>::value
               || std::is_same<coordinate, double>::value, "Star buffer export requires native coordinates");
    Polygon candidate;
    auto& exterior = exterior_ring(candidate);
    bool constexpr cw = geometry::point_order<Polygon>::value == clockwise;
    exact_number const axis_budget = displacement / 2;
    std::vector<rational_xy> proposed;
    proposed.reserve(ring.size());
    for (id i = 0; i < ring.size(); ++i)
    {
        coordinate x, y;
        if (!env.native(ring[cw ? ring.size() - 1 - i : i].at, witness, axis_budget, x, y)) { return false; }
        point p;
        geometry::set<0>(p, x);
        geometry::set<1>(p, y);
        range::push_back(exterior, std::move(p));
        proposed.push_back({common::rational_value(x) - witness.x, common::rational_value(y) - witness.y});
    }
    if constexpr (closure<Polygon>::value == closed)
    {
        if (boost::begin(exterior) == boost::end(exterior)) { return false; }
        range::push_back(exterior, point(*boost::begin(exterior)));
    }
    // The star test reads the actual stored points in output order.
    auto expected = proposed.begin();
    auto const check_vertex = [&](rational_xy const& p)
    { return expected != proposed.end() && capsule::same_point(*expected++, p); };
    if (!common::certify_ring(candidate, witness, ring.size(), check_vertex,
                              [](auto const&, auto const&) { return true; }))
    { return false; }
    output = std::move(candidate);
    return true;
}

}} // namespace detail::certified_star_buffer
#endif // DOXYGEN_NO_DETAIL

/*!
\brief Opt-in native export of a support-exact star-shaped linestring buffer.
\details The witness must be strictly inside every radius-(r-sagitta) source
    capsule. The ideal result is the union of exact inscribed chord capsules;
    only the final contour is converted to native output. The stored polygon
    must make one simple turn about the witness, with corresponding ideal
    and stored vertices (and edges by interpolation) at Euclidean distance
    at most max_displacement. This is not the lossless buffer contract.
\return success, invalid_input, or cannot_certify. Failure leaves output
    unchanged. Piece, contour and local algebraic fallback limits apply.
*/
template <typename Linestring, typename Point, typename Polygon, typename Radius, typename Error, typename Displacement>
[[nodiscard]] inline certified_buffer_status buffer_linestring_star_certified(
    Linestring const& input, Point const& witness, Polygon& output, Radius const& radius,
    Error const& sagitta, Displacement const& max_displacement)
{
    namespace impl = detail::certified_star_buffer;
    using status = certified_buffer_status;
    impl::capsule::check_types<linestring_tag, Linestring, Polygon>();
    static_assert(std::is_same<tag_t<Point>, point_tag>::value && dimension<Point>::value == 2
               && std::is_same<cs_tag_t<Point>, cartesian_tag>::value, "Witness must be a 2D Cartesian Point");
    concepts::check<Point const>();

    auto it = boost::begin(input);
    auto const end = boost::end(input);
    if (it == end) { return status::invalid_input; }
    impl::rational_xy center;
    impl::exact_number r, e, displacement;
    std::vector<impl::capsule::rational_segment> pieces;
    try
    {
        center = impl::common::read_point(witness);
        r = impl::common::rational_value(radius);
        e = impl::common::rational_value(sagitta);
        displacement = impl::common::rational_value(max_displacement);
        if (r <= 0 || e < 0 || e >= r || displacement < 0) { return status::invalid_input; }
        if (e == 0) { return status::cannot_certify; }
        auto previous = impl::difference(impl::common::read_point(*it++), center);
        impl::exact_number const inner = r - e, inner2 = inner * inner;
        for (; it != end; ++it)
        {
            auto current = impl::difference(impl::common::read_point(*it), center);
            if (impl::capsule::same_point(previous, current)) { continue; }
            if (pieces.size() == impl::max_pieces) { return status::cannot_certify; }
            pieces.emplace_back(previous, current);
            if (pieces.back().squared_distance({0, 0}) >= inner2) { return status::cannot_certify; }
            previous = std::move(current);
        }
    }
    catch (std::domain_error const&)
    {
        return status::invalid_input;
    }
    if (pieces.empty()) { return status::cannot_certify; }

    try
    {
        impl::envelope env;
        std::vector<impl::contour> layer;
        for (auto const& p : pieces) { layer.push_back(env.capsule_contour(p, r, e)); }
        while (layer.size() > 1)
        {
            std::vector<impl::contour> next;
            for (impl::id i = 0; i < layer.size(); i += 2)
            {
                next.push_back(i + 1 == layer.size() ? std::move(layer[i]) : env.merge(layer[i], layer[i + 1]));
            }
            layer = std::move(next);
        }
        return impl::export_contour(env, layer.front(), center, displacement, output)
            ? status::success : status::cannot_certify;
    }
    catch (impl::uncertified const&) { return status::cannot_certify; }
    catch (std::overflow_error const&) { return status::cannot_certify; }
    catch (std::domain_error const&) { return status::cannot_certify; }
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_LINESTRING_STAR_HPP
