// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2012-2014 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_TURN_IN_PIECE_VISITOR
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_TURN_IN_PIECE_VISITOR

#include <boost/range.hpp>

#include <boost/geometry/arithmetic/dot_product.hpp>
#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/expand.hpp>
#include <boost/geometry/algorithms/detail/disjoint/point_box.hpp>
#include <boost/geometry/algorithms/detail/overlay/segment_identifier.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_turn_info.hpp>
#include <boost/geometry/policies/compare.hpp>
#include <boost/geometry/strategies/buffer.hpp>

#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/algorithms/comparable_distance.hpp>

namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace buffer
{


struct turn_get_box
{
    template <typename Box, typename Turn>
    static inline void apply(Box& total, Turn const& turn)
    {
        geometry::expand(total, turn.robust_point);
    }
};

struct turn_ovelaps_box
{
    template <typename Box, typename Turn>
    static inline bool apply(Box const& box, Turn const& turn)
    {
        return ! dispatch::disjoint
            <
                typename Turn::robust_point_type,
                Box
            >::apply(turn.robust_point, box);
    }
};


enum analyse_result
{
    analyse_unknown,
    analyse_continue,
    analyse_on_offsetted,
    analyse_near_offsetted
};

class analyse_turn_wrt_piece
{
    template <typename Point>
    static inline analyse_result check_segment(Point const& previous, Point const& current, Point const& point)
    {
        typedef typename strategy::side::services::default_strategy
            <
                typename cs_tag<Point>::type
            >::type side_strategy;
        typedef typename geometry::coordinate_type<Point>::type coordinate_type;
        typedef geometry::model::box<Point> box_type;


        // Get its box (TODO: this can be prepared-on-demand later)
        box_type box;
        bg::assign_inverse(box);
        bg::expand(box, previous);
        bg::expand(box, current);

        coordinate_type const twice_area
            = side_strategy::template side_value
                <
                    coordinate_type,
                    coordinate_type
                >(previous, current, point);

        if (twice_area == 0)
        {
            // Collinear, only on segment if it is covered by its bbox
            if (geometry::covered_by(point, box))
            {
                return analyse_on_offsetted;
            }
        }

        if (twice_area < 0 && geometry::covered_by(point, box))
        {
            // It is in the triangle right-of the segment where the
            // segment is the hypothenusa. Check if it is close
            // (within rounding-area)
            if (twice_area * twice_area < bg::comparable_distance(previous, current))
            {
                return analyse_near_offsetted;
            }
        }
//        if (twice_area > 0)
//        {
//            // Left of segment
//            // TODO: use within state here
//        }
        return analyse_continue;
    }

public :
    template <typename Point, typename Piece>
    static inline analyse_result apply(Point const& point, Piece const& piece)
    {
        // TODO: we will check first helper-segments here, if it is left of
        // any helper segment we're done (assuming joins stay within the area
        // of the helper-segments, even if they have concavities there

        geometry::equal_to<Point> comparator;

        for (int i = 1; i < piece.offsetted_count; i++)
        {
            Point const& previous = piece.robust_ring[i - 1];
            Point const& current = piece.robust_ring[i];

            // The robust ring can contain duplicates
            // (on which any side or side-value would return 0)
            if (! comparator(previous, current))
            {
                analyse_result code = check_segment(previous, current, point);
                if (code != analyse_continue)
                {
                    return code;
                }
            }
        }
        return analyse_unknown; // Not collinear or very close
    }

};


template <typename Turns, typename Pieces>
class turn_in_piece_visitor
{
    Turns& m_turns; // because partition is currently operating on const input only
    Pieces const& m_pieces; // to check for piece-type

public:

    inline turn_in_piece_visitor(Turns& turns, Pieces const& pieces)
        : m_turns(turns)
        , m_pieces(pieces)
    {}

    template <typename Turn, typename Piece>
    inline void apply(Turn const& turn, Piece const& piece, bool first = true)
    {
        boost::ignore_unused_variable_warning(first);

        if (turn.count_within > 0)
        {
            // Already inside - no need to check again
            return;
        }

        if (piece.type == strategy::buffer::buffered_flat_end
            || piece.type == strategy::buffer::buffered_concave)
        {
            // Turns cannot be inside a flat end (though they can be on border)
            // Neither we need to check if they are inside concave helper pieces
            return;
        }

        if (! geometry::covered_by(turn.robust_point, piece.robust_envelope))
        {
            // Easy check: if the turn is not in the envelope, we can safely return
            return;
        }

        // TEMPORARY solution to enable one-sided buffer.
        // TODO: this will replaced by analyse_piece
        bool has_concave = false;

        bool neighbour = false;
        for (int i = 0; i < 2; i++)
        {
            // Don't compare against one of the two source-pieces
            if (turn.operations[i].piece_index == piece.index)
            {
                return;
            }

            Piece const& pc = m_pieces[turn.operations[i].piece_index];

            if (pc.type == strategy::buffer::buffered_concave)
            {
                has_concave = true;
            }

            if (pc.left_index == piece.index
                || pc.right_index == piece.index)
            {
                if (pc.type == strategy::buffer::buffered_flat_end)
                {
                    // If it is a flat end, don't compare against its neighbor:
                    // it will always be located on one of the helper segments
                    return;
                }
                if (pc.type == strategy::buffer::buffered_concave)
                {
                    // If it is concave, the same applies: the IP will be
                    // located on one of the helper segments
                    return;
                }
                neighbour = true;
            }
        }

        // TODO: this point_in_geometry is a performance-bottleneck here and
        // will be replaced completely by extending analyse_piece functionality
        int geometry_code = detail::within::point_in_geometry(turn.robust_point, piece.robust_ring);

        if (geometry_code == -1)
        {
            // Outside, always return
            return;
        }

        if (geometry_code == 0 && neighbour)
        {
            // The IP falling on the border of its neighbour is a normal situation
            return;
        }

        // TODO: mutable_piece to make some on-demand preparations in analyse
        analyse_result analyse_code
            = analyse_turn_wrt_piece::apply(turn.robust_point, piece);

        Turn& mutable_turn = m_turns[turn.turn_index];

        switch(analyse_code)
        {
            case analyse_on_offsetted :
                mutable_turn.count_on_offsetted++; // value is not used anymore
                break;
            case analyse_near_offsetted :
                if (geometry_code == 1)
                {
                    mutable_turn.count_within_near_offsetted++;
                }
                break;
            default :
                if (geometry_code == 1 || (geometry_code == 0 && ! has_concave))
                {
                    mutable_turn.count_within++;
                }

                break;
        }
    }
};


}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_TURN_IN_PIECE_VISITOR
