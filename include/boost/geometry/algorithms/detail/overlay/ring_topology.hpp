// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_RING_TOPOLOGY_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_RING_TOPOLOGY_HPP

#include <algorithm>
#include <cstddef>
#include <vector>
#include <boost/geometry/strategies/detail/ring_topology.hpp>

namespace boost { namespace geometry { namespace detail { namespace overlay
{ namespace ring_topology
{

// Cartesian straight-segment topology. The kernel supplies coordinate order,
// point equality, orientation and, for containment, an exact midpoint.
struct edge
{
    std::size_t first, second;
    std::ptrdiff_t flow;
};

enum class cycle_certificate : unsigned char { none, check, certified };

template <typename Point>
struct boundary
{
    using point_type = Point;
    std::vector<point_type> points;
    std::vector<edge> edges;
    bool closed = true;
    // Cache the candidate's result lazily: most pairs never touch.
    mutable cycle_certificate cycle = cycle_certificate::none;
    int order = 1;
    std::size_t min_x = 0, max_x = 0, min_y = 0, max_y = 0;
};

template <typename Point, typename Predicates>
void find_extrema(boundary<Point>& result, Predicates const& predicates)
{
    for (std::size_t i = 1; i < result.points.size(); ++i)
    {
        if (predicates.template compare<0>(result.points[i], result.points[result.min_x]) < 0) result.min_x = i;
        if (predicates.template compare<0>(result.points[i], result.points[result.max_x]) > 0) result.max_x = i;
        if (predicates.template compare<1>(result.points[i], result.points[result.min_y]) < 0) result.min_y = i;
        if (predicates.template compare<1>(result.points[i], result.points[result.max_y]) > 0) result.max_y = i;
    }
}

template <typename A, typename B, typename P, typename Predicates>
inline bool on_segment(A const& a, B const& b, P const& point, Predicates const& predicates)
{
    return predicates.side(a, b, point) == 0
        && predicates.template compare<0>(a, point)
            * predicates.template compare<0>(b, point) <= 0
        && predicates.template compare<1>(a, point)
            * predicates.template compare<1>(b, point) <= 0;
}

template <bool Closed, typename Iterator, typename Read, typename Predicates>
bool analyze_extreme(Iterator first, Iterator end, Iterator& minimum, int order,
                     strategies::detail::ring_topology_result& result,
                     Read const& read, Predicates const& predicates)
{
    if (first == end) { result = {}; return true; }
    auto tail = first, before_tail = first;
    std::size_t count = 0;
    for (auto it = first; it != end; ++it) { before_tail = tail; tail = it; ++count; }
    auto equal = [&](auto const& a, auto const& b)
    {
        return predicates.template compare<0>(a, b) == 0
            && predicates.template compare<1>(a, b) == 0;
    };
    auto logical_end = end, logical_last = tail;
    if (count > 1 && equal(read(first), read(tail)))
    {
        logical_end = tail;
        logical_last = before_tail;
        --count;
    }
    else if (Closed) return false;
    if (count < 3) return false;
    minimum = first;
    auto previous = logical_last, minimum_previous = logical_last;
    std::size_t occurrences = 0;
    for (auto it = first; it != logical_end; ++it)
    {
        int comparison = predicates.template compare<0>(read(it), read(minimum));
        if (comparison == 0) comparison = predicates.template compare<1>(read(it), read(minimum));
        if (comparison < 0) { minimum = it; minimum_previous = previous; occurrences = 1; }
        else if (comparison == 0) ++occurrences;
        previous = it;
    }
    if (occurrences != 1) return false;
    auto next = minimum;
    if (++next == logical_end) next = first;
    int const turn = predicates.side(read(minimum_previous), read(minimum), read(next));
    if (turn == 0) return false;
    bool const clockwise = turn < 0;
    result = {true, clockwise, (clockwise ? 1 : -1)
        * order};
    return true;
}

template <typename Point, typename Predicates>
inline bool certify_simple_cycle(boundary<Point> const& input, Predicates const& predicates)
{
    if (input.cycle != cycle_certificate::check)
        return input.cycle == cycle_certificate::certified;
    input.cycle = cycle_certificate::none;
    auto const n = input.points.size();
    if (!input.closed || n < 3 || input.edges.size() != n) return false;

    if (n == 3)
    {
        // append(ring) supplies one cyclic edge per vertex. Three
        // noncollinear points give three distinct, nonoverlapping segments
        // and no vertex in an edge interior, regardless of winding.
        if (predicates.side(input.points[0], input.points[1], input.points[2]) == 0)
            return false;
        input.cycle = cycle_certificate::certified;
        return true;
    }

    auto const extrema = std::minmax_element(input.points.begin(), input.points.end(),
        [&](auto const& a, auto const& b) { return predicates.less(a, b); });
    auto const first = static_cast<std::size_t>(extrema.first - input.points.begin());
    auto const last = static_cast<std::size_t>(extrema.second - input.points.begin());
    auto chain = [&](bool forward, int& side)
    {
        auto i = first;
        while (i != last)
        {
            auto const next = forward ? (i + 1 == n ? 0 : i + 1) : (i == 0 ? n - 1 : i - 1);
            if (!predicates.less(input.points[i], input.points[next])) return false;
            if (next != last)
            {
                int const s = predicates.side(*extrema.first, *extrema.second, input.points[next]);
                if (s == 0 || (side != 0 && s != side)) return false;
                side = s;
            }
            i = next;
        }
        return true;
    };
    int forward_side = 0, backward_side = 0;
    if (!chain(true, forward_side) || !chain(false, backward_side)
        || forward_side == backward_side) return false;
    // Along either strictly ordered chain, a nonadjacent segment cannot share
    // a lexicographic interval. The chains are on opposite open sides of the
    // endpoint chord (one may be the chord), so meet only at the endpoints.
    // Both point location and residual ordering retain the input edge flow:
    // the no-cancellation certificate is independent of winding.
    input.cycle = cycle_certificate::certified;
    return true;
}

template <bool Simple, typename Point, typename Predicates>
inline boundary<Point> residual_impl(boundary<Point> const& input, Predicates const& predicates)
{
    boundary<Point> result;
    result.closed = input.closed;
    result.order = input.order;
    std::vector<std::size_t> order(input.points.size()), mapping(input.points.size());
    for (std::size_t i = 0; i != order.size(); ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b)
        { return predicates.less(input.points[a], input.points[b]); });
    for (auto index : order)
    {
        if (Simple || result.points.empty() || !predicates.equal(result.points.back(), input.points[index]))
            result.points.push_back(input.points[index]);
        mapping[index] = result.points.size() - 1;
    }
    std::vector<std::size_t> cuts;
    for (auto const& original : input.edges)
    {
        auto const a = mapping[original.first], b = mapping[original.second];
        if (a == b) continue;
        if constexpr (Simple)
        {
            // No other vertex lies on this edge and no other edge overlaps
            // it. Preserve the original residual's lexicographic edge order.
            auto const sign = a < b ? original.flow : -original.flow;
            result.edges.push_back({(std::min)(a, b), (std::max)(a, b), sign});
        }
        else
        {
            cuts.clear();
            for (std::size_t i = 0; i != result.points.size(); ++i)
                if (on_segment(result.points[a], result.points[b], result.points[i], predicates)) cuts.push_back(i);
            // Lexicographic vertex indices run monotonically along every segment.
            auto const sign = a < b ? original.flow : -original.flow;
            for (std::size_t i = 1; i < cuts.size(); ++i)
                result.edges.push_back({cuts[i - 1], cuts[i], sign});
        }
    }
    std::sort(result.edges.begin(), result.edges.end(), [](edge const& a, edge const& b)
    {
        return a.first < b.first || (a.first == b.first && a.second < b.second);
    });
    if constexpr (!Simple)
    {
        std::size_t write = 0;
        for (std::size_t i = 0; i < result.edges.size();)
        {
            auto current = result.edges[i++];
            while (i < result.edges.size() && result.edges[i].first == current.first
                && result.edges[i].second == current.second) current.flow += result.edges[i++].flow;
            if (current.flow != 0) result.edges[write++] = current;
        }
        result.edges.resize(write);
    }
    find_extrema(result, predicates);
    return result;
}

template <typename Point, typename Predicates>
inline boundary<Point> residual(boundary<Point> const& input, Predicates const& predicates)
{
    return certify_simple_cycle(input, predicates)
        ? residual_impl<true>(input, predicates) : residual_impl<false>(input, predicates);
}

template <typename Point, typename Predicates>
inline strategies::detail::ring_topology_result analyze(boundary<Point> const& input,
    Point& representative, Predicates const& predicates)
{
    using result_type = strategies::detail::ring_topology_result;
    if (input.points.empty() || input.edges.empty() || !input.closed) return {};
    auto const minimum = static_cast<std::size_t>(std::min_element(input.points.begin(), input.points.end(),
        [&](auto const& a, auto const& b) { return predicates.less(a, b); }) - input.points.begin());
    representative = input.points[minimum];
    struct ray { std::size_t neighbor; std::ptrdiff_t flow; };
    std::vector<ray> rays;
    for (auto const& edge : input.edges)
    {
        if (predicates.equal(input.points[edge.first], representative)) rays.push_back({edge.second, edge.flow});
        if (predicates.equal(input.points[edge.second], representative)) rays.push_back({edge.first, -edge.flow});
    }
    std::sort(rays.begin(), rays.end(), [&](ray const& a, ray const& b)
    {
        return predicates.side(representative, input.points[a.neighbor], input.points[b.neighbor]) > 0;
    });
    // All rays lie in the right half-plane at the lexicographic minimum.
    // Reduce each geometric ray, not just adjacent edges of the input walk:
    // spikes and repeated reverse traversals must not choose its orientation.
    for (std::size_t i = 0; i < rays.size();)
    {
        auto const neighbor = rays[i].neighbor;
        std::ptrdiff_t flow = 0;
        do { flow += rays[i++].flow; }
        while (i < rays.size()
            && predicates.side(representative, input.points[neighbor], input.points[rays[i].neighbor]) == 0);
        if (flow != 0)
        {
            bool const clockwise = flow < 0;
            return result_type{true, clockwise, (clockwise ? 1 : -1) * input.order};
        }
    }
    // The extreme may belong solely to a canceled excursion. Splitting at
    // every existing vertex handles reverse edges with unequal subdivisions.
    auto const reduced = residual(input, predicates);
    if (reduced.edges.empty()) return {};
    auto const minimum_vertex = reduced.edges.front().first;
    auto chosen = reduced.edges.front();
    representative = reduced.points[minimum_vertex];
    for (auto const& edge : reduced.edges)
    {
        if (edge.first != minimum_vertex) break;
        if (predicates.side(representative, reduced.points[edge.second], reduced.points[chosen.second]) > 0)
            chosen = edge;
    }
    bool const clockwise = chosen.flow < 0;
    return result_type{true, clockwise, (clockwise ? 1 : -1) * input.order};
}

struct scan_all_edges
{
    template <typename Query, typename Point, typename Visit>
    bool operator()(Query const&, boundary<Point> const& ring, Visit const& visit) const
    {
        return std::any_of(ring.edges.begin(), ring.edges.end(), visit);
    }
};

template <typename Point, typename Query, typename Predicates, typename EdgeScan = scan_all_edges>
inline int locate_raw(Query const& point, boundary<Point> const& ring,
                      Predicates const& predicates, EdgeScan const& scan = {})
{
    if (ring.points.empty() || predicates.template compare<0>(point, ring.points[ring.min_x]) < 0
        || predicates.template compare<0>(point, ring.points[ring.max_x]) > 0
        || predicates.template compare<1>(point, ring.points[ring.min_y]) < 0
        || predicates.template compare<1>(point, ring.points[ring.max_y]) > 0) return -1;
    std::ptrdiff_t winding = 0;
    auto const visit = [&](edge const& edge)
    {
        auto const& a = ring.points[edge.first];
        auto const& b = ring.points[edge.second];
        int const ay = predicates.template compare<1>(a, point);
        int const by = predicates.template compare<1>(b, point);
        if (ay * by > 0) return false;
        int const side = predicates.side(a, b, point);
        if (side == 0 && on_segment(a, b, point, predicates)) return true;
        if (ay <= 0 && by > 0 && side > 0) winding += edge.flow;
        else if (ay > 0 && by <= 0 && side < 0) winding -= edge.flow;
        return false;
    };
    return scan(point, ring, visit) ? 0 : winding == 0 ? -1 : 1;
}

template <typename Point, typename Query, typename Predicates, typename EdgeScan = scan_all_edges>
inline int locate(Query const& point, boundary<Point> const& ring,
                  Predicates const& predicates, EdgeScan const& scan = {})
{
    int const result = locate_raw(point, ring, predicates, scan);
    if (result != 0 || certify_simple_cycle(ring, predicates)) return result;
    // A point on a canceled excursion is not on the surviving boundary.
    return locate_raw(point, residual_impl<false>(ring, predicates), predicates);
}

template <typename InnerPoint, typename OuterPoint, typename Query, typename Predicates,
          typename EdgeScan = scan_all_edges>
inline int classify(boundary<InnerPoint> const& inner, boundary<OuterPoint> const& outer,
                    Query const& representative, Predicates const& predicates, EdgeScan const& scan = {})
{
    int const first = locate(representative, outer, predicates, scan);
    if (first != 0) return first;
    auto const reduced = residual(inner, predicates);
    for (auto const& edge : reduced.edges)
    {
        auto const& a = reduced.points[edge.first];
        auto const& b = reduced.points[edge.second];
        int const vertex = locate(a, outer, predicates, scan);
        if (vertex != 0) return vertex;
        int const middle = locate(predicates.midpoint(a, b), outer, predicates, scan);
        if (middle != 0) return middle;
    }
    return 0;
}

}}}}} // namespace boost::geometry::detail::overlay::ring_topology

#endif
