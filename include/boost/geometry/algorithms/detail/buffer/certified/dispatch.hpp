// Boost.Geometry
// Copyright (c) 2026
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_DISPATCH_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_DISPATCH_HPP

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <boost/geometry/algorithms/detail/buffer/certified/linestring_arrangement.hpp>
#include <boost/geometry/algorithms/detail/buffer/certified/convex_dilation.hpp>
#include <boost/geometry/algorithms/detail/buffer/certified/multi_backend.hpp>
#include <boost/geometry/algorithms/detail/buffer/certified/polygon_backend.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/multi_polygon.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace certified_round_buffer
{

namespace line = certified_linestring_buffer;
namespace capsule = certified_segment_buffer;
using common::rational_xy;
using common::rational_value;
using line::exact_point;
using line::exact_polygon;

template <typename Output, typename Tag = tag_t<Output>>
struct output_stage
{
    using type = Output;
};

template <typename Polygon>
struct output_stage<Polygon, polygon_tag>
{
    using type = model::multi_polygon<Polygon>;
};

struct budget
{
    exact_number distance, sagitta, displacement;
};

// The rational producers have already certified the ideal chord ring. Check
// actual stored vertices and topology separately, even for adapted outputs.
template <typename Polygon>
bool export_convex(exact_polygon const& ideal, Polygon& candidate, exact_number const& displacement)
{
    using point = point_type_t<Polygon>;
    using rounder = common::coordinate_writer<coordinate_type_t<point>>;
    constexpr bool cw = geometry::point_order<Polygon>::value == clockwise;
    constexpr bool close = closure<Polygon>::value == closed;
    auto& ring = exterior_ring(candidate);
    auto const& vertices = ideal.outer();
    if (vertices.size() < 3) { return false; }
    auto const size = vertices.size() + (close ? 1 : 0);
    auto const at = [&](std::size_t i) -> exact_point const&
    {
        if (i == vertices.size()) { i = 0; }
        return vertices[cw ? vertices.size() - 1 - i : i];
    };
    for (std::size_t i = 0; i < size; ++i)
    {
        point p;
        geometry::set<0>(p, rounder::apply(geometry::get<0>(at(i))));
        geometry::set<1>(p, rounder::apply(geometry::get<1>(at(i))));
        range::push_back(ring, std::move(p));
    }

    std::vector<rational_xy> stored;
    exact_number const limit2 = displacement * displacement;
    auto it = boost::begin(ring);
    auto const end = boost::end(ring);
    for (std::size_t i = 0; i < size; ++i)
    {
        if (it == end) { return false; }
        auto const p = common::read_point(*it++);
        auto const delta = common::difference(p, common::read_point(at(i)));
        if (common::dot(delta, delta) > limit2) { return false; }
        if (i < vertices.size()) { stored.push_back(p); }
        else if (!capsule::same_point(p, stored.front())) { return false; }
    }
    if (cw) { std::reverse(stored.begin(), stored.end()); }
    return it == end && boost::empty(interior_rings(candidate))
        && certified_convex_erosion::strict_convex(stored);
}

template <typename Geometry>
status read_trace(Geometry const& input, std::vector<rational_xy>& trace)
{
    try
    {
        if constexpr (std::is_same_v<tag_t<Geometry>, point_tag>)
        {
            trace.push_back(common::read_point(input));
        }
        else if constexpr (std::is_same_v<tag_t<Geometry>, segment_tag>)
        {
            trace.push_back({rational_value(geometry::get<0, 0>(input)), rational_value(geometry::get<0, 1>(input))});
            trace.push_back({rational_value(geometry::get<1, 0>(input)), rational_value(geometry::get<1, 1>(input))});
        }
        else
        {
            for (auto const& p : input)
            {
                if (trace.size() == 4096) { return status::cannot_certify; }
                trace.push_back(common::read_point(p));
            }
        }
    }
    catch (std::domain_error const&) { return status::invalid_input; }
    if (trace.empty()) { return status::invalid_input; }
    for (auto const& p : trace)
    {
        if (!certified_convex_erosion::bounded(p.x) || !certified_convex_erosion::bounded(p.y))
        { return status::cannot_certify; }
    }
    return status::success;
}

template <typename Polygon>
status dilate_trace(std::vector<rational_xy> const& trace, Polygon& output, budget const& b)
{
    if (b.sagitta == 0) { return status::cannot_certify; }
    auto const& r = b.distance;
    exact_number const e = b.sagitta < r ? b.sagitta : exact_number(r / 2);
    auto const ends = std::minmax_element(trace.begin(), trace.end(), [](auto const& p, auto const& q)
    {
        return p.x < q.x || (p.x == q.x && p.y < q.y);
    });
    auto const& a = *ends.first;
    auto const& z = *ends.second;
    if (capsule::same_point(a, z))
    {
        exact_polygon ideal;
        auto const result = geometry::buffer_point_certified(exact_point(a.x, a.y), ideal, r, e);
        if (result != status::success) { return result; }
        return export_convex(ideal, output, b.displacement) ? status::success : status::cannot_certify;
    }

    auto const axis = common::difference(z, a);
    bool const collinear = std::all_of(trace.begin(), trace.end(), [&](auto const& p)
    {
        return common::cross(axis, common::difference(p, a)) == 0;
    });
    model::linestring<exact_point> path;
    if (collinear)
    {
        exact_polygon ideal;
        Polygon candidate;
        if (capsule::build(a, z, ideal, r, e, true) == status::success
            && export_convex(ideal, candidate, b.displacement))
        {
            output = std::move(candidate);
            return status::success;
        }
        // A connected collinear trace equals the segment between its extremes.
        path.emplace_back(a.x, a.y);
        path.emplace_back(z.x, z.y);
    }
    else
    {
        for (auto const& p : trace) { path.emplace_back(p.x, p.y); }
    }

    if constexpr (common::use_rational_backend<coordinate_type_t<Polygon>>::value)
    {
        return geometry::buffer_linestring_certified(path, output, r, e);
    }
    else
    {
        auto const ys = std::minmax_element(trace.begin(), trace.end(), [](auto const& p, auto const& q)
        {
            return p.y < q.y;
        });
        exact_point const witness(exact_number((a.x + z.x) / 2),
            exact_number((ys.first->y + ys.second->y) / 2));
        auto const result = geometry::buffer_linestring_star_certified(path, witness, output, r, e, b.displacement);
        if (result == status::success) { return result; }
        // A backend may require a tighter budget than the caller does.
        exact_number const d = b.displacement < r - e ? b.displacement : exact_number((r - e) / 2);
        return geometry::buffer_linestring_arrangement_certified(path, output, r, e, d);
    }
}

template <typename Geometry, typename MultiPolygon>
status dispatch(Geometry const& input, MultiPolygon& output, budget const& b)
{
    if constexpr (multi_buffer_input<tag_t<Geometry>>)
    {
        return multi_backend(input, output, b.distance, b.sagitta, b.displacement);
    }
    else if constexpr (std::is_same_v<tag_t<Geometry>, polygon_tag>)
    {
        if (b.distance < 0)
        {
            auto const result = geometry::erode_convex_polygon_certified(
                input, output, exact_number(-b.distance), b.displacement);
            // Only a source-domain rejection permits the generic path.
            // Do not substitute a chord result for a failed exact export.
            if (result != status::invalid_input) { return result; }
        }
        using polygon = typename boost::range_value<MultiPolygon>::type;
        if constexpr (! common::use_rational_backend<coordinate_type_t<polygon>>::value)
        {
            if (b.distance > 0 && b.sagitta > 0)
            {
                polygon candidate;
                auto const result = certified_convex_dilation::apply(
                    input, candidate, b.distance, b.sagitta, b.displacement);
                if (result == status::success)
                {
                    range::push_back(output, std::move(candidate));
                    return result;
                }
            }
        }
        return polygon_backend(input, output, b.distance, b.sagitta, b.displacement);
    }
    else
    {
        std::vector<rational_xy> trace;
        auto const read = read_trace(input, trace);
        if (read != status::success) { return read; }
        // Regularization discards every point or linear trace at distance <= 0.
        if (b.distance <= 0) { return status::success; }
        typename boost::range_value<MultiPolygon>::type polygon;
        auto const result = dilate_trace(trace, polygon, b);
        if (result == status::success) { range::push_back(output, std::move(polygon)); }
        return result;
    }
}

}} // namespace detail::certified_round_buffer
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_DISPATCH_HPP
