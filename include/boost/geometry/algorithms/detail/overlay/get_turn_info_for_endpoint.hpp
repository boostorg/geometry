// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013, 2014.
// Modifications copyright (c) 2013-2014 Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_GET_TURN_INFO_FOR_ENDPOINT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_GET_TURN_INFO_FOR_ENDPOINT_HPP

#include <boost/geometry/algorithms/detail/overlay/get_turn_info.hpp>

namespace boost { namespace geometry {

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay {

// TURN_OPERATION

enum turn_position { position_middle, position_front, position_back };

template <typename SegmentRatio>
struct turn_operation_linear
    : public turn_operation<SegmentRatio>
{
    turn_operation_linear()
        : position(position_middle)
        , is_collinear(false)
    {}

    turn_position position;
    bool is_collinear; // valid only for Linear geometry
};

// IS_SPIKE
    
template <typename Point, typename RobustPolicy>
inline bool is_spike_of_collinear(model::referring_segment<Point const> const& s1,
                                  model::referring_segment<Point const> const& s2,
                                  RobustPolicy const& robust_policy)
{
    typedef strategy_intersection
        <
            typename cs_tag<Point>::type, Point, Point, Point, RobustPolicy
        > si;
        
    typedef typename si::segment_intersection_strategy_type strategy;
        
    typename strategy::return_type result = strategy::apply(s1, s2, robust_policy);
        
    return result.template get<0>().count == 2;
}

template <typename Point1, typename Point2, typename RobustPolicy>
inline bool is_spike_p(side_calculator<Point1, Point2> const& side_calc,
                       model::referring_segment<Point1 const> const& p1,
                       model::referring_segment<Point1 const> const& p2,
                       RobustPolicy const& robust_policy)
{
    if ( side_calc.pk_wrt_p1() == 0 )
    {
        int const qk_p1 = side_calc.qk_wrt_p1();
        int const qk_p2 = side_calc.qk_wrt_p2();
                
        if ( qk_p1 == -qk_p2 )
        {
            if ( qk_p1 == 0 )
            {
                return is_spike_of_collinear(p1, p2, robust_policy);
            }
                        
            return true;
        }
    }
        
    return false;
}

template <typename Point1, typename Point2, typename RobustPolicy>
inline bool is_spike_q(side_calculator<Point1, Point2> const& side_calc,
                       model::referring_segment<Point2 const> const& q1,
                       model::referring_segment<Point2 const> const& q2,
                       RobustPolicy const& robust_policy)
{
    if ( side_calc.qk_wrt_q1() == 0 )
    {
        int const pk_q1 = side_calc.pk_wrt_q1();
        int const pk_q2 = side_calc.pk_wrt_q2();
                
        if ( pk_q1 == -pk_q2 )
        {
            if ( pk_q1 == 0 )
            {
                return is_spike_of_collinear(q1, q2, robust_policy);
            }
                        
            return true;
        }
    }
        
    return false;
}

template <typename Point1, typename Point2, typename RobustPolicy>
inline bool is_spike_p(Point1 const& pi, Point1 const& pj, Point1 const& pk,
                       Point2 const& qi, Point2 const& qj, Point2 const& qk,
                       RobustPolicy const& robust_policy)
{
    typedef model::referring_segment<Point1 const> segment_type1;
    segment_type1 p1(pi, pj), p2(pj, pk);
    side_calculator<Point1, Point2> side_calc(pi, pj, pk, qi, qj, qk);
    return is_spike_p(side_calc, p1, p2, robust_policy);
}

template <typename Point1, typename Point2, typename RobustPolicy>
inline bool is_spike_q(Point1 const& pi, Point1 const& pj, Point1 const& pk,
                       Point2 const& qi, Point2 const& qj, Point2 const& qk,
                       RobustPolicy const& robust_policy)
{
    typedef model::referring_segment<Point2 const> segment_type2;
    segment_type2 q1(qi, qj), q2(qj, qk);
    side_calculator<Point1, Point2> side_calc(pi, pj, pk, qi, qj, qk);
    return is_spike_q(side_calc, q1, q2, robust_policy);
}

// SEGMENT_INTERSECTION RESULT

//                   C  H0  H1  A0  A1   O              IP1 IP2

// D0 and D1 == 0

// |-------->        2   0   0   0   0   F              i/i x/x
// |-------->
//
// |-------->        2   0   0   0   0   T              i/x x/i
// <--------|
//
// |----->           1   0   0   0   0   T              x/x
//       <-----|
//

// |--------->       2   0   0   0   1   T              i/x x/i
//      <----|
//
// |--------->       2   0   0   0   0   F              i/i x/x
//      |---->
//
// |--------->       2   0   0  -1   1   F              i/i u/x
// |---->
//
// |--------->       2   0   0  -1   0   T              i/x u/i
// <----|

// |------->         2   0   0   1  -1   F   and        i/i x/u
//     |------->     2   0   0  -1   1   F   symetric   i/i u/x
// |------->
//
//     |------->     2   0   0  -1  -1   T              i/u u/i
// <-------|
//
// |------->         2   0   0   1   1   T              i/x x/i
//     <-------|
//
// |-------->        2   0   0  -1   1   F              i/i u/x
//   |---->
//
// |-------->        2   0   0  -1   1   T              i/x u/i
//   <----|

//       |----->     1  -1  -1  -1  -1   T              u/u
// <-----|
//
//       |----->     1  -1   0  -1   0   F   and        u/x
// |----->           1   0  -1   0  -1   F   symetric   x/u
//       |----->

// D0 or D1 != 0

//          ^
//          |
//          +        1  -1   1  -1   1   F   and        u/x  (P is vertical)
// |-------->        1   1  -1   1  -1   F   symetric   x/u  (P is horizontal)
// ^
// |
// +
//
//          +
//          |
//          v
// |-------->        1   1   1   1   1   F              x/x  (P is vertical)
//
// ^
// |
// +
// |-------->        1  -1  -1  -1  -1   F              u/u  (P is vertical)
//
//      ^
//      |
//      +
// |-------->        1   0  -1   0  -1   F              u/u  (P is vertical)
//
//      +
//      |
//      v
// |-------->        1   0   1   0   1   F              u/x  (P is vertical)
//

template <typename AssignPolicy, bool EnableFirst, bool EnableLast>
struct get_turn_info_for_endpoint
{
    BOOST_STATIC_ASSERT(EnableFirst || EnableLast);

    template<typename Point1,
             typename Point2,
             typename TurnInfo,
             typename IntersectionResult,
             typename RobustPolicy,
             typename OutputIterator
    >
    static inline bool apply(Point1 const& pi, Point1 const& pj, Point1 const& pk,
                             Point2 const& qi, Point2 const& qj, Point2 const& qk,
                             bool is_p_first, bool is_p_last,
                             bool is_q_first, bool is_q_last,
                             TurnInfo const& tp_model,
                             IntersectionResult const& result,
                             RobustPolicy const& robust_policy,
                             method_type /*method*/,
                             OutputIterator out)
    {
        std::size_t ip_count = result.template get<0>().count;
        // no intersection points
        if ( ip_count == 0 )
            return false;

        int segment_index0 = tp_model.operations[0].seg_id.segment_index;
        int segment_index1 = tp_model.operations[1].seg_id.segment_index;
        BOOST_ASSERT(segment_index0 >= 0 && segment_index1 >= 0);

        if ( !is_p_first && !is_p_last && !is_q_first && !is_q_last )
            return false;

        operation_type p_operation0 = operation_none;
        operation_type q_operation0 = operation_none;
        operation_type p_operation1 = operation_none;
        operation_type q_operation1 = operation_none;
        bool p0i, p0j, q0i, q0j; // assign false?
        bool p1i, p1j, q1i, q1j; // assign false?

        bool opposite = result.template get<1>().opposite;

        {
            int p_how = result.template get<1>().how_a;
            int q_how = result.template get<1>().how_b;
            int p_arrival = result.template get<1>().arrival[0];
            int q_arrival = result.template get<1>().arrival[1];
            bool same_dirs = result.template get<1>().dir_a == 0 && result.template get<1>().dir_b == 0;

            handle_segment(is_p_first, is_p_last, p_how, p_arrival,
                           is_q_first, is_q_last, q_how, q_arrival,
                           opposite, ip_count, same_dirs,
                           result.template get<0>().intersections[0],
                           result.template get<0>().intersections[1],
                           p_operation0, q_operation0, p_operation1, q_operation1,
                           p0i, p0j, q0i, q0j,
                           p1i, p1j, q1i, q1j,
                           pi, pj, pk, qi, qj, qk);
        }

        bool append0_last
            = analyse_segment_and_assign_ip(pi, pj, pk, qi, qj, qk,
                                            is_p_first, is_p_last, is_q_first, is_q_last,
                                            p0i, p0j, q0i, q0j,
                                            p_operation0, q_operation0,
                                            tp_model, result, 0,
                                            robust_policy, out);

        // NOTE: opposite && ip_count == 1 may be true!

        // don't ignore only for collinear opposite
        bool result_ignore_ip0 = append0_last && ( ip_count == 1 || !opposite );

        if ( p_operation1 == operation_none )
            return result_ignore_ip0;
        
        bool append1_last
            = analyse_segment_and_assign_ip(pi, pj, pk, qi, qj, qk,
                                            is_p_first, is_p_last, is_q_first, is_q_last,
                                            p1i, p1j, q1i, q1j,
                                            p_operation1, q_operation1,
                                            tp_model, result, 1,
                                            robust_policy, out);

        // don't ignore only for collinear opposite
        bool result_ignore_ip1 = append1_last && !opposite /*&& ip_count == 2*/;

        return result_ignore_ip0 || result_ignore_ip1;
    }

    // TODO remove how_a and how_b
    template<typename Point, typename Point1, typename Point2>
    static inline
    void handle_segment(bool /*first_a*/, bool last_a, int how_a, int arrival_a,
                        bool /*first_b*/, bool last_b, int how_b, int arrival_b,
                        bool opposite, std::size_t ip_count, bool same_dirs/*collinear*/,
                        Point const& ip0, Point const& /*ip1*/,
                        operation_type & op0_a, operation_type & op0_b,
                        operation_type & op1_a, operation_type & op1_b,
                        bool & i0_a, bool & j0_a, bool & i0_b, bool & j0_b,
                        bool & i1_a, bool & j1_a, bool & i1_b, bool & j1_b,
                        Point1 const& pi, Point1 const& /*pj*/, Point1 const& /*pk*/,
                        Point2 const& qi, Point2 const& /*qj*/, Point2 const& /*qk*/)
    {
        namespace ov = overlay;

        i0_a = false; j0_a = false; i0_b = false; j0_b = false;
        i1_a = false; j1_a = false; i1_b = false; j1_b = false;

        if ( same_dirs )
        {
            if ( ip_count == 2 )
            {
                BOOST_ASSERT( how_a == 0 && how_b == 0 );

                if ( !opposite )
                {
                    op0_a = operation_intersection;
                    op0_b = operation_intersection;
                    op1_a = union_or_blocked_same_dirs(arrival_a, last_a);
                    op1_b = union_or_blocked_same_dirs(arrival_b, last_b);

                    i0_a = equals::equals_point_point(pi, ip0);
                    i0_b = equals::equals_point_point(qi, ip0);
                    j1_a = arrival_a != -1;
                    j1_b = arrival_b != -1;
                }
                else
                {
                    op0_a = operation_intersection;
                    op0_b = union_or_blocked_same_dirs(arrival_b, last_b);
                    op1_a = union_or_blocked_same_dirs(arrival_a, last_a);
                    op1_b = operation_intersection;

                    i0_a = arrival_b != 1;
                    j0_b = arrival_b != -1;
                    j1_a = arrival_a != -1;
                    i1_b = arrival_a != 1;
                }
            }
            else
            {
                BOOST_ASSERT(ip_count == 1);
                op0_a = union_or_blocked_same_dirs(arrival_a, last_a);
                op0_b = union_or_blocked_same_dirs(arrival_b, last_b);

                i0_a = arrival_a == -1;
                i0_b = arrival_b == -1;
                j0_a = arrival_a == 0;
                j0_b = arrival_b == 0;
            }
        }
        else
        {
            op0_a = union_or_blocked_different_dirs(arrival_a, last_a);
            op0_b = union_or_blocked_different_dirs(arrival_b, last_b);

            i0_a = arrival_a == -1;
            i0_b = arrival_b == -1;
            j0_a = arrival_a == 1;
            j0_b = arrival_b == 1;
        }
    }

    // only if collinear (same_dirs)
    static inline operation_type union_or_blocked_same_dirs(int arrival, bool is_last)
    {
        if ( arrival == 1 )
            return operation_blocked;
        else if ( arrival == -1 )
            return operation_union;
        else
            return is_last ? operation_blocked : operation_union;
            //return operation_blocked;
    }

    // only if not collinear (!same_dirs)
    static inline operation_type union_or_blocked_different_dirs(int arrival, bool is_last)
    {
        if ( arrival == 1 )
            //return operation_blocked;
            return is_last ? operation_blocked : operation_union;
        else
            return operation_union;
    }

    template <typename Point1,
              typename Point2,
              typename TurnInfo,
              typename IntersectionResult,
              typename RobustPolicy,
              typename OutputIterator>
    static inline
    bool analyse_segment_and_assign_ip(Point1 const& pi, Point1 const& pj, Point1 const& pk,
                                       Point2 const& qi, Point2 const& qj, Point2 const& qk,
                                       bool is_p_first, bool is_p_last,
                                       bool is_q_first, bool is_q_last,
                                       bool is_pi_ip, bool is_pj_ip,
                                       bool is_qi_ip, bool is_qj_ip,
                                       operation_type p_operation,
                                       operation_type q_operation,
                                       TurnInfo const& tp_model,
                                       IntersectionResult const& result,
                                       int ip_index,
                                       RobustPolicy const& robust_policy,
                                       OutputIterator out)
    {
#ifdef BOOST_GEOMETRY_DEBUG_GET_TURNS_LINEAR_LINEAR
        // may this give false positives for INTs?
        typename IntersectionResult::point_type const&
            inters_pt = result.template get<0>().intersections[ip_index];
        BOOST_ASSERT(is_pi_ip == equals::equals_point_point(pi, inters_pt));
        BOOST_ASSERT(is_qi_ip == equals::equals_point_point(qi, inters_pt));
        BOOST_ASSERT(is_pj_ip == equals::equals_point_point(pj, inters_pt));
        BOOST_ASSERT(is_qj_ip == equals::equals_point_point(qj, inters_pt));
#endif

        // TODO - calculate first/last only if needed
        bool is_p_first_ip = is_p_first && is_pi_ip;
        bool is_p_last_ip = is_p_last && is_pj_ip;
        bool is_q_first_ip = is_q_first && is_qi_ip;
        bool is_q_last_ip = is_q_last && is_qj_ip;
        bool append_first = EnableFirst && (is_p_first_ip || is_q_first_ip);
        bool append_last = EnableLast && (is_p_last_ip || is_q_last_ip);

        if ( append_first || append_last )
        {
            bool handled = handle_internal(pi, pj, pk, qi, qj, qk,
                                           is_p_first_ip, is_p_last_ip,
                                           is_q_first_ip, is_q_last_ip,
                                           is_qi_ip, is_qj_ip,
                                           tp_model, result, ip_index, p_operation, q_operation);
            if ( !handled )
            {
                handle_internal(qi, qj, qk, pi, pj, pk,
                                is_q_first_ip, is_q_last_ip,
                                is_p_first_ip, is_p_last_ip,
                                is_pi_ip, is_pj_ip,
                                tp_model, result, ip_index, q_operation, p_operation);
            }

            if ( p_operation != operation_none )
            {
                method_type method = endpoint_ip_method(is_pi_ip, is_pj_ip, is_qi_ip, is_qj_ip);
                turn_position p_pos = ip_position(is_p_first_ip, is_p_last_ip);
                turn_position q_pos = ip_position(is_q_first_ip, is_q_last_ip);

                // handle spikes

                // P is spike and should be handled
                if ( !is_p_last && is_pj_ip
                  && result.template get<0>().count == 2
                  && is_spike_p(pi, pj, pk, qi, qj, qk, robust_policy) )
                {
                    assign(pi, qi, result, ip_index, method, operation_blocked, q_operation,
                           p_pos, q_pos, tp_model, out);
                    assign(pi, qi, result, ip_index, method, operation_intersection, q_operation,
                           p_pos, q_pos, tp_model, out);
                }
                // Q is spike and should be handled
                else if ( !is_q_last && is_qj_ip
                       && result.template get<0>().count == 2
                       && is_spike_q(pi, pj, pk, qi, qj, qk, robust_policy) )
                {
                    assign(pi, qi, result, ip_index, method, p_operation, operation_blocked,
                           p_pos, q_pos, tp_model, out);
                    assign(pi, qi, result, ip_index, method, p_operation, operation_intersection,
                           p_pos, q_pos, tp_model, out);
                }
                // no spikes
                else
                {
                    assign(pi, qi, result, ip_index, method, p_operation, q_operation,
                        p_pos, q_pos, tp_model, out);
                }
            }
        }

        return append_last;
    }

    // TODO: IT'S ALSO PROBABLE THAT ALL THIS FUNCTION COULD BE INTEGRATED WITH handle_segment
    //       however now it's lazily calculated and then it would be always calculated

    template<typename Point1,
             typename Point2,
             typename TurnInfo,
             typename IntersectionResult
    >
    static inline bool handle_internal(Point1 const& i1, Point1 const& j1, Point1 const& /*k1*/,
                                       Point2 const& i2, Point2 const& j2, Point2 const& k2,
                                       bool first1, bool last1,
                                       bool first2, bool last2,
                                       bool ip_i2, bool ip_j2,
                                       TurnInfo const& tp_model,
                                       IntersectionResult const& result,
                                       int ip_index,
                                       operation_type & op1, operation_type & op2)
    {
        boost::ignore_unused_variable_warning(ip_index);
        boost::ignore_unused_variable_warning(tp_model);

        if ( !first2 && !last2 )
        {
            if ( first1 )
            {
#ifdef BOOST_GEOMETRY_DEBUG_GET_TURNS_LINEAR_LINEAR
                // may this give false positives for INTs?
                typename IntersectionResult::point_type const&
                    inters_pt = result.template get<0>().intersections[ip_index];
                BOOST_ASSERT(ip_i2 == equals::equals_point_point(i2, inters_pt));
                BOOST_ASSERT(ip_j2 == equals::equals_point_point(j2, inters_pt));
#endif
                if ( ip_i2 )
                {
                    // don't output this IP - for the first point of other geometry segment
                    op1 = operation_none;
                    op2 = operation_none;
                    return true;
                }
                else if ( ip_j2 )
                {
// NOTE: this conversion may be problematic
                    Point1 i2_conv;
                    geometry::convert(i2, i2_conv);
                    side_calculator<Point1, Point2> side_calc(i2_conv, i1, j1, i2, j2, k2);

                    std::pair<operation_type, operation_type>
                        operations = operations_of_equal(side_calc);

// TODO: must the above be calculated?
// wouldn't it be enough to check if segments are collinear?

                    if ( operations_both(operations, operation_continue) )
                    {
                        // THIS IS WRT THE ORIGINAL SEGMENTS! NOT THE ONES ABOVE!
                        bool opposite = result.template get<1>().opposite;

                        op1 = operation_intersection;
                        op2 = opposite ? operation_union : operation_intersection;
                    }
                    else
                    {
                        BOOST_ASSERT(operations_combination(operations, operation_intersection, operation_union));
                        //op1 = operation_union;
                        //op2 = operation_union;
                    }

                    return true;
                }
                // else do nothing - shouldn't be handled this way
            }
            else if ( last1 )
            {
#ifdef BOOST_GEOMETRY_DEBUG_GET_TURNS_LINEAR_LINEAR
                // may this give false positives for INTs?
                typename IntersectionResult::point_type const&
                    inters_pt = result.template get<0>().intersections[ip_index];
                BOOST_ASSERT(ip_i2 == equals::equals_point_point(i2, inters_pt));
                BOOST_ASSERT(ip_j2 == equals::equals_point_point(j2, inters_pt));
#endif
                if ( ip_i2 )
                {
                    // don't output this IP - for the first point of other geometry segment
                    op1 = operation_none;
                    op2 = operation_none;
                    return true;
                }
                else if ( ip_j2 )
                {
// NOTE: this conversion may be problematic
                    Point1 i2_conv;
                    geometry::convert(i2, i2_conv);
                    
                    side_calculator<Point1, Point2> side_calc(i2_conv, j1, i1, i2, j2, k2);
                    
                    std::pair<operation_type, operation_type>
                        operations = operations_of_equal(side_calc);

// TODO: must the above be calculated?
// wouldn't it be enough to check if segments are collinear?

                    if ( operations_both(operations, operation_continue) )
                    {
                        // THIS IS WRT THE ORIGINAL SEGMENTS! NOT THE ONES ABOVE!
                        bool second_going_out = result.template get<0>().count > 1;

                        op1 = operation_blocked;
                        op2 = second_going_out ? operation_union : operation_intersection;
                    }
                    else
                    {
                        BOOST_ASSERT(operations_combination(operations, operation_intersection, operation_union));
                        //op1 = operation_blocked;
                        //op2 = operation_union;
                    }

                    return true;
                }
                // else do nothing - shouldn't be handled this way
            }
            // else do nothing - shouldn't be handled this way
        }

        return false;
    }

    static inline method_type endpoint_ip_method(bool ip_pi, bool ip_pj, bool ip_qi, bool ip_qj)
    {
        if ( (ip_pi || ip_pj) && (ip_qi || ip_qj) )
            return method_touch;
        else
            return method_touch_interior;
    }

    static inline turn_position ip_position(bool is_ip_first_i, bool is_ip_last_j)
    {
        return is_ip_first_i ? position_front :
               ( is_ip_last_j ? position_back : position_middle );
    }

    template <typename Point1,
              typename Point2,
              typename IntersectionResult,
              typename TurnInfo,
              typename OutputIterator>
    static inline void assign(Point1 const& pi, Point2 const& qi,
                              IntersectionResult const& result,
                              int ip_index,
                              method_type method,
                              operation_type op0, operation_type op1,
                              turn_position pos0, turn_position pos1,
                              TurnInfo const& tp_model,
                              OutputIterator out)
    {
        TurnInfo tp = tp_model;
        
        //geometry::convert(ip, tp.point);
        //tp.method = method;
        base_turn_handler::assign_point(tp, method, result.template get<0>(), ip_index);

        tp.operations[0].operation = op0;
        tp.operations[1].operation = op1;
        tp.operations[0].position = pos0;
        tp.operations[1].position = pos1;

        // NOTE: this probably shouldn't be set for the first point
        // for which there is no preceding segment
        if ( result.template get<0>().count > 1 )
        {
            //BOOST_ASSERT( result.template get<1>().dir_a == 0 && result.template get<1>().dir_b == 0 );
            tp.operations[0].is_collinear = true;
            tp.operations[1].is_collinear = true;
        }
        else //if ( result.template get<0>().count == 1 )
        {
            if ( op0 == operation_blocked && op1 == operation_intersection )
            {
                tp.operations[0].is_collinear = true;
            }
            else if ( op0 == operation_intersection && op1 == operation_blocked )
            {
                tp.operations[1].is_collinear = true;
            }
        }

        AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());
        *out++ = tp;
    }

    template <typename SidePolicy>
    static inline std::pair<operation_type, operation_type> operations_of_equal(SidePolicy const& side)
    {
        int const side_pk_q2 = side.pk_wrt_q2();
        int const side_pk_p = side.pk_wrt_p1();
        int const side_qk_p = side.qk_wrt_p1();

        // If pk is collinear with qj-qk, they continue collinearly.
        // This can be on either side of p1 (== q1), or collinear
        // The second condition checks if they do not continue
        // oppositely
        if ( side_pk_q2 == 0 && side_pk_p == side_qk_p )
        {
            return std::make_pair(operation_continue, operation_continue);
        }

        // If they turn to same side (not opposite sides)
        if ( ! base_turn_handler::opposite(side_pk_p, side_qk_p) )
        {
            int const side_pk_q2 = side.pk_wrt_q2();
            // If pk is left of q2 or collinear: p: union, q: intersection
            if ( side_pk_q2 != -1 )
            {
                return std::make_pair(operation_union, operation_intersection);
            }
            else
            {
               return std::make_pair(operation_intersection, operation_union);
            }
        }
        else
        {
            // They turn opposite sides. If p turns left (or collinear),
           // p: union, q: intersection
            if ( side_pk_p != -1 )
            {
                return std::make_pair(operation_union, operation_intersection);
            }
           else
            {
                return std::make_pair(operation_intersection, operation_union);
            }
        }
   }

    static inline bool operations_both(
                            std::pair<operation_type, operation_type> const& operations,
                            operation_type const op)
    {
        return operations.first == op && operations.second == op;
    }

    static inline bool operations_combination(
                            std::pair<operation_type, operation_type> const& operations,
                            operation_type const op1, operation_type const op2)
    {
        return ( operations.first == op1 && operations.second == op2 )
            || ( operations.first == op2 && operations.second == op1 );
    }
};

}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_GET_TURN_INFO_FOR_ENDPOINT_HPP
