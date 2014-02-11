// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013, 2014.
// Modifications copyright (c) 2013-2014 Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_GET_TURN_INFO_LL_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_GET_TURN_INFO_LL_HPP

#include <boost/geometry/algorithms/detail/overlay/get_turn_info.hpp>

namespace boost { namespace geometry {

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay {

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

// GET_TURN_INFO

template<typename AssignPolicy>
struct get_turn_info_linear_linear
{
    template
    <
        typename Point1,
        typename Point2,
        typename TurnInfo,
        typename RescalePolicy,
        typename OutputIterator
    >
    static inline OutputIterator apply(
                Point1 const& pi, Point1 const& pj, Point1 const& pk,
                Point2 const& qi, Point2 const& qj, Point2 const& qk,
// TODO: should this always be std::size_t or replace with template parameter?
                std::size_t p_segments_count,
                std::size_t q_segments_count,
                TurnInfo const& tp_model,
                RescalePolicy const& , // TODO: this will be used. rescale_policy,
                OutputIterator out)
    {
        typedef model::referring_segment<Point1 const> segment_type1;
        typedef model::referring_segment<Point2 const> segment_type2;
        segment_type1 p1(pi, pj), p2(pj, pk);
        segment_type2 q1(qi, qj), q2(qj, qk);

        side_calculator<Point1, Point2> side_calc(pi, pj, pk, qi, qj, qk);

        typedef strategy_intersection
            <
                typename cs_tag<typename TurnInfo::point_type>::type,
                Point1,
                Point2,
                typename TurnInfo::point_type
            > si;

        typedef typename si::segment_intersection_strategy_type strategy;

        typename strategy::return_type result = strategy::apply(p1, q1);

        char const method = result.template get<1>().how;

        // Copy, to copy possibly extended fields
        TurnInfo tp = tp_model;

        // Select method and apply
        switch(method)
        {
            case 'a' : // collinear, "at"
            case 'f' : // collinear, "from"
            case 's' : // starts from the middle
                handle_first_last(pi, pj, pk, qi, qj, qk, p_segments_count, q_segments_count,
                                  tp_model, result, method_none, out, true, true);
                break;

            case 'd' : // disjoint: never do anything
                break;

            case 'm' :
            {
                if ( handle_first_last(pi, pj, pk, qi, qj, qk, p_segments_count, q_segments_count,
                                       tp_model, result, method_touch_interior, out, false, true) )
                {
                    // do nothing
                }
                else
                {
                    typedef touch_interior
                        <
                            TurnInfo
                        > policy;

                    // If Q (1) arrives (1)
                    if (result.template get<1>().arrival[1] == 1)
                    {
                        policy::template apply<0>(pi, pj, pk, qi, qj, qk,
                                    tp, result.template get<0>(), result.template get<1>(),
                                    side_calc);
                    }
                    else
                    {
                        // Swap p/q
                        side_calculator<Point1, Point2> swapped_side_calc(qi, qj, qk, pi, pj, pk);
                        policy::template apply<1>(qi, qj, qk, pi, pj, pk,
                                    tp, result.template get<0>(), result.template get<1>(),
                                    swapped_side_calc);
                    }

                    replace_method_and_operations_tm(tp.method, tp.operations[0].operation, tp.operations[1].operation);
                    
                    AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());
                    *out++ = tp;
                }
            }
            break;
            case 'i' :
            {
                crosses<TurnInfo>::apply(pi, pj, pk, qi, qj, qk,
                    tp, result.template get<0>(), result.template get<1>());

                replace_operations_i(tp.operations[0].operation, tp.operations[1].operation);

                AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());
                *out++ = tp;
            }
            break;
            case 't' :
            {
                // Both touch (both arrive there)
                if ( handle_first_last(pi, pj, pk, qi, qj, qk, p_segments_count, q_segments_count,
                                       tp_model, result, method_touch, out, false, true) )
                {
                    // do nothing
                }
                else 
                {
                    touch<TurnInfo>::apply(pi, pj, pk, qi, qj, qk,
                        tp, result.template get<0>(), result.template get<1>(), side_calc);

                    replace_method_and_operations_tm(tp.method, tp.operations[0].operation, tp.operations[1].operation);

                    AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());
                    *out++ = tp;
                }
            }
            break;
            case 'e':
            {
                if ( handle_first_last(pi, pj, pk, qi, qj, qk, p_segments_count, q_segments_count,
                                       tp_model, result, method_equal, out, true, true) )
                {
                    // do nothing
                }
                else if ( ! result.template get<1>().opposite )
                {
                    // Both equal
                    // or collinear-and-ending at intersection point
                    equal<TurnInfo>::apply(pi, pj, pk, qi, qj, qk,
                        tp, result.template get<0>(), result.template get<1>(), side_calc);

                    replacer_of_method_and_operations_ec replacer(method_touch);
                    replacer(tp.method, tp.operations[0].operation, tp.operations[1].operation);
                    
                    AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());
                    *out++ = tp;
                }
                else
                {
                    equal_opposite
                        <
                            TurnInfo,
                            AssignPolicy
                        >::apply(pi, qi,
                            tp, out, result.template get<0>(), result.template get<1>());
                }
            }
            break;
            case 'c' :
            {
                // Collinear
                if ( handle_first_last(pi, pj, pk, qi, qj, qk, p_segments_count, q_segments_count,
                                       tp_model, result, method_collinear, out, true, true) )
                {
                    // do nothing
                }
                else if (! result.template get<1>().opposite)
                {

                    if (result.template get<1>().arrival[0] == 0)
                    {
                        // Collinear, but similar thus handled as equal
                        equal<TurnInfo>::apply(pi, pj, pk, qi, qj, qk,
                                tp, result.template get<0>(), result.template get<1>(), side_calc);

                        // NOTE: don't change the method only if methods are WRT IPs, not segments!
                        // (currently this approach is used)
                        // override assigned method
                        //tp.method = method_collinear;

                        replacer_of_method_and_operations_ec replacer(method_touch);
                        replacer(tp.method, tp.operations[0].operation, tp.operations[1].operation);
                    }
                    else
                    {
                        collinear<TurnInfo>::apply(pi, pj, pk, qi, qj, qk,
                                tp, result.template get<0>(), result.template get<1>(), side_calc);

                        replacer_of_method_and_operations_ec replacer(method_touch_interior);
                        replacer(tp.method, tp.operations[0].operation, tp.operations[1].operation);
                    }

                    AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());
                    *out++ = tp;
                }
                else
                {
                    // If this always 'm' ?
                    replacer_of_method_and_operations_ec replacer(method_touch_interior);

                    collinear_opposite
                        <
                            TurnInfo,
                            AssignPolicy
                        >::apply(pi, pj, pk, qi, qj, qk,
                            tp, out, result.template get<0>(), result.template get<1>(), side_calc,
                            replacer);
                }
            }
            break;
            case '0' :
            {
                // degenerate points
                if (AssignPolicy::include_degenerate)
                {
                    only_convert<TurnInfo>::apply(tp, result.template get<0>());
                    AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());
                    *out++ = tp;
                }
            }
            break;
            default :
            {
#if defined(BOOST_GEOMETRY_DEBUG_ROBUSTNESS)
                std::cout << "TURN: Unknown method: " << method << std::endl;
#endif
#if ! defined(BOOST_GEOMETRY_OVERLAY_NO_THROW)
                throw turn_info_exception(method);
#endif
            }
            break;
        }

        return out;
    }

    static inline void replace_method_and_operations_tm(method_type & method,
                                                        operation_type & op0,
                                                        operation_type & op1)
    {
        if ( op0 == operation_blocked && op1 == operation_blocked )
        {
            // NOTE: probably only if methods are WRT IPs, not segments!
            method = (method == method_touch ? method_equal : method_collinear);
            op0 = operation_continue;
            op1 = operation_continue;
        }
        else
        {
            if ( op0 == operation_continue || op0 == operation_blocked )
                op0 = operation_intersection;
            else if ( op0 == operation_intersection )
                op0 = operation_union;

            if ( op1 == operation_continue || op1 == operation_blocked )
                op1 = operation_intersection;
            else if ( op1 == operation_intersection )
                op1 = operation_union;
        }
    }

    class replacer_of_method_and_operations_ec
    {
    public:
        explicit replacer_of_method_and_operations_ec(method_type method_t_or_m)
            : m_method(method_t_or_m)
        {}

       void operator()(method_type & method,
                       operation_type & op0,
                       operation_type & op1) const
        {
            BOOST_ASSERT(op0 != operation_blocked || op1 != operation_blocked );

            if ( op0 == operation_blocked )
                op0 = operation_intersection;
            else if ( op0 == operation_intersection )
                op0 = operation_union;

            if ( op1 == operation_blocked )
                op1 = operation_intersection;
            else if ( op1 == operation_intersection )
                op1 = operation_union;

            if ( op0 == operation_intersection || op0 == operation_union
              || op1 == operation_intersection || op1 == operation_union )
            {
                method = m_method;
            }
        }

    private:
        method_type m_method;
    };

    static inline void replace_operations_i(operation_type & op0, operation_type & op1)
    {
        if ( op0 == operation_intersection )
            op0 = operation_union;

        if ( op1 == operation_intersection )
            op1 = operation_union;
    }

    template<typename Point1,
             typename Point2,
             typename TurnInfo,
             typename IntersectionResult,
             typename OutputIterator
    >
    static inline bool handle_first_last(Point1 const& pi, Point1 const& pj, Point1 const& pk,
                                         Point2 const& qi, Point2 const& qj, Point2 const& qk,
// TODO: should this always be std::size_t or replace with template parameter?
                                         std::size_t p_segments_count,
                                         std::size_t q_segments_count,
                                         TurnInfo const& tp_model,
                                         IntersectionResult const& result,
                                         method_type method,
                                         OutputIterator out,
                                         bool enable_first = true,
                                         bool enable_last = true)
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

        {
            int p_how = result.template get<1>().how_a;
            int q_how = result.template get<1>().how_b;
            int p_arrival = result.template get<1>().arrival[0];
            int q_arrival = result.template get<1>().arrival[1];
            bool opposite = result.template get<1>().opposite;
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

        bool result_ignore_ip = false;

        {
            BOOST_ASSERT(p0i == equals::equals_point_point(pi, result.template get<0>().intersections[0]));
            BOOST_ASSERT(q0i == equals::equals_point_point(qi, result.template get<0>().intersections[0]));
            BOOST_ASSERT(p0j == equals::equals_point_point(pj, result.template get<0>().intersections[0]));
            BOOST_ASSERT(q0j == equals::equals_point_point(qj, result.template get<0>().intersections[0]));
            // TODO - calculate first/last only if needed
            bool p0_first = is_p_first && p0i;
            bool p0_last = is_p_last && p0j;
            bool q0_first = is_q_first && q0i;
            bool q0_last = is_q_last && q0j;
            bool append0_first = enable_first && (p0_first || q0_first);
            bool append0_last = enable_last && (p0_last || q0_last);

            result_ignore_ip = append0_last;

            if ( append0_first || append0_last )
            {
                bool handled = handle_internal(pi, pj, pk, qi, qj, qk,  result.template get<0>().intersections[0],
                                               p0_first, p0_last, q0_first, q0_last, q0i, q0j,
                                               tp_model, result, p_operation0, q_operation0);
                if ( !handled )
                {
                    handled = handle_internal(qi, qj, qk, pi, pj, pk, result.template get<0>().intersections[0],
                                              q0_first, q0_last, p0_first, p0_last, p0i, p0j,
                                              tp_model, result, q_operation0, p_operation0);
                }

                method = endpoint_ip_method(p0i, p0j, q0i, q0j);

                if ( p_operation0 != operation_none )
                    assign(pi, qi, result, result.template get<0>().intersections[0],
                           method, p_operation0, q_operation0,
                           tp_model, out);
            }
        }

        if ( p_operation1 != ov::operation_none )
        {
            BOOST_ASSERT(p1i == equals::equals_point_point(pi, result.template get<0>().intersections[1]));
            BOOST_ASSERT(q1i == equals::equals_point_point(qi, result.template get<0>().intersections[1]));
            BOOST_ASSERT(p1j == equals::equals_point_point(pj, result.template get<0>().intersections[1]));
            BOOST_ASSERT(q1j == equals::equals_point_point(qj, result.template get<0>().intersections[1]));
            // TODO - calculate first/last only if needed
            bool p1_first = is_p_first && p1i;
            bool p1_last = is_p_last && p1j;
            bool q1_first = is_q_first && q1i;
            bool q1_last = is_q_last && q1j;
            bool append1_first = enable_first && (p1_first || q1_first);
            bool append1_last = enable_last && (p1_last || q1_last);

            result_ignore_ip = result_ignore_ip || append1_last;

            if ( append1_first || append1_last )
            {
                bool handled = handle_internal(pi, pj, pk, qi, qj, qk,  result.template get<0>().intersections[1],
                                               p1_first, p1_last, q1_first, q1_last, q1i, q1j,
                                               tp_model, result, p_operation1, q_operation1);
                if ( !handled )
                {
                    handled = handle_internal(qi, qj, qk, pi, pj, pk, result.template get<0>().intersections[1],
                                              q1_first, q1_last, p1_first, p1_last, p1i, p1j,
                                              tp_model, result, q_operation1, p_operation1);
                }

                if ( p_operation1 != operation_none )
                {
                    method = endpoint_ip_method(p1i, p1j, q1i, q1j);

                    assign(pi, qi, result, result.template get<0>().intersections[1],
                           method, p_operation1, q_operation1,
                           tp_model, out);
                }
            }
        }

        return result_ignore_ip;
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

// TODO: IT'S ALSO PROBABLE THAT ALL THIS FUNCTION COULD BE INTEGRATED WITH handle_segment

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
        if ( first1 || last1 )
        {
            if ( !first2 && !last2 )
            {
                BOOST_ASSERT(ip_i2 == equals::equals_point_point(i2, ip));
                BOOST_ASSERT(ip_j2 == equals::equals_point_point(j2, ip));

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
                    if ( first1 )
                    {
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
                            op1 = operation_union;
                            op2 = operation_union;
                        }
                    }
                    else // last1
                    {
                        side_calculator<Point1, Point2> side_calc(i2, j1, i1, i2, j2, k2);
                        equal<TurnInfo>::apply(i2, j1, i1, i2, j2, k2,
                            tp, result.template get<0>(), result.template get<1>(), side_calc);
                        if ( tp.both(operation_continue) )
                        {
                            op1 = operation_blocked;
                            op2 = opposite ? operation_intersection : operation_union;
                        }
                        else
                        {
                            BOOST_ASSERT(tp.combination(operation_intersection, operation_union));
                            op1 = operation_blocked;
                            op2 = operation_union;
                        }
                    }

                    return true;
                }
                else
                {
                    // do nothing
                    // shouldn't be handled this way
                }
            }
        }

        return false;
    }

    static inline method_type endpoint_ip_method(bool ip_pi, bool ip_pj, bool ip_qi, bool ip_qj)
    {
        int pc = (ip_pi ? 1 : 0) + (ip_pj ? 1 : 0);
        int qc = (ip_qi ? 1 : 0) + (ip_qj ? 1 : 0);
        if ( pc > 0 && qc > 0 )
            return method_touch;
        else
            return method_touch_interior;
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
                              TurnInfo const& tp_model,
                              OutputIterator out)
    {
        TurnInfo tp = tp_model;
        geometry::convert(ip, tp.point);
        tp.method = method;
        tp.operations[0].operation = op0;
        tp.operations[1].operation = op1;
        AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());
        *out++ = tp;
    }
};

}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_GET_TURN_INFO_LL_HPP
