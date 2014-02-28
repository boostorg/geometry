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

enum turn_position { position_middle, position_front, position_back };

struct turn_operation_linear
    : public turn_operation
{
    turn_operation_linear()
        : position(position_middle)
    {}

    turn_position position;
};

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

    template<typename Point1,
             typename Point2,
             typename TurnInfo,
             typename IntersectionResult,
             typename OutputIterator
    >
    static inline bool apply(Point1 const& pi, Point1 const& pj, Point1 const& pk,
                             Point2 const& qi, Point2 const& qj, Point2 const& qk,
// TODO: should this always be std::size_t or replace with template parameter?
                             std::size_t p_segments_count,
                             std::size_t q_segments_count,
                             TurnInfo const& tp_model,
                             IntersectionResult const& result,
                             method_type method,
                             OutputIterator out)
    {
        namespace ov = overlay;

        //if ( !enable_first && !enable_last )
        //    return false;

        std::size_t ip_count = result.template get<0>().count;
        // no intersection points
        if ( ip_count == 0 )
            return false;

        int segment_index0 = tp_model.operations[0].seg_id.segment_index;
        int segment_index1 = tp_model.operations[1].seg_id.segment_index;
        BOOST_ASSERT(segment_index0 >= 0 && segment_index1 >= 0);

        bool is_p_first = segment_index0 == 0;
        bool is_q_first = segment_index1 == 0;
        bool is_p_last = static_cast<std::size_t>(segment_index0) + 1 == p_segments_count;
        bool is_q_last = static_cast<std::size_t>(segment_index1) + 1 == q_segments_count;

        if ( !is_p_first && !is_p_last && !is_q_first && !is_q_last )
            return false;

        ov::operation_type p_operation0 = ov::operation_none;
        ov::operation_type q_operation0 = ov::operation_none;
        ov::operation_type p_operation1 = ov::operation_none;
        ov::operation_type q_operation1 = ov::operation_none;
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
                                            result.template get<0>().intersections[0],
                                            is_p_first, is_p_last, is_q_first, is_q_last,
                                            p0i, p0j, q0i, q0j,
                                            p_operation0, q_operation0,
                                            tp_model, result, out);

        bool result_ignore_ip0 = !opposite ? // <=> ip_count == 1 || ip_count == 2 && !opposite
                append0_last :
                (append0_last && (p0j || (is_q_last && q0j && q1i)));
                // NOTE: based on how collinear is calculated for opposite segments

        if ( p_operation1 == ov::operation_none )
            return result_ignore_ip0;
        
        bool append1_last
            = analyse_segment_and_assign_ip(pi, pj, pk, qi, qj, qk,
                                            result.template get<0>().intersections[1],
                                            is_p_first, is_p_last, is_q_first, is_q_last,
                                            p1i, p1j, q1i, q1j,
                                            p_operation1, q_operation1,
                                            tp_model, result, out);

        bool result_ignore_ip1 = !opposite ? // <=> ip_count == 2 && !opposite
                append1_last :
                (append1_last && (q1j || (is_p_last && p1j && p0i)));
                // NOTE: based on how collinear is calculated for opposite segments
                //       this condition is symmetric to the one above

        return result_ignore_ip0 || result_ignore_ip1;
    }

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
                    op1_a = arrival_to_union_or_blocked(arrival_a, last_a);
                    op1_b = arrival_to_union_or_blocked(arrival_b, last_b);

                    i0_a = equals::equals_point_point(pi, ip0);
                    i0_b = equals::equals_point_point(qi, ip0);
                    j1_a = arrival_a != -1;
                    j1_b = arrival_b != -1;
                }
                else
                {
                    op0_a = operation_intersection;
                    op0_b = arrival_to_union_or_blocked(arrival_b, last_b);
                    op1_a = arrival_to_union_or_blocked(arrival_a, last_a);
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
                op0_a = arrival_to_union_or_blocked(arrival_a, last_a);
                op0_b = arrival_to_union_or_blocked(arrival_b, last_b);

                i0_a = how_a == -1;
                i0_b = how_b == -1;
                j0_a = arrival_a == 0;
                j0_b = arrival_b == 0;
            }
        }
        else
        {
            op0_a = how_to_union_or_blocked(how_a, last_a);
            op0_b = how_to_union_or_blocked(how_b, last_b);

            i0_a = how_a == -1;
            i0_b = how_b == -1;
            j0_a = how_a == 1;
            j0_b = how_b == 1;
        }
    }

    // only if collinear (same_dirs)
    static inline operation_type arrival_to_union_or_blocked(int arrival, bool is_last)
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
    static inline operation_type how_to_union_or_blocked(int how, bool is_last)
    {
        if ( how == 1 )
            //return operation_blocked;
            return is_last ? operation_blocked : operation_union;
        else
            return operation_union;
    }

    template <typename Point1,
              typename Point2,
              typename Point,              
              typename TurnInfo,
              typename IntersectionResult,
              typename OutputIterator>
    static inline
    bool analyse_segment_and_assign_ip(Point1 const& pi, Point1 const& pj, Point1 const& pk,
                                       Point2 const& qi, Point2 const& qj, Point2 const& qk,
                                       Point const& ip,
                                       bool is_p_first, bool is_p_last,
                                       bool is_q_first, bool is_q_last,
                                       bool is_pi_ip, bool is_pj_ip,
                                       bool is_qi_ip, bool is_qj_ip,
                                       operation_type p_operation,
                                       operation_type q_operation,
                                       TurnInfo const& tp_model,
                                       IntersectionResult const& result,
                                       OutputIterator out)
    {
#ifdef BOOST_GEOMETRY_DEBUG_GET_TURNS_LINEAR_LINEAR
        // may this give false positives for INTs?
        BOOST_ASSERT(is_pi_ip == equals::equals_point_point(pi, ip));
        BOOST_ASSERT(is_qi_ip == equals::equals_point_point(qi, ip));
        BOOST_ASSERT(is_pj_ip == equals::equals_point_point(pj, ip));
        BOOST_ASSERT(is_qj_ip == equals::equals_point_point(qj, ip));
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
            bool handled = handle_internal(pi, pj, pk, qi, qj, qk, ip,
                                           is_p_first_ip, is_p_last_ip, is_q_first_ip, is_q_last_ip, is_qi_ip, is_qj_ip,
                                           tp_model, result, p_operation, q_operation);
            if ( !handled )
            {
                handle_internal(qi, qj, qk, pi, pj, pk, ip,
                                is_q_first_ip, is_q_last_ip, is_p_first_ip, is_p_last_ip, is_pi_ip, is_pj_ip,
                                tp_model, result, q_operation, p_operation);
            }

            if ( p_operation != operation_none )
            {
                assign(pi, qi, result, ip,
                       endpoint_ip_method(is_pi_ip, is_pj_ip, is_qi_ip, is_qj_ip),
                       p_operation, q_operation,
                       ip_position(is_p_first_ip, is_p_last_ip),
                       ip_position(is_q_first_ip, is_q_last_ip),
                       tp_model, out);
            }
        }

        return append_last;
    }

    // TODO: IT'S ALSO PROBABLE THAT ALL THIS FUNCTION COULD BE INTEGRATED WITH handle_segment
    //       however now it's lazily calculated and then it would be always calculated

    template<typename Point1,
             typename Point2,
             typename Point,
             typename TurnInfo,
             typename IntersectionResult
    >
    static inline bool handle_internal(Point1 const& i1, Point1 const& j1, Point1 const& /*k1*/,
                                       Point2 const& i2, Point2 const& j2, Point2 const& k2,
                                       Point const& ip,
                                       bool first1, bool last1, bool first2, bool last2,
                                       bool ip_i2, bool ip_j2,
                                       TurnInfo const& tp_model,
                                       IntersectionResult const& result,
                                       operation_type & op1, operation_type & op2)
    {
        if ( !first2 && !last2 )
        {
            if ( first1 )
            {
#ifdef BOOST_GEOMETRY_DEBUG_GET_TURNS_LINEAR_LINEAR
                // may this give false positives for INTs?
                BOOST_ASSERT(ip_i2 == equals::equals_point_point(i2, ip));
                BOOST_ASSERT(ip_j2 == equals::equals_point_point(j2, ip));
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
                    bool opposite = result.template get<1>().opposite;

                    TurnInfo tp = tp_model;
                    side_calculator<Point1, Point2> side_calc(i2, i1, j1, i2, j2, k2);
                    equal<TurnInfo>::apply(i2, i1, j1, i2, j2, k2,
                        tp, result.template get<0>(), result.template get<1>(), side_calc);

                    if ( tp.both(operation_continue) )
                    {
                        op1 = operation_intersection;
                        op2 = opposite ? operation_union : operation_intersection;
                    }
                    else
                    {
                        BOOST_ASSERT(tp.combination(operation_intersection, operation_union));
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
                BOOST_ASSERT(ip_i2 == equals::equals_point_point(i2, ip));
                BOOST_ASSERT(ip_j2 == equals::equals_point_point(j2, ip));
#endif
                if ( ip_j2 )
                {
                    // don't output this IP - for the first point of other geometry segment
                    op1 = operation_none;
                    op2 = operation_none;
                    return true;
                }
                else if ( ip_i2 )
                {
                    bool opposite = result.template get<1>().opposite;

                    TurnInfo tp = tp_model;
                    side_calculator<Point1, Point2> side_calc(j2, j1, i1, i2, j2, k2);
                    equal<TurnInfo>::apply(j2, j1, i1, i2, j2, k2,
                        tp, result.template get<0>(), result.template get<1>(), side_calc);

                    if ( tp.both(operation_continue) )
                    {
                        op1 = operation_blocked;
                        op2 = opposite ? operation_intersection : operation_union;
                    }
                    else
                    {
                        BOOST_ASSERT(tp.combination(operation_intersection, operation_union));
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
              typename Point,
              typename TurnInfo,
              typename OutputIterator>
    static inline void assign(Point1 const& pi, Point2 const& qi,
                              IntersectionResult const& result,
                              Point const& ip,
                              method_type method,
                              operation_type op0, operation_type op1,
                              turn_position pos0, turn_position pos1,
                              TurnInfo const& tp_model,
                              OutputIterator out)
    {
        TurnInfo tp = tp_model;
        geometry::convert(ip, tp.point);
        tp.method = method;
        tp.operations[0].operation = op0;
        tp.operations[1].operation = op1;
        tp.operations[0].position = pos0;
        tp.operations[1].position = pos1;
        AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());
        *out++ = tp;
    }
};

}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_GET_TURN_INFO_FOR_ENDPOINT_HPP
