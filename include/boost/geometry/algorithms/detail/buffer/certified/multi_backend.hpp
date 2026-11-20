// Boost.Geometry
// Copyright (c) 2026
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_MULTI_BACKEND_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_MULTI_BACKEND_HPP

#include <cstddef>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

#include <boost/geometry/algorithms/detail/buffer/certified/multi_arrangement.hpp>
#include <boost/geometry/algorithms/detail/buffer/certified/convex_erosion.hpp>

namespace boost { namespace geometry { namespace detail { namespace certified_round_buffer
{

namespace multi_arrangement = certified_multi_buffer;
using multi_arrangement::exact_number;
using status = certified_buffer_status;

template <typename Tag>
constexpr bool multi_buffer_input = std::is_same_v<Tag, multi_point_tag>
                                 || std::is_same_v<Tag, multi_linestring_tag>
                                 || std::is_same_v<Tag, multi_polygon_tag>;

template <typename Geometry>
status validate_multi_trace(Geometry const& input, bool& empty)
{
    std::size_t count = 0;
    auto const visit = [&](auto const& p)
    {
        if (++count > 4096) { return false; }
        auto const q = multi_arrangement::read_point(p);
        return certified_convex_erosion::bounded(q.x) && certified_convex_erosion::bounded(q.y);
    };
    try
    {
        for (auto const& m : input)
        {
            if constexpr (std::is_same_v<tag_t<Geometry>, multi_point_tag>)
            {
                if (!visit(m)) { return status::cannot_certify; }
            }
            else
            {
                for (auto const& p : m) { if (!visit(p)) { return status::cannot_certify; } }
            }
        }
    }
    catch (multi_arrangement::invalid_coordinate const&) { return status::invalid_input; }
    empty = count == 0;
    return status::success;
}

// Inputs have already passed the same disjoint-filled-member test as the
// nonzero path. Copy the actual source rings, checking each stored coordinate;
// the caller's displacement budget never relaxes a zero-distance identity.
template <typename MultiPolygon>
bool copy_multi_identity(std::vector<multi_arrangement::polygon_rings> const& members, MultiPolygon& output)
{
    using polygon = typename boost::range_value<MultiPolygon>::type;
    constexpr bool cw = geometry::point_order<polygon>::value == clockwise;
    constexpr bool close = closure<polygon>::value == closed;
    MultiPolygon candidate;
    range::resize(candidate, members.size());
    auto p = boost::begin(candidate);
    auto const end = boost::end(candidate);
    auto const copy = [&](auto const& from, auto& to, bool want_ccw)
    {
        return certified_linestring_buffer::write_ring_lossless(
            from.points, to, (from.area2 > 0) != want_ccw, close);
    };
    for (auto const& member : members)
    {
        if (p == end || !copy(member.front(), exterior_ring(*p), !cw)) { return false; }
        auto& holes = interior_rings(*p);
        range::resize(holes, member.size() - 1);
        auto h = boost::begin(holes);
        auto const h_end = boost::end(holes);
        for (auto i = member.begin() + 1; i != member.end(); ++i)
        {
            if (h == h_end || !copy(*i, *h, cw)) { return false; }
            ++h;
        }
        if (h != h_end) { return false; }
        ++p;
    }
    if (p != end) { return false; }
    output = std::move(candidate);
    return true;
}

template <typename Geometry, typename MultiPolygon>
status multi_backend(Geometry const& input, MultiPolygon& output, exact_number const& distance,
                     exact_number const& sagitta, exact_number const& max_displacement)
{
    constexpr bool areal = std::is_same_v<tag_t<Geometry>, multi_polygon_tag>;
    std::vector<multi_arrangement::polygon_rings> members;
    if constexpr (areal)
    {
        auto const read = multi_arrangement::read_members(input, members);
        if (read != status::success) { return read; }
        if (distance == 0 || members.empty())
        {
            return copy_multi_identity(members, output) ? status::success : status::cannot_certify;
        }
    }
    else
    {
        bool empty = false;
        auto const read = validate_multi_trace(input, empty);
        if (read != status::success) { return read; }
        if (empty || distance <= 0)
        {
            output = MultiPolygon();
            return status::success;
        }
    }

    if constexpr (certified_point_buffer::use_rational_backend<coordinate_type_t<MultiPolygon>>::value)
    {
        return status::cannot_certify; // No lossless multi chord backend yet.
    }
    else
    {
        if (sagitta == 0) { return status::cannot_certify; }
        exact_number const r = distance > 0 ? distance : exact_number(-distance);
        exact_number const e = sagitta < r ? sagitta : exact_number(r / 2);
        exact_number const inner = r - e;
        exact_number const d = max_displacement < inner ? max_displacement : exact_number(inner / 2);
        if constexpr (areal)
        {
            // Strict source separation makes boundary(P) the union of the
            // member boundaries. Select material from the whole P in one
            // arrangement, not by buffering/exporting the members in turn.
            try { return multi_arrangement::build_areal_members(members, output, r, e, d, distance < 0); }
            catch (certified_star_buffer::uncertified const&) { return status::cannot_certify; }
        }
        else
        {
            return geometry::buffer_multi_arrangement_certified(input, output, r, e, d);
        }
    }
}

}}}} // namespace boost::geometry::detail::certified_round_buffer

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_MULTI_BACKEND_HPP
