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
        typename RescalePolicy,
        typename OutputIterator
    >
    static inline OutputIterator apply(
                Point1 const& pi, Point1 const& pj, Point1 const& pk,
                Point2 const& qi, Point2 const& qj, Point2 const& qk,
                bool is_p_first, bool is_p_last,
                bool is_q_first, bool is_q_last,
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
                get_turn_info_for_endpoint<AssignPolicy, true, true>
                    ::apply(pi, pj, pk, qi, qj, qk,
                            is_p_first, is_p_last, is_q_first, is_q_last,
                            tp_model, result, method_none, out);
                break;

            case 'd' : // disjoint: never do anything
                break;

            case 'm' :
            {
                if ( get_turn_info_for_endpoint<AssignPolicy, false, true>
                        ::apply(pi, pj, pk, qi, qj, qk,
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
                    // for spike !,-/- or theoretically m,c/c

                    if ( tp.operations[0].operation == operation_blocked )
                    {
                        tp.operations[1].is_collinear = true;
                    }
                    if ( tp.operations[1].operation == operation_blocked )
                    {
                        tp.operations[0].is_collinear = true;
                    }

                    replace_method_and_operations_tm(tp.method, tp.operations[0].operation, tp.operations[1].operation);
                    // after replacement the spike will be m,u/u or theoretically m,i/i
                    
                    // TODO: should m,i/i be handled?
                    
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
                                tp_model, result, method_touch, out) )
                {
                    // do nothing
                }
                else 
                {
                    touch<TurnInfo>::apply(pi, pj, pk, qi, qj, qk,
                        tp, result.template get<0>(), result.template get<1>(), side_calc);
                    // for spike x/x is returned

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

                    // after replacement the spike will be e,c/c
                    if ( ! handle_spikes
                      || ! append_opposite_spikes(tp, result, side_calc,
                                                  p1, p2, q1, q2,
                                                  is_p_last, is_q_last,
                                                  operation_continue, out) )
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
                                tp_model, result, method_equal, out) )
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
                        // for spikes u/i or i/u is returned

                        replacer_of_method_and_operations_ec replacer(method_touch);
                        replacer(tp.method, tp.operations[0].operation, tp.operations[1].operation);

                        AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());

                        // conditionally handle spikes
                        if ( ! handle_spikes
                          || ! append_collinear_spikes(tp, side_calc, p1, p2, q1, q2,
                                                       is_p_last, is_q_last,
                                                       method_touch, operation_union, out) )
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
                                tp_model, result, method_collinear, out) )
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
                            // for spikes u/i or i/u is returned

                            // NOTE: don't change the method only if methods are WRT IPs, not segments!
                            // (currently this approach is used)
                            // override assigned method
                            //tp.method = method_collinear;

                            method_replace = method_touch;
                            spike_op = operation_union;
                        }
                        else
                        {
                            collinear<TurnInfo>::apply(pi, pj, pk, qi, qj, qk,
                                    tp, result.template get<0>(), result.template get<1>(), side_calc);
                            // for spikes c,c/c is returned

                            //method_replace = method_touch_interior;
                            //spike_op = operation_continue;
                        }

                        replacer_of_method_and_operations_ec replacer(method_replace);
                        replacer(tp.method, tp.operations[0].operation, tp.operations[1].operation);
                        
                        AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());

                        // conditionally handle spikes
                        if ( ! handle_spikes
                          || ! append_collinear_spikes(tp, side_calc, p1, p2, q1, q2,
                                                       is_p_last, is_q_last,
                                                       method_replace, spike_op, out) )
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
                            append_opposite_spikes(tp, result, side_calc,
                                                   p1, p2, q1, q2,
                                                   is_p_last, is_q_last,
                                                   operation_none, out, true);
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
                    only_convert<TurnInfo>::apply(tp, result.template get<0>());

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
              typename OutIt>
    static inline bool append_collinear_spikes(TurnInfo & tp,
                                               SideCalc const& side_calc,
                                               SegmentP const& p1, SegmentP const& p2,
                                               SegmentQ const& q1, SegmentQ const& q2,
                                               bool is_p_last, bool is_q_last,
                                               method_type method, operation_type spike_op,
                                               OutIt out)
    {
        // method == touch || touch_interior
        // both position == middle

        bool is_p_spike = tp.operations[0].operation == spike_op
                       && ! is_p_last
                       && is_spike_p(side_calc, p1, p2);
        bool is_q_spike = tp.operations[1].operation == spike_op
                       && ! is_q_last
                       && is_spike_q(side_calc, q1, q2);

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
            *out++ = tp;

            return true;
        }
        else if ( is_q_spike )
        {
            tp.method = method;
            tp.operations[0].operation = operation_union;
            tp.operations[1].operation = operation_blocked;
            *out++ = tp;
            tp.operations[1].operation = operation_intersection;
            *out++ = tp;

            return true;
        }
        
        return false;
    }

    template <typename TurnInfo,
              typename Result,
              typename SideCalc,
              typename SegmentP,
              typename SegmentQ,
              typename OutIt>
    static inline bool append_opposite_spikes(TurnInfo & tp,
                                              Result const& result,
                                              SideCalc const& side_calc,
                                              SegmentP const& p1, SegmentP const& p2,
                                              SegmentQ const& q1, SegmentQ const& q2,
                                              bool is_p_last, bool is_q_last,
                                              operation_type spike_op, OutIt out,
                                              bool collinear_opposite = false)
    {
        bool is_p_spike = tp.operations[0].operation == spike_op
                       && ! is_p_last
                       && is_spike_p(side_calc, p1, p2);
        bool is_q_spike = tp.operations[1].operation == spike_op
                       && ! is_q_last
                       && is_spike_q(side_calc, q1, q2);

        if ( is_p_spike || is_q_spike )
        {
            tp.method = method_touch;

            // arrivals 0 and 1 are handled by touches? 

            if ( is_p_spike && ( !collinear_opposite || result.template get<1>().arrival[0] == 1 ) )
            {
                if ( collinear_opposite )
                {
                    tp.method = method_touch_interior; // only because arrival != 0
                    BOOST_ASSERT(result.template get<0>().count > 1);
                    geometry::convert(result.template get<0>().intersections[1], tp.point);
                    AssignPolicy::apply(tp, p1.first, q1.first, result.template get<0>(), result.template get<1>());
                }

                tp.operations[0].operation = operation_blocked;
                tp.operations[1].operation = operation_intersection;
                *out++ = tp;
                tp.operations[0].operation = operation_intersection;
                *out++ = tp;
            }

            if ( is_q_spike && ( !collinear_opposite || result.template get<1>().arrival[1] == 1 ) )
            {
                if ( collinear_opposite )
                {
                    tp.method = method_touch_interior; // only because arrival != 0
                    BOOST_ASSERT(result.template get<0>().count > 0);
                    geometry::convert(result.template get<0>().intersections[0], tp.point);
                    AssignPolicy::apply(tp, p1.first, q1.first, result.template get<0>(), result.template get<1>());
                }

                tp.operations[0].operation = operation_intersection;
                tp.operations[1].operation = operation_blocked;
                *out++ = tp;
                tp.operations[1].operation = operation_intersection;
                *out++ = tp;
            }

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
