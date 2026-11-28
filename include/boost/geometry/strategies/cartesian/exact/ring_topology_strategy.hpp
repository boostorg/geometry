// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_RING_TOPOLOGY_STRATEGY_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_RING_TOPOLOGY_STRATEGY_HPP

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
#include <boost/geometry/algorithms/detail/overlay/get_ring.hpp>
#include <boost/geometry/algorithms/detail/overlay/ring_topology.hpp>
#include <boost/geometry/strategies/cartesian/exact/box_primitives.hpp>
#include <boost/geometry/strategies/cartesian/exact/predicate_dispatch.hpp>
#include <boost/geometry/strategies/cartesian/exact/midpoint_query.hpp>
#include <boost/geometry/views/detail/iterator_point_view.hpp>
#include <boost/geometry/strategies/detail/ring_topology.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

namespace ring_topology_detail
{

namespace algorithms = geometry::detail::overlay::ring_topology;
using algorithms::boundary;
using algorithms::cycle_certificate;

template <typename Coordinate>
struct predicates
{
    template <std::size_t D, typename A, typename B>
    static int compare(A const& a, B const& b) { return static_compare_coordinate<D, Coordinate>(a, b); }

    template <typename A, typename B>
    static bool less(A const& a, B const& b)
    {
        int const x = compare<0>(a, b);
        return x < 0 || (x == 0 && compare<1>(a, b) < 0);
    }

    template <typename A, typename B>
    static bool equal(A const& a, B const& b) { return static_equal_points<Coordinate>(a, b); }

