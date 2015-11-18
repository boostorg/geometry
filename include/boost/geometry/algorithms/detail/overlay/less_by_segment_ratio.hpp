// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2015 Barend Gehrels, Amsterdam, the Netherlands.

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

#include <boost/range.hpp>

#include <boost/geometry/algorithms/detail/overlay/copy_segment_point.hpp>
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
    typedef TurnOperation type;

    std::size_t turn_index;
    std::size_t operation_index;
    bool discarded;
    // use pointers to avoid copies, const& is not possible because of usage in vector
    segment_identifier const* other_seg_id; // segment id of other segment of intersection of two segments
    TurnOperation const* subject;

    inline indexed_turn_operation(std::size_t ti, std::size_t oi,
                TurnOperation const& sub,
                segment_identifier const& oid)
        : turn_index(ti)
        , operation_index(oi)
        , discarded(false)
        , other_seg_id(&oid)
        , subject(boost::addressof(sub))
    {}

};

template
<
    typename TurnPoints,
    typename Indexed,
    typename Geometry1, typename Geometry2,
    typename RobustPolicy,
    bool Reverse1, bool Reverse2
>
struct less_by_segment_ratio
{
    inline less_by_segment_ratio(TurnPoints const& turn_points
            , operation_type for_operation
            , Geometry1 const& geometry1
            , Geometry2 const& geometry2
            , RobustPolicy const& robust_policy
            , bool* clustered)
        : m_turn_points(turn_points)
        , m_for_operation(for_operation)
        , m_geometry1(geometry1)
        , m_geometry2(geometry2)
        , m_robust_policy(robust_policy)
        , m_clustered(clustered)
    {
    }

private :

    TurnPoints const& m_turn_points;
    operation_type m_for_operation;
    Geometry1 const& m_geometry1;
    Geometry2 const& m_geometry2;
    RobustPolicy const& m_robust_policy;
    mutable bool* m_clustered;

    typedef typename geometry::point_type<Geometry1>::type point_type;

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

        typedef typename strategy::side::services::default_strategy
            <
                typename cs_tag<point_type>::type
            >::type strategy;

        int const side_rj_p = strategy::apply(pi, pj, rj);
        int const side_sj_p = strategy::apply(pi, pj, sj);

        // Put the one turning left (1; right == -1) as last
        if (side_rj_p != side_sj_p)
        {
            return side_rj_p < side_sj_p;
        }

        int const side_sj_r = strategy::apply(ri, rj, sj);
        int const side_rj_s = strategy::apply(si, sj, rj);

        // If they both turn left: the most left as last
        // If they both turn right: this is not relevant, but take also here most left
        if (side_rj_s != side_sj_r)
        {
            return side_rj_s < side_sj_r;
        }

