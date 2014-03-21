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
                get_turn_info_for_endpoint<AssignPolicy, true, true>
                    ::apply(pi, pj, pk, qi, qj, qk, p_segments_count, q_segments_count,
                          tp_model, result, method_none, out);
                break;

            case 'd' : // disjoint: never do anything
                break;

            case 'm' :
            {
                if ( get_turn_info_for_endpoint<AssignPolicy, false, true>
                        ::apply(pi, pj, pk, qi, qj, qk, p_segments_count, q_segments_count,
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
                        ::apply(pi, pj, pk, qi, qj, qk, p_segments_count, q_segments_count,
                                tp_model, result, method_touch, out) )
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

                    replace_method_and_operations_tm(tp.method, tp.operations[0].operation, tp.operations[1].operation);

                    AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());
                    *out++ = tp;
                }
            }
            break;
            case 'e':
            {
                if ( get_turn_info_for_endpoint<AssignPolicy, true, true>
                        ::apply(pi, pj, pk, qi, qj, qk, p_segments_count, q_segments_count,
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

                        replacer_of_method_and_operations_ec replacer(method_touch);
                        replacer(tp.method, tp.operations[0].operation, tp.operations[1].operation);

                        handle_possible_spike_after_equal(tp.operations[0].operation,
                                                          tp.operations[1].operation,
                                                          side_calc);
                    
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
            }
            break;
            case 'c' :
            {
                // Collinear
                if ( get_turn_info_for_endpoint<AssignPolicy, true, true>
                        ::apply(pi, pj, pk, qi, qj, qk, p_segments_count, q_segments_count,
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

                            handle_possible_spike_after_equal(tp.operations[0].operation,
                                                              tp.operations[1].operation,
                                                              side_calc);
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

    template <typename SideCalc>
    static inline void handle_possible_spike_after_equal(operation_type & op0,
                                                         operation_type & op1,
                                                         SideCalc const& side_calc)
    {
        // spike on P
        if ( op0 == operation_union
            && side_calc.pk_wrt_p1() == 0
            && side_calc.qk_wrt_p1() == -side_calc.qk_wrt_p2() )
        {
            op0 = operation_continue;
        }
        // spike on Q
        if ( op1 == operation_union
            && side_calc.qk_wrt_q1() == 0
            && side_calc.pk_wrt_q1() == -side_calc.pk_wrt_q2() )
        {
            op1 = operation_continue;
        }
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

    
};

}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_GET_TURN_INFO_LL_HPP
