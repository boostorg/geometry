// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2012-2020 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2017 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2016-2022.
// Modifications copyright (c) 2016-2022 Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_TURN_IN_PIECE_VISITOR_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_TURN_IN_PIECE_VISITOR_HPP

#include <type_traits>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/assert.hpp>
#include <boost/geometry/core/config.hpp>

#include <boost/geometry/algorithms/comparable_distance.hpp>
#include <boost/geometry/algorithms/covered_by.hpp>
#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/detail/disjoint/point_box.hpp>
#include <boost/geometry/algorithms/detail/disjoint/box_box.hpp>
#include <boost/geometry/algorithms/detail/dummy_geometries.hpp>
#include <boost/geometry/algorithms/detail/buffer/buffer_policies.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/strategies/cartesian/buffer_box.hpp>


namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL

namespace detail { namespace buffer
{

template
<
    typename CsTag,
    typename Turns,
    typename Clusters,
    typename Pieces,
    typename DistanceStrategy,
    typename UmbrellaStrategy

>
class turn_in_piece_visitor
{
    Turns& m_turns; // because partition is currently operating on const input only
    Clusters const& m_clusters;
    Pieces const& m_pieces; // to check for piece-type
    DistanceStrategy const& m_distance_strategy; // to check if point is on original or one_sided
    UmbrellaStrategy const& m_umbrella_strategy;

    template <typename Point>
    static inline bool same_vertex(Point const& a, Point const& b)
    {
        return geometry::get<0>(a) == geometry::get<0>(b)
            && geometry::get<1>(a) == geometry::get<1>(b);
    }

    template <typename Operation, typename Piece>
    static inline bool on_original_side(Operation const& op, Piece const& empty_side,
                                        Piece const& segment)
    {
        auto const& original = segment.m_piece_border;
        auto const* ring = empty_side.m_piece_border.m_ring;
        if (original.m_original_size != 2 || ring == nullptr)
        {
            return false;
        }
        auto const index = op.seg_id.segment_index;
        auto const& from = (*ring)[index];
        auto const& to = (*ring)[index + 1];
        auto const& a = original.m_originals[0];
        auto const& b = original.m_originals[1];
        return (same_vertex(from, a) && same_vertex(to, b))
            || (same_vertex(from, b) && same_vertex(to, a));
    }

    template <typename Operation, typename Piece>
    inline bool skip(Operation const& op, Piece const& piece) const
    {
        if (op.piece_index == piece.index)
        {
            return true;
        }
        Piece const& pc = m_pieces[op.piece_index];
        if (std::is_same<CsTag, cartesian_tag>::value
            && pc.type == strategy::buffer::buffered_empty_side
            && piece.type == strategy::buffer::buffered_segment
            && op.seg_id.multi_index == piece.first_seg_id.multi_index
            && on_original_side(op, pc, piece))
        {
            // The empty offset edge is the original border of this strip.
            // Re-testing a rounded intersection against that border can put
            // it inside, although the turn is incident to the border.
            return true;
        }
        if (pc.left_index == piece.index || pc.right_index == piece.index)
        {
            if (pc.type == strategy::buffer::buffered_flat_end)
            {
                // If it is a flat end, don't compare against its neighbor:
                // it will always be located on one of the helper segments
                return true;
            }
            if (pc.type == strategy::buffer::buffered_concave)
            {
                // If it is concave, the same applies: the IP will be
                // located on one of the helper segments
                return true;
            }
        }

        return false;
    }

    // Returns true if the turn is part of a cluster, and one of the other turns in the same
    // cluster is involved in the same piece as the operations in the turn.
    template <typename Turn, typename Piece>
    inline bool skip_by_same_cluster(Turn const& turn, Piece const& piece) const
    {
        auto it = m_clusters.find(turn.cluster_id);
        if (it == m_clusters.end())
        {
            return false;
        }

        for (auto const& index : it->second.turn_indices)
        {
            if (index == static_cast<signed_size_type>(turn.turn_index))
            {
                continue;
            }
            auto const& other_turn = m_turns[index];
            auto const& seg_id0 = other_turn.operations[0].seg_id;
            auto const& seg_id1 = other_turn.operations[1].seg_id;

            if (seg_id0.piece_index == piece.index
                || seg_id1.piece_index == piece.index)
            {
                // One of the other turns in the same cluster is an intersection
                // with the same piece.
                // Therefore, the turn under inspection cannot be within that piece.
                return true;
            }
        }
        return false;
    }

    template <typename Point, typename Piece, typename Tag>
    static bool on_flat_cap(Point const&, Piece const&, Tag)
    {
        return false;
    }

    template <typename Point, typename Piece>
    bool on_flat_cap(Point const& p, Piece const& neighbor, cartesian_tag) const
    {
        auto const& cap = neighbor.m_piece_border;
        if (neighbor.type != strategy::buffer::buffered_flat_end || cap.m_end - cap.m_begin != 2)
        {
            return false;
        }
        auto const& a = (*cap.m_ring)[cap.m_begin];
        auto const& b = (*cap.m_ring)[cap.m_end - 1];
        return strategy::buffer::cartesian_buffer_detail::cartesian_buffer_box::locate(p, a, b) >= 0
            && m_umbrella_strategy.side().apply(a, b, p) == 0;
    }

