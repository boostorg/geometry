// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_STATIC_RING_TOPOLOGY_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_STATIC_RING_TOPOLOGY_HPP

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/iterator.hpp>
#include <boost/geometry/core/closure.hpp>
#include <boost/geometry/core/exterior_ring.hpp>
#include <boost/geometry/core/interior_rings.hpp>
#include <boost/geometry/core/point_order.hpp>
#include <boost/geometry/algorithms/detail/overlay/overlay_type.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_ring.hpp>
#include <boost/geometry/strategies/cartesian/exact/static_box.hpp>
#include <boost/geometry/strategies/cartesian/exact/static_predicates.hpp>
#include <boost/geometry/strategies/cartesian/exact/borrowed_query.hpp>
#include <boost/geometry/strategies/cartesian/exact/iterator_point_view.hpp>
#include <boost/geometry/strategies/detail/ring_properties.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

// The stock metadata still fixes both samples to owning exact points. Borrowed
// metadata must select a source-aware point type, not infer one from Coordinate.
template <typename Coordinate>
using ring_metadata = strategies::detail::topological_ring_tag<exact_point<Coordinate>>;

namespace ring_topology_detail
{

struct edge
{
    std::size_t first, second;
    std::ptrdiff_t flow;
};

enum class cycle_certificate : unsigned char { none, clockwise, counterclockwise, triangle, certified };

template <typename Coordinate, typename Point = exact_point<Coordinate>>
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

template <typename A, typename B>
inline bool less(A const& a, B const& b)
{
    int const x = static_compare_coordinate<0>(a, b);
    return x < 0 || (x == 0 && static_compare_coordinate<1>(a, b) < 0);
}

template <typename Coordinate, typename A, typename B, typename P>
inline bool on_segment(A const& a, B const& b, P const& point)
{
    return static_orientation<Coordinate>(a, b, point) == 0
        && static_compare_coordinate<0, Coordinate>(a, point)
            * static_compare_coordinate<0, Coordinate>(b, point) <= 0
        && static_compare_coordinate<1, Coordinate>(a, point)
            * static_compare_coordinate<1, Coordinate>(b, point) <= 0;
}

template <typename Coordinate, typename Point, typename Iterator>
inline Point from_iterator(Iterator const& iterator)
{
    if constexpr (!std::is_void<typename static_predicates_detail::root_of<Point>::type>::value)
        return Point::from_iterator(iterator);
    else if constexpr (std::is_same<Point, iterator_point_view<Iterator>>::value)
    {
        static_assert(lossless_coordinate_conversion<geometry::coordinate_type_t<
            typename std::iterator_traits<Iterator>::value_type>, Coordinate>::value,
            "Borrowed ring inputs require lossless coordinate promotion");
        return Point(iterator);
    }
    else if constexpr (std::is_same<Point, typename std::iterator_traits<Iterator>::value_type>::value)
        return iterator_point_view<Iterator>(iterator).read();
    else
        return make_point<Coordinate>(iterator_point_view<Iterator>(iterator).read());
}

template <typename Coordinate, typename Geometry, typename Tag = geometry::tag_t<Geometry>>
struct boundary_point_type { using type = exact_point<Coordinate>; };

template <typename Coordinate, typename Box>
struct boundary_point_type<Coordinate, Box, box_tag>
{
    static_assert(std::is_void<typename static_predicates_detail::root_of<Box>::type>::value,
                  "Root coordinate-origin bounds are metadata, not geometry boundaries");
    using type = std::conditional_t<fixed_native_supported<geometry::coordinate_type_t<Box>>::value,
        borrowed_detail::indexed_vertex<Box>, exact_point<Coordinate>>;
};

template <typename Coordinate, typename Point, typename Box>
inline Point box_vertex(Box const& box, std::size_t index)
{
    static_assert(std::is_void<typename static_predicates_detail::root_of<Box>::type>::value,
                  "Root coordinate-origin bounds cannot supply geometry corners");
    if constexpr (fixed_native_supported<geometry::coordinate_type_t<Box>>::value)
    {
        borrowed_detail::indexed_vertex<Box> const source{&box, index};
        if constexpr (std::is_same<Point, std::decay_t<decltype(source)>>::value) return source;
        else if constexpr (!std::is_void<typename static_predicates_detail::root_of<Point>::type>::value)
            return Point::borrow(source);
        else return make_point<Coordinate>(source);
    }
    else return make_point<Coordinate>(index == 0 ? static_box_detail::corner<min_corner>(box)
                                                  : static_box_detail::corner<max_corner>(box));
}

template <typename Coordinate, typename Ring>
struct boundary_point_type<Coordinate, Ring, ring_tag>
{
    using iterator = typename boost::range_iterator<Ring const>::type;
    using input = typename std::iterator_traits<iterator>::value_type;
    using type = std::conditional_t<fixed_native_supported<geometry::coordinate_type_t<input>>::value,
        iterator_point_view<iterator>,
        std::conditional_t<static_predicates_detail::exact_input<std::decay_t<input>>::value,
            input, exact_point<Coordinate>>>;
};

template <typename Coordinate, typename Polygon>
struct boundary_point_type<Coordinate, Polygon, polygon_tag>
{
    using outer = decltype(geometry::exterior_ring(std::declval<Polygon const&>()));
    using holes = decltype(geometry::interior_rings(std::declval<Polygon const&>()));
    using inner = decltype(*boost::begin(std::declval<holes&>()));
    using outer_iterator = typename boost::range_iterator<std::remove_reference_t<outer> const>::type;
    using inner_iterator = typename boost::range_iterator<std::remove_reference_t<inner> const>::type;
    // Do not retain iterators into an adapted accessor's temporary owning ring.
    using type = std::conditional_t<std::is_lvalue_reference<outer>::value
        && std::is_lvalue_reference<holes>::value && std::is_lvalue_reference<inner>::value
        && std::is_same<outer_iterator, inner_iterator>::value,
        typename boundary_point_type<Coordinate, std::remove_reference_t<outer>, ring_tag>::type,
        exact_point<Coordinate>>;
};

template <typename Coordinate, typename Point, typename Ring>
inline void append(boundary<Coordinate, Point>& result, Ring const& ring, ring_tag)
{
    auto const begin = result.points.size();
    for (auto it = boost::begin(ring); it != boost::end(ring); ++it)
    {
        auto point = from_iterator<Coordinate, Point>(it);
        if (result.points.size() == begin || !static_equal_points<Coordinate>(result.points.back(), point))
            result.points.push_back(std::move(point));
    }
    auto end = result.points.size();
    if (end - begin < 2) return;
    if (static_equal_points<Coordinate>(result.points[begin], result.points[end - 1]))
    {
        result.points.pop_back();
        --end;
    }
    else if (geometry::closure<Ring>::value == geometry::closed)
    {
        result.closed = false;
        return;
    }
    if (end - begin < 2) return;
    for (auto i = begin; i != end; ++i)
        result.edges.push_back({i, i + 1 == end ? begin : i + 1, 1});
}

template <typename Coordinate, typename Point, typename Polygon>
inline void append(boundary<Coordinate, Point>& result, Polygon const& polygon, polygon_tag)
{
    append(result, geometry::exterior_ring(polygon), ring_tag());
    for (auto const& ring : geometry::interior_rings(polygon)) append(result, ring, ring_tag());
}

template <typename Coordinate, typename Point, typename Box>
inline void append(boundary<Coordinate, Point>& result, Box const& box, box_tag)
{
    if constexpr (fixed_native_supported<geometry::coordinate_type_t<Box>>::value)
    {
        auto const low = box_vertex<Coordinate, Point>(box, 0);
        auto const high = box_vertex<Coordinate, Point>(box, 2);
        if (static_compare_coordinate<0, Coordinate>(low, high) >= 0
            || static_compare_coordinate<1, Coordinate>(low, high) >= 0) return;
        auto const offset = result.points.size();
        for (std::size_t i = 0; i != 4; ++i) result.points.push_back(box_vertex<Coordinate, Point>(box, i));
        for (std::size_t i = 0; i != 4; ++i)
            result.edges.push_back({offset + i, offset + (i + 1) % 4, 1});
    }
    else
    {
    auto const low = make_point<Coordinate>(static_box_detail::corner<min_corner>(box));
    auto const high = make_point<Coordinate>(static_box_detail::corner<max_corner>(box));
    if (compare_x(low, high) >= 0 || compare_y(low, high) >= 0) return;
    auto const offset = result.points.size();
    result.points.push_back(low);
    result.points.push_back(exact_point<Coordinate>::splice_coordinates(low, 0, high, 1));
    result.points.push_back(high);
    result.points.push_back(exact_point<Coordinate>::splice_coordinates(high, 0, low, 1));
    for (std::size_t i = 0; i != 4; ++i)
        result.edges.push_back({offset + i, offset + (i + 1) % 4, 1});
    }
}

template <typename Coordinate, typename Point, typename Geometry>
inline boundary<Coordinate, Point> make_boundary_as(Geometry const& geometry)
{
    boundary<Coordinate, Point> result;
    result.order = geometry::point_order<Geometry>::value == geometry::clockwise ? 1 : -1;
    append(result, geometry, geometry::tag_t<Geometry>());
    for (std::size_t i = 1; i < result.points.size(); ++i)
    {
        if (static_compare_coordinate<0, Coordinate>(result.points[i], result.points[result.min_x]) < 0) result.min_x = i;
        if (static_compare_coordinate<0, Coordinate>(result.points[i], result.points[result.max_x]) > 0) result.max_x = i;
        if (static_compare_coordinate<1, Coordinate>(result.points[i], result.points[result.min_y]) < 0) result.min_y = i;
        if (static_compare_coordinate<1, Coordinate>(result.points[i], result.points[result.max_y]) > 0) result.max_y = i;
    }
    return result;
}

template <typename Coordinate, typename Geometry>
inline auto make_boundary(Geometry const& geometry)
{
    return make_boundary_as<Coordinate, typename boundary_point_type<Coordinate, Geometry>::type>(geometry);
}

template <typename Coordinate, typename Geometry, typename Point, typename Tag>
inline bool fast_analyze(Geometry const&, Point&,
                         strategies::detail::ring_topology_result&, Tag) { return false; }

template <typename Coordinate, typename Ring, typename Point>
inline bool fast_analyze(Ring const& ring, Point& representative,
                         strategies::detail::ring_topology_result& result, ring_tag)
{
    auto first = boost::begin(ring), end = boost::end(ring);
    if (first == end) { result = {}; return true; }
    auto read = [](auto& iterator) -> decltype(auto)
    {
        using iterator_type = std::decay_t<decltype(iterator)>;
        using point_type = typename std::iterator_traits<iterator_type>::value_type;
        if constexpr (fixed_native_supported<geometry::coordinate_type_t<point_type>>::value)
            return iterator_point_view<iterator_type>(iterator);
        else
            return *iterator;
    };
    auto tail = first, before_tail = first;
    std::size_t count = 0;
    for (auto it = first; it != end; ++it) { before_tail = tail; tail = it; ++count; }
    auto equal = [](auto const& a, auto const& b)
    {
        return static_compare_coordinate<0, Coordinate>(a, b) == 0
            && static_compare_coordinate<1, Coordinate>(a, b) == 0;
    };
    auto logical_end = end, logical_last = tail;
    if (count > 1 && equal(read(first), read(tail)))
    {
        logical_end = tail;
        logical_last = before_tail;
        --count;
    }
    else if (geometry::closure<Ring>::value == geometry::closed) return false;
    if (count < 3) return false;
    auto minimum = first, previous = logical_last, minimum_previous = logical_last;
    std::size_t occurrences = 0;
    for (auto it = first; it != logical_end; ++it)
    {
        int comparison = static_compare_coordinate<0, Coordinate>(read(it), read(minimum));
        if (comparison == 0) comparison = static_compare_coordinate<1, Coordinate>(read(it), read(minimum));
        if (comparison < 0) { minimum = it; minimum_previous = previous; occurrences = 1; }
        else if (comparison == 0) ++occurrences;
        previous = it;
    }
    if (occurrences != 1) return false;
    auto next = minimum;
    if (++next == logical_end) next = first;
    int const turn = static_orientation<Coordinate>(read(minimum_previous), read(minimum), read(next));
    if (turn == 0) return false;
    representative = from_iterator<Coordinate, Point>(minimum);
    bool const clockwise = turn < 0;
    result = {true, clockwise, (clockwise ? 1 : -1)
        * (geometry::point_order<Ring>::value == geometry::clockwise ? 1 : -1)};
    return true;
}

template <typename Coordinate, typename Polygon, typename Point>
inline bool fast_analyze(Polygon const& polygon, Point& representative,
                         strategies::detail::ring_topology_result& result, polygon_tag)
{
    auto const& holes = geometry::interior_rings(polygon);
    if (boost::begin(holes) != boost::end(holes)) return false;
    if (!fast_analyze<Coordinate>(geometry::exterior_ring(polygon), representative, result, ring_tag())) return false;
    if (result.valid)
        result.direction = (result.clockwise ? 1 : -1)
            * (geometry::point_order<Polygon>::value == geometry::clockwise ? 1 : -1);
    return true;
}

template <typename Coordinate, typename Box, typename Point>
inline bool fast_analyze(Box const& box, Point& representative,
                         strategies::detail::ring_topology_result& result, box_tag)
{
    auto const low = box_vertex<Coordinate, Point>(box, 0);
    auto const high = box_vertex<Coordinate, Point>(box, 2);
    if (static_compare_coordinate<0, Coordinate>(low, high) >= 0
        || static_compare_coordinate<1, Coordinate>(low, high) >= 0) result = {};
    else
    {
        representative = low;
        result = {true, true, geometry::point_order<Box>::value == geometry::clockwise ? 1 : -1};
    }
    return true;
}

template <typename Coordinate, typename Point>
inline bool certify_simple_cycle(boundary<Coordinate, Point> const& input)
{
    auto const expected = input.cycle;
    if (expected == cycle_certificate::certified) return true;
    if (expected == cycle_certificate::none) return false;
    input.cycle = cycle_certificate::none;
    auto const n = input.points.size();
    if (!input.closed || n < 3 || input.edges.size() != n) return false;

    if (expected == cycle_certificate::triangle)
    {
        // append(ring) supplies one cyclic edge per vertex. Three
        // noncollinear points give three distinct, nonoverlapping segments
        // and no vertex in an edge interior, regardless of winding.
        if (n != 3 || static_orientation<Coordinate>(input.points[0], input.points[1], input.points[2]) == 0)
            return false;
        input.cycle = cycle_certificate::certified;
        return true;
    }

    auto const extrema = std::minmax_element(input.points.begin(), input.points.end(),
        [](auto const& a, auto const& b) { return less(a, b); });
    auto const first = static_cast<std::size_t>(extrema.first - input.points.begin());
    auto const last = static_cast<std::size_t>(extrema.second - input.points.begin());
    auto chain = [&](bool forward, int& side)
    {
        auto i = first;
        while (i != last)
        {
            auto const next = forward ? (i + 1 == n ? 0 : i + 1) : (i == 0 ? n - 1 : i - 1);
            if (!less(input.points[i], input.points[next])) return false;
            if (next != last)
            {
                int const s = static_orientation<Coordinate>(*extrema.first, *extrema.second, input.points[next]);
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
    bool const clockwise = forward_side != 0 ? forward_side > 0 : backward_side < 0;
    if (clockwise != (expected == cycle_certificate::clockwise)) return false;
    input.cycle = cycle_certificate::certified;
    return true;
}

template <bool Simple, typename Coordinate, typename Point>
inline boundary<Coordinate, Point> residual_impl(boundary<Coordinate, Point> const& input)
{
    boundary<Coordinate, Point> result;
    result.closed = input.closed;
    result.order = input.order;
    std::vector<std::size_t> order(input.points.size()), mapping(input.points.size());
    for (std::size_t i = 0; i != order.size(); ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](std::size_t a, std::size_t b)
        { return less(input.points[a], input.points[b]); });
    for (auto index : order)
    {
        if (Simple || result.points.empty() || !static_equal_points<Coordinate>(result.points.back(), input.points[index]))
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
                if (on_segment<Coordinate>(result.points[a], result.points[b], result.points[i])) cuts.push_back(i);
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
    for (std::size_t i = 1; i < result.points.size(); ++i)
    {
        if (static_compare_coordinate<0, Coordinate>(result.points[i], result.points[result.min_x]) < 0) result.min_x = i;
        if (static_compare_coordinate<0, Coordinate>(result.points[i], result.points[result.max_x]) > 0) result.max_x = i;
        if (static_compare_coordinate<1, Coordinate>(result.points[i], result.points[result.min_y]) < 0) result.min_y = i;
        if (static_compare_coordinate<1, Coordinate>(result.points[i], result.points[result.max_y]) > 0) result.max_y = i;
    }
    return result;
}

template <typename Coordinate, typename Point>
inline boundary<Coordinate, Point> residual(boundary<Coordinate, Point> const& input)
{
    return certify_simple_cycle(input) ? residual_impl<true>(input) : residual_impl<false>(input);
}

template <typename Coordinate, typename Point>
inline strategies::detail::ring_topology_result analyze(boundary<Coordinate, Point> const& input,
                                                        Point& representative)
{
    using result_type = strategies::detail::ring_topology_result;
    if (input.points.empty() || input.edges.empty() || !input.closed) return {};
    auto const minimum = static_cast<std::size_t>(std::min_element(input.points.begin(), input.points.end(),
        [](auto const& a, auto const& b) { return less(a, b); }) - input.points.begin());
    representative = input.points[minimum];
    struct ray { std::size_t neighbor; std::ptrdiff_t flow; };
    std::vector<ray> rays;
    for (auto const& edge : input.edges)
    {
        if (static_equal_points<Coordinate>(input.points[edge.first], representative)) rays.push_back({edge.second, edge.flow});
        if (static_equal_points<Coordinate>(input.points[edge.second], representative)) rays.push_back({edge.first, -edge.flow});
    }
    std::sort(rays.begin(), rays.end(), [&](ray const& a, ray const& b)
    {
        return static_orientation<Coordinate>(representative, input.points[a.neighbor], input.points[b.neighbor]) > 0;
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
            && static_orientation<Coordinate>(representative, input.points[neighbor], input.points[rays[i].neighbor]) == 0);
        if (flow != 0)
        {
            bool const clockwise = flow < 0;
            return result_type{true, clockwise, (clockwise ? 1 : -1) * input.order};
        }
    }
    // The extreme may belong solely to a canceled excursion. Splitting at
    // every existing vertex handles reverse edges with unequal subdivisions.
    auto const reduced = residual(input);
    if (reduced.edges.empty()) return {};
    auto const minimum_vertex = reduced.edges.front().first;
    auto chosen = reduced.edges.front();
    // A borrowed point copies its root iterator, never an address in reduced.
    representative = reduced.points[minimum_vertex];
    for (auto const& edge : reduced.edges)
    {
        if (edge.first != minimum_vertex) break;
        if (static_orientation<Coordinate>(representative, reduced.points[edge.second], reduced.points[chosen.second]) > 0)
            chosen = edge;
    }
    bool const clockwise = chosen.flow < 0;
    return result_type{true, clockwise, (clockwise ? 1 : -1) * input.order};
}

template <typename Coordinate, typename Point, typename Query>
inline int locate_raw(Query const& point, boundary<Coordinate, Point> const& ring)
{
    if (ring.points.empty() || static_compare_coordinate<0, Coordinate>(point, ring.points[ring.min_x]) < 0
        || static_compare_coordinate<0, Coordinate>(point, ring.points[ring.max_x]) > 0
        || static_compare_coordinate<1, Coordinate>(point, ring.points[ring.min_y]) < 0
        || static_compare_coordinate<1, Coordinate>(point, ring.points[ring.max_y]) > 0) return -1;
    std::ptrdiff_t winding = 0;
    for (auto const& edge : ring.edges)
    {
        auto const& a = ring.points[edge.first];
        auto const& b = ring.points[edge.second];
        int const ay = static_compare_coordinate<1, Coordinate>(a, point);
        int const by = static_compare_coordinate<1, Coordinate>(b, point);
        if (ay * by > 0) continue;
        int const side = static_orientation<Coordinate>(a, b, point);
        if (side == 0 && on_segment<Coordinate>(a, b, point)) return 0;
        if (ay <= 0 && by > 0 && side > 0) winding += edge.flow;
        else if (ay > 0 && by <= 0 && side < 0) winding -= edge.flow;
    }
    return winding == 0 ? -1 : 1;
}

template <typename Coordinate, typename Point, typename Query>
inline int locate(Query const& point, boundary<Coordinate, Point> const& ring)
{
    int const result = locate_raw(point, ring);
    if (result != 0 || certify_simple_cycle(ring)) return result;
    // A point on a canceled excursion is not on the surviving boundary.
    return locate_raw(point, residual_impl<false>(ring));
}

template <typename Coordinate, typename A, typename B>
inline auto midpoint(A const& a, B const& b)
{
    using root = typename static_predicates_detail::common_root<A, B>::type;
    if constexpr (!std::is_void<root>::value) return query_point<root>::midpoint(a, b);
    else return static_predicates_detail::with_homogeneous(a, [&](auto const& first)
    {
        return static_predicates_detail::with_homogeneous(b, [&](auto const& second)
        {
            return exact_point<Coordinate>::from_homogeneous(fixed_midpoint(first, second));
        });
    });
}

template <typename Coordinate, typename InnerPoint, typename OuterPoint, typename Query>
inline int classify(boundary<Coordinate, InnerPoint> const& inner, boundary<Coordinate, OuterPoint> const& outer,
                    Query const& representative)
{
    int const first = locate(representative, outer);
    if (first != 0) return first;
    auto const reduced = residual(inner);
    for (auto const& edge : reduced.edges)
    {
        auto const& a = reduced.points[edge.first];
        auto const& b = reduced.points[edge.second];
        int const vertex = locate(a, outer);
        if (vertex != 0) return vertex;
        int const middle = locate(midpoint<Coordinate>(a, b), outer);
        if (middle != 0) return middle;
    }
    return 0;
}

template <typename Coordinate, typename Tag, typename Geometry>
inline auto selected_source(ring_identifier const& id, Geometry const& geometry,
                            bool allow_certificate = false)
{
    decltype(auto) ring = geometry::detail::overlay::get_ring<Tag>::apply(id, geometry);
    if constexpr (std::is_lvalue_reference<decltype(ring)>::value)
    {
        using ring_type = std::remove_reference_t<decltype(ring)>;
        auto boundary = make_boundary<Coordinate>(ring);
        if constexpr (std::is_same<geometry::tag_t<ring_type>, ring_tag>::value
            && fixed_native_supported<geometry::coordinate_type_t<ring_type>>::value)
        {
            if (allow_certificate)
            {
                int const direction = id.ring_index < 0 ? boundary.order : -boundary.order;
                boundary.cycle = direction > 0 ? cycle_certificate::clockwise : cycle_certificate::counterclockwise;
            }
        }
        return boundary;
    }
    else
        return make_boundary_as<Coordinate, exact_point<Coordinate>>(ring);
}

template <typename Coordinate, typename Geometry1, typename Geometry2, typename Collection>
using selected_boundary = std::variant<
    decltype(selected_source<Coordinate, geometry::tag_t<Geometry1>>(
        std::declval<ring_identifier const&>(), std::declval<Geometry1 const&>())),
    decltype(selected_source<Coordinate, geometry::tag_t<Geometry2>>(
        std::declval<ring_identifier const&>(), std::declval<Geometry2 const&>())),
    decltype(selected_source<Coordinate, void>(
        std::declval<ring_identifier const&>(), std::declval<Collection const&>()))>;

template <typename Coordinate, typename Geometry1, typename Geometry2, typename Collection>
inline selected_boundary<Coordinate, Geometry1, Geometry2, Collection>
selected(ring_identifier const& id, Geometry1 const& a, Geometry2 const& b, Collection const& collection,
         bool allow_certificate)
{
    using result = selected_boundary<Coordinate, Geometry1, Geometry2, Collection>;
    switch (id.source_index)
    {
    case 0: return result(std::in_place_index<0>, selected_source<Coordinate, geometry::tag_t<Geometry1>>(id, a, allow_certificate));
    case 1: return result(std::in_place_index<1>, selected_source<Coordinate, geometry::tag_t<Geometry2>>(id, b, allow_certificate));
    case 2:
    {
        auto boundary = selected_source<Coordinate, void>(id, collection);
        if constexpr (std::is_same<geometry::tag_t<typename boost::range_value<Collection>::type>, ring_tag>::value)
        {
            if (allow_certificate && boundary.points.size() == 3)
                boundary.cycle = cycle_certificate::triangle;
        }
        return result(std::in_place_index<2>, std::move(boundary));
    }
    }
    return {};
}

template <typename Coordinate, typename Source, typename Point>
inline void representative(Source const& source, Point& target)
{
    if constexpr (!std::is_void<typename static_predicates_detail::root_of<Point>::type>::value)
        target = Point::borrow(source);
    else if constexpr (std::is_same<Source, Point>::value) target = source;
    else if constexpr (std::is_same<Point, exact_point<Coordinate>>::value)
        target = make_point<Coordinate>(source);
    else if constexpr (fixed_native_supported<geometry::coordinate_type_t<Source>>::value)
        geometry::convert(source, target);
    else assign_output(source, target);
}

template <typename Coordinate, typename Ring, typename Point>
inline bool first_boundary_point(Ring const& ring, Point& output, ring_tag)
{
    auto const first = boost::begin(ring);
    if (first == boost::end(ring)) return false;
    using point_type = typename boundary_point_type<Coordinate, Ring, ring_tag>::type;
    representative<Coordinate>(from_iterator<Coordinate, point_type>(first), output);
    return true;
}

template <typename Coordinate, typename Polygon, typename Point>
inline bool first_boundary_point(Polygon const& polygon, Point& output, polygon_tag)
{
    auto const& ring = geometry::exterior_ring(polygon);
    auto const first = boost::begin(ring);
    if (first == boost::end(ring)) return false;
    using point_type = typename boundary_point_type<Coordinate, Polygon, polygon_tag>::type;
    representative<Coordinate>(from_iterator<Coordinate, point_type>(first), output);
    return true;
}

template <typename Coordinate, typename Box, typename Point>
inline bool first_boundary_point(Box const& box, Point& output, box_tag)
{
    using source_point = typename boundary_point_type<Coordinate, Box>::type;
    representative<Coordinate>(box_vertex<Coordinate, source_point>(box, 0), output);
    return true;
}

} // namespace ring_topology_detail

template <typename Coordinate>
class ring_topology
{
public:
    using point_type = exact_point<Coordinate>;
    using properties_tag = ring_metadata<Coordinate>;
    using result_type = strategies::detail::ring_topology_result;

    template <typename Box>
    void expand_envelope(Box&) const {}

    template <typename Geometry, typename Point>
    bool representative(Geometry const& geometry, Point& point) const
    {
        return ring_topology_detail::first_boundary_point<Coordinate>(geometry, point, geometry::tag_t<Geometry>());
    }

    template <typename Geometry, typename Point>
    result_type analyze(Geometry const& geometry, Point& representative) const
    {
        typename ring_topology_detail::boundary_point_type<Coordinate, Geometry>::type value;
        result_type fast;
        if (ring_topology_detail::fast_analyze<Coordinate>(geometry, value, fast, geometry::tag_t<Geometry>()))
        {
            if (fast.valid) ring_topology_detail::representative<Coordinate>(value, representative);
            return fast;
        }
        auto const result = ring_topology_detail::analyze(ring_topology_detail::make_boundary<Coordinate>(geometry), value);
        if (result.valid) ring_topology_detail::representative<Coordinate>(value, representative);
        return result;
    }

    template <typename Geometry>
    result_type analyze(Geometry const& geometry) const
    {
        typename ring_topology_detail::boundary_point_type<Coordinate, Geometry>::type representative;
        return analyze(geometry, representative);
    }

    template <geometry::overlay_type OverlayType, typename Geometry1, typename Geometry2,
              typename Collection, typename RingMap, typename Strategy>
    void assign_parents(Geometry1 const& a, Geometry2 const& b, Collection const& collection,
                        RingMap& map, Strategy const&) const
    {
        if (map.size() == 1)
        {
            auto& info = map.begin()->second;
            if (info.valid && info.effective_direction() > 0)
            {
                info.children.clear();
                info.parent = ring_identifier();
                return;
            }
        }
        using properties = typename RingMap::mapped_type;
        struct selected_ring
        {
            ring_identifier id;
            properties* info;
            ring_topology_detail::selected_boundary<Coordinate, Geometry1, Geometry2, Collection> boundary;
        };
        std::vector<selected_ring> rings;
        constexpr bool allow_certificate = OverlayType == overlay_union
            || OverlayType == overlay_intersection || OverlayType == overlay_difference;
        for (auto& pair : map)
        {
            auto& info = pair.second;
            info.children.clear();
            info.parent = ring_identifier();
            if (!info.valid || info.effective_direction() == 0) { info.discarded = true; continue; }
            rings.push_back({pair.first, &info, ring_topology_detail::selected<Coordinate>(
                pair.first, a, b, collection, allow_certificate)});
        }
        auto relation = [&](std::size_t inner, std::size_t outer)
        {
            return std::visit([&](auto const& inner_boundary, auto const& outer_boundary)
            {
                return ring_topology_detail::classify(inner_boundary, outer_boundary,
                    rings[inner].info->boundary_point);
            }, rings[inner].boundary, rings[outer].boundary);
        };
        // Coincident chains are not strict parents. Reduce their directed
        // multiplicities before choosing containers, so parent links cannot cycle.
        std::vector<std::size_t> group(rings.size());
        for (std::size_t i = 0; i < rings.size(); ++i) group[i] = i;
        auto root = [&](std::size_t i) { while (group[i] != i) i = group[i]; return i; };
        for (std::size_t i = 0; i < rings.size(); ++i)
            for (std::size_t j = i + 1; j < rings.size(); ++j)
                if (root(i) != root(j) && relation(i, j) == 0 && relation(j, i) == 0) group[root(j)] = root(i);
        std::vector<std::ptrdiff_t> net(rings.size(), 0);
        for (std::size_t i = 0; i < rings.size(); ++i) net[root(i)] += rings[i].info->effective_direction();
        std::vector<bool> kept(rings.size(), false);
        for (std::size_t i = 0; i < rings.size(); ++i)
        {
            auto const parent = root(i);
            auto const direction = rings[i].info->effective_direction();
            if (!kept[parent] && ((net[parent] > 0 && direction > 0) || (net[parent] < 0 && direction < 0)))
                kept[parent] = true;
            else rings[i].info->discarded = true;
        }
        constexpr bool check_orientation = OverlayType == overlay_buffer || OverlayType == overlay_dissolve;
        for (std::size_t inner = 0; inner < rings.size(); ++inner)
        {
            auto& info = *rings[inner].info;
            if (info.discarded || (!check_orientation && info.effective_direction() > 0)) continue;
            std::size_t parent = rings.size();
            for (std::size_t outer = 0; outer < rings.size(); ++outer)
            {
                if (inner == outer || rings[outer].info->discarded
                    || (!check_orientation && rings[outer].info->effective_direction() <= 0)) continue;
                if (relation(inner, outer) < 0 || relation(outer, inner) >= 0) continue;
                if (parent == rings.size()
                    || (relation(outer, parent) >= 0 && relation(parent, outer) < 0)) parent = outer;
            }
            if (parent != rings.size()) info.parent = rings[parent].id;
        }
        if (check_orientation)
        {
            for (auto& pair : map)
            {
                auto& info = pair.second;
                if (info.discarded) continue;
                bool const positive = info.effective_direction() > 0;
                if (info.parent.source_index >= 0)
                {
                    auto const& parent = map.find(info.parent)->second;
                    bool const parent_positive = parent.direction > 0;
                    if ((positive && parent_positive)
                        || (OverlayType == overlay_dissolve && !positive && !parent_positive)) info.discarded = true;
                    if (positive || info.discarded) info.parent = ring_identifier();
                }
                else if (!positive) info.reversed = !info.reversed;
            }
        }
        for (auto& pair : map)
            if (!pair.second.discarded && pair.second.parent.source_index >= 0)
                map.find(pair.second.parent)->second.children.push_back(pair.first);
    }
};

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

#endif
