// Boost.Geometry
// Copyright (c) 2026
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_POLYGON_BACKEND_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_POLYGON_BACKEND_HPP

#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include <boost/geometry/algorithms/detail/buffer/certified/polygon_arrangement.hpp>
#include <boost/geometry/algorithms/detail/buffer/certified/convex_erosion.hpp>
#include <boost/geometry/algorithms/is_valid.hpp>
#include <boost/geometry/strategies/relate/cartesian.hpp>

namespace boost { namespace geometry { namespace detail { namespace certified_round_buffer
{

namespace common = certified_point_buffer;
using common::exact_number;
using status = certified_buffer_status;

template <typename Polygon>
using lifted_polygon = model::polygon<model::point<exact_number, 2, cs::cartesian>,
    geometry::point_order<Polygon>::value == clockwise, closure<Polygon>::value == closed>;

// The chord backends' disjoint-boundary precondition is stronger than validity.
template <typename Polygon>
status read_valid_polygon(Polygon const& input, lifted_polygon<Polygon>& lifted)
{
    std::size_t positions = 0;
    auto append = [&](auto const& from, auto& to)
    {
        for (auto const& p : from)
        {
            if (++positions > 4096) { return false; }
            auto const q = common::read_point(p);
            if (!certified_convex_erosion::bounded(q.x) || !certified_convex_erosion::bounded(q.y))
            { return false; }
            to.emplace_back(q.x, q.y);
        }
        return true;
    };
    try
    {
        if (!append(exterior_ring(input), lifted.outer())) { return status::cannot_certify; }
        if (lifted.outer().empty()) { return status::invalid_input; }
        for (auto const& hole : interior_rings(input))
        {
            if (lifted.inners().size() == 4096) { return status::cannot_certify; }
            lifted.inners().emplace_back();
            if (!append(hole, lifted.inners().back())) { return status::cannot_certify; }
        }
    }
    catch (std::domain_error const&) { return status::invalid_input; }
    return geometry::is_valid(lifted, strategies::relate::cartesian<exact_number>())
        ? status::success : status::invalid_input;
}

template <typename Polygon, typename MultiPolygon>
status zero_distance(Polygon const& input, MultiPolygon& output)
{
    lifted_polygon<Polygon> lifted;
    auto const read = read_valid_polygon(input, lifted);
    if (read != status::success) { return read; }

    using polygon = typename boost::range_value<MultiPolygon>::type;
    constexpr bool reverse = geometry::point_order<Polygon>::value != geometry::point_order<polygon>::value;
    constexpr bool close = closure<polygon>::value == closed;
    auto const copy_ring = [](auto const& from, auto& to)
    {
        certified_linestring_buffer::piece points;
        for (auto const& p : from) { points.push_back(common::read_point(p)); }
        if constexpr (closure<Polygon>::value == closed) { points.pop_back(); }
        return certified_linestring_buffer::write_ring_lossless(points, to, reverse, close);
    };

    MultiPolygon candidate;
    range::resize(candidate, 1);
    auto const p = boost::begin(candidate);
    auto const end = boost::end(candidate);
    if (p == end || std::next(p) != end || !copy_ring(lifted.outer(), exterior_ring(*p)))
    { return status::cannot_certify; }
    auto& holes = interior_rings(*p);
    range::resize(holes, lifted.inners().size());
    auto h = boost::begin(holes);
    for (auto const& ring : lifted.inners())
    {
        if (h == boost::end(holes) || !copy_ring(ring, *h++)) { return status::cannot_certify; }
    }
    if (h != boost::end(holes)) { return status::cannot_certify; }
    output = std::move(candidate);
    return status::success;
}

// Both arrangements export all ideal faces, or refuse. For erosion their
// inscribed band T is a subset of the true radius-r neighborhood of the
// boundary, so reg(P \ N_r(boundary)) is a subset of reg(P \ T). An empty
// successful export proves true regularized empty; a nonempty chord result
// neither proves that the smooth erosion is nonempty nor fixes its topology.
template <typename Polygon, typename MultiPolygon>
status polygon_backend(Polygon const& input, MultiPolygon& output,
                       exact_number const& distance, exact_number const& sagitta,
                       exact_number const& max_displacement)
{
    if (distance == 0) { return zero_distance(input, output); }
    if (sagitta > 0)
    {
        exact_number const r = distance > 0 ? distance : exact_number(-distance);
        exact_number const e = sagitta < r ? sagitta : exact_number(r / 2);
        using polygon = typename boost::range_value<MultiPolygon>::type;
        if constexpr (! common::use_rational_backend<coordinate_type_t<polygon>>::value)
        {
            // The native winding certificate needs strictly positive r-e-d.
            // Tightening a loose budget is safe; never relax a zero budget.
            exact_number const inner = r - e;
            exact_number const d = max_displacement < inner ? max_displacement : exact_number(inner / 2);
            auto const result = geometry::buffer_polygon_arrangement_certified(input, output, distance, e, d);
            if (result == status::success) { return result; }
        }

        // Rational coordinates are not a native export grid. This fallback
        // exports the rational chord result losslessly on either grid.
        auto const mode = distance > 0 ? certified_polygon_buffer_mode::dilation
                                       : certified_polygon_buffer_mode::erosion;
        auto const result = geometry::buffer_polygon_certified(input, output, r, e, mode);
        if (result != status::invalid_input) { return result; }
    }

    lifted_polygon<Polygon> lifted;
    auto const read = read_valid_polygon(input, lifted);
    return read == status::success ? status::cannot_certify : read;
}

}}}} // namespace boost::geometry::detail::certified_round_buffer

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_POLYGON_BACKEND_HPP