    template <typename NumericType>
    inline bool is_one_sided(NumericType const& left, NumericType const& right) const
    {
        static NumericType const zero = 0;
        return geometry::math::equals(left, zero)
            || geometry::math::equals(right, zero);
    }

    template <typename Point>
    inline bool has_zero_distance_at(Point const& point) const
    {
        return is_one_sided(m_distance_strategy.apply(point, point,
                strategy::buffer::buffer_side_left),
            m_distance_strategy.apply(point, point,
                strategy::buffer::buffer_side_right));
    }

public:

    inline turn_in_piece_visitor(Turns& turns, Clusters const& clusters, Pieces const& pieces,
                                 DistanceStrategy const& distance_strategy,
                                 UmbrellaStrategy const& umbrella_strategy)
        : m_turns(turns)
        , m_clusters(clusters)
        , m_pieces(pieces)
        , m_distance_strategy(distance_strategy)
        , m_umbrella_strategy(umbrella_strategy)
    {}

    template <typename Turn, typename Piece>
    inline bool apply(Turn const& turn, Piece const& piece)
    {
        if (! turn.is_traversable)
        {
            // Already handled
            return true;
        }

        if (piece.type == strategy::buffer::buffered_flat_end
            || piece.type == strategy::buffer::buffered_concave)
        {
            // Turns cannot be located within flat-end or concave pieces
            return true;
        }

        if (skip(turn.operations[0], piece) || skip(turn.operations[1], piece))
        {
            return true;
        }

        return apply(turn, piece, piece.m_piece_border);
    }

    template <typename Turn, typename Piece, typename Border>
    inline bool apply(Turn const& turn, Piece const& piece, Border const& border)
    {
        if (! geometry::covered_by(turn.point, border.m_envelope, m_umbrella_strategy))
        {
            // Easy check: if turn is not in the (expanded) envelope
            return true;
        }

        if (piece.type == geometry::strategy::buffer::buffered_empty_side)
        {
            return true;
        }

        if (skip_by_same_cluster(turn, piece))
        {
            return true;
        }

        if (piece.type == geometry::strategy::buffer::buffered_point && ! border.m_rectangular_offset)
        {
            // Optimization for a buffer around points: if distance from center
            // is not between min/max radius, it is either inside or outside,
            // and more expensive checks are not necessary.
            auto const d = geometry::comparable_distance(piece.m_center, turn.point,
                                                         m_umbrella_strategy);

            if (d < border.m_min_comparable_radius)
            {
                Turn& mutable_turn = m_turns[turn.turn_index];
                mutable_turn.is_traversable = false;
                return true;
            }
            if (d > border.m_max_comparable_radius)
            {
                return true;
            }
        }

        // Check if buffer is one-sided (at this point), because then a point
        // on the original border is not considered as within.
        bool const one_sided = has_zero_distance_at(turn.point);
        // Internal spikes also create flat caps, without setting the flags
        // used for the first and last pieces of a linestring.
        bool const is_flat_start = piece.is_flat_start
            || m_pieces[piece.left_index].type == strategy::buffer::buffered_flat_end;
        bool const is_flat_end = piece.is_flat_end
            || m_pieces[piece.right_index].type == strategy::buffer::buffered_flat_end;
        auto const exposed_original = [&](auto const& original, Piece const& neighbor)
        {
            // A reversal need not be a linestring endpoint. Its original vertex
            // is exposed if it lies on the actual straight cap of this piece.
            return geometry::equals(turn.point, original, m_umbrella_strategy)
                && (turn.is_linear_end_point || on_flat_cap(turn.point, neighbor, CsTag{}));
        };
        bool const is_exposed_original = border.m_original_size > 0
            && ((is_flat_start && exposed_original(border.m_originals[border.m_original_size - 1],
                                                   m_pieces[piece.left_index]))
                || (is_flat_end && exposed_original(border.m_originals[0],
                                                    m_pieces[piece.right_index])));

        bool const is_exposed_start = is_flat_start
            || m_pieces[piece.left_index].type == strategy::buffer::buffered_concave;
        bool const is_exposed_end = is_flat_end
            || m_pieces[piece.right_index].type == strategy::buffer::buffered_concave;

        typename Border::state_type state;
        if (! border.point_on_piece(turn.point, one_sided,
                                    is_exposed_original, state,
                                    is_exposed_start, is_exposed_end))
        {
            return true;
        }

        if (state.is_inside() && ! state.is_on_boundary())
        {
            Turn& mutable_turn = m_turns[turn.turn_index];
            mutable_turn.is_traversable = false;
        }

        return true;
    }
};


}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_TURN_IN_PIECE_VISITOR_HPP
