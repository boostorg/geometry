// Boost.Geometry

// Copyright (c) 2007-2023 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2017-2023 Adam Wulkiewicz, Lodz, Poland.

// This file was modified by Oracle on 2015-2024.
// Modifications copyright (c) 2015-2024 Oracle and/or its affiliates.
// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_GET_TURN_INFO_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_GET_TURN_INFO_HPP

#include <boost/core/ignore_unused.hpp>
#include <boost/throw_exception.hpp>

#include <boost/geometry/core/access.hpp>
#include <boost/geometry/core/assert.hpp>
#include <boost/geometry/core/config.hpp>
#include <boost/geometry/core/exception.hpp>

#include <boost/geometry/algorithms/convert.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_distance_measure.hpp>
#include <boost/geometry/algorithms/detail/overlay/turn_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_turn_info_helpers.hpp>

#include <boost/geometry/util/constexpr.hpp>


namespace boost { namespace geometry
{

#if ! defined(BOOST_GEOMETRY_OVERLAY_NO_THROW)
class turn_info_exception : public geometry::exception
{
    std::string message;
public:

    // NOTE: "char" will be replaced by enum in future version
    inline turn_info_exception(char const method)
    {
        message = "Boost.Geometry Turn exception: ";
        message += method;
    }

    virtual char const* What() const noexcept
    {
        return message.c_str();
    }
};
#endif

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay
{


struct policy_verify_nothing
{
    static bool const use_side_verification = false;
    static bool const use_start_turn = false;
    static bool const use_handle_as_touch = false;
    static bool const use_handle_as_equal = false;
    static bool const use_handle_imperfect_touch = false;
};

struct policy_verify_all
{
    static bool const use_side_verification = true;
    static bool const use_start_turn = true;
    static bool const use_handle_as_touch = true;
    static bool const use_handle_as_equal = true;
    static bool const use_handle_imperfect_touch = true;
};

using verify_policy_aa = policy_verify_all;
using verify_policy_ll = policy_verify_nothing;
using verify_policy_la = policy_verify_nothing;

struct base_turn_handler
{
    // Returns true if both sides are opposite
    static inline bool opposite(int side1, int side2)
    {
        // We cannot state side1 == -side2, because 0 == -0
        // So either side1*side2==-1 or side1==-side2 && side1 != 0
        return side1 * side2 == -1;
    }

    // Same side of a segment (not being 0)
    static inline bool same(int side1, int side2)
    {
        return side1 * side2 == 1;
    }

    // Both get the same operation
    template <typename TurnInfo>
    static inline void both(TurnInfo& ti, operation_type const op)
    {
        ti.operations[0].operation = op;
        ti.operations[1].operation = op;
    }

    // If condition, first union/second intersection, else vice versa
    template <typename TurnInfo>
    static inline void ui_else_iu(bool condition, TurnInfo& ti)
    {
        ti.operations[0].operation = condition
                    ? operation_union : operation_intersection;
        ti.operations[1].operation = condition
                    ? operation_intersection : operation_union;
    }

    // If condition, both union, else both intersection
    template <typename TurnInfo>
    static inline void uu_else_ii(bool condition, TurnInfo& ti)
    {
        both(ti, condition ? operation_union : operation_intersection);
    }

    template <typename TurnInfo, typename IntersectionInfo>
    static inline void assign_point(TurnInfo& ti,
                method_type method,
                IntersectionInfo const& info, unsigned int index)
    {
        ti.method = method;

        BOOST_GEOMETRY_ASSERT(index < info.count);

        geometry::convert(info.intersections[index], ti.point);
        ti.operations[0].fraction = info.fractions[index].ra;
        ti.operations[1].fraction = info.fractions[index].rb;
    }

    template <typename TurnInfo, typename IntersectionInfo, typename DirInfo>
    static inline void assign_point_and_correct(TurnInfo& ti,
                method_type method,
                IntersectionInfo const& info, DirInfo const& dir_info)
    {
        ti.method = method;

        // For touch/touch interior always take the intersection point 0
        // (usually there is only one - but if collinear is handled as touch, both could be taken).
        static int const index = 0;

        geometry::convert(info.intersections[index], ti.point);

        for (int i = 0; i < 2; i++)
        {
            if (dir_info.arrival[i] == 1)
            {
                // The segment arrives at the intersection point, its fraction should be 1
                // (due to precision it might be nearly so, but not completely, in rare cases)
                ti.operations[i].fraction = {1, 1};
            }
            else if (dir_info.arrival[i] == -1)
            {
                // The segment leaves from the intersection point, likewise its fraction should be 0
                ti.operations[i].fraction = {0, 1};
            }
            else
            {
                ti.operations[i].fraction = i == 0 ? info.fractions[index].ra
                                                   : info.fractions[index].rb;
            }
        }

#if defined(BOOST_GEOMETRY_CONCEPT_FIX_ARRIVAL)
        // Override the assignments above, they are sometimes (but not always) wrong.
        ti.operations[0].fraction = info.fractions[index].ra;
        ti.operations[1].fraction = info.fractions[index].rb;
#endif
    }

    template <typename IntersectionInfo>
    static inline unsigned int non_opposite_to_index(IntersectionInfo const& info)
    {
        return info.fractions[0].rb < info.fractions[1].rb
            ? 1 : 0;
    }

};

template<typename VerifyPolicy>
struct turn_info_verification_functions
{
    template <typename Point1, typename Point2>
    static inline
    typename select_coordinate_type<Point1, Point2>::type
    distance_measure(Point1 const& a, Point2 const& b)
    {
        // TODO: revise this using comparable distance for various
        // coordinate systems
        using coor_t = typename select_coordinate_type<Point1, Point2>::type;

        coor_t const dx = get<0>(a) - get<0>(b);
        coor_t const dy = get<1>(a) - get<1>(b);
        return dx * dx + dy * dy;
    }

    template
    <
            std::size_t IndexP,
            std::size_t IndexQ,
            typename UniqueSubRange1,
            typename UniqueSubRange2,
            typename UmbrellaStrategy,
            typename TurnInfo
    >
    static inline void set_both_verified(
            UniqueSubRange1 const& range_p,
            UniqueSubRange2 const& range_q,
            UmbrellaStrategy const& umbrella_strategy,
            std::size_t index_p, std::size_t index_q,
            TurnInfo& ti)
    {
        BOOST_GEOMETRY_ASSERT(IndexP + IndexQ == 1);
        BOOST_GEOMETRY_ASSERT(index_p > 0 && index_p <= 2);
        BOOST_GEOMETRY_ASSERT(index_q > 0 && index_q <= 2);

        using distance_measure_result_type = geometry::coordinate_type_t<decltype(ti.point)>;

        bool const p_in_range = index_p < range_p.size();
        bool const q_in_range = index_q < range_q.size();
        std::array<distance_measure_result_type, 2> distance_measures{};
        if (p_in_range)
        {
            distance_measures[IndexP] = distance_measure(ti.point, range_p.at(index_p));
        }
        if (q_in_range)
        {
            distance_measures[IndexQ] = distance_measure(ti.point, range_q.at(index_q));
        }

        if (p_in_range && q_in_range)
        {
            // pk/q2 is considered as collinear, but there might be
            // a tiny measurable difference. If so, use that.
            // Calculate pk // qj-qk
            bool const p_closer = distance_measures[IndexP] < distance_measures[IndexQ];
            auto const dm
                = p_closer
                ? get_distance_measure(range_q.at(index_q - 1),
                                       range_q.at(index_q), range_p.at(index_p),
                                       umbrella_strategy)
                : get_distance_measure(range_p.at(index_p - 1),
                                       range_p.at(index_p), range_q.at(index_q),
                                       umbrella_strategy);

            if (! dm.is_zero())
            {
                // Not truely collinear, distinguish for union/intersection
                // If p goes left (positive), take that for a union
                bool const p_left
                    = p_closer ? dm.is_positive() : dm.is_negative();

                ti.operations[IndexP].operation = p_left
                            ? operation_union : operation_intersection;
                ti.operations[IndexQ].operation = p_left
                            ? operation_intersection : operation_union;
                return;
            }
        }

        base_turn_handler::both(ti, operation_continue);
    }

    template
    <
            std::size_t IndexP,
            std::size_t IndexQ,
            typename UniqueSubRange1,
            typename UniqueSubRange2,
            typename UmbrellaStrategy,
            typename TurnInfo
    >
    static inline void both_collinear(
            UniqueSubRange1 const& range_p,
            UniqueSubRange2 const& range_q,
            UmbrellaStrategy const& umbrella_strategy,
            std::size_t index_p, std::size_t index_q,
            TurnInfo& ti)
    {
        if BOOST_GEOMETRY_CONSTEXPR (VerifyPolicy::use_side_verification)
        {
            set_both_verified<IndexP, IndexQ>(range_p, range_q, umbrella_strategy,
                                              index_p, index_q, ti);
        }
        else
        {
            base_turn_handler::both(ti, operation_continue);
        }
    }

    template
    <
        typename UniqueSubRange1,
        typename UniqueSubRange2,
        typename UmbrellaStrategy
    >
    static inline int verified_side(int side,
                                    UniqueSubRange1 const& range_p,
                                    UniqueSubRange2 const& range_q,
                                    UmbrellaStrategy const& umbrella_strategy,
                                    int index_p, int index_q)
    {
        if BOOST_GEOMETRY_CONSTEXPR (VerifyPolicy::use_side_verification)
        {
            if (side == 0)
            {
                if (index_p >= 1 && range_p.is_last_segment())
                {
                    return 0;
                }
                if (index_q >= 2 && range_q.is_last_segment())
                {
                    return 0;
                }

                auto const dm = get_distance_measure(range_p.at(index_p),
                                                     range_p.at(index_p + 1),
                                                     range_q.at(index_q),
                                                     umbrella_strategy);
                static decltype(dm.measure) const zero = 0;
                return dm.measure == zero ? 0 : dm.measure > zero ? 1 : -1;
            }
        }

        return side;
    }

};


template
<
    typename TurnInfo,
    typename VerifyPolicy
>
struct touch_interior : public base_turn_handler
{
    using fun = turn_info_verification_functions<VerifyPolicy>;

    template
    <
        typename IntersectionInfo,
        typename SideCalculator,
        typename UniqueSubRange1,
        typename UniqueSubRange2
    >
    static bool handle_as_touch(IntersectionInfo const& info,
                                SideCalculator const& side,
                                UniqueSubRange1 const& non_touching_range,
                                UniqueSubRange2 const& other_range)
    {
        if BOOST_GEOMETRY_CONSTEXPR (! VerifyPolicy::use_handle_as_touch)
        {
            return false;
        }
        else // else prevents unreachable code warning
        {
            bool const has_k = ! non_touching_range.is_last_segment()
                && ! other_range.is_last_segment();
            if (has_k
                && (same(side.pj_wrt_q1(), side.qj_wrt_p2())
                 || same(side.pj_wrt_q2(), side.qj_wrt_p1())))
            {
                // At a touch, the touching points (pj and qj) should be collinear
                // with both other segments.
                // If that is not the case (both left or both right), it should not be handled as a touch,
                // (though the intersection point might be close to the end),
                // because segments might cross each other or touch the other in the middle.
                return false;
            }

            //
            //
            //                         ^  Q(i)                ^ P(i)
            //                          \                    /
            //                           \                  /
            //                            \                /
            //                             \              /
            //                              \            /
            //                               \          /
            //                                \        /
            //                                 \      /
            //                                  \    /
            //                                   \  / it is about buffer_rt_r
            //                  P(k)              v/  they touch here "in the middle", but at the intersection...
            //                  <---------------->v   there is no follow up IP
            //                                   /
            //                                  /
            //                                 /
            //                                /
            //                               /
            //                              /
            //                             v Q(k)
            //

            // Measure where the IP is located. If it is really close to the end,
            // then there is no space for the next IP (on P(1)/Q(2). A "from"
            // intersection will be generated, but those are never handled.
            // Therefore handle it as a normal touch (two segments arrive at the
            // intersection point). It currently checks for zero, but even a
            // distance a little bit larger would do.
            auto const dm = fun::distance_measure(info.intersections[0], non_touching_range.at(1));
            decltype(dm) const zero = 0;
            bool const result = math::equals(dm, zero);
            return result;
        }
    }

    // Index: 0, P is the interior, Q is touching and vice versa
    template
    <
        unsigned int Index,
        typename UniqueSubRange1,
        typename UniqueSubRange2,
        typename IntersectionInfo,
        typename DirInfo,
        typename SidePolicy,
        typename UmbrellaStrategy
    >
    static inline void apply(UniqueSubRange1 const& range_p,
                UniqueSubRange2 const& range_q,
                TurnInfo& ti,
                IntersectionInfo const& intersection_info,
                DirInfo const& dir_info,
                SidePolicy const& side,
                UmbrellaStrategy const& umbrella_strategy)
    {
        assign_point_and_correct(ti, method_touch_interior, intersection_info, dir_info);

        // Both segments of q touch segment p somewhere in its interior
        // 1) We know: if q comes from LEFT or RIGHT
        // (i.e. dir_info.sides.get<Index,0>() == 1 or -1)
        // 2) Important is: if q_k goes to LEFT, RIGHT, COLLINEAR
        //    and, if LEFT/COLL, if it is lying LEFT or RIGHT w.r.t. q_i

        BOOST_STATIC_ASSERT(Index <= 1);
        static unsigned int const index_p = Index;
        static unsigned int const index_q = 1 - Index;

        bool const has_pk = ! range_p.is_last_segment();
        bool const has_qk = ! range_q.is_last_segment();
        int const side_qi_p = dir_info.sides.template get<index_q, 0>();
        int const side_qk_p = has_qk ? side.qk_wrt_p1() : 0;

        if (side_qi_p == -side_qk_p)
        {
            // Q crosses P from left->right or from right->left (test "ML1")
            // Union: folow P (left->right) or Q (right->left)
            // Intersection: other turn
            unsigned int index = side_qk_p == -1 ? index_p : index_q;
            ti.operations[index].operation = operation_union;
            ti.operations[1 - index].operation = operation_intersection;
            return;
        }

        int const side_qk_q = has_qk ? side.qk_wrt_q1() : 0;

        // Only necessary if rescaling is turned off:
        int const side_pj_q2 = has_qk ? side.pj_wrt_q2() : 0;

        if (side_qi_p == -1 && side_qk_p == -1 && side_qk_q == 1)
        {
            // Q turns left on the right side of P (test "MR3")
            // Both directions for "intersection"
            both(ti, operation_intersection);
            ti.touch_only = true;
        }
        else if (side_qi_p == 1 && side_qk_p == 1 && side_qk_q == -1)
        {
            if (has_qk && side_pj_q2 == -1)
            {
                // Q turns right on the left side of P (test "ML3")
                // Union: take both operations
                // Intersection: skip
                both(ti, operation_union);
            }
            else
            {
                // q2 is collinear with p1, so it does not turn back. This
                // can happen in floating point precision. In this case,
                // block one of the operations to avoid taking that path.
                ti.operations[index_p].operation = operation_union;
                ti.operations[index_q].operation = operation_blocked;
            }
            ti.touch_only = true;
        }
        else if (side_qi_p == side_qk_p && side_qi_p == side_qk_q)
        {
            // Q turns left on the left side of P (test "ML2")
            // or Q turns right on the right side of P (test "MR2")
            // Union: take left turn (Q if Q turns left, P if Q turns right)
            // Intersection: other turn
            unsigned int index = side_qk_q == 1 ? index_q : index_p;
            if (has_qk && side_pj_q2 == 0)
            {
                // Even though sides xk w.r.t. 1 are distinct, pj is collinear
                // with q. Therefore swap the path
                index = 1 - index;
            }

            if (has_pk && has_qk && opposite(side_pj_q2, side_qi_p))
            {
                // Without rescaling, floating point requires extra measures
                int const side_qj_p1 = side.qj_wrt_p1();
                int const side_qj_p2 = side.qj_wrt_p2();

                if (same(side_qj_p1, side_qj_p2))
                {
                    int const side_pj_q1 = side.pj_wrt_q1();
                    if (opposite(side_pj_q1, side_pj_q2))
                    {
                        index = 1 - index;
                    }
                }
            }

            ti.operations[index].operation = operation_union;
            ti.operations[1 - index].operation = operation_intersection;
            ti.touch_only = true;
        }
        else if (side_qk_p == 0)
        {
            // Q intersects on interior of P and continues collinearly
            if (side_qk_q == side_qi_p)
            {
                fun::template both_collinear<index_p, index_q>(range_p, range_q, umbrella_strategy,
                                                               1, 2, ti);
            }
            else
            {
                // Opposite direction, which is never travelled.
                // If Q turns left, P continues for intersection
                // If Q turns right, P continues for union
                ti.operations[index_p].operation = side_qk_q == 1
                    ? operation_intersection
                    : operation_union;
                ti.operations[index_q].operation = operation_blocked;
            }
        }
        else
        {
            // Should not occur!
            ti.method = method_error;
        }
    }
};


template
<
    typename TurnInfo,
    typename VerifyPolicy
>
struct touch : public base_turn_handler
{
    using fun = turn_info_verification_functions<VerifyPolicy>;

    static inline bool between(int side1, int side2, int turn)
    {
        return side1 == side2 && ! opposite(side1, turn);
    }

    template
    <
        typename UniqueSubRange1,
        typename UniqueSubRange2,
        typename UmbrellaStrategy
    >
    static inline bool handle_imperfect_touch(UniqueSubRange1 const& range_p,
                                              UniqueSubRange2 const& range_q,
                                              int side_pk_q2,
                                              UmbrellaStrategy const& umbrella_strategy,
                                              TurnInfo& ti)
    {
        if BOOST_GEOMETRY_CONSTEXPR (! VerifyPolicy::use_handle_imperfect_touch)
        {
            return false;
        }
        else // else prevents unreachable code warning
        {
            //  Q
            //  ^
            // ||
            // ||
            // |^----
            // >----->P
            // *            * they touch here (P/Q are (nearly) on top of each other)
            //
            // Q continues from where P comes.
            // P continues from where Q comes
            // This is often a blocking situation,
            // unless there are FP issues: there might be a distance
            // between Pj and Qj, in that case handle it as a union.
            //
            // Exaggerated:
            //  Q
            //  ^           Q is nearly vertical
            //   \          but not completely - and still ends above P
            // |  \qj       In this case it should block P and
            // |  ^------   set Q to Union
            // >----->P     qj is LEFT of P1 and pi is LEFT of Q2
            //              (the other way round is also possible)

            // There are also cases like this:
            //      P
            //      ^
            //      ||
            //      ||
            // P----^-----<Q
            // This code is not for these cases because of the condition opposite(side.pi_wrt_q1(), side.qk_wrt_p2())

            auto has_distance = [&](auto const& r1, auto const& r2) -> bool
            {
                auto const d1 = get_distance_measure(r1.at(0), r1.at(1), r2.at(1), umbrella_strategy);
                auto const d2 = get_distance_measure(r2.at(1), r2.at(2), r1.at(0), umbrella_strategy);
                return d1.measure > 0 && d2.measure > 0;
            };

            if (side_pk_q2 == -1 && has_distance(range_p, range_q))
            {
                // Even though there is a touch, Q(j) is left of P1
                // and P(i) is still left from Q2.
                // Q continues to the right.
                // It can continue.
                ti.operations[0].operation = operation_blocked;
                // Q turns right -> union (both independent),
                // Q turns left -> intersection
                ti.operations[1].operation = operation_union;
                ti.touch_only = true;
                return true;
            }

            if (side_pk_q2 == 1 && has_distance(range_q, range_p))
            {
                // Similarly, but the other way round.
                // Q continues to the left.
                ti.operations[0].operation = operation_union;
                ti.operations[1].operation = operation_blocked;
                ti.touch_only = true;
                return true;
            }
            return false;
        }
    }

    template
    <
        typename UniqueSubRange1,
        typename UniqueSubRange2,
        typename IntersectionInfo,
        typename DirInfo,
        typename SideCalculator,
        typename UmbrellaStrategy
    >
    static inline void apply(UniqueSubRange1 const& range_p,
                UniqueSubRange2 const& range_q,
                TurnInfo& ti,
                IntersectionInfo const& intersection_info,
                DirInfo const& dir_info,
                SideCalculator const& side,
                UmbrellaStrategy const& umbrella_strategy)
    {
        assign_point_and_correct(ti, method_touch, intersection_info, dir_info);

        bool const has_pk = ! range_p.is_last_segment();
        bool const has_qk = ! range_q.is_last_segment();

        int const side_pk_q1 = has_pk ? side.pk_wrt_q1() : 0;

        int const side_qi_p1 = fun::verified_side(dir_info.sides.template get<1, 0>(),
                                                  range_p, range_q, umbrella_strategy, 0, 0);
        int const side_qk_p1 = has_qk
                             ? fun::verified_side(side.qk_wrt_p1(), range_p, range_q,
                                                  umbrella_strategy, 0, 2)
                             : 0;

        // If Qi and Qk are both at same side of Pi-Pj,
        // or collinear (so: not opposite sides)
        if (! opposite(side_qi_p1, side_qk_p1))
        {
            int const side_pk_q2 = has_pk && has_qk ? side.pk_wrt_q2() : 0;
            int const side_pk_p  = has_pk ? side.pk_wrt_p1() : 0;
            int const side_qk_q  = has_qk ? side.qk_wrt_q1() : 0;

            bool const q_turns_left = side_qk_q == 1;

            bool const block_q = side_qk_p1 == 0
                        && ! same(side_qi_p1, side_qk_q)
                        ;

            // If Pk at same side as Qi/Qk
            // (the "or" is for collinear case)
            // or Q is fully collinear && P turns not to left
            if (side_pk_p == side_qi_p1
                || side_pk_p == side_qk_p1
                || (side_qi_p1 == 0 && side_qk_p1 == 0 && side_pk_p != -1))
            {
                if (side_qk_p1 == 0 && side_pk_q1 == 0
                    && has_pk && has_qk
                    && opposite(side.pi_wrt_q1(), side.qk_wrt_p2())
                    && handle_imperfect_touch(range_p, range_q, side_pk_q2, umbrella_strategy, ti))
                {
                    // If q continues collinearly (opposite) with p, it should be blocked
                    // but (FP) not if there is just a tiny space in between
                    return;
                }
                // Collinear -> lines join, continue
                // (#BRL2)
                if (side_pk_q2 == 0 && ! block_q)
                {
                    fun::template both_collinear<0, 1>(range_p, range_q, umbrella_strategy,
                                                       2, 2, ti);
                    return;
                }

                // Collinear opposite case -> block P
                // (#BRL4, #BLR8)
                if (side_pk_q1 == 0)
                {
                    ti.operations[0].operation = operation_blocked;
                    // Q turns right -> union (both independent),
                    // Q turns left -> intersection
                    // NOTE: the block is suspicious!
                    ti.operations[1].operation = block_q ? operation_blocked
                        : q_turns_left ? operation_intersection
                        : operation_union;
                    return;
                }

                // Pk between Qi and Qk
                // (#BRL3, #BRL7)
                if (between(side_pk_q1, side_pk_q2, side_qk_q))
                {
                    ui_else_iu(q_turns_left, ti);
                    if (block_q)
                    {
                        // The block is suspicious! It is sometimes wrong!
                        ti.operations[1].operation = operation_blocked;
                    }
                    return;
                }

                // Pk between Qk and P, so left of Qk (if Q turns right) and vv
                // (#BRL1)
                if (side_pk_q2 == -side_qk_q)
                {
                    ui_else_iu(! q_turns_left, ti);
                    ti.touch_only = true;
                    return;
                }

                //
                // (#BRL5, #BRL9)
                if (side_pk_q1 == -side_qk_q)
                {
                    uu_else_ii(! q_turns_left, ti);
                    if (block_q)
                    {
                        ti.operations[1].operation = operation_blocked;
                    }
                    else
                    {
                        ti.touch_only = true;
                    }
                    return;
                }
            }
            else
            {
                // Pk at other side than Qi/Pk
                ti.operations[0].operation = q_turns_left
                            ? operation_intersection
                            : operation_union;
                ti.operations[1].operation = block_q
                            ? operation_blocked
                            : side_qi_p1 == 1 || side_qk_p1 == 1
                            ? operation_union
                            : operation_intersection;
#if defined(BOOST_GEOMETRY_CONCEPT_FIX_BLOCK_Q)
                // NOTE: this block is suspicious! Override it.
                // This concept fix is not complete.
                // The exact situation should be adapted.
                ti.operations[1].operation = side_qi_p1 == 1 || side_qk_p1 == 1
                            ? operation_union
                            : operation_intersection;
#endif

                if (! block_q)
                {
                    ti.touch_only = true;
                }

                return;
            }
        }
        else
        {
            // The qi/qk are opposite to each other, w.r.t. p1
            // From left to right or from right to left
            int const side_pk_p = has_pk
                                ? fun::verified_side(side.pk_wrt_p1(), range_p, range_p,
                                                     umbrella_strategy, 0, 2)
                                : 0;
            bool const right_to_left = side_qk_p1 == 1;

            // If p turns into direction of qi (1,2)
            if (side_pk_p == side_qi_p1)
            {
                // Collinear opposite case -> block P
                if (side_pk_q1 == 0)
                {
                    ti.operations[0].operation = operation_blocked;
                    ti.operations[1].operation = right_to_left
                                ? operation_union : operation_intersection;
                    return;
                }

                if (side_pk_q1 == side_qk_p1)
                {
                    uu_else_ii(right_to_left, ti);
                    ti.touch_only = true;
                    return;
                }
            }

            // If p turns into direction of qk (4,5)
            if (side_pk_p == side_qk_p1)
            {
                int const side_pk_q2 = has_pk ? side.pk_wrt_q2() : 0;

                // Collinear case -> lines join, continue
                if (side_pk_q2 == 0)
                {
                    both(ti, operation_continue);
                    return;
                }
                if (side_pk_q2 == side_qk_p1)
                {
                    ui_else_iu(right_to_left, ti);
                    ti.touch_only = true;
                    return;
                }
            }
            // otherwise (3)
            ui_else_iu(! right_to_left, ti);
            return;
        }
    }
};


template
<
    typename TurnInfo,
    typename VerifyPolicy
>
struct equal : public base_turn_handler
{
    using fun = turn_info_verification_functions<VerifyPolicy>;

    template
    <
        typename UniqueSubRange1,
        typename UniqueSubRange2,
        typename IntersectionInfo,
        typename DirInfo,
        typename SideCalculator,
        typename UmbrellaStrategy
    >
    static inline void apply(UniqueSubRange1 const& range_p,
                UniqueSubRange2 const& range_q,
                TurnInfo& ti,
                IntersectionInfo const& info,
                DirInfo const& ,
                SideCalculator const& side,
                UmbrellaStrategy const& umbrella_strategy)
    {
        // Copy the intersection point in TO direction
        assign_point(ti, method_equal, info, non_opposite_to_index(info));

        bool const has_pk = ! range_p.is_last_segment();
        bool const has_qk = ! range_q.is_last_segment();

        int const side_pk_q2 = has_pk && has_qk ? side.pk_wrt_q2() : 0;
        int const side_pk_p = has_pk ? side.pk_wrt_p1() : 0;
        int const side_qk_p = has_qk ? side.qk_wrt_p1() : 0;

        if (has_pk && has_qk && side_pk_p == side_qk_p)
        {
            // They turn to the same side, or continue both collinearly
            // To check for union/intersection, try to check side values
            int const side_qk_p2 = side.qk_wrt_p2();

            if (opposite(side_qk_p2, side_pk_q2))
            {
                ui_else_iu(side_pk_q2 == 1, ti);
                return;
            }
        }

        // If pk is collinear with qj-qk, they continue collinearly.
        // This can be on either side of p1 (== q1), or collinear
        // The second condition checks if they do not continue
        // oppositely
        if (side_pk_q2 == 0 && side_pk_p == side_qk_p)
        {
            fun::template both_collinear<0, 1>(range_p, range_q, umbrella_strategy, 2, 2, ti);
            return;
        }


        // If they turn to same side (not opposite sides)
        if (! opposite(side_pk_p, side_qk_p))
        {
            // If pk is left of q2 or collinear: p: union, q: intersection
            ui_else_iu(side_pk_q2 != -1, ti);
        }
        else
        {
            // They turn opposite sides. If p turns left (or collinear),
            // p: union, q: intersection
            ui_else_iu(side_pk_p != -1, ti);
        }
    }
};

template
<
    typename TurnInfo,
    typename VerifyPolicy
>
struct start : public base_turn_handler
{
    template
    <
        typename UniqueSubRange1,
        typename UniqueSubRange2,
        typename IntersectionInfo,
        typename DirInfo,
        typename SideCalculator,
        typename UmbrellaStrategy
    >
    static inline bool apply(UniqueSubRange1 const& /*range_p*/,
                UniqueSubRange2 const& /*range_q*/,
                TurnInfo& ti,
                IntersectionInfo const& info,
                DirInfo const& dir_info,
                SideCalculator const& side,
                UmbrellaStrategy const& )
    {
        if BOOST_GEOMETRY_CONSTEXPR (! VerifyPolicy::use_start_turn)
        {
            return false;
        }
        else // else prevents unreachable code warning
        {
            // Start turns have either how_a = -1, or how_b = -1 (either p leaves or q leaves)
            BOOST_GEOMETRY_ASSERT(dir_info.how_a != dir_info.how_b);
            BOOST_GEOMETRY_ASSERT(dir_info.how_a == -1 || dir_info.how_b == -1);
            BOOST_GEOMETRY_ASSERT(dir_info.how_a == 0 || dir_info.how_b == 0);

            if (dir_info.how_b == -1)
            {
                // p --------------->
                //             |
                //             | q         q leaves
                //             v
                //

                int const side_qj_p1 = side.qj_wrt_p1();
                ui_else_iu(side_qj_p1 == -1, ti);
            }
            else if (dir_info.how_a == -1)
            {
                // p leaves
                int const side_pj_q1 = side.pj_wrt_q1();
                ui_else_iu(side_pj_q1 == 1, ti);
            }

            // Copy intersection point
            assign_point_and_correct(ti, method_start, info, dir_info);
            return true;
        }
    }
};


template
<
    typename TurnInfo,
    typename AssignPolicy
>
struct equal_opposite : public base_turn_handler
{
    template
    <
        typename UniqueSubRange1,
        typename UniqueSubRange2,
        typename OutputIterator,
        typename IntersectionInfo
    >
    static inline void apply(
                UniqueSubRange1 const& /*range_p*/,
                UniqueSubRange2 const& /*range_q*/,
                /* by value: */ TurnInfo tp,
                OutputIterator& out,
                IntersectionInfo const& intersection_info)
    {
        // For equal-opposite segments, normally don't do anything.
        if BOOST_GEOMETRY_CONSTEXPR (AssignPolicy::include_opposite)
        {
            tp.method = method_equal;
            for (unsigned int i = 0; i < 2; i++)
            {
                tp.operations[i].operation = operation_opposite;
            }
            for (unsigned int i = 0; i < intersection_info.i_info().count; i++)
            {
                assign_point(tp, method_none, intersection_info.i_info(), i);
                *out++ = tp;
            }
        }
    }
};

template
<
    typename TurnInfo,
    typename VerifyPolicy
>
struct collinear : public base_turn_handler
{
    using fun = turn_info_verification_functions<VerifyPolicy>;

    template
    <
        typename IntersectionInfo,
        typename UniqueSubRange1,
        typename UniqueSubRange2,
        typename DirInfo
    >
    static bool handle_as_equal(IntersectionInfo const& info,
                                UniqueSubRange1 const& range_p,
                                UniqueSubRange2 const& range_q,
                                DirInfo const& dir_info)
    {
        if BOOST_GEOMETRY_CONSTEXPR (! VerifyPolicy::use_handle_as_equal)
        {
            return false;
        }
        else // else prevents unreachable code warning
        {
            int const arrival_p = dir_info.arrival[0];
            int const arrival_q = dir_info.arrival[1];
            if (arrival_p * arrival_q != -1 || info.count != 2)
            {
                // Code below assumes that either p or q arrives in the other segment
                return false;
            }

            auto const dm = arrival_p == 1
                          ? fun::distance_measure(info.intersections[1], range_q.at(1))
                          : fun::distance_measure(info.intersections[1], range_p.at(1));
            decltype(dm) const zero = 0;
            return math::equals(dm, zero);
        }
    }

    /*
        Either P arrives within Q (arrival_p == -1) or Q arrives within P.

        Typical situation:
              ^q2
             /
            ^q1
           /         ____ ip[1]
          //|p1  } this section of p/q is colllinear
       q0// |    }   ____ ip[0]
         /  |
        /   v
       p0   p2

       P arrives (at p1) in segment Q (between q0 and q1).
       Therefore arrival_p == 1
       P (p2) goes to the right (-1). Follow P for intersection, or follow Q for union.
       Therefore if (arrival_p==1) and side_p==-1, result = iu

       Complete table:

        arrival P   pk//p1  qk//q1   product   case    result
         1           1                1        CLL1    ui
        -1                   1       -1        CLL2    iu
         1           1                1        CLR1    ui
        -1                  -1        1        CLR2    ui

         1          -1               -1        CRL1    iu
        -1                   1       -1        CRL2    iu
         1          -1               -1        CRR1    iu
        -1                  -1        1        CRR2    ui

         1           0                0        CC1     cc
        -1                   0        0        CC2     cc

         Resulting in the rule:
         The arrival-info multiplied by the relevant side delivers the result.
         product = arrival * (pk//p1 or qk//q1)

         Stated otherwise:
         - if P arrives: look at turn P
         - if Q arrives: look at turn Q
         - if P arrives and P turns left: union for P
         - if P arrives and P turns right: intersection for P
         - if Q arrives and Q turns left: union for Q (=intersection for P)
         - if Q arrives and Q turns right: intersection for Q (=union for P)
    */
    template
    <
        typename UniqueSubRange1,
        typename UniqueSubRange2,
        typename IntersectionInfo,
        typename DirInfo,
        typename SidePolicy
    >
    static inline void apply(
                UniqueSubRange1 const& range_p,
                UniqueSubRange2 const& range_q,
                TurnInfo& ti,
                IntersectionInfo const& info,
                DirInfo const& dir_info,
                SidePolicy const& side)
    {
        // Copy the intersection point in TO direction
        assign_point(ti, method_collinear, info, non_opposite_to_index(info));

        int const arrival_p = dir_info.arrival[0];
        // Should not be 0, this is checked before
        BOOST_GEOMETRY_ASSERT(arrival_p != 0);

        bool const has_pk = ! range_p.is_last_segment();
        bool const has_qk = ! range_q.is_last_segment();
        int const side_p = has_pk ? side.pk_wrt_p1() : 0;
        int const side_q = has_qk ? side.qk_wrt_q1() : 0;

        // If p arrives, use p, else use q
        int const side_p_or_q = arrival_p == 1
            ? side_p
            : side_q
            ;

        // Calculate product according to comments above.
        int const product = arrival_p * side_p_or_q;

        if (product == 0)
        {
            both(ti, operation_continue);
        }
        else
        {
            ui_else_iu(product == 1, ti);
        }

    }
};

template
<
    typename TurnInfo,
    typename AssignPolicy
>
struct collinear_opposite : public base_turn_handler
{
private :
    /*
        arrival P  arrival Q  pk//p1   qk//q1  case  result2  result
        --------------------------------------------------------------
         1          1          1       -1      CLO1    ix      xu
         1          1          1        0      CLO2    ix      (xx)
         1          1          1        1      CLO3    ix      xi

         1          1          0       -1      CCO1    (xx)    xu
         1          1          0        0      CCO2    (xx)    (xx)
         1          1          0        1      CCO3    (xx)    xi

         1          1         -1       -1      CRO1    ux      xu
         1          1         -1        0      CRO2    ux      (xx)
         1          1         -1        1      CRO3    ux      xi

        -1          1                  -1      CXO1    xu
        -1          1                   0      CXO2    (xx)
        -1          1                   1      CXO3    xi

         1         -1          1               CXO1    ix
         1         -1          0               CXO2    (xx)
         1         -1         -1               CXO3    ux
    */

    template <unsigned int Index, typename IntersectionInfo>
    static inline bool set_tp(int side_rk_r, TurnInfo& tp,
                              IntersectionInfo const& intersection_info)
    {
        BOOST_STATIC_ASSERT(Index <= 1);

        operation_type blocked = operation_blocked;
        switch(side_rk_r)
        {
            case 1 :
                // Turning left on opposite collinear: intersection
                tp.operations[Index].operation = operation_intersection;
                break;
            case -1 :
                // Turning right on opposite collinear: union
                tp.operations[Index].operation = operation_union;
                break;
            case 0 :
                // No turn on opposite collinear: block, do not traverse
                // But this "xx" is usually ignored, it is useless to include
                // two operations blocked, so the whole point does not need
                // to be generated.
                // So return false to indicate nothing is to be done.
                if BOOST_GEOMETRY_CONSTEXPR (AssignPolicy::include_opposite)
                {
                    tp.operations[Index].operation = operation_opposite;
                    blocked = operation_opposite;
                }
                else
                {
                    return false;
                }
                break;
        }

        // The other direction is always blocked when collinear opposite
        tp.operations[1 - Index].operation = blocked;

        // If P arrives within Q, set info on P (which is done above, index=0),
        // this turn-info belongs to the second intersection point, index=1
        // (see e.g. figure CLO1)
        assign_point(tp, method_collinear, intersection_info, 1 - Index);
        return true;
    }

public:
    static inline void empty_transformer(TurnInfo &) {}

    template
    <
        typename UniqueSubRange1,
        typename UniqueSubRange2,
        typename OutputIterator,
        typename IntersectionInfo,
        typename SidePolicy
    >
    static inline void apply(
                UniqueSubRange1 const& range_p,
                UniqueSubRange2 const& range_q,

                // Opposite collinear can deliver 2 intersection points,
                TurnInfo const& tp_model,
                OutputIterator& out,

                IntersectionInfo const& intersection_info,
                SidePolicy const& side)
    {
        apply(range_p, range_q,
              tp_model, out, intersection_info, side, empty_transformer);
    }

    template
    <
        typename UniqueSubRange1,
        typename UniqueSubRange2,
        typename OutputIterator,
        typename IntersectionInfo,
        typename SidePolicy,
        typename TurnTransformer
    >
    static inline void apply(
                UniqueSubRange1 const& range_p,
                UniqueSubRange2 const& range_q,

                // Opposite collinear can deliver 2 intersection points,
                TurnInfo const& tp_model,
                OutputIterator& out,

                IntersectionInfo const& info,
                SidePolicy const& side,
                TurnTransformer turn_transformer)
    {
        TurnInfo tp = tp_model;

        int const arrival_p = info.d_info().arrival[0];
        int const arrival_q = info.d_info().arrival[1];

        // If P arrives within Q, there is a turn dependent on P
        if ( arrival_p == 1
          && ! range_p.is_last_segment()
          && set_tp<0>(side.pk_wrt_p1(), tp, info.i_info()) )
        {
            turn_transformer(tp);

            *out++ = tp;
        }

        // If Q arrives within P, there is a turn dependent on Q
        if ( arrival_q == 1
          && ! range_q.is_last_segment()
          && set_tp<1>(side.qk_wrt_q1(), tp, info.i_info()) )
        {
            turn_transformer(tp);

            *out++ = tp;
        }

        if BOOST_GEOMETRY_CONSTEXPR (AssignPolicy::include_opposite)
        {
            // Handle cases not yet handled above
            if ((arrival_q == -1 && arrival_p == 0)
                || (arrival_p == -1 && arrival_q == 0))
            {
                for (unsigned int i = 0; i < 2; i++)
                {
                    tp.operations[i].operation = operation_opposite;
                }
                for (unsigned int i = 0; i < info.i_info().count; i++)
                {
                    assign_point(tp, method_collinear, info.i_info(), i);
                    *out++ = tp;
                }
            }
        }

    }
};


template
<
    typename TurnInfo
>
struct crosses : public base_turn_handler
{
    template <typename IntersectionInfo, typename DirInfo>
    static inline void apply(TurnInfo& ti,
                IntersectionInfo const& intersection_info,
                DirInfo const& dir_info)
    {
        assign_point(ti, method_crosses, intersection_info, 0);

        // In all cases:
        // If Q crosses P from left to right
        // Union: take P
        // Intersection: take Q
        // Otherwise: vice versa
        int const side_qi_p1 = dir_info.sides.template get<1, 0>();
        unsigned int const index = side_qi_p1 == 1 ? 0 : 1;
        ti.operations[index].operation = operation_union;
        ti.operations[1 - index].operation = operation_intersection;
    }
};

struct only_convert : public base_turn_handler
{
    template<typename TurnInfo, typename IntersectionInfo>
    static inline void apply(TurnInfo& ti, IntersectionInfo const& intersection_info)
    {
        assign_point(ti, method_none, intersection_info, 0);
        ti.operations[0].operation = operation_continue;
        ti.operations[1].operation = operation_continue;
    }
};

/*!
\brief Policy doing nothing
\details get_turn_info can have an optional policy include extra
    truns. By default it does not, and this class is that default.
 */
struct assign_null_policy
{
    static bool const include_no_turn = false;
    static bool const include_degenerate = false;
    static bool const include_opposite = false;
    static bool const include_start_turn = false;
};

struct assign_policy_only_start_turns
{
    static bool const include_no_turn = false;
    static bool const include_degenerate = false;
    static bool const include_opposite = false;
    static bool const include_start_turn = true;
};

/*!
    \brief Turn information: intersection point, method, and turn information
    \details Information necessary for traversal phase (a phase
        of the overlay process). The information is gathered during the
        get_turns (segment intersection) phase.
    \tparam AssignPolicy policy to assign extra info,
        e.g. to calculate distance from segment's first points
        to intersection points.
        It also defines if a certain class of points
        (degenerate, non-turns) should be included.
 */
template<typename AssignPolicy>
struct get_turn_info
{
    // Intersect a segment p with a segment q
    // Both p and q are modelled as sub_ranges to provide more points
    // to be able to give more information about the turn (left/right)
    template
    <
        typename UniqueSubRange1,
        typename UniqueSubRange2,
        typename TurnInfo,
        typename UmbrellaStrategy,
        typename OutputIterator
    >
    static inline OutputIterator apply(
                UniqueSubRange1 const& range_p,
                UniqueSubRange2 const& range_q,
                TurnInfo const& tp_model,
                UmbrellaStrategy const& umbrella_strategy,
                OutputIterator out)
    {
        using inters_info = intersection_info
            <
                UniqueSubRange1, UniqueSubRange2,
                typename TurnInfo::point_type,
                UmbrellaStrategy
            >;

        inters_info inters(range_p, range_q, umbrella_strategy);

        char const method = inters.d_info().how;

        if (method == 'd')
        {
            // Disjoint
            return out;
        }

        // Copy, to copy possibly extended fields
        TurnInfo tp = tp_model;

        bool const handle_as_touch_interior = method == 'm';
        bool const handle_as_cross = method == 'i';
        bool handle_as_touch = method == 't';
        bool handle_as_equal = method == 'e';
        bool const handle_as_collinear = method == 'c';
        bool const handle_as_degenerate = method == '0';
        bool const handle_as_start = method == 's';

        // (angle, from)
        bool do_only_convert = method == 'a' || method == 'f';

        if (handle_as_start)
        {
            // It is in some cases necessary to handle a start turn
            using handler = start<TurnInfo, verify_policy_aa>;
            if (BOOST_GEOMETRY_CONDITION(AssignPolicy::include_start_turn)
                && handler::apply(range_p, range_q, tp,
                               inters.i_info(), inters.d_info(), inters.sides(),
                               umbrella_strategy))
            {
                *out++ = tp;
            }
            else
            {
              do_only_convert = true;
            }
        }

        if (handle_as_touch_interior)
        {
            using handler = touch_interior<TurnInfo, verify_policy_aa>;

            if ( inters.d_info().arrival[1] == 1 )
            {
                // Q arrives
                if (handler::handle_as_touch(inters.i_info(), inters.sides(), range_p, range_q))
                {
                    handle_as_touch = true;
                }
                else
                {
                    handler::template apply<0>(range_p, range_q, tp, inters.i_info(), inters.d_info(),
                                inters.sides(), umbrella_strategy);
                    *out++ = tp;
                }
            }
            else
            {
                // P arrives, swap p/q
                if (handler::handle_as_touch(inters.i_info(), inters.swapped_sides(), range_q, range_p))
                {
                    handle_as_touch = true;
                }
                else
                {
                    handler::template apply<1>(range_q, range_p, tp, inters.i_info(), inters.d_info(),
                                inters.swapped_sides(), umbrella_strategy);
                    *out++ = tp;
                }
            }
        }

        if (handle_as_cross)
        {
            crosses<TurnInfo>::apply(tp, inters.i_info(), inters.d_info());
            *out++ = tp;
        }

        if (handle_as_touch)
        {
            // Touch: both segments arrive at the intersection point
            using handler = touch<TurnInfo, verify_policy_aa>;
            handler::apply(range_p, range_q, tp, inters.i_info(), inters.d_info(), inters.sides(),
                           umbrella_strategy);
            *out++ = tp;
        }

        if (handle_as_collinear)
        {
            // Collinear
            if ( ! inters.d_info().opposite )
            {
                using handler = collinear<TurnInfo, verify_policy_aa>;
                if (inters.d_info().arrival[0] == 0
                    || handler::handle_as_equal(inters.i_info(), range_p, range_q, inters.d_info()))
                {
                    // Both segments arrive at the second intersection point
                    handle_as_equal = true;
                }
                else
                {
                    handler::apply(range_p, range_q, tp, inters.i_info(),
                                   inters.d_info(), inters.sides());
                    *out++ = tp;
                }
            }
            else
            {
                collinear_opposite
                    <
                        TurnInfo,
                        AssignPolicy
                    >::apply(range_p, range_q, tp, out, inters, inters.sides());
                // Zero, or two, turn points are assigned to *out++
            }
        }

        if (handle_as_equal)
        {
            if ( ! inters.d_info().opposite )
            {
                // Both equal
                // or collinear-and-ending at intersection point
                using handler = equal<TurnInfo, verify_policy_aa>;
                handler::apply(range_p, range_q, tp,
                        inters.i_info(), inters.d_info(), inters.sides(),
                        umbrella_strategy);
                if (handle_as_collinear)
                {
                    // Keep info as collinear,
                    // so override already assigned method
                    tp.method = method_collinear;
                }
                *out++ = tp;
            }
            else
            {
                equal_opposite
                    <
                        TurnInfo,
                        AssignPolicy
                    >::apply(range_p, range_q, tp, out, inters);
                // Zero, or two, turn points are assigned to *out++
            }
        }

        if ((handle_as_degenerate
             && BOOST_GEOMETRY_CONDITION(AssignPolicy::include_degenerate))
            || (do_only_convert
                && BOOST_GEOMETRY_CONDITION(AssignPolicy::include_no_turn)))
        {
            if (inters.i_info().count > 0)
            {
                only_convert::apply(tp, inters.i_info());
                *out++ = tp;
            }
        }

        return out;
    }
};


}} // namespace detail::overlay
#endif //DOXYGEN_NO_DETAIL


}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_GET_TURN_INFO_HPP
