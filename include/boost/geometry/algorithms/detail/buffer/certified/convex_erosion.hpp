// Boost.Geometry
// Copyright (c) 2026
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_CONVEX_EROSION_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_CONVEX_EROSION_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <boost/range/value_type.hpp>
#include <boost/geometry/algorithms/detail/buffer/certified/point.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/strategies/cartesian/exact/bounded_radical_field.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace certified_convex_erosion
{

namespace common = certified_point_buffer;
namespace ec = common::ec;
using common::cross;
using common::dot;
using common::exact_number;
using common::rational_xy;
using status = certified_buffer_status;
using field = common::rb::bounded_radical_field;
using id = std::size_t;

constexpr id max_vertices = 512;
struct uncertified {};

using common::bounded;

inline rational_xy subtract(rational_xy const& a, rational_xy const& b)
{ return {a.x - b.x, a.y - b.y}; }

inline bool same(rational_xy const& a, rational_xy const& b)
{ return a.x == b.x && a.y == b.y; }

// The directions must make exactly one strict CCW sweep. For each edge the
// following edge directions are first to its left, then to its right. Their
// cumulative transverse displacement starts and ends at zero and is strictly
// positive in between. Thus every edge strictly supports the other vertices;
// local left turns alone would also admit a star with multiple sweeps.
inline bool strict_convex(std::vector<rational_xy> const& points)
{
    if (points.size() < 3) { return false; }
    unsigned wraps = 0;
    auto direction = subtract(points.front(), points.back());
    for (id i = 0; i < points.size(); ++i)
    {
        auto next = subtract(points[(i + 1) % points.size()], points[i]);
        if (!common::advance_ccw(direction, next, wraps)) { return false; }
        direction = std::move(next);
    }
    return wraps == 1;
}

// Co-directed subdivisions contribute no new supporting halfspace.
// Reject reversals and zero edges, not just clockwise bends.
inline bool convex_corners(std::vector<rational_xy> const& points,
                           std::vector<rational_xy>& corners)
{
    if (points.size() < 3) { return false; }
    for (id i = 0; i < points.size(); ++i)
    {
        auto const before = subtract(points[i], points[i == 0 ? points.size()-1 : i-1]);
        auto const after = subtract(points[(i+1) % points.size()], points[i]);
        auto const turn = cross(before, after);
        if (turn < 0 || (turn == 0 && dot(before, after) <= 0)) { return false; }
        if (turn > 0) { corners.push_back(points[i]); }
    }
    return strict_convex(corners);
}

template <bool KeepSubdivisions = false, typename Polygon>
status read_source(Polygon const& input, std::vector<rational_xy>& corners)
{
    auto const& holes = interior_rings(input);
    if (boost::begin(holes) != boost::end(holes)) { return status::invalid_input; }
    bool constexpr closed_input = closure<Polygon>::value == closed;
    std::vector<rational_xy> points;
    id count = 0;
    for (auto const& p : exterior_ring(input))
    {
        if (++count > max_vertices + (closed_input ? 1u : 0u)) { return status::cannot_certify; }
        auto q = common::read_point(p);
        if (!bounded(q.x) || !bounded(q.y)) { return status::cannot_certify; }
        if (points.empty() || !same(q, points.back())) { points.push_back(std::move(q)); }
    }
    if (points.size() < 3 || same(points.front(), points.back()) != closed_input)
    { return status::invalid_input; }
    if constexpr (closed_input) { points.pop_back(); }
    if constexpr (geometry::point_order<Polygon>::value == clockwise) { std::reverse(points.begin(), points.end()); }

    if (!convex_corners(points, corners)) { return status::invalid_input; }
    if constexpr (KeepSubdivisions) { corners.swap(points); }
    return status::success;
}

struct enclosure
{
    exact_number low, high;

    void add_scaled(enclosure const& root, exact_number const& scale)
    {
        low += scale * (scale < 0 ? root.high : root.low);
        high += scale * (scale < 0 ? root.low : root.high);
    }
};

inline enclosure sqrt_enclosure(exact_number const& q)
{
    using namespace boost::multiprecision;
    constexpr unsigned bits = 128;
    cpp_int const den = denominator(q);
    cpp_int const scaled = (numerator(q) * den) << (2 * bits);
    cpp_int remainder;
    cpp_int const lower = sqrt(scaled, remainder);
    cpp_int const divisor = den << bits;
    exact_number const low = exact_number(lower) / divisor;
    return {low, remainder == 0 ? low : exact_number(lower + 1) / divisor};
}

inline ec::interval binary_enclosure(enclosure const& bound)
{
    using rounder = ec::point_detail::output_rounder<double>;
    using namespace ec::arithmetic_detail;
    double const a = rounder::apply(bound.low), b = rounder::apply(bound.high);
    if (a == b && std::isfinite(a) && bound.low == bound.high && ec::exact_value(a) == bound.low)
    { return {a, 0}; }
    return enclose(a, downward(a), upward(b));
}

inline ec::interval binary_enclosure(exact_number const& q)
{ return binary_enclosure(enclosure{q, q}); }

struct side
{
    rational_xy normal;
    exact_number base, length2;
    enclosure root;
    ec::interval nx, ny, original, shifted;

    side(rational_xy const& a, rational_xy const& b, exact_number const& radius)
        : normal{a.y - b.y, b.x - a.x}, base(dot(normal, a)), length2(dot(normal, normal))
        , root(sqrt_enclosure(length2)), nx(binary_enclosure(normal.x)), ny(binary_enclosure(normal.y))
        , original(binary_enclosure(base))
        , shifted(binary_enclosure(enclosure{base + radius * root.low, base + radius * root.high}))
    {}
};

// IDs below side_count are original supports; the rest are shifted supports.
// The incoming edge uses 'first' and the outgoing edge uses 'second'.
struct vertex
{
    id first, second;
    exact_number determinant;
    ec::interval x, y;
};

struct affine_point
{
    rational_xy base, first, second;
};

struct term
{
    exact_number coefficient;
    side const* support;
};

inline field::value radical(field& f, term const& t)
{
    field::value root;
    if (t.coefficient == 0) { return root; }
    if (!f.try_sqrt(t.support->length2, root)) { throw uncertified{}; }
    return f.scale(root, t.coefficient);
}

inline int sign(exact_number const& q, std::array<term, 3> const& terms)
{
    enclosure bracket{q, q};
    for (auto const& t : terms) { bracket.add_scaled(t.support->root, t.coefficient); }
    if (bracket.low > 0) { return 1; }
    if (bracket.high < 0) { return -1; }
    if (bracket.low == 0 && bracket.high == 0) { return 0; }
    field f;
    auto value = f.rational(q);
    for (auto const& t : terms) { value = f.add(value, radical(f, t)); }
    return f.sign(value);
}

class clipping
{
public:
    clipping(std::vector<rational_xy> const& corners, exact_number const& distance)
        : origin(corners.front()), radius(distance)
    {
        sides.reserve(corners.size());
        for (id i = 0; i < corners.size(); ++i)
        {
            sides.emplace_back(subtract(corners[i], origin),
                subtract(corners[(i+1) % corners.size()], origin), radius);
        }
    }

    std::vector<vertex> run() const
    {
        id const n = sides.size();
        std::vector<vertex> polygon, next;
        std::vector<int> signs;
        polygon.reserve(2*n);
        next.reserve(2*n);
        for (id i = 0; i < n; ++i) { polygon.push_back(intersection(i == 0 ? n-1 : i-1, i)); }
        for (id k = n; k < 2*n; ++k)
        {
            signs.clear();
            bool positive = false, negative = false;
            for (auto const& v : polygon)
            {
                int const s = halfspace_sign(v, k);
                signs.push_back(s);
                positive |= s > 0;
                negative |= s < 0;
            }
            // A full-dimensional convex polygon has an open intersection
            // with this halfspace iff some vertex is strictly inside it.
            // The empty set, a lone vertex and a supporting segment all
            // regularize to empty, and further cuts cannot increase dimension.
            if (!positive) { return {}; }
            if (!negative) { continue; }

            next.clear();
            for (id j = 0, i = polygon.size()-1; j < polygon.size(); i = j++)
            {
                auto const& v = polygon[j];
                if (signs[j] == 0)
                {
                    // Replace an incident support at an existing on-line
                    // vertex instead of inserting a zero-length edge.
                    if (signs[i] < 0) { next.push_back(intersection(k, v.second)); }
                    else if (signs[(j+1) % polygon.size()] < 0) { next.push_back(intersection(v.first, k)); }
                    else { next.push_back(v); }
                }
                else if (signs[j] > 0)
                {
                    if (signs[i] < 0) { next.push_back(intersection(k, v.first)); }
                    next.push_back(v);
                }
                else if (signs[i] > 0)
                {
                    next.push_back(intersection(v.first, k));
                }
            }
            if (next.size() < 3 || next.size() > 2*n) { throw uncertified{}; }
            for (id i = 0; i < next.size(); ++i)
            {
                if (next[i].second != next[(i+1) % next.size()].first) { throw uncertified{}; }
            }
            polygon.swap(next);
        }
        for (auto const& v : polygon)
        {
            // Each shifted halfspace strictly dominates its original.
            if (v.first < n || v.second < n) { throw uncertified{}; }
        }
        return polygon;
    }

    rational_xy origin;

    affine_point coordinates(vertex const& v) const
    {
        auto const& a = support(v.first);
        auto const& b = support(v.second);
        auto const& det = v.determinant;
        exact_number const r = offset(v.first) / det, s = offset(v.second) / det;
        return {{(a.base * b.normal.y - b.base * a.normal.y) / det,
                 (a.normal.x * b.base - b.normal.x * a.base) / det},
                {r * b.normal.y, -r * b.normal.x}, {-s * a.normal.y, s * a.normal.x}};
    }

    enclosure bound(vertex const& v, exact_number const& base,
                    exact_number const& a, exact_number const& b) const
    {
        enclosure result{base, base};
        result.add_scaled(support(v.first).root, a);
        result.add_scaled(support(v.second).root, b);
        return result;
    }

    bool within(vertex const& v, affine_point const& c, rational_xy const& stored,
                exact_number const& budget2) const
    {
        auto const delta = subtract(c.base, stored);
        auto const squared = [](enclosure const& b) -> exact_number
        { return (std::max)(exact_number(b.low*b.low), exact_number(b.high*b.high)); };
        if (squared(bound(v, delta.x, c.first.x, c.second.x))
            + squared(bound(v, delta.y, c.first.y, c.second.y)) <= budget2) { return true; }
        field f;
        auto const& a = support(v.first);
        auto const& b = support(v.second);
        auto component = [&](exact_number const& q, exact_number const& x, exact_number const& y)
        { return f.add(f.add(f.rational(q), radical(f, {x, &a})), radical(f, {y, &b})); };
        auto const x = component(delta.x, c.first.x, c.second.x);
        auto const y = component(delta.y, c.first.y, c.second.y);
        return f.sign(f.subtract(f.add(f.multiply(x,x), f.multiply(y,y)), f.rational(budget2))) <= 0;
    }

private:
    side const& support(id i) const { return sides[i % sides.size()]; }
    exact_number offset(id i) const { return i < sides.size() ? exact_number(0) : radius; }
    ec::interval rhs(id i) const { return i < sides.size() ? support(i).original : support(i).shifted; }

    vertex intersection(id i, id j) const
    {
        auto const& a = support(i);
        auto const& b = support(j);
        auto det = cross(a.normal, b.normal);
        if (det == 0) { throw uncertified{}; }
        auto const d = binary_enclosure(det), u = rhs(i), v = rhs(j);
        return {i, j, std::move(det), (u*b.ny - v*a.ny)/d, (a.nx*v - b.nx*u)/d};
    }

    int halfspace_sign(vertex const& v, id k) const
    {
        if (k == v.first || k == v.second) { return 0; }
        auto const& t = support(k);
        auto const fast = ec::sign(t.nx*v.x + t.ny*v.y - rhs(k));
        if (fast.certain) { return fast.value; }
        auto const c = coordinates(v);
        // A Cramer vertex uses just the two support roots; the tested
        // halfspace contributes at most one more, even on a many-sided ring.
        return sign(dot(t.normal,c.base) - t.base, {{
            {dot(t.normal,c.first), &support(v.first)},
            {dot(t.normal,c.second), &support(v.second)}, {-radius, &t}}});
    }

    exact_number radius;
    std::vector<side> sides;
};

template <typename MultiPolygon>
bool export_polygon(clipping const& clip, std::vector<vertex> const& polygon,
                    exact_number const& displacement, MultiPolygon& output)
{
    MultiPolygon candidate;
    if (!polygon.empty())
    {
        using polygon_type = typename boost::range_value<MultiPolygon>::type;
        using point = point_type_t<polygon_type>;
        using coordinate = coordinate_type_t<point>;
        using rounder = common::coordinate_writer<coordinate>;
        bool constexpr cw = geometry::point_order<polygon_type>::value == clockwise;
        bool constexpr closed_output = closure<polygon_type>::value == closed;
        range::resize(candidate, 1);
        auto const component = boost::begin(candidate);
        if (component == boost::end(candidate) || std::next(component) != boost::end(candidate)) { return false; }
        auto const& holes = interior_rings(*component);
        if (boost::begin(holes) != boost::end(holes)) { return false; }
        auto& ring = exterior_ring(*component);
        auto vertex_at = [&](id i) -> vertex const& { return polygon[cw ? polygon.size()-1-i : i]; };
        auto proposal = [](enclosure const& e, exact_number const& origin)
        { return rounder::apply(origin + (e.low + e.high)/2); };
        for (id i = 0; i < polygon.size(); ++i)
        {
            auto const& v = vertex_at(i);
            auto const c = clip.coordinates(v);
            point p;
            geometry::set<0>(p, proposal(clip.bound(v,c.base.x,c.first.x,c.second.x), clip.origin.x));
            geometry::set<1>(p, proposal(clip.bound(v,c.base.y,c.first.y,c.second.y), clip.origin.y));
            range::push_back(ring, std::move(p));
        }
        if constexpr (closed_output)
        {
            if (boost::begin(ring) == boost::end(ring)) { return false; }
            range::push_back(ring, point(*boost::begin(ring)));
        }

        // Read back the stored output. A nearby set of rounded vertices is
        // not by itself a topology certificate (e.g. a collapsed erosion).
        exact_number const budget2 = displacement * displacement;
        std::vector<rational_xy> stored;
        auto it = boost::begin(ring);
        for (id i = 0; i < polygon.size(); ++i, ++it)
        {
            if (it == boost::end(ring)) { return false; }
            auto p = subtract(common::read_point(*it), clip.origin);
            auto const& v = vertex_at(i);
            if (!clip.within(v, clip.coordinates(v), p, budget2)) { return false; }
            stored.push_back(std::move(p));
        }
        if constexpr (closed_output)
        {
            if (it == boost::end(ring) || !same(subtract(common::read_point(*it),clip.origin),stored.front()))
            { return false; }
            ++it;
        }
        if constexpr (cw) { std::reverse(stored.begin(), stored.end()); }
        if (it != boost::end(ring) || !strict_convex(stored)) { return false; }
    }
    else if (boost::begin(candidate) != boost::end(candidate)) { return false; }
    output = std::move(candidate);
    return true;
}

}} // namespace detail::certified_convex_erosion
#endif // DOXYGEN_NO_DETAIL

