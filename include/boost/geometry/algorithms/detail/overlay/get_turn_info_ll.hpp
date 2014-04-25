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
#include <boost/geometry/algorithms/detail/overlay/get_turn_info_for_endpoint.hpp>

namespace boost { namespace geometry {

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay {

template<typename AssignPolicy>
struct get_turn_info_linear_linear
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
                get_turn_info_for_endpoint<AssignPolicy, true, true>
                    ::apply(pi, pj, pk, qi, qj, qk,
                            is_p_first, is_p_last, is_q_first, is_q_last,
                            tp_model, result, robust_policy, method_none, out);
                break;

            case 'd' : // disjoint: never do anything
                break;

            case 'm' :
            {
                if ( get_turn_info_for_endpoint<AssignPolicy, false, true>
                        ::apply(pi, pj, pk, qi, qj, qk,
                                is_p_first, is_p_last, is_q_first, is_q_last,
                                tp_model, result, robust_policy, method_touch_interior, out) )
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
                        side_calculator<Point2, Point1> swapped_side_calc(qi, qj, qk, pi, pj, pk);
                        policy::template apply<1>(qi, qj, qk, pi, pj, pk,
                                    tp, result.template get<0>(), result.template get<1>(),
                                    swapped_side_calc);
                    }
                    
                    if ( tp.operations[0].operation == operation_blocked )
                    {
                        tp.operations[1].is_collinear = true;
                    }
                    if ( tp.operations[1].operation == operation_blocked )
                    {
                        tp.operations[0].is_collinear = true;
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
                if ( get_turn_info_for_endpoint<AssignPolicy, false, true>
                        ::apply(pi, pj, pk, qi, qj, qk,
                                is_p_first, is_p_last, is_q_first, is_q_last,
                                tp_model, result, robust_policy, method_touch, out) )
                {
                    // do nothing
                }
                else 
                {
                    touch<TurnInfo>::apply(pi, pj, pk, qi, qj, qk,
                        tp, result.template get<0>(), result.template get<1>(), side_calc);

                    if ( tp.operations[0].operation == operation_blocked )
                    {
                        tp.operations[1].is_collinear = true;
                    }
                    if ( tp.operations[1].operation == operation_blocked )
                    {
                        tp.operations[0].is_collinear = true;
                    }

                    replace_method_and_operations_tm(tp.method,
                                                     tp.operations[0].operation,
                                                     tp.operations[1].operation);

// TODO: move this into the append_xxx and call for each turn?
                    AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());

                    if ( ! handle_spikes
                      || ! append_opposite_spikes<append_touches>(
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
                if ( get_turn_info_for_endpoint<AssignPolicy, true, true>
                        ::apply(pi, pj, pk, qi, qj, qk,
                                is_p_first, is_p_last, is_q_first, is_q_last,
                                tp_model, result, robust_policy, method_equal, out) )
                {
                    // do nothing
                }
                else
                {
                    tp.operations[0].is_collinear = true;
                    tp.operations[1].is_collinear = true;

                    if ( ! result.template get<1>().opposite )
                    {
                        // Both equal
                        // or collinear-and-ending at intersection point
                        equal<TurnInfo>::apply(pi, pj, pk, qi, qj, qk,
                            tp, result.template get<0>(), result.template get<1>(), side_calc);

                        replacer_of_method_and_operations_ec replacer(method_touch);
                        replacer(tp.method, tp.operations[0].operation, tp.operations[1].operation);

// TODO: move this into the append_xxx and call for each turn?
                        AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());

                        // conditionally handle spikes
                        if ( ! handle_spikes
                          || ! append_collinear_spikes(tp, side_calc, p1, p2, q1, q2,
                                                       is_p_last, is_q_last,
                                                       method_touch, operation_union,
                                                       robust_policy, out) )
                        {
                            *out++ = tp; // no spikes
                        }
                    }
                    else
                    {
                        // TODO: ignore for spikes or generate something else than opposite?

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
                if ( get_turn_info_for_endpoint<AssignPolicy, true, true>
                        ::apply(pi, pj, pk, qi, qj, qk,
                                is_p_first, is_p_last, is_q_first, is_q_last,
                                tp_model, result, robust_policy, method_collinear, out) )
                {
                    // do nothing
                }
                else
                {
                    tp.operations[0].is_collinear = true;
                    tp.operations[1].is_collinear = true;

                    if (! result.template get<1>().opposite)
                    {
                        method_type method_replace = method_touch_interior;
                        operation_type spike_op = operation_continue;

                        if (result.template get<1>().arrival[0] == 0)
                        {
                            // Collinear, but similar thus handled as equal
                            equal<TurnInfo>::apply(pi, pj, pk, qi, qj, qk,
                                    tp, result.template get<0>(), result.template get<1>(), side_calc);

                            method_replace = method_touch;
                            spike_op = operation_union;
                        }
                        else
                        {
                            collinear<TurnInfo>::apply(pi, pj, pk, qi, qj, qk,
                                    tp, result.template get<0>(), result.template get<1>(), side_calc);

                            //method_replace = method_touch_interior;
                            //spike_op = operation_continue;
                        }

                        replacer_of_method_and_operations_ec replacer(method_replace);
                        replacer(tp.method, tp.operations[0].operation, tp.operations[1].operation);
                        
// TODO: move this into the append_xxx and call for each turn?
                        AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());

                        // conditionally handle spikes
                        if ( ! handle_spikes
                          || ! append_collinear_spikes(tp, side_calc, p1, p2, q1, q2,
                                                       is_p_last, is_q_last,
                                                       method_replace, spike_op,
                                                       robust_policy, out) )
                        {
                            // no spikes
                            *out++ = tp;
                        }
                    }
                    else
                    {
                        // If this always 'm' ?
                        replacer_of_method_and_operations_ec replacer(method_touch_interior);

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
                                replacer, !is_p_last, !is_q_last);
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

                    // if any, only one of those should be true
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
                    else if ( is_q_first
                           && equals::equals_point_point(qi, tp.point) )
                    {
                        tp.operations[1].position = position_front;
                    }
                    else if ( is_q_last
                           && equals::equals_point_point(qj, tp.point) )
                    {
                        tp.operations[1].position = position_back;
                    }

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
                                               method_type method, operation_type spike_op,
                                               RobustPolicy const& robust_policy, OutIt out)
    {
        // method == touch || touch_interior
        // both position == middle

        bool is_p_spike = tp.operations[0].operation == spike_op
                       && ! is_p_last
                       && is_spike_p(side_calc, p1, p2, robust_policy);
        bool is_q_spike = tp.operations[1].operation == spike_op
                       && ! is_q_last
                       && is_spike_q(side_calc, q1, q2, robust_policy);

        if ( is_p_spike && is_q_spike )
        {
            tp.method = method;
            tp.operations[0].operation = operation_blocked;
            tp.operations[1].operation = operation_blocked;
            *out++ = tp;
            tp.operations[0].operation = operation_intersection;
            tp.operations[1].operation = operation_intersection;
            *out++ = tp;

            return true;
        }
        else if ( is_p_spike )
        {
            tp.method = method;
            tp.operations[0].operation = operation_blocked;
            tp.operations[1].operation = operation_union;
            *out++ = tp;
            tp.operations[0].operation = operation_intersection;
            //tp.operations[1].operation = operation_union;
            *out++ = tp;

            return true;
        }
        else if ( is_q_spike )
        {
            tp.method = method;
            tp.operations[0].operation = operation_union;
            tp.operations[1].operation = operation_blocked;
            *out++ = tp;
            //tp.operations[0].operation = operation_union;
            tp.operations[1].operation = operation_intersection;
            *out++ = tp;

            return true;
        }
        
        return false;
    }

    enum append_version { append_touches, append_collinear_opposite };

    template <append_version Version,
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
        bool is_q_spike = ( Version == append_touches ?
                            ( tp.operations[1].operation == operation_continue
                           || tp.operations[1].operation == operation_intersection ) :
                            true )
                       && ! is_q_last
                       && is_spike_q(side_calc, q1, q2, robust_policy);

        bool res = false;

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
                
                //tp.method = method_touch_interior; // only because arrival != 0
                BOOST_ASSERT(result.template get<0>().count > 1);
                //geometry::convert(result.template get<0>().intersections[1], tp.point);
                base_turn_handler::assign_point(tp, method_touch_interior, result.template get<0>(), 1);

                AssignPolicy::apply(tp, p1.first, q1.first, result.template get<0>(), result.template get<1>());
            }

            tp.operations[0].operation = operation_blocked;
            tp.operations[1].operation = operation_intersection;
            *out++ = tp;
            tp.operations[0].operation = operation_intersection;
            //tp.operations[1].operation = operation_intersection;
            *out++ = tp;

            res = true;
        }

        if ( is_q_spike && ( Version == append_touches || result.template get<1>().arrival[1] == 1 ) )
        {
            if ( Version == append_touches )
            {
                //tp.operations[0].is_collinear = ???
                tp.operations[1].is_collinear = true;
                tp.method = method_touch;
            }
            else
            {
                //tp.operations[0].is_collinear = true;
                //tp.operations[1].is_collinear = true;
                
                //tp.method = method_touch_interior; // only because arrival != 0
                BOOST_ASSERT(result.template get<0>().count > 0);
                //geometry::convert(result.template get<0>().intersections[0], tp.point);
                base_turn_handler::assign_point(tp, method_touch_interior, result.template get<0>(), 0);

                AssignPolicy::apply(tp, p1.first, q1.first, result.template get<0>(), result.template get<1>());
            }

            tp.operations[0].operation = operation_intersection;
            tp.operations[1].operation = operation_blocked;
            *out++ = tp;
            //tp.operations[0].operation = operation_intersection;
            tp.operations[1].operation = operation_intersection;
            *out++ = tp;

            res = true;
        }
        
        return res;
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
            {
                op0 = operation_intersection;
            }
            else if ( op0 == operation_intersection )
            {
                op0 = operation_union;
            }

            if ( op1 == operation_continue || op1 == operation_blocked )
            {
                op1 = operation_intersection;
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
            {
                op0 = operation_intersection;
            }
            else if ( op0 == operation_intersection )
            {
                op0 = operation_union;
            }

            if ( op1 == operation_blocked )
            {
                op1 = operation_intersection;
            }
            else if ( op1 == operation_intersection )
            {
                op1 = operation_union;
            }

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
        {
            op0 = operation_union;
        }

        if ( op1 == operation_intersection )
        {
            op1 = operation_union;
        }
    }
};

}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_GET_TURN_INFO_LL_HPP
