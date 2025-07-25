// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2015 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2017-2024.
// Modifications copyright (c) 2017-2024 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SORT_ON_SEGMENT_RATIO_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SORT_ON_SEGMENT_RATIO_HPP

#include <cstddef>
#include <algorithm>
#include <map>
#include <set>
#include <vector>

#include <boost/core/addressof.hpp>
#include <boost/range/value_type.hpp>

#include <boost/geometry/algorithms/detail/overlay/copy_segment_point.hpp>
#include <boost/geometry/algorithms/detail/overlay/overlay_type.hpp>
#include <boost/geometry/algorithms/detail/overlay/segment_identifier.hpp>
#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>
#include <boost/geometry/strategies/side.hpp>

namespace boost { namespace geometry
{

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{

// Wraps "turn_operation" from turn_info.hpp,
// giving it extra information, necessary for sorting
template <typename TurnOperation>
struct indexed_turn_operation
{
    using type = TurnOperation;

    std::size_t turn_index;
    std::size_t operation_index;
    // use pointers to avoid copies, const& is not possible because of usage in vector
    segment_identifier const* other_seg_id; // segment id of other segment of intersection of two segments
    TurnOperation const* subject;
    bool discarded{false};
    bool skip{false};

    inline indexed_turn_operation(std::size_t ti, std::size_t oi,
                TurnOperation const& sub,
                segment_identifier const& oid,
                bool dc = false)
        : turn_index(ti)
        , operation_index(oi)
        , other_seg_id(&oid)
        , subject(boost::addressof(sub))
        , discarded(dc)
    {}

};

template
<
    typename Turns,
    typename Indexed,
    typename Geometry1, typename Geometry2,
    typename Strategy,
    bool Reverse1, bool Reverse2
>
struct less_by_segment_ratio
{
    inline less_by_segment_ratio(Turns const& turns
            , Geometry1 const& geometry1
            , Geometry2 const& geometry2
            , Strategy const& strategy)
        : m_turns(turns)
        , m_geometry1(geometry1)
        , m_geometry2(geometry2)
        , m_strategy(strategy)
    {
    }

private :

    Turns const& m_turns;
    Geometry1 const& m_geometry1;
    Geometry2 const& m_geometry2;
    Strategy const& m_strategy;

    using point_type = geometry::point_type_t<Geometry1>;

    inline bool default_order(Indexed const& left, Indexed const& right) const
    {
        // We've nothing to sort on. Take the indexes
        return left.turn_index < right.turn_index;
    }

    inline bool consider_relative_order(Indexed const& left,
                    Indexed const& right) const
    {
        point_type pi, pj, ri, rj, si, sj;

        geometry::copy_segment_points<Reverse1, Reverse2>(m_geometry1, m_geometry2,
            left.subject->seg_id,
            pi, pj);
        geometry::copy_segment_points<Reverse1, Reverse2>(m_geometry1, m_geometry2,
            *left.other_seg_id,
            ri, rj);
        geometry::copy_segment_points<Reverse1, Reverse2>(m_geometry1, m_geometry2,
            *right.other_seg_id,
            si, sj);

        auto side_strategy = m_strategy.side();
        int const side_rj_p = side_strategy.apply(pi, pj, rj);
        int const side_sj_p = side_strategy.apply(pi, pj, sj);

        // Put the one turning left (1; right == -1) as last
        if (side_rj_p != side_sj_p)
        {
            return side_rj_p < side_sj_p;
        }

        int const side_sj_r = side_strategy.apply(ri, rj, sj);
        int const side_rj_s = side_strategy.apply(si, sj, rj);

        // If they both turn left: the most left as last
        // If they both turn right: this is not relevant, but take also here most left
        if (side_rj_s != side_sj_r)
        {
            return side_rj_s < side_sj_r;
        }

        return default_order(left, right);
    }


public :

    // Note that left/right do NOT correspond to m_geometry1/m_geometry2
    // but to the "indexed_turn_operation"
    inline bool operator()(Indexed const& left, Indexed const& right) const
    {
        if (! (left.subject->seg_id == right.subject->seg_id))
        {
            return left.subject->seg_id < right.subject->seg_id;
        }

        // Both left and right are located on the SAME segment.

        if (! (left.subject->fraction == right.subject->fraction))
        {
            return left.subject->fraction < right.subject->fraction;
        }

        auto const& left_turn = m_turns[left.turn_index];
        auto const& right_turn = m_turns[right.turn_index];

        // First check "real" intersection (crosses)
        // -> distance zero due to precision, solve it by sorting
        if (left_turn.method == method_crosses
            && right_turn.method == method_crosses)
        {
            return consider_relative_order(left, right);
        }

        bool const left_both_xx = left_turn.both(operation_blocked);
        bool const right_both_xx = right_turn.both(operation_blocked);
        if (left_both_xx && ! right_both_xx)
        {
            return true;
        }
        if (! left_both_xx && right_both_xx)
        {
            return false;
        }

        bool const left_both_uu = left_turn.both(operation_union);
        bool const right_both_uu = right_turn.both(operation_union);
        if (left_both_uu && ! right_both_uu)
        {
            return true;
        }
        if (! left_both_uu && right_both_uu)
        {
            return false;
        }

        return default_order(left, right);
    }
};


}} // namespace detail::overlay
#endif //DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SORT_ON_SEGMENT_RATIO_HPP
