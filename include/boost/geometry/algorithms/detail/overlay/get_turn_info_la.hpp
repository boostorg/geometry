// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013, 2014.
// Modifications copyright (c) 2013-2014 Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_GET_TURN_INFO_LA_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_GET_TURN_INFO_LA_HPP

#include <boost/geometry/algorithms/detail/overlay/get_turn_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_turn_info_for_endpoint.hpp>

// TEMP, for spikes detector
//#include <boost/geometry/algorithms/detail/overlay/get_turn_info_ll.hpp>

namespace boost { namespace geometry {

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay {

template<typename AssignPolicy>
struct get_turn_info_linear_areal
{
    static const bool handle_spikes = true;

    template
    <
        typename Point1,
        typename Point2,
        typename TurnInfo,
        typename RobustPolicy,
        typename OutputIterator
    >
    static inline OutputIterator apply(
                Point1 const& pi, Point1 const& pj, Point1 const& pk,
                Point2 const& qi, Point2 const& qj, Point2 const& qk,
                bool is_p_first, bool is_p_last,
                bool is_q_first, bool is_q_last,
                TurnInfo const& tp_model,
                RobustPolicy const& robust_policy,
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
                typename TurnInfo::point_type,
                RobustPolicy
            > si;

        typedef typename si::segment_intersection_strategy_type strategy;

        typename strategy::return_type result = strategy::apply(p1, q1, robust_policy);

        char const method = result.template get<1>().how;

        // Copy, to copy possibly extended fields
        TurnInfo tp = tp_model;

        // Select method and apply
        switch(method)
        {
            case 'a' : // collinear, "at"
            case 'f' : // collinear, "from"
            case 's' : // starts from the middle
                get_turn_info_for_endpoint<true, true>(
                    pi, pj, pk, qi, qj, qk,
                    is_p_first, is_p_last, is_q_first, is_q_last,
                    tp_model, result, method_none, out);
                break;

            case 'd' : // disjoint: never do anything
                break;

            case 'm' :
            {
                if ( get_turn_info_for_endpoint<false, true>(
                        pi, pj, pk, qi, qj, qk,
                        is_p_first, is_p_last, is_q_first, is_q_last,
                        tp_model, result, method_touch_interior, out) )
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

                    if ( tp.operations[1].operation == operation_blocked )
                    {
                        tp.operations[0].is_collinear = true;
                    }

                    replace_method_and_operations_tm(tp.method,
                                                     tp.operations[0].operation,
                                                     tp.operations[1].operation);
                    
                    // this function assumes that 'u' must be set for a spike
                    calculate_spike_operation(tp.operations[0].operation,
                                              side_calc, p1, p2,
                                              is_p_last, robust_policy);
                    
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
                if ( get_turn_info_for_endpoint<false, true>(
                        pi, pj, pk, qi, qj, qk,
                        is_p_first, is_p_last, is_q_first, is_q_last,
                        tp_model, result, method_touch, out) )
                {
                    // do nothing
                }
                else 
                {
                    touch<TurnInfo>::apply(pi, pj, pk, qi, qj, qk,
                        tp, result.template get<0>(), result.template get<1>(), side_calc);

                    if ( tp.operations[1].operation == operation_blocked )
                    {
                        tp.operations[0].is_collinear = true;
                    }

                    replace_method_and_operations_tm(tp.method,
                                                     tp.operations[0].operation,
                                                     tp.operations[1].operation);

                    // this function assumes that 'u' must be set for a spike
                    bool ignore_spike
                        = calculate_spike_operation(tp.operations[0].operation,
                                                    side_calc, p1, p2,
                                                    is_p_last, robust_policy);

// TODO: move this into the append_xxx and call for each turn?
                    AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());

                    if ( ! handle_spikes
                      || ignore_spike
                      || ! append_opposite_spikes<append_touches>( // for 'i' or 'c'
                                tp, result, side_calc,
                                p1, p2, q1, q2,
                                is_p_last, is_q_last,
                                robust_policy, out) )
                    {
                        *out++ = tp;
                    }
                }
            }
            break;
            case 'e':
            {
                if ( get_turn_info_for_endpoint<true, true>(
                        pi, pj, pk, qi, qj, qk,
                        is_p_first, is_p_last, is_q_first, is_q_last,
                        tp_model, result, method_equal, out) )
                {
                    // do nothing
                }
                else
                {
                    tp.operations[0].is_collinear = true;

                    if ( ! result.template get<1>().opposite )
                    {
                        // Both equal
                        // or collinear-and-ending at intersection point
                        equal<TurnInfo>::apply(pi, pj, pk, qi, qj, qk,
                            tp, result.template get<0>(), result.template get<1>(), side_calc);

                        replacer_of_method_and_operations_ec<false> replacer(method_touch);
                        replacer(tp.method, tp.operations[0].operation, tp.operations[1].operation);
                    
// TODO: move this into the append_xxx and call for each turn?
                        AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());
                        
                        // conditionally handle spikes
                        if ( ! handle_spikes
                          || ! append_collinear_spikes(
                                    tp, side_calc, p1, p2, q1, q2,
                                    is_p_last, is_q_last,
                                    method_touch, append_equal,
                                    robust_policy, out) )
                        {
                            *out++ = tp; // no spikes
                        }
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
            }
            break;
            case 'c' :
            {
                // Collinear
                if ( get_turn_info_for_endpoint<true, true>(
                        pi, pj, pk, qi, qj, qk,
                        is_p_first, is_p_last, is_q_first, is_q_last,
                        tp_model, result, method_collinear, out) )
                {
                    // do nothing
                }
                else
                {
                    tp.operations[0].is_collinear = true;

                    if (! result.template get<1>().opposite)
                    {
                        method_type method_replace = method_touch_interior;
                        append_version_c version = append_collinear;

                        if (result.template get<1>().arrival[0] == 0)
                        {
                            // Collinear, but similar thus handled as equal
                            equal<TurnInfo>::apply(pi, pj, pk, qi, qj, qk,
                                    tp, result.template get<0>(), result.template get<1>(), side_calc);

                            method_replace = method_touch;
                            version = append_equal;
                        }
                        else
                        {
                            collinear<TurnInfo>::apply(pi, pj, pk, qi, qj, qk,
                                    tp, result.template get<0>(), result.template get<1>(), side_calc);

                            //method_replace = method_touch_interior;
                            //version = append_collinear;
                        }

                        replacer_of_method_and_operations_ec<false> replacer(method_replace);
                        replacer(tp.method, tp.operations[0].operation, tp.operations[1].operation);

// TODO: move this into the append_xxx and call for each turn?
                        AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());
                        
                        // conditionally handle spikes
                        if ( ! handle_spikes
                          || ! append_collinear_spikes(
                                    tp, side_calc, p1, p2, q1, q2,
                                    is_p_last, is_q_last,
                                    method_replace, version,
                                    robust_policy, out) )
                        {
                            // no spikes
                            *out++ = tp;
                        }
                    }
                    else
                    {
                        // Is this always 'm' ?
                        replacer_of_method_and_operations_ec<false> replacer(method_touch_interior);

                        // conditionally handle spikes
                        if ( handle_spikes )
                        {
                            append_opposite_spikes<append_collinear_opposite>(
                                    tp, result, side_calc,
                                    p1, p2, q1, q2,
                                    is_p_last, is_q_last,
                                    robust_policy, out);
                        }

                        // TODO: ignore for spikes?
                        //       E.g. pass is_p_valid = !is_p_last && !is_pj_spike,
                        //       the same with is_q_valid

                        collinear_opposite
                            <
                                TurnInfo,
                                AssignPolicy
                            >::apply(pi, pj, pk, qi, qj, qk,
                                tp, out, result.template get<0>(), result.template get<1>(), side_calc,
                                replacer);
                    }
                }
            }
            break;
            case '0' :
            {
                // degenerate points
                if (AssignPolicy::include_degenerate)
                {
                    only_convert::apply(tp, result.template get<0>());

                    if ( is_p_first
                      && equals::equals_point_point(pi, tp.point) )
                    {
                        tp.operations[0].position = position_front;
                    }
                    else if ( is_p_last
                           && equals::equals_point_point(pj, tp.point) )
                    {
                        tp.operations[0].position = position_back;
                    }
                    // tp.operations[1].position = position_middle;

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

    template <typename Operation,
              typename SideCalc,
              typename PSegment,
              typename RobustPolicy>
    static inline bool calculate_spike_operation(Operation & op,
                                                 SideCalc const& side_calc,
                                                 PSegment const& p1,
                                                 PSegment const& p2,
                                                 bool is_p_last,
                                                 RobustPolicy const& robust_policy)
    {
        bool is_p_spike = op == operation_union
                       && ! is_p_last
                       && is_spike_p(side_calc, p1, p2, robust_policy);

        // we don't know where the spike is going since for both directions 'u' is set
        if ( is_p_spike )
        {
            if ( side_calc.pk_wrt_q1() < 0 )
            {
                op = operation_intersection;
                return true;
            }
        }

        return false;
    }

    enum append_version_c { append_equal, append_collinear };

    template <typename TurnInfo,
              typename SideCalc,
              typename SegmentP,
              typename SegmentQ,
              typename RobustPolicy,
              typename OutIt>
    static inline bool append_collinear_spikes(TurnInfo & tp,
                                               SideCalc const& side_calc,
                                               SegmentP const& p1, SegmentP const& p2,
                                               SegmentQ const& q1, SegmentQ const& q2,
                                               bool is_p_last, bool is_q_last,
                                               method_type method, append_version_c version,
                                               RobustPolicy const& robust_policy, OutIt out)
    {
        // method == touch || touch_interior
        // both position == middle

        bool is_p_spike = ( version == append_equal ?
                            ( tp.operations[0].operation == operation_union
                           || tp.operations[0].operation == operation_intersection ) :
                            tp.operations[0].operation == operation_continue )
                       && ! is_p_last
                       && is_spike_p(side_calc, p1, p2, robust_policy);

        // TODO: throw an exception for spike in Areal?
        /*bool is_q_spike = tp.operations[1].operation == spike_op
                       && ! is_q_last
                       && is_spike_q(side_calc, q1, q2, robust_policy);*/

        if ( is_p_spike )
        {
            tp.method = method;
            tp.operations[0].operation = operation_blocked;
            tp.operations[1].operation = operation_union;
            *out++ = tp;
            tp.operations[0].operation = operation_continue; // boundary
            //tp.operations[1].operation = operation_union;
            *out++ = tp;

            return true;
        }
        
        return false;
    }

    enum append_version_o { append_touches, append_collinear_opposite };

    template <append_version_o Version,
              typename TurnInfo,
              typename Result,
              typename SideCalc,
              typename SegmentP,
              typename SegmentQ,
              typename RobustPolicy,
              typename OutIt>
    static inline bool append_opposite_spikes(TurnInfo & tp,
                                              Result const& result,
                                              SideCalc const& side_calc,
                                              SegmentP const& p1, SegmentP const& p2,
                                              SegmentQ const& q1, SegmentQ const& q2,
                                              bool is_p_last, bool is_q_last,
                                              RobustPolicy const& robust_policy, OutIt out)
    {
        bool is_p_spike = ( Version == append_touches ?
                            ( tp.operations[0].operation == operation_continue
                           || tp.operations[0].operation == operation_intersection ) :
                            true )
                       && ! is_p_last
                       && is_spike_p(side_calc, p1, p2, robust_policy);
        // TODO: throw an exception for spike in Areal?
        /*bool is_q_spike = ( Version == append_touches ?
                            ( tp.operations[1].operation == operation_continue
                           || tp.operations[1].operation == operation_intersection ) :
                            true )
                       && ! is_q_last
                       && is_spike_q(side_calc, q1, q2, robust_policy);*/

        if ( is_p_spike && ( Version == append_touches || result.template get<1>().arrival[0] == 1 ) )
        {
            if ( Version == append_touches )
            {
                tp.operations[0].is_collinear = true;
                //tp.operations[1].is_collinear = ???
                tp.method = method_touch;
            }
            else
            {
                //tp.operations[0].is_collinear = true;
                //tp.operations[1].is_collinear = true;
                tp.method = method_touch_interior; // only because arrival != 0
                BOOST_ASSERT(result.template get<0>().count > 1);
                geometry::convert(result.template get<0>().intersections[1], tp.point);
                AssignPolicy::apply(tp, p1.first, q1.first, result.template get<0>(), result.template get<1>());
            }

            tp.operations[0].operation = operation_blocked;
            tp.operations[1].operation = operation_continue; // boundary
            *out++ = tp;
            tp.operations[0].operation = operation_continue; // boundary
            //tp.operations[1].operation = operation_continue; // boundary
            *out++ = tp;

            return true;
        }

        return false;
    }

    static inline void replace_method_and_operations_tm(method_type & method,
                                                        operation_type & op0,
                                                        operation_type & op1)
    {
        if ( op0 == operation_blocked && op1 == operation_blocked )
        {
            // NOTE: probably only if methods are WRT IPs, not segments!
            method = (method == method_touch ? method_equal : method_collinear);
        }

        // Assuming G1 is always Linear
        if ( op0 == operation_blocked )
        {
            op0 = operation_continue;
        }

        if ( op1 == operation_blocked )
        {
            op1 = operation_continue;
        }
        else if ( op1 == operation_intersection )
        {
            op1 = operation_union;
        }

        // spikes in 'm'
        if ( method == method_error )
        {
            method = method_touch_interior;
            op0 = operation_union;
            op1 = operation_union;
        }
    }

    template <bool IsFront>
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
            // NOTE: probably only if methods are WRT IPs, not segments!
            if ( IsFront
              || op0 == operation_intersection || op0 == operation_union
              || op1 == operation_intersection || op1 == operation_union )
            {
                method = m_method;
            }

            // Assuming G1 is always Linear
            if ( op0 == operation_blocked )
            {
                op0 = operation_continue;
            }

            if ( op1 == operation_blocked )
            {
                op1 = operation_continue;
            }
            else if ( op1 == operation_intersection )
            {
                op1 = operation_union;
            }
        }

    private:
        method_type m_method;
    };

    static inline void replace_operations_i(operation_type & /*op0*/, operation_type & op1)
    {
        // assuming Linear is always the first one
        op1 = operation_union;
    }

    // NOTE: Spikes may NOT be handled for Linear endpoints because it's not
    //       possible to define a spike on an endpoint. Areal geometries must
    //       NOT have spikes at all. One thing that could be done is to throw
    //       an exception when spike is detected in Areal geometry.
    
    template <bool EnableFirst,
              bool EnableLast,
              typename Point1,
              typename Point2,
              typename TurnInfo,
              typename IntersectionResult,
              typename OutputIterator>
    static inline bool get_turn_info_for_endpoint(
                            Point1 const& pi, Point1 const& pj, Point1 const& pk,
                            Point2 const& qi, Point2 const& qj, Point2 const& qk,
                            bool is_p_first, bool is_p_last,
                            bool is_q_first, bool is_q_last,
                            TurnInfo const& tp_model,
                            IntersectionResult const& result,
                            method_type /*method*/,
                            OutputIterator out)
    {
        namespace ov = overlay;
        typedef ov::get_turn_info_for_endpoint<AssignPolicy, EnableFirst, EnableLast> get_info_e;

        const std::size_t ip_count = result.template get<0>().count;
        // no intersection points
        if ( ip_count == 0 )
            return false;

        const int segment_index0 = tp_model.operations[0].seg_id.segment_index;
        const int segment_index1 = tp_model.operations[1].seg_id.segment_index;
        BOOST_ASSERT(segment_index0 >= 0 && segment_index1 >= 0);

        if ( !is_p_first && !is_p_last )
            return false;

        ov::operation_type p_operation0 = ov::operation_none;
        ov::operation_type q_operation0 = ov::operation_none;
        ov::operation_type p_operation1 = ov::operation_none;
        ov::operation_type q_operation1 = ov::operation_none;
        bool p0i, p0j, q0i, q0j; // assign false?
        bool p1i, p1j, q1i, q1j; // assign false?

        const bool opposite = result.template get<1>().opposite;
        const bool same_dirs = result.template get<1>().dir_a == 0 && result.template get<1>().dir_b == 0;

        {
            const int p_how = result.template get<1>().how_a;
            const int q_how = result.template get<1>().how_b;
            const int p_arrival = result.template get<1>().arrival[0];
            const int q_arrival = result.template get<1>().arrival[1];

            get_info_e::handle_segment(
                           is_p_first, is_p_last, p_how, p_arrival,
                           is_q_first, is_q_last, q_how, q_arrival,
                           opposite, ip_count, same_dirs,
                           result.template get<0>().intersections[0],
                           result.template get<0>().intersections[1],
                           p_operation0, q_operation0, p_operation1, q_operation1,
                           p0i, p0j, q0i, q0j,
                           p1i, p1j, q1i, q1j,
                           pi, pj, pk, qi, qj, qk);
        }

        // ANALYSE AND ASSIGN FIRST

        // IP on the first point of Linear Geometry
        if ( EnableFirst && is_p_first && p0i && !q0i ) // !q0i prevents duplication
        {
            TurnInfo tp = tp_model;
            tp.operations[0].position = position_front;
            tp.operations[1].position = position_middle;

            if ( opposite ) // opposite -> collinear
            {
                tp.operations[0].operation = operation_continue;
                tp.operations[1].operation = operation_union;
                tp.method = q0j ? method_touch : method_touch_interior;
            }
            else
            {
                // NOTE: the conversion may be problematic
                Point1 qi_conv;
                geometry::convert(qi, qi_conv);

                method_type replaced_method = method_touch_interior;

                if ( q0j )
                {
                    side_calculator<Point1, Point2> side_calc(qi_conv, pi, pj, qi, qj, qk);

                    std::pair<operation_type, operation_type>
                        operations = get_info_e::operations_of_equal(side_calc);

                    tp.operations[0].operation = operations.first;
                    tp.operations[1].operation = operations.second;

                    replaced_method = method_touch;
                }
                else
                {
// NOTE: the conversion may be problematic
                    Point2 pi_conv;
                    geometry::convert(pi, pi_conv);

                    side_calculator<Point1, Point2> side_calc(qi_conv, pi, pj, qi, pi_conv, qj);

                    std::pair<operation_type, operation_type>
                        operations = get_info_e::operations_of_equal(side_calc);

                    tp.operations[0].operation = operations.first;
                    tp.operations[1].operation = operations.second;
                }

                replacer_of_method_and_operations_ec<true> replacer(replaced_method);
                replacer(tp.method, tp.operations[0].operation, tp.operations[1].operation);
            }

            // equals<> or collinear<> will assign the second point,
            // we'd like to assign the first one
            //geometry::convert(result.template get<0>().intersections[0], tp.point);
            base_turn_handler::assign_point(tp, tp.method, result.template get<0>(), 0);

            // NOTE: not really needed especially for the first point
            // for which there is no preceding segment (but consistent with the L/L)
            if ( result.template get<0>().count > 1 )
            {
                //BOOST_ASSERT( result.template get<1>().dir_a == 0 && result.template get<1>().dir_b == 0 );
                tp.operations[0].is_collinear = true;
            }

            AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());
            *out++ = tp;
        }

        // ANALYSE AND ASSIGN LAST

        // IP on the last point of Linear Geometry
        if ( EnableLast
          && is_p_last
          && ( ip_count > 1 ? (p1j && !q1i) : (p0j && !q0i) ) ) // prevents duplication
        {
            TurnInfo tp = tp_model;
            
            if ( result.template get<0>().count > 1 )
            {
                //BOOST_ASSERT( result.template get<1>().dir_a == 0 && result.template get<1>().dir_b == 0 );
                tp.operations[0].is_collinear = true;
                tp.operations[1].operation = opposite ? operation_continue : operation_union;
            }
            else //if ( result.template get<0>().count == 1 )
            {
                Point1 qi_conv;
                geometry::convert(qi, qi_conv);
                side_calculator<Point1, Point2> side_calc(qi_conv, pj, pi, qi, qj, qk);

                std::pair<operation_type, operation_type>
                    operations = get_info_e::operations_of_equal(side_calc);

                tp.operations[0].operation = operations.first;
                tp.operations[1].operation = operations.second;

                replacer_of_method_and_operations_ec<false> replacer(method_none);
                replacer(tp.method, tp.operations[0].operation, tp.operations[1].operation);

                if ( tp.both(operation_continue) )
                {
                    tp.operations[0].is_collinear = true;
                }
            }

            tp.method = ( ip_count > 1 ? q1j : q0j ) ? method_touch : method_touch_interior;
            tp.operations[0].operation = operation_blocked;
            tp.operations[0].position = position_back;
            tp.operations[1].position = position_middle;
            
            // equals<> or collinear<> will assign the second point,
            // we'd like to assign the first one
            std::size_t ip_index = ip_count > 1 ? 1 : 0;
            //geometry::convert(result.template get<0>().intersections[ip_index], tp.point);
            base_turn_handler::assign_point(tp, tp.method, result.template get<0>(), ip_index);

            AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());
            *out++ = tp;

            return true;
        }

        // don't ignore anything for now
        return false;
    }
};

}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_GET_TURN_INFO_LA_HPP
