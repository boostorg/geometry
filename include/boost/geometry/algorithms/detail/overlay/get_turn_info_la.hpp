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

namespace boost { namespace geometry {

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay {

template<typename AssignPolicy>
struct get_turn_info_linear_areal
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
                get_turn_info_for_endpoint<true, true>(
                    pi, pj, pk, qi, qj, qk, p_segments_count, q_segments_count,
                    tp_model, result, method_none, out);
                break;

            case 'd' : // disjoint: never do anything
                break;

            case 'm' :
            {
                if ( get_turn_info_for_endpoint<false, true>(
                        pi, pj, pk, qi, qj, qk, p_segments_count, q_segments_count,
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
                if ( get_turn_info_for_endpoint<false, true>(
                        pi, pj, pk, qi, qj, qk, p_segments_count, q_segments_count,
                        tp_model, result, method_touch, out) )
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
                if ( get_turn_info_for_endpoint<true, true>(
                        pi, pj, pk, qi, qj, qk, p_segments_count, q_segments_count,
                        tp_model, result, method_equal, out) )
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
                if ( get_turn_info_for_endpoint<true, true>(
                        pi, pj, pk, qi, qj, qk, p_segments_count, q_segments_count,
                        tp_model, result, method_collinear, out) )
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
        }

        // assuming Linear is always the first one
        if ( op0 == operation_blocked )
            op0 = operation_continue;

        op1 = operation_union;
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
            // NOTE: probably only if methods are WRT IPs, not segments!
            if ( op0 == operation_intersection || op0 == operation_union
                || op1 == operation_intersection || op1 == operation_union )
            {
                method = m_method;
            }

            // assuming Linear is always the first one
            if ( op0 == operation_blocked )
                op0 = operation_continue;

            op1 = operation_union;
        }

    private:
        method_type m_method;
    };

    static inline void replace_operations_i(operation_type & op0, operation_type & op1)
    {
        // assuming Linear is always the first one
        op1 = operation_union;
    }

    
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
// TODO: should this always be std::size_t or replace with template parameter?
                            std::size_t p_segments_count,
                            std::size_t q_segments_count,
                            TurnInfo const& tp_model,
                            IntersectionResult const& result,
                            method_type method,
                            OutputIterator out)
    {
        namespace ov = overlay;
        typedef ov::get_turn_info_for_endpoint<AssignPolicy, EnableFirst, EnableLast> get_info_e;

        std::size_t ip_count = result.template get<0>().count;
        // no intersection points
        if ( ip_count == 0 )
            return false;

        int segment_index0 = tp_model.operations[0].seg_id.segment_index;
        int segment_index1 = tp_model.operations[1].seg_id.segment_index;
        BOOST_ASSERT(segment_index0 >= 0 && segment_index1 >= 0);

        bool is_p_first = segment_index0 == 0;
        bool is_q_first = segment_index1 == 0; // not used
        bool is_p_last = static_cast<std::size_t>(segment_index0) + 1 == p_segments_count;
        bool is_q_last = static_cast<std::size_t>(segment_index1) + 1 == q_segments_count; // not used

        if ( !is_p_first && !is_p_last )
            return false;

        ov::operation_type p_operation0 = ov::operation_none;
        ov::operation_type q_operation0 = ov::operation_none;
        ov::operation_type p_operation1 = ov::operation_none;
        ov::operation_type q_operation1 = ov::operation_none;
        bool p0i, p0j, q0i, q0j; // assign false?
        bool p1i, p1j, q1i, q1j; // assign false?

        bool opposite = result.template get<1>().opposite;
        bool same_dirs = result.template get<1>().dir_a == 0 && result.template get<1>().dir_b == 0;

        {
            int p_how = result.template get<1>().how_a;
            int q_how = result.template get<1>().how_b;
            int p_arrival = result.template get<1>().arrival[0];
            int q_arrival = result.template get<1>().arrival[1];

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

// NOTE: the conversion may be problematic
            Point1 qi_conv;
            geometry::convert(qi, qi_conv);

            method_type replaced_method = method_touch_interior;

            if ( q0j )
            {
                if ( !opposite )
                {
                    side_calculator<Point1, Point2> side_calc(qi_conv, pi, pj, qi, qj, qk);
                    equal<TurnInfo>::apply(qi_conv, pi, pj, qi, qj, qk,
                        tp, result.template get<0>(), result.template get<1>(), side_calc);
                }
                else // opposite -> collinear
                {
                    tp.operations[0].operation = operation_continue;
                    tp.operations[1].operation = operation_union;
                }

                replaced_method = method_touch;
            }
            else
            {
// NOTE: the conversion may be problematic
                Point2 pi_conv;
                geometry::convert(pi, pi_conv);

                side_calculator<Point1, Point2> side_calc(qi_conv, pi, pj, qi, pi_conv, qj);

                // Collinear, but similar thus handled as equal
                equal<TurnInfo>::apply(qi_conv, pi, pj, qi, pi_conv, qj,
                    tp, result.template get<0>(), result.template get<1>(), side_calc);
            }

            replacer_of_method_and_operations_ec replacer(replaced_method);
            replacer(tp.method, tp.operations[0].operation, tp.operations[1].operation);

            // equals<> or collinear<> will assign the second point,
            // we'd like to assign the first one
            geometry::convert(result.template get<0>().intersections[0], tp.point);

            AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());
            *out++ = tp;
        }

        // ANALYSE AND ASSIGN LAST

        // IP on the first point of Linear Geometry
        if ( EnableLast && is_p_last && p0j && !q0i ) // !q0i prevents duplication
        {
            TurnInfo tp = tp_model;
            tp.method = q0j ? method_touch : method_touch_interior;
            tp.operations[0].operation = operation_blocked;
            tp.operations[1].operation = operation_union;
            tp.operations[0].position = position_back;
            tp.operations[1].position = position_middle;
            
            // equals<> or collinear<> will assign the second point,
            // we'd like to assign the first one
            geometry::convert(result.template get<0>().intersections[0], tp.point);

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
