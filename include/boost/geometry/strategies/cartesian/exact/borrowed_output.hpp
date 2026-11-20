// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_OUTPUT_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_OUTPUT_HPP

#include <algorithm>
#include <cstddef>
#include <boost/assert.hpp>
#include <boost/geometry/algorithms/detail/signed_size_type.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian
{

enum class supported_append_mode { none, duplicates, spikes, collinear };

// Side provides apply(a,b,p) and direction_code(a,b,p); Equal is a binary
// predicate. Only the predicates inspect points, including their producers.
template <typename Range, typename Point, typename Side, typename Equal>
void append_supported(Range& range, Point const& point, Side const& side,
                      Equal const& equal, supported_append_mode mode)
{
    BOOST_ASSERT(range.incoming.size() == range.size());
    if (mode != supported_append_mode::none && !range.empty()
        && (mode == supported_append_mode::duplicates || range.size() == 1)
        && equal(range.back(), point))
    {
        return;
    }

    // The argument may be a vertex of range, e.g. its closing point.
    typename Range::value_type const candidate(point);
    range.push_back(candidate);
    if (mode == supported_append_mode::none || mode == supported_append_mode::duplicates)
    {
        return;
    }

    while (range.size() >= 3)
    {
        auto const size = range.size();
        auto const& a = range[size - 3];
        auto const& b = range[size - 2];
        // A nonzero exact determinant also certifies distinct vertices.
        if (side.apply(a, b, candidate) != 0) break;
        bool const equals_a = equal(a, candidate);
        bool const equals_b = equal(b, candidate);
        if (!equals_a && !equals_b
            && mode == supported_append_mode::spikes
            && side.direction_code(a, b, candidate) >= 1)
        {
            break;
        }

        auto incoming = range.incoming.back();
        if (equals_a)
        {
            // Cancelling a-b-a must not relabel the preceding edge with the
            // support of the spike, even when the two a's have different producers.
            incoming = range.incoming[size - 3];
        }
        else if (equals_b)
        {
            incoming = range.incoming[size - 2];
        }
        range.resize(size - 2);
        range.push_back(candidate);
        range.incoming.back() = incoming;
    }
}

// Closed means an explicit duplicate endpoint. Slot zero then aliases the
// closing edge stored at the last vertex; otherwise slot zero is unused.
// Pending describes caller-selected continuation and is not changed here.
template <bool Closed, typename Range>
void reverse_supported(Range& range)
{
    BOOST_ASSERT(range.incoming.size() == range.size());
    if (range.size() < 2)
    {
        return;
    }
    std::reverse(range.begin(), range.end());
    std::reverse(range.incoming.begin() + 1, range.incoming.end());
    if (Closed)
    {
        range.incoming.front() = range.incoming.back();
    }
}

// Circular view_size includes the duplicate closing vertex, as in copy_segments.
// Callbacks receive (logical index, Reverse). PointAt borrows an original vertex;
// SupportAt resolves the outgoing edge at that index to an original support.
template <bool Reverse, bool RemoveSpikes, typename SegmentIdentifier,
          typename Range, typename Side, typename Equal, typename PointAt, typename SupportAt>
void copy_supported(signed_size_type view_size, bool circular,
                    SegmentIdentifier const& id, signed_size_type to_index,
                    Range& out, Side const& side, Equal const& equal,
                    PointAt const& point_at, SupportAt const& support_at)
{
    BOOST_ASSERT(out.incoming.size() == out.size());
    signed_size_type const from_index = id.segment_index + 1;
    if (!circular && (from_index > to_index || from_index < 0 || to_index >= view_size))
    {
        return;
    }
    if (circular)
    {
        BOOST_ASSERT(view_size >= 2 && from_index >= 0 && from_index < view_size);
        BOOST_ASSERT(to_index >= 0 && to_index < view_size);
    }

    signed_size_type const count = from_index <= to_index
        ? to_index - from_index + 1
        : view_size - from_index + to_index + 1;
    signed_size_type const edge_count = view_size - 1;
    auto index = from_index;
    for (signed_size_type i = 0; i < count; ++i)
    {
        if (edge_count > 0 && (circular || index > 0))
        {
            auto const previous = index == 0 ? edge_count - 1 : index - 1;
            out.pending = support_at(previous, Reverse);
        }
        append_supported(out, point_at(index, Reverse), side, equal,
                         RemoveSpikes ? supported_append_mode::spikes
                                      : supported_append_mode::duplicates);
        if (++index == view_size && circular) index = 0;
    }
    // Each intermediate pending label would be overwritten before the next append.
    if (edge_count > 0)
    {
        auto const next = circular && to_index == edge_count ? 0 : (std::min)(to_index, edge_count - 1);
        out.pending = support_at(next, Reverse);
    }
}

}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian

#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_BORROWED_OUTPUT_HPP