/*!
\brief Opt-in regularized erosion of a convex, hole-free Cartesian Polygon.
\details The ideal polygon uses exact straight Euclidean offsets. Empty,
    point and segment intersections all export as an empty MultiPolygon.
    Nonempty export is a strictly convex cycle in the declared output order
    and closure. Its matching vertices are within Euclidean max_displacement
    of the ideal vertices, hence so are the interpolated boundary and filled
    polygon in Hausdorff distance. This is not an inward-rounding guarantee.
    A zero displacement budget requires lossless coordinates.
\return success, invalid_input, or cannot_certify. Failure leaves output
    unchanged. The radius must be positive. Limits apply to input size,
    scalar capacity and export; the default buffer is not changed.
*/
template <typename Polygon, typename MultiPolygon, typename Radius, typename Displacement>
[[nodiscard]] inline certified_buffer_status erode_convex_polygon_certified(
    Polygon const& input, MultiPolygon& output, Radius const& radius, Displacement const& max_displacement)
{
    namespace impl = detail::certified_convex_erosion;
    using status = certified_buffer_status;
    static_assert(std::is_same<tag_t<Polygon>, polygon_tag>::value
               && std::is_same<tag_t<MultiPolygon>, multi_polygon_tag>::value,
                  "Certified convex erosion requires a Polygon and a MultiPolygon");
    static_assert(dimension<Polygon>::value == 2 && dimension<MultiPolygon>::value == 2
               && std::is_same<cs_tag_t<Polygon>, cartesian_tag>::value
               && std::is_same<cs_tag_t<MultiPolygon>, cartesian_tag>::value,
                  "Certified convex erosion requires 2D Cartesian geometries");
    concepts::check<Polygon const>();
    concepts::check<MultiPolygon>();

    impl::exact_number r, d;
    std::vector<impl::rational_xy> corners;
    try
    {
        r = impl::common::rational_value(radius);
        d = impl::common::rational_value(max_displacement);
        if (r <= 0 || d < 0) { return status::invalid_input; }
        if (!impl::bounded(r) || !impl::bounded(d)) { return status::cannot_certify; }
        auto const read = impl::read_source(input, corners);
        if (read != status::success) { return read; }
    }
    catch (std::domain_error const&) { return status::invalid_input; }

    try
    {
        impl::clipping clip(corners, r);
        auto const result = clip.run();
        return impl::export_polygon(clip, result, d, output) ? status::success : status::cannot_certify;
    }
    catch (impl::uncertified const&) { return status::cannot_certify; }
    catch (std::overflow_error const&) { return status::cannot_certify; }
    catch (std::domain_error const&) { return status::cannot_certify; }
}

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_CONVEX_EROSION_HPP