    template <typename A, typename B, typename P>
    static int side(A const& a, B const& b, P const& p) { return static_orientation<Coordinate>(a, b, p); }
};

template <typename Root>
struct containment_predicates : predicates<typename Root::coordinate_type>
{
    template <typename A, typename B>
    static auto midpoint(A const& a, B const& b) { return midpoint_query<Root>::midpoint(a, b); }
};

template <typename Coordinate, typename Point, typename Iterator>
inline Point from_iterator(Iterator const& iterator)
{
    if constexpr (!std::is_void<typename static_predicates_detail::root_of<Point>::type>::value)
        return Point::from_iterator(iterator);
    else if constexpr (std::is_same<Point, geometry::detail::iterator_point_view<Iterator>>::value)
    {
        static_assert(lossless_coordinate_conversion<geometry::coordinate_type_t<
            typename std::iterator_traits<Iterator>::value_type>, Coordinate>::value,
            "Exact ring inputs require lossless coordinate promotion");
        return Point(iterator);
    }
    else if constexpr (std::is_same<Point, typename std::iterator_traits<Iterator>::value_type>::value)
        return geometry::detail::iterator_point_view<Iterator>(iterator).read();
    else
        return Point(geometry::detail::iterator_point_view<Iterator>(iterator).read());
}

template <typename Coordinate, typename Geometry, typename Tag = geometry::tag_t<Geometry>>
struct boundary_point_type;

template <typename Coordinate, typename Box>
struct boundary_point_type<Coordinate, Box, box_tag>
{
    static_assert(std::is_void<typename static_predicates_detail::root_of<Box>::type>::value,
                  "Root coordinate-origin bounds are metadata, not geometry boundaries");
    using type = source_geometry_detail::indexed_vertex<Box>;
};

template <typename Coordinate, typename Point, typename Box>
inline Point box_vertex(Box const& box, std::size_t index)
{
    static_assert(std::is_void<typename static_predicates_detail::root_of<Box>::type>::value,
                  "Root coordinate-origin bounds cannot supply geometry corners");
    return Point{&box, index};
}

template <typename Coordinate, typename Ring>
struct boundary_point_type<Coordinate, Ring, ring_tag>
{
    using iterator = typename boost::range_iterator<Ring const>::type;
    using input = typename std::iterator_traits<iterator>::value_type;
    using type = std::conditional_t<fixed_native_supported<geometry::coordinate_type_t<input>>::value,
        geometry::detail::iterator_point_view<iterator>, input>;
};

template <typename Coordinate, typename Polygon>
struct boundary_point_type<Coordinate, Polygon, polygon_tag>
{
    using outer = decltype(geometry::exterior_ring(std::declval<Polygon const&>()));
    using holes = decltype(geometry::interior_rings(std::declval<Polygon const&>()));
    using inner = decltype(*boost::begin(std::declval<holes&>()));
    using outer_iterator = typename boost::range_iterator<std::remove_reference_t<outer> const>::type;
    using inner_iterator = typename boost::range_iterator<std::remove_reference_t<inner> const>::type;
    static_assert(std::is_lvalue_reference<outer>::value && std::is_lvalue_reference<holes>::value
                  && std::is_lvalue_reference<inner>::value
                  && std::is_same<outer_iterator, inner_iterator>::value,
                  "Borrowed rings require stable, common boundary iterators");
    using type = typename boundary_point_type<Coordinate, std::remove_reference_t<outer>, ring_tag>::type;
};

template <typename Coordinate, typename Point, typename Ring>
inline void append(boundary<Point>& result, Ring const& ring, ring_tag)
{
    auto const begin = result.points.size();
    for (auto it = boost::begin(ring); it != boost::end(ring); ++it)
    {
        auto point = from_iterator<Coordinate, Point>(it);
        if (result.points.size() == begin || !predicates<Coordinate>::equal(result.points.back(), point))
            result.points.push_back(std::move(point));
    }
    auto end = result.points.size();
    if (end - begin < 2) return;
    if (predicates<Coordinate>::equal(result.points[begin], result.points[end - 1]))
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
inline void append(boundary<Point>& result, Polygon const& polygon, polygon_tag)
{
    append<Coordinate>(result, geometry::exterior_ring(polygon), ring_tag());
    for (auto const& ring : geometry::interior_rings(polygon)) append<Coordinate>(result, ring, ring_tag());
}

template <typename Coordinate, typename Point, typename Box>
inline void append(boundary<Point>& result, Box const& box, box_tag)
{
    auto const low = box_vertex<Coordinate, Point>(box, 0);
    auto const high = box_vertex<Coordinate, Point>(box, 2);
    if (predicates<Coordinate>::template compare<0>(low, high) >= 0
        || predicates<Coordinate>::template compare<1>(low, high) >= 0) return;
    auto const offset = result.points.size();
    for (std::size_t i = 0; i != 4; ++i) result.points.push_back(box_vertex<Coordinate, Point>(box, i));
    for (std::size_t i = 0; i != 4; ++i)
        result.edges.push_back({offset + i, offset + (i + 1) % 4, 1});
    result.cycle = cycle_certificate::certified;
}

template <typename Coordinate, typename Geometry>
inline auto make_boundary(Geometry const& geometry)
{
    boundary<typename boundary_point_type<Coordinate, Geometry>::type> result;
    result.order = geometry::point_order<Geometry>::value == geometry::clockwise ? 1 : -1;
    append<Coordinate>(result, geometry, geometry::tag_t<Geometry>());
    algorithms::find_extrema(result, predicates<Coordinate>{});
    return result;
}

template <typename Coordinate, typename Geometry, typename Point, typename Tag>
inline bool fast_analyze(Geometry const&, Point&,
                         strategies::detail::ring_topology_result&, Tag) { return false; }

template <typename Coordinate, typename Ring, typename Point>
inline bool fast_analyze(Ring const& ring, Point& representative,
                         strategies::detail::ring_topology_result& result, ring_tag)
{
    auto first = boost::begin(ring), end = boost::end(ring);
    auto read = [](auto& iterator) -> decltype(auto)
    {
        using iterator_type = std::decay_t<decltype(iterator)>;
        using point_type = typename std::iterator_traits<iterator_type>::value_type;
        if constexpr (fixed_native_supported<geometry::coordinate_type_t<point_type>>::value)
            return geometry::detail::iterator_point_view<iterator_type>(iterator);
        else
            return *iterator;
    };
    auto minimum = first;
    if (!algorithms::analyze_extreme<geometry::closure<Ring>::value == geometry::closed>(
            first, end, minimum, geometry::point_order<Ring>::value == geometry::clockwise ? 1 : -1,
            result, read, predicates<Coordinate>{}))
        return false;
    if (result.valid) representative = from_iterator<Coordinate, Point>(minimum);
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
    if (predicates<Coordinate>::template compare<0>(low, high) >= 0
        || predicates<Coordinate>::template compare<1>(low, high) >= 0) result = {};
    else
    {
        representative = low;
        result = {true, true, geometry::point_order<Box>::value == geometry::clockwise ? 1 : -1};
    }
    return true;
}

template <typename Coordinate>
class winding_edge_scan
{
    using interval = std::pair<double, double>;
    static constexpr std::size_t block_size = 8;
    mutable std::vector<interval> m_blocks;

    template <typename Point>
    static interval scan_bounds(Point const& point)
    {
        auto const estimate = static_box_detail::approximation<1>(point);
        return {static_box_detail::lower(estimate), static_box_detail::upper(estimate)};
    }

    template <typename Point>
    void prepare(boundary<Point> const& ring) const
    {
        std::vector<interval> ordinates;
        ordinates.reserve(ring.points.size());
        for (auto const& point : ring.points)
            ordinates.push_back(scan_bounds(point));

        for (std::size_t i = 0; i < ring.edges.size(); i += block_size)
        {
            auto bounds = ordinates[ring.edges[i].first];
            for (std::size_t j = i; j < (std::min)(ring.edges.size(), i + block_size); ++j)
            {
                auto const& edge = ring.edges[j];
                bounds.first = (std::min)({bounds.first, ordinates[edge.first].first,
                                          ordinates[edge.second].first});
                bounds.second = (std::max)({bounds.second, ordinates[edge.first].second,
                                           ordinates[edge.second].second});
            }
            m_blocks.push_back(bounds);
        }
    }

public:
    template <typename Query, typename Point, typename Visit>
    bool operator()(Query const& point, boundary<Point> const& ring, Visit const& visit) const
    {
        if (ring.edges.size() < 4 * block_size)
            return algorithms::scan_all_edges{}(point, ring, visit);
        if (m_blocks.empty()) prepare(ring);

        // Closed bounds: touching the scan line can still be a boundary hit.
        auto const query = scan_bounds(point);
        for (std::size_t i = 0; i < m_blocks.size(); ++i)
        {
            auto const& bounds = m_blocks[i];
            if (bounds.second < query.first || bounds.first > query.second) continue;
            auto const begin = ring.edges.begin() + i * block_size;
            auto const end = ring.edges.begin() + (std::min)(ring.edges.size(), (i + 1) * block_size);
            if (std::any_of(begin, end, visit)) return true;
        }
        return false;
    }
};

template <typename Coordinate, typename Boundary>
struct prepared_boundary
{
    using scan_type = std::conditional_t<fixed_native_supported<Coordinate>::value,
                                          winding_edge_scan<Coordinate>, algorithms::scan_all_edges>;
    Boundary ring;
    scan_type scan;
};

template <typename Coordinate, typename Tag, typename Geometry>
inline auto selected_source(ring_identifier const& id, Geometry const& geometry,
                            bool allow_certificate = false)
{
    decltype(auto) ring = geometry::detail::overlay::get_ring<Tag>::apply(id, geometry);
    static_assert(std::is_lvalue_reference<decltype(ring)>::value,
                  "Retained boundary iterators must refer to a stable ring");
    using ring_type = std::remove_reference_t<decltype(ring)>;
    auto boundary = make_boundary<Coordinate>(ring);
    if constexpr (std::is_same<geometry::tag_t<ring_type>, ring_tag>::value)
    {
        if (allow_certificate && (boundary.points.size() == 3
                || fixed_native_supported<geometry::coordinate_type_t<ring_type>>::value))
            boundary.cycle = cycle_certificate::check;
    }
    return prepared_boundary<Coordinate, decltype(boundary)>{std::move(boundary), {}};
}

template <typename Coordinate, typename Geometry1, typename Geometry2, typename Collection>
using selected_boundary = std::variant<
    decltype(selected_source<Coordinate, geometry::tag_t<Geometry1>>(
        std::declval<ring_identifier const&>(), std::declval<Geometry1 const&>())),
    decltype(selected_source<Coordinate, geometry::tag_t<Geometry2>>(
        std::declval<ring_identifier const&>(), std::declval<Geometry2 const&>())),
    decltype(selected_source<Coordinate, void>(
        std::declval<ring_identifier const&>(), std::declval<Collection const&>()))>;

template <typename Coordinate, typename Source, typename Point>
inline void representative(Source const& source, Point& target)
{
    if constexpr (!std::is_void<typename static_predicates_detail::root_of<Point>::type>::value)
        target = Point::borrow(source);
    else if constexpr (std::is_assignable<Point&, Source const&>::value)
        target = source;
    else
        predicates<Coordinate>::assign(source, target);
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

template <typename Coordinate, typename PointType, typename ContainmentPredicates>
class cartesian_ring_topology
{
public:
    using point_type = PointType;
    using result_type = strategies::detail::ring_topology_result;

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
        auto const result = ring_topology_detail::algorithms::analyze(
            ring_topology_detail::make_boundary<Coordinate>(geometry), value, ring_topology_detail::predicates<Coordinate>{});
        if (result.valid) ring_topology_detail::representative<Coordinate>(value, representative);
        return result;
    }

    template <typename Geometry>
    result_type analyze(Geometry const& geometry) const
    {
        typename ring_topology_detail::boundary_point_type<Coordinate, Geometry>::type representative;
        return analyze(geometry, representative);
    }

    template <typename Geometry1, typename Geometry2, typename Collection>
    static auto boundary(geometry::ring_identifier const& id, Geometry1 const& a,
                         Geometry2 const& b, Collection const& collection, bool allow_certificate)
    {
        using result = ring_topology_detail::selected_boundary<Coordinate, Geometry1, Geometry2, Collection>;
        using ring_topology_detail::selected_source;
        switch (id.source_index)
        {
        case 0: return result(std::in_place_index<0>,
            selected_source<Coordinate, geometry::tag_t<Geometry1>>(id, a, allow_certificate));
        case 1: return result(std::in_place_index<1>,
            selected_source<Coordinate, geometry::tag_t<Geometry2>>(id, b, allow_certificate));
        case 2: return result(std::in_place_index<2>,
            selected_source<Coordinate, void>(id, collection, allow_certificate));
        }
        return result{};
    }

    template <typename Inner, typename Outer, typename Point>
    static int classify(Inner const& inner, Outer const& outer, Point const& sample)
    {
        return std::visit([&](auto const& inner_boundary, auto const& outer_boundary)
        {
            return ring_topology_detail::algorithms::classify(inner_boundary.ring, outer_boundary.ring,
                sample, ContainmentPredicates{}, outer_boundary.scan);
        }, inner, outer);
    }

    template <typename A, typename B, typename Point>
    static bool coincident(A const& a, B const& b, Point const& a_sample, Point const& b_sample)
    {
        // analyze() chooses the least surviving boundary vertex. Identical
        // chains must have the same least vertex even with different splits.
        return ContainmentPredicates::equal(a_sample, b_sample)
            && classify(a, b, a_sample) == 0 && classify(b, a, b_sample) == 0;
    }
};

template <typename Root>
using ring_topology = cartesian_ring_topology<typename Root::coordinate_type, point_ref<Root>,
    ring_topology_detail::containment_predicates<Root>>;

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

#endif
