// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2012-2014 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_GET_PIECE_TURNS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_GET_PIECE_TURNS_HPP

#include <boost/geometry/algorithms/expand.hpp>
#include <boost/geometry/algorithms/detail/disjoint/box_box.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace buffer
{


struct piece_get_box
{
    template <typename Box, typename Piece>
    static inline void apply(Box& total, Piece const& piece)
    {
        geometry::expand(total, piece.robust_envelope);
    }
};

struct piece_ovelaps_box
{
    template <typename Box, typename Piece>
    static inline bool apply(Box const& box, Piece const& piece)
    {
        return ! geometry::detail::disjoint::disjoint_box_box(box, piece.robust_envelope);
    }
};

template <typename Calculator, typename Piece>
class piece_turn_visitor
{
    Calculator& m_calculator;
    int m_last_piece_index;

    inline bool is_neighbor(Piece const& piece1, Piece const& piece2) const
    {
        if (piece1.first_seg_id.multi_index != piece2.first_seg_id.multi_index)
        {
            return false;
        }

        if (std::abs(piece1.index - piece2.index) == 1)
        {
            return true;
        }

        return (piece1.index == 0 && piece2.index == m_last_piece_index)
            || (piece1.index == m_last_piece_index && piece2.index == 0)
            ;
    }

    inline bool skip_neighbor(Piece const& piece1, Piece const& piece2) const
    {
        return piece1.type != piece2.type && is_neighbor(piece1, piece2);
    }

public:
    piece_turn_visitor(Calculator& calculator, int last_piece_index)
        : m_calculator(calculator)
        , m_last_piece_index(last_piece_index)
    {}

    inline void apply(Piece const& piece1, Piece const& piece2,
                    bool first = true) const
    {
        boost::ignore_unused_variable_warning(first);
        if (! skip_neighbor(piece1, piece2))
        {
            m_calculator.calculate_turns(piece1, piece2);
        }
    }
};


}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_GET_PIECE_TURNS_HPP
