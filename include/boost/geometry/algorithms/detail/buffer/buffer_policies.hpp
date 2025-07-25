// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2012-2014 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2017-2024.
// Modifications copyright (c) 2017-2024, Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_BUFFER_POLICIES_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_BUFFER_POLICIES_HPP

#include <cstddef>

#include <boost/range/value_type.hpp>

#include <boost/geometry/core/coordinate_type.hpp>
#include <boost/geometry/core/point_type.hpp>

#include <boost/geometry/algorithms/disjoint.hpp>
#include <boost/geometry/algorithms/expand.hpp>
#include <boost/geometry/algorithms/detail/overlay/traversal_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>

#include <boost/geometry/strategies/buffer.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace buffer
{

struct buffer_overlay_visitor
{
public :
    template <typename Turns>
    void visit_turns(int , Turns const& ) {}

    template <typename Clusters, typename Turns>
    void visit_clusters(Clusters const& , Turns const& ) {}

    template <typename Turns, typename Turn, typename Operation>
    void visit_traverse(Turns const& /*turns*/, Turn const& /*turn*/, Operation const& /*op*/, const char* /*header*/)
    {
    }

    template <typename Rings>
    void visit_generated_rings(Rings const& )
    {}
};


// Should follow traversal-turn-concept (enrichment, visit structure)
// and adds index in piece vector to find it back
template <typename Point, typename SegmentRatio>
struct buffer_turn_operation
    : public detail::overlay::traversal_turn_operation<Point, SegmentRatio>
{
    signed_size_type piece_index;
    signed_size_type index_in_ring;

    inline buffer_turn_operation()
        : piece_index(-1)
        , index_in_ring(-1)
    {}
};

// Version of turn_info for buffer with its turn index and other helper variables
template <typename Point, typename SegmentRatio>
struct buffer_turn_info
    : public detail::overlay::turn_info
        <
            Point,
            SegmentRatio,
            buffer_turn_operation<Point, SegmentRatio>
        >
{
    using point_type = Point;

    std::size_t turn_index;

    // Information if turn can be used. It is not traversable if it is within
    // another piece, or within the original (depending on deflation),
    // or (for deflate) if there are not enough points to traverse it.

    bool is_linear_end_point;
    bool within_original;
    signed_size_type count_in_original; // increased by +1 for in ext.ring, -1 for int.ring

    inline buffer_turn_info()
        : turn_index(0)
        , is_linear_end_point(false)
        , within_original(false)
        , count_in_original(0)
    {}
};

template <typename Strategy>
struct piece_get_box
{
    explicit piece_get_box(Strategy const& strategy)
        : m_strategy(strategy)
    {}

    template <typename Box, typename Piece>
    inline void apply(Box& total, Piece const& piece) const
    {
        assert_coordinate_type_equal(total, piece.m_piece_border.m_envelope);

        if (piece.m_piece_border.m_has_envelope)
        {
            geometry::expand(total, piece.m_piece_border.m_envelope,
                             m_strategy);
        }
    }

    Strategy const& m_strategy;
};

template <typename Strategy>
struct piece_overlaps_box
{
    explicit piece_overlaps_box(Strategy const& strategy)
        : m_strategy(strategy)
    {}

    template <typename Box, typename Piece>
    inline bool apply(Box const& box, Piece const& piece) const
    {
        assert_coordinate_type_equal(box, piece.m_piece_border.m_envelope);

        if (piece.type == strategy::buffer::buffered_flat_end
            || piece.type == strategy::buffer::buffered_concave)
        {
            // Turns cannot be inside a flat end (though they can be on border)
            // Neither we need to check if they are inside concave helper pieces

            // Skip all pieces not used as soon as possible
            return false;
        }
        if (! piece.m_piece_border.m_has_envelope)
        {
            return false;
        }

        return ! geometry::detail::disjoint::disjoint_box_box(box, piece.m_piece_border.m_envelope,
                                                              m_strategy);
    }

    Strategy const& m_strategy;
};

template <typename Strategy>
struct turn_get_box
{
    explicit turn_get_box(Strategy const& strategy)
        : m_strategy(strategy)
    {}

    template <typename Box, typename Turn>
    inline void apply(Box& total, Turn const& turn) const
    {
        assert_coordinate_type_equal(total, turn.point);
        geometry::expand(total, turn.point, m_strategy);
    }

    Strategy const& m_strategy;
};

template <typename Strategy>
struct turn_overlaps_box
{
    explicit turn_overlaps_box(Strategy const& strategy)
        : m_strategy(strategy)
    {}

    template <typename Box, typename Turn>
    inline bool apply(Box const& box, Turn const& turn) const
    {
        assert_coordinate_type_equal(turn.point, box);
        return ! geometry::detail::disjoint::disjoint_point_box(turn.point, box,
                                                                m_strategy);
    }

    Strategy const& m_strategy;
};

}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_BUFFER_POLICIES_HPP
