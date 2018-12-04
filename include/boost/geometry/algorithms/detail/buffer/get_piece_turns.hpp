// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2012-2014 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2017 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2017.
// Modifications copyright (c) 2017 Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_GET_PIECE_TURNS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_GET_PIECE_TURNS_HPP

#include <boost/core/ignore_unused.hpp>
#include <boost/range.hpp>

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/expand.hpp>
#include <boost/geometry/algorithms/detail/disjoint/box_box.hpp>
#include <boost/geometry/algorithms/detail/overlay/segment_identifier.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_turn_info.hpp>
#include <boost/geometry/algorithms/detail/sections/section_functions.hpp>
#include <boost/geometry/algorithms/detail/buffer/buffer_policies.hpp>


namespace boost { namespace geometry
{


#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace buffer
{


#if defined(BOOST_GEOMETRY_BUFFER_USE_SIDE_OF_INTERSECTION)
struct buffer_assign_turn
{
    static bool const include_no_turn = false;
    static bool const include_degenerate = false;
    static bool const include_opposite = false;

    template
    <
        typename Info,
        typename Point1,
        typename Point2,
        typename IntersectionInfo
    >
    static inline void apply(Info& info,
                             Point1 const& /*p1*/,
                             Point2 const& /*p2*/,
                             IntersectionInfo const& iinfo)
    {
        info.rob_pi = iinfo.rpi();
        info.rob_pj = iinfo.rpj();
        info.rob_qi = iinfo.rqi();
        info.rob_qj = iinfo.rqj();
    }

};
#endif

template <typename Ring>
struct unique_sub_range_from_piece
{
    typedef typename boost::range_iterator<Ring const>::type iterator_type;
    typedef typename geometry::point_type<Ring const>::type point_type;

    unique_sub_range_from_piece(Ring const& ring, iterator_type it0, iterator_type it1)
        : m_ring(ring)
        , m_point_i(*it0)
        , m_point_j(*it1)
        , m_iterator(it1)
        , m_point_retrieved(false)
    {}

    static inline bool is_first() { return false; }

    static inline std::size_t size() { return 3u; }

    inline point_type const& at(std::size_t index) const
    {
        switch (index)
        {
            case 0 : return m_point_i;
            case 1 : return m_point_j;
            case 2 : return get_next_point();
            default : return m_point_i;
        }
    }

private :

    inline point_type const& get_next_point() const
    {
        if (! m_point_retrieved)
        {
            m_point = *next_point(m_iterator);
            m_point_retrieved = true;
        }
        return m_point;
    }

private :

    inline void circular_move_along_ring(iterator_type& next) const
    {
        ++next;
        if (next == boost::end(m_ring))
        {
            next = boost::begin(m_ring) + 1;
        }
    }

    inline iterator_type next_point(iterator_type it) const
    {
        iterator_type result = it;
        circular_move_along_ring(result);

        // TODO: we could also use piece-boundaries
        // to check if the point equals the last one
        while (geometry::equals(*it, *result))
        {
            circular_move_along_ring(result);
        }
        return result;
    }

    Ring const& m_ring;
    point_type m_point_i;
    point_type m_point_j;
    iterator_type m_iterator;

    mutable point_type m_point;
    mutable bool m_point_retrieved;
};

template
<
    typename Pieces,
    typename Rings,
    typename Turns,
    typename IntersectionStrategy,
    typename RobustPolicy
>
class piece_turn_visitor
{
    Pieces const& m_pieces;
    Rings const& m_rings;
    Turns& m_turns;
    IntersectionStrategy const& m_intersection_strategy;
    RobustPolicy const& m_robust_policy;

    template <typename Piece>
    inline bool is_adjacent(Piece const& piece1, Piece const& piece2) const
    {
        if (piece1.first_seg_id.multi_index != piece2.first_seg_id.multi_index)
        {
            return false;
        }

        return piece1.index == piece2.left_index
            || piece1.index == piece2.right_index;
    }

    template <typename Piece>
    inline bool is_on_same_convex_ring(Piece const& piece1, Piece const& piece2) const
    {
        if (piece1.first_seg_id.multi_index != piece2.first_seg_id.multi_index)
        {
            return false;
        }

        return ! m_rings[piece1.first_seg_id.multi_index].has_concave;
    }


    template <std::size_t Dimension, typename Iterator, typename Box>
    inline void move_begin_iterator(Iterator& it_begin, Iterator it_beyond,
                                    signed_size_type& index, int dir,
                                    Box const& this_bounding_box,
                                    Box const& other_bounding_box)
    {
        for(; it_begin != it_beyond
                && it_begin + 1 != it_beyond
                && detail::section::preceding<Dimension>(dir, *(it_begin + 1),
                                                         this_bounding_box,
                                                         other_bounding_box,
                                                         m_robust_policy);
            ++it_begin, index++)
        {}
    }

    template <std::size_t Dimension, typename Iterator, typename Box>
    inline void move_end_iterator(Iterator it_begin, Iterator& it_beyond,
                                  int dir, Box const& this_bounding_box,
                                  Box const& other_bounding_box)
    {
        while (it_beyond != it_begin
            && it_beyond - 1 != it_begin
            && it_beyond - 2 != it_begin)
        {
            if (detail::section::exceeding<Dimension>(dir, *(it_beyond - 2),
                        this_bounding_box, other_bounding_box, m_robust_policy))
            {
                --it_beyond;
            }
            else
            {
                return;
            }
        }
    }

    template <typename Piece, typename Section>
    inline void calculate_turns(Piece const& piece1, Piece const& piece2,
        Section const& section1, Section const& section2)
    {
        typedef typename boost::range_value<Rings const>::type ring_type;
        typedef typename boost::range_value<Turns const>::type turn_type;
        typedef typename boost::range_iterator<ring_type const>::type iterator;

        signed_size_type const piece1_first_index = piece1.first_seg_id.segment_index;
        signed_size_type const piece2_first_index = piece2.first_seg_id.segment_index;
        if (piece1_first_index < 0 || piece2_first_index < 0)
        {
            return;
        }

        // Get indices of part of offsetted_rings for this monotonic section:
        signed_size_type const sec1_first_index = piece1_first_index + section1.begin_index;
        signed_size_type const sec2_first_index = piece2_first_index + section2.begin_index;

        // index of last point in section, beyond-end is one further
        signed_size_type const sec1_last_index = piece1_first_index + section1.end_index;
        signed_size_type const sec2_last_index = piece2_first_index + section2.end_index;

        // get geometry and iterators over these sections
        ring_type const& ring1 = m_rings[piece1.first_seg_id.multi_index];
        iterator it1_first = boost::begin(ring1) + sec1_first_index;
        iterator it1_beyond = boost::begin(ring1) + sec1_last_index + 1;

        ring_type const& ring2 = m_rings[piece2.first_seg_id.multi_index];
        iterator it2_first = boost::begin(ring2) + sec2_first_index;
        iterator it2_beyond = boost::begin(ring2) + sec2_last_index + 1;

        // Set begin/end of monotonic ranges, in both x/y directions
        signed_size_type index1 = sec1_first_index;
        move_begin_iterator<0>(it1_first, it1_beyond, index1,
                    section1.directions[0], section1.bounding_box, section2.bounding_box);
        move_end_iterator<0>(it1_first, it1_beyond,
                    section1.directions[0], section1.bounding_box, section2.bounding_box);
        move_begin_iterator<1>(it1_first, it1_beyond, index1,
                    section1.directions[1], section1.bounding_box, section2.bounding_box);
        move_end_iterator<1>(it1_first, it1_beyond,
                    section1.directions[1], section1.bounding_box, section2.bounding_box);

        signed_size_type index2 = sec2_first_index;
        move_begin_iterator<0>(it2_first, it2_beyond, index2,
                    section2.directions[0], section2.bounding_box, section1.bounding_box);
        move_end_iterator<0>(it2_first, it2_beyond,
                    section2.directions[0], section2.bounding_box, section1.bounding_box);
        move_begin_iterator<1>(it2_first, it2_beyond, index2,
                    section2.directions[1], section2.bounding_box, section1.bounding_box);
        move_end_iterator<1>(it2_first, it2_beyond,
                    section2.directions[1], section2.bounding_box, section1.bounding_box);

        turn_type the_model;
        the_model.operations[0].piece_index = piece1.index;
        the_model.operations[0].seg_id = piece1.first_seg_id;
        the_model.operations[0].seg_id.segment_index = index1; // override

        iterator it1 = it1_first;
        for (iterator prev1 = it1++;
                it1 != it1_beyond;
                prev1 = it1++, the_model.operations[0].seg_id.segment_index++)
        {
            the_model.operations[1].piece_index = piece2.index;
            the_model.operations[1].seg_id = piece2.first_seg_id;
            the_model.operations[1].seg_id.segment_index = index2; // override

            unique_sub_range_from_piece<ring_type> unique_sub_range1(ring1, prev1, it1);

            iterator it2 = it2_first;
            for (iterator prev2 = it2++;
                    it2 != it2_beyond;
                    prev2 = it2++, the_model.operations[1].seg_id.segment_index++)
            {
                unique_sub_range_from_piece<ring_type> unique_sub_range2(ring2, prev2, it2);

                // TODO: internally get_turn_info calculates robust points.
                // But they are already calculated.
                // We should be able to use them.
                // this means passing them to this visitor,
                // and iterating in sync with them...
                typedef detail::overlay::get_turn_info
                    <
#if defined(BOOST_GEOMETRY_BUFFER_USE_SIDE_OF_INTERSECTION)
                        buffer_assign_turn
#else
                        detail::overlay::assign_null_policy
#endif
                    > turn_policy;


                turn_policy::apply(the_model,
                                   m_intersection_strategy,
                                   unique_sub_range1, unique_sub_range2,
                                   m_robust_policy,
                                   std::back_inserter(m_turns));
            }
        }
    }

public:

    piece_turn_visitor(Pieces const& pieces,
            Rings const& ring_collection,
            Turns& turns,
            IntersectionStrategy const& intersection_strategy,
            RobustPolicy const& robust_policy)
        : m_pieces(pieces)
        , m_rings(ring_collection)
        , m_turns(turns)
        , m_intersection_strategy(intersection_strategy)
        , m_robust_policy(robust_policy)
    {}

    template <typename Section>
    inline bool apply(Section const& section1, Section const& section2,
                    bool first = true)
    {
        boost::ignore_unused(first);

        typedef typename boost::range_value<Pieces const>::type piece_type;
        piece_type const& piece1 = m_pieces[section1.ring_id.source_index];
        piece_type const& piece2 = m_pieces[section2.ring_id.source_index];

        if ( piece1.index == piece2.index
          || is_adjacent(piece1, piece2)
          || is_on_same_convex_ring(piece1, piece2)
          || detail::disjoint::disjoint_box_box(section1.bounding_box,
                                                section2.bounding_box) )
        {
            return true;
        }

        calculate_turns(piece1, piece2, section1, section2);

        return true;
    }
};


}} // namespace detail::buffer
#endif // DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_GET_PIECE_TURNS_HPP