        return default_order(left, right);
    }

    inline bool select_by_side(bool& result, Indexed const& left, Indexed const& right) const
    {
        typedef typename boost::range_value<TurnPoints>::type turn_type;
        typedef typename turn_type::turn_operation_type turn_op_type;

        turn_type const& left_turn = m_turn_points[left.turn_index];
        turn_type const& right_turn = m_turn_points[right.turn_index];

        turn_op_type const& left_op = left_turn.operations[left.operation_index];
        turn_op_type const& right_op = right_turn.operations[right.operation_index];
        turn_op_type const& left_other_op = left_turn.operations[1 - left.operation_index];
        turn_op_type const& right_other_op = right_turn.operations[1 - right.operation_index];

        // Check, otherwise it would have been sorted on left/right segid
        BOOST_ASSERT(left_op.seg_id == right_op.seg_id);

        point_type p_both1, p_both2, p_both3;
        point_type p_left_o1, p_left_o2, p_left_o3;
        point_type p_right_o1, p_right_o2, p_right_o3;
        geometry::copy_segment_points<Reverse1, Reverse2>(m_geometry1, m_geometry2, left_op.seg_id, p_both1, p_both2, p_both3);
        geometry::copy_segment_points<Reverse1, Reverse2>(m_geometry1, m_geometry2, left_other_op.seg_id, p_left_o1, p_left_o2, p_left_o3);
        geometry::copy_segment_points<Reverse1, Reverse2>(m_geometry1, m_geometry2, right_other_op.seg_id, p_right_o1, p_right_o2, p_right_o3);

        // If the operation is union, get the two union-operations
        point_type p_lhs;
        if (left_op.operation == m_for_operation
                && left_other_op.operation != m_for_operation)
        {
            p_lhs = left_op.fraction.is_one() ? p_both3 : p_both2;
        }
        else if (left_other_op.operation == m_for_operation
                 && left_op.operation  != m_for_operation)
        {
            p_lhs = left_other_op.fraction.is_one() ? p_left_o3 : p_left_o2;
        }
        else
        {
            return false;
        }

        point_type p_rhs;
        if (right_op.operation == m_for_operation
                && right_other_op.operation != m_for_operation)
        {
            p_rhs = right_op.fraction.is_one() ? p_both3 : p_both2;
        }
        else if (right_other_op.operation == m_for_operation
                 && right_op.operation != m_for_operation)
        {
            p_rhs = right_other_op.fraction.is_one() ? p_right_o3 : p_right_o2;
        }
        else
        {
            return false;
        }

        typedef typename strategy::side::services::default_strategy
            <
                typename cs_tag<point_type>::type
            >::type strategy;

        // Check if lhs is opposite from rhs
        //                    |lhs
        //     both===========|
        //                    |rhs
        // In that case, take spatially right for intersection,
        // spatially left for union
        // Taking spatially right means: order that one as LAST
        // such that the other one is taken first, passed through this one

        // Also, either lhs or rhs might continue collinearly


        int const side_lhs = strategy::apply(p_both1, p_both2, p_lhs);
        int const side_rhs = strategy::apply(p_both1, p_both2, p_rhs);

        if (side_lhs * side_rhs == -1)
        {
            // 1,-1 or -1,1
            // Order the one going right first. So if lhs=right, result=true
            result = side_lhs == -1;
            return true;
        }
        else if (side_lhs != 0 && side_rhs == 0)
        {
            // 1,0 or -1,0
            // For union, if lhs goes left, order first
            result = side_lhs == -1;
            return true;
        }
        else if (side_lhs == 0 && side_rhs != 0)
        {
            // 0,1 or 0,-1
            // For union, if rhs goes left, order first so return false
            // to apply the predicate lhs<rhs
            result = side_rhs == 1;
            return true;
        }

        // Check if one union option (of left hand side) is located
        // left (spatially left) of the other union option (of right hand side)
        int const side = strategy::apply(p_both2, p_lhs, p_rhs);
        if (side == 0)
        {
            return false;
        }

        // If lhs is left of rhs, order first, so return true;
        result = side == 1;
        return true;
    }

    // TODO: alternatively we might return a tristate bool but we don' t have a
    // dependency yet on that library
    inline bool select_operation(bool& result,
                                Indexed const& left, Indexed const& right) const
    {
        result = false;
        typedef typename boost::range_value<TurnPoints>::type turn_type;

        turn_type const& left_turn = m_turn_points[left.turn_index];
        turn_type const& right_turn = m_turn_points[right.turn_index];

        // If operation is intersection, and only one of the operations is
        // intersection, that gets priority

        if (left_turn.has(m_for_operation)
            && ! left_turn.both(m_for_operation)
            && ! right_turn.has(m_for_operation))
        {
            // There is one targetted operation in left and nothing in right
            // Order left first, consider as left<right
            result = true;
            return true;
        }

        if (right_turn.has(m_for_operation)
            && ! right_turn.both(m_for_operation)
            && ! left_turn.has(m_for_operation))
        {
            // Order right first, so consider as ! left<right
            result = false;
            return true;
        }

        // In case of an intersection, if both left and right do not contain
        // such an operation, but there is a cc, than prioritize cc
        if (! left_turn.has(m_for_operation)
            && ! right_turn.has(m_for_operation))
        {
            if (left_turn.both(operation_continue))
            {
                result = true;
                return true;
            }
            if (right_turn.both(operation_continue))
            {
                result = false;
                return true;
            }
        }

        return false;
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


        typedef typename boost::range_value<TurnPoints>::type turn_type;
        turn_type const& left_turn = m_turn_points[left.turn_index];
        turn_type const& right_turn = m_turn_points[right.turn_index];

        // First check "real" intersection (crosses)
        // -> distance zero due to precision, solve it by sorting
        if (left_turn.method == method_crosses
            && right_turn.method == method_crosses)
        {
            return consider_relative_order(left, right);
        }

        bool const left_ux = left_turn.combination(operation_blocked, operation_union);
        bool const right_ux = right_turn.combination(operation_blocked, operation_union);

        operation_type const left_other_op = left_turn.operations[1 - left.operation_index].operation;
        operation_type const right_other_op = right_turn.operations[1 - right.operation_index].operation;

        if (left_ux && right_ux)
        {

            // Two ux on same ring at same point. Sort the open one last
            // For example:
            // 3 POINT(2 3) (left)  x s:0, v:0, m:1 // u s:1, v:3, m:1
            // 2 POINT(2 3) (right) u s:0, v:0, m:1 // x s:1, v:1, m:0
            // Should be sorted as: first right, then left, because left has
            // the union operation on the other ring

            BOOST_ASSERT(left_other_op != right_other_op);

            int const left_code = left_other_op == operation_blocked ? 0 : 1;
            int const right_code = right_other_op == operation_blocked ? 0 : 1;

            return left_code < right_code;
        }


        bool result = false;
        if (select_by_side(result, left, right))
        {
            return result;
        }

        if (select_operation(result, left, right))
        {
            return result;
        }

        // OBSOLETE
        // If that is not the case, cluster it later on.
        // Indicate that this is necessary.

        *m_clustered = true;

        return default_order(left, right);
    }
};


}} // namespace detail::overlay
#endif //DOXYGEN_NO_DETAIL


}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_SORT_ON_SEGMENT_RATIO_HPP
