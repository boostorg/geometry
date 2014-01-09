// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013, 2014.
// Modifications copyright (c) 2013-2014 Oracle and/or its affiliates.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_TURNS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_TURNS_HPP

#include <boost/geometry/strategies/distance.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_turns.hpp>
#include <boost/geometry/algorithms/detail/overlay/get_turn_info.hpp>
#include <boost/geometry/algorithms/detail/overlay/calculate_distance_policy.hpp>

#include <boost/type_traits/is_base_of.hpp>

namespace boost { namespace geometry {

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace relate { namespace turns {

// TURN_INFO

template<typename P>
struct distance_info
{
    typedef typename strategy::distance::services::return_type
        <
            typename strategy::distance::services::comparable_type
                <
                    typename strategy::distance::services::default_strategy
                        <
                            point_tag,
                            P
                        >::type
                >::type,
            P, P
        >::type distance_type;

    inline distance_info()
        : distance(distance_type())
    {}

    distance_type distance; // distance-measurement from segment.first to IP
};

template <typename P>
struct turn_operation_with_distance : public overlay::turn_operation
{
    distance_info<P> enriched;
};

// GET_TURN_INFO

template<typename AssignPolicy>
struct get_turn_info_linear_linear
{
    template
    <
        typename Point1,
        typename Point2,
        typename Geometry1,
        typename Geometry2,
        typename TurnInfo,
        typename RescalePolicy,
        typename OutputIterator
    >
    static inline OutputIterator apply(
                Point1 const& pi, Point1 const& pj, Point1 const& pk,
                Point2 const& qi, Point2 const& qj, Point2 const& qk,
                Geometry1 const& geometry1,
                Geometry2 const& geometry2,
                TurnInfo const& tp_model,
                RescalePolicy const& , // TODO: this will be used. rescale_policy,
                OutputIterator out)
    {
        typedef model::referring_segment<Point1 const> segment_type1;
        typedef model::referring_segment<Point2 const> segment_type2;
        segment_type1 p1(pi, pj), p2(pj, pk);
        segment_type2 q1(qi, qj), q2(qj, qk);

        overlay::side_calculator<Point1, Point2> side_calc(pi, pj, pk, qi, qj, qk);

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
                handle_first_last(pi, pj, pk, qi, qj, qk, geometry1, geometry2,
                                  tp_model, result, overlay::method_none, out, true, false);
                break;

            case 'd' : // disjoint: never do anything
                break;

            case 'm' :
            {
                if ( handle_first_last(pi, pj, pk, qi, qj, qk, geometry1, geometry2,
                                       tp_model, result, overlay::method_touch_interior, out, false, true) )
                {
                    // do nothing
                }
                else
                {
                    typedef overlay::touch_interior
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
                        overlay::side_calculator<Point1, Point2> swapped_side_calc(qi, qj, qk, pi, pj, pk);
                        policy::template apply<1>(qi, qj, qk, pi, pj, pk,
                                    tp, result.template get<0>(), result.template get<1>(),
                                    swapped_side_calc);
                    }
                    AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());
                    *out++ = tp;
                }
            }
            break;
            case 'i' :
            {
                overlay::crosses<TurnInfo>::apply(pi, pj, pk, qi, qj, qk,
                    tp, result.template get<0>(), result.template get<1>());
                AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());
                *out++ = tp;
            }
            break;
            case 't' :
            {
                // Both touch (both arrive there)
                if ( handle_first_last(pi, pj, pk, qi, qj, qk, geometry1, geometry2,
                                       tp_model, result, overlay::method_touch, out, false, true) )
                {
                    // do nothing
                }
                else 
                {
                    overlay::touch<TurnInfo>::apply(pi, pj, pk, qi, qj, qk,
                        tp, result.template get<0>(), result.template get<1>(), side_calc);
                    AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());
                    *out++ = tp;
                }
            }
            break;
            case 'e':
            {
                if ( handle_first_last(pi, pj, pk, qi, qj, qk, geometry1, geometry2,
                                       tp_model, result, overlay::method_equal, out, true, true) )
                {
                    // do nothing
                }
                else if ( ! result.template get<1>().opposite )
                {
                    // Both equal
                    // or collinear-and-ending at intersection point
                    overlay::equal<TurnInfo>::apply(pi, pj, pk, qi, qj, qk,
                        tp, result.template get<0>(), result.template get<1>(), side_calc);
                    AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());
                    *out++ = tp;
                }
                else
                {
                    overlay::equal_opposite
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
                if ( handle_first_last(pi, pj, pk, qi, qj, qk, geometry1, geometry2,
                                       tp_model, result, overlay::method_collinear, out, true, true) )
                {
                    // do nothing
                }
                else if (! result.template get<1>().opposite)
                {

                    if (result.template get<1>().arrival[0] == 0)
                    {
                        // Collinear, but similar thus handled as equal
                        overlay::equal<TurnInfo>::apply(pi, pj, pk, qi, qj, qk,
                                tp, result.template get<0>(), result.template get<1>(), side_calc);

                        // override assigned method
                        tp.method = overlay::method_collinear;
                    }
                    else
                    {
                        overlay::collinear<TurnInfo>::apply(pi, pj, pk, qi, qj, qk,
                                tp, result.template get<0>(), result.template get<1>(), side_calc);
                    }

                    AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());
                    *out++ = tp;
                }
                else
                {
                    overlay::collinear_opposite
                        <
                            TurnInfo,
                            AssignPolicy
                        >::apply(pi, pj, pk, qi, qj, qk,
                            tp, out, result.template get<0>(), result.template get<1>(), side_calc);
                }
            }
            break;
            case '0' :
            {
                // degenerate points
                if (AssignPolicy::include_degenerate)
                {
                    overlay::only_convert<TurnInfo>::apply(tp, result.template get<0>());
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

    template<typename Point1,
             typename Point2,
             typename Geometry1,
             typename Geometry2,
             typename TurnInfo,
             typename IntersectionResult,
             typename OutputIterator
    >
    static inline bool handle_first_last(Point1 const& pi, Point1 const& pj, Point1 const& pk,
                                         Point2 const& qi, Point2 const& qj, Point2 const& qk,
                                         Geometry1 const& geometry1,
                                         Geometry2 const& geometry2,
                                         TurnInfo const& tp_model,
                                         IntersectionResult const& result,
                                         overlay::method_type method,
                                         OutputIterator out,
                                         bool enable_first = true,
                                         bool enable_last = true)
    {
        namespace ov = overlay;

        std::size_t ip_count = result.template get<0>().count;

        if ( ip_count == 0 )
            return false;

        std::pair<bool, std::size_t> first_p = find_if(pi, geometry1, tp_model.operations[0].seg_id, result, is_first_segment());
        std::pair<bool, std::size_t> first_q = find_if(qi, geometry2, tp_model.operations[1].seg_id, result, is_first_segment());
        std::pair<bool, std::size_t> last_p = find_if(pj, geometry1, tp_model.operations[0].seg_id, result, is_last_segment());
        std::pair<bool, std::size_t> last_q = find_if(qj, geometry2, tp_model.operations[1].seg_id, result, is_last_segment());

        bool is_p_first_ip = first_p.second < ip_count;
        bool is_q_first_ip = first_q.second < ip_count;
        bool is_p_last_ip = last_p.second < ip_count;
        bool is_q_last_ip = last_q.second < ip_count;

        // handle first
        bool handle_first_p = false;
        bool handle_first_q = false;

// NEW VERSION BEGINS HERE

// TODO: maybe turns should be calculated WRT Q since we might assume that Q may be Areal CW
// TODO: replace checks for equality of points with result...arrival

        int spi = result.template get<1>().sides.sides[0].first;
        int spj = result.template get<1>().sides.sides[0].second;
        int sqi = result.template get<1>().sides.sides[1].first;
        int sqj = result.template get<1>().sides.sides[1].second;
        bool col_op = result.template get<1>().opposite;

        TurnInfo tp = tp_model;

        if ( enable_first )
        {        
            // Both first points are IPs
            if ( is_p_first_ip && is_q_first_ip )
            {
                // collinear         collinear
                // |------->         |------->
                // |---->-->-->   <--<----|--|
                if ( ip_count == 2 )
                {
                    if ( !col_op )
                    {
                        assign(pi, qi, result, pi, ov::method_touch, ov::operation_continue, ov::operation_continue, tp, out);
                    }
                    else
                    {
                        bool equal_ij = equals::equals_point_point(pi, qj);
                        bool equal_ji = equals::equals_point_point(pj, qi);
                        if ( equal_ij && equal_ji )
                        {
                            assign(pi, qi, result, pi, ov::method_touch, ov::operation_continue, ov::operation_continue, tp, out);
                            assign(pi, qi, result, qi, ov::method_equal, ov::operation_opposite, ov::operation_opposite, tp, out);
                        }
                        else
                        {
                            // NOTE: we don't know anything about previous segments
                            // so for pi we don't know the method and for qi we don't know the operation
                            if ( equal_ij )
                            {
                                assign(pi, qi, result, pi, ov::method_touch, ov::operation_continue, ov::operation_continue, tp, out);
                                assign(pi, qi, result, qi, ov::method_touch_interior, ov::operation_opposite, ov::operation_opposite, tp, out);
                            }
                            else if ( equal_ji )
                            {
                                assign(pi, qi, result, pi, ov::method_touch_interior, ov::operation_continue, ov::operation_continue, tp, out);
                                assign(pi, qi, result, qi, ov::method_touch, ov::operation_intersection, ov::operation_blocked, tp, out);
                            }
                            else
                            {
                                assign(pi, qi, result, pi, ov::method_touch_interior, ov::operation_continue, ov::operation_continue, tp, out);
                                assign(pi, qi, result, qi, ov::method_touch_interior, ov::operation_union, ov::operation_blocked, tp, out);
                            }
                        }
                    }
                }
                // arbitrary
                //     |--->
                // <---|
                else // ip_count == 1
                {
// TODO: Is this ok? Assuming that Q is considered as CW

                    // NOTE: we don't know anything about the previous segments
                    // so we can't really set correct method or operation here
                    assign(pi, qi, result, pi, ov::method_touch, ov::operation_union, ov::operation_union, tp, out);
                }
            }
            // The first point of P is IP
            else if ( is_p_first_ip )
            {
                // always collinear, P may be also last, Q may be arbitrary
                // P  |--+-->        <--+--|
                // Q |---+--->      |---+--->
                if ( ip_count == 2 )
                {
                    // same direction
                    if ( !col_op )
                    {
                        bool equal_ii = equals::equals_point_point(pi, qi);
                        ov::method_type method = equal_ii ? ov::method_touch : ov::method_touch_interior;
                        assign(pi, qi, result, pi, method, ov::operation_continue, ov::operation_continue, tp, out);
                    }
                    // opposite direction
                    else
                    {
                        bool equal_ij = equals::equals_point_point(pi, qj);
                        ov::method_type method = equal_ij ? ov::method_equal : ov::method_collinear;
                        assign(pi, qi, result, pi, method, ov::operation_opposite, ov::operation_opposite, tp, out);
                    }
                }
                // non-collinear
                // P    | | | | | |
                // Q  |---+---+--->
                else // ip_count == 1
                {
                    bool equal_ii = equals::equals_point_point(pi, qi);
                    bool equal_ij = equals::equals_point_point(pi, qj);
                    // p starts at the beginning of segment, but not at the first point of Q
                    // P      |   |
                    // Q  |---+---+--->
                    if ( equal_ii )
                    {
                        BOOST_ASSERT(tp.operations[1].seg_id.segment_index > 0);
                        Point2 const& qh = *(boost::begin(sub_geometry::get(geometry2, tp.operations[1].seg_id))
                                                + tp.operations[1].seg_id.segment_index - 1);
                        // t+u/i or t+i/u
                        assign_using_equal(qh, pi, pj, qh, qi, qj, result, pi, overlay::method_touch, tp, out);
                        // OR JUST
                        // assign(pi, qi, result, pi, ov::method_touch, ov::operation_union, ov::operation_union, tp, out);
                    }
                    // p starts at the end of segment
                    // P      |   |   |
                    // Q  |---+---+--->
                    else if ( equal_ij )
                    {
                        // the end of Q's segment is the last point
                        // P              |
                        // Q  |---+---+--->
                        if ( last_q.first )
                        {
                            // t+u/u
                            assign(pi, qi, result, pi, ov::method_touch, ov::operation_union, ov::operation_union, tp, out);
                        }
                        // the qj is some internal point
                        // P      |   |
                        // Q  |---+---+--->
                        else
                        {
                            // t+u/i or t+i/u
                            assign_using_equal(qi, pi, pj, qi, qj, qk, result, pi, overlay::method_touch, tp, out);
                        }
                    }
                    // p starts somewhere in the middle of Q's segment
                    // P    |   |   |
                    // Q  |---+---+--->
                    else
                    {
                        if ( last_q.first )
                        {
                            // m+u/u
                            assign(pi, qi, result, pi, ov::method_touch_interior, ov::operation_union, ov::operation_union, tp, out);
                        }
                        else
                        {
                            // t+u/i or t+i/u
                            assign_using_collinear(qi, pi, pj, qi, qj, qk, result, pi, overlay::method_touch_interior, tp, out);
                        }
                    }
                }
            }
// NEW VERSION ENDS HERE
            else
            {
                if ( is_p_first_ip )
                {
                    bool equal_ii = equals::equals_point_point(pi, qi);
                    bool equal_ij = equals::equals_point_point(pi, qj);

                    handle_first_p = first_q.first && last_q.first // Q is one-segment LS
                                    || equal_ii && !last_q.first     // both starts
                                    || equal_ij && last_q.first
                                    || !equal_ii && !equal_ij;

                    //TEST
                    handle_first_p = true;
                }

                if ( is_q_first_ip )
                {
                    bool equal_ii = equals::equals_point_point(pi, qi);
                    bool equal_ji = equals::equals_point_point(pj, qi);

                    handle_first_q = first_p.first && last_p.first || equal_ii && !last_p.first || equal_ji && last_p.first || !equal_ii && !equal_ji;

                    //TEST
                    handle_first_q = true;
                }
            }
        }

        // handle last
        bool handle_last_p = false;
        bool handle_last_q = false;

        if ( is_p_last_ip )
        {
            bool equal_ji = equals::equals_point_point(pj, qi);
            bool equal_jj = equals::equals_point_point(pj, qj);

            handle_last_p = equal_jj || equal_ji && is_q_first_ip || !equal_ji && !equal_jj;

            //TEST
            handle_last_p = true;
        }

        if ( is_q_last_ip )
        {
            bool equal_ij = equals::equals_point_point(pi, qj);
            bool equal_jj = equals::equals_point_point(pj, qj);

            handle_last_q = equal_jj || equal_ij && is_p_first_ip || !equal_ij && !equal_jj;

            //TEST
            handle_last_q = true;
        }

        bool first_ok = handle_first_p || handle_first_q;
        bool last_ok = handle_last_p || handle_last_q;

        if ( enable_first && first_ok )
        {
            TurnInfo tp = tp_model;
            overlay::side_calculator<Point1, Point2> side_calc(pi, pi, pj, qi, qi, qj);
            overlay::equal<TurnInfo>::apply(pi, pi, pj, qi, qi, qj,
                tp, result.template get<0>(), result.template get<1>(), side_calc);
// TODO: better method calculation, check above! E.g. if equal_ii then method_equal
            tp.method = overlay::method_collinear;
// TODO: what if both are first?
            geometry::convert(handle_first_p ? pi : qi, tp.point);
            AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());
            *out++ = tp;
        }

        if ( enable_last && last_ok )
        {
            TurnInfo tp = tp_model;
            overlay::side_calculator<Point1, Point2> side_calc(pj, pj, pi, qj, qj, qi);
            overlay::equal<TurnInfo>::apply(pj, pj, pi, qj, qj, qi,
                tp, result.template get<0>(), result.template get<1>(), side_calc);
            std::swap(tp.operations[0].operation, tp.operations[1].operation);
            tp.method = method;
// TODO: what if both are first?
            geometry::convert(handle_last_p ? pj : qj, tp.point);
            AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());
            *out++ = tp;

            return true;
        }

        return false;
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
                              overlay::method_type method,
                              overlay::operation_type op0, overlay::operation_type op1,
                              TurnInfo & tp,
                              OutputIterator out)
    {
        geometry::convert(ip, tp.point);
        tp.method = method;
        tp.operations[0].operation = op0;
        tp.operations[1].operation = op1;
        AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());
        *out++ = tp;
    }

    template <typename Point1,
              typename Point2,
              typename IntersectionResult,
              typename Point,
              typename TurnInfo,
              typename OutputIterator>
    static inline void assign_using_equal(Point1 const& pi, Point1 const& pj, Point1 const& pk,
                                          Point2 const& qi, Point2 const& qj, Point2 const& qk,
                                          IntersectionResult const& result,
                                          Point const& ip,
                                          overlay::method_type method,
                                          TurnInfo & tp,
                                          OutputIterator out)
    {
        overlay::side_calculator<Point1, Point2> side_calc(pi, pj, pk, qi, qj, qk);
        overlay::collinear<TurnInfo>::apply(pi, pj, pk, qi, qj, qk,
            tp, result.template get<0>(), result.template get<1>(), side_calc);
        tp.method = method;
        geometry::convert(ip, tp.point);
        AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());
        *out++ = tp;
    }

    template <typename Point1,
              typename Point2,
              typename IntersectionResult,
              typename Point,
              typename TurnInfo,
              typename OutputIterator>
    static inline void assign_using_collinear(Point1 const& pi, Point1 const& pj, Point1 const& pk,
                                              Point2 const& qi, Point2 const& qj, Point2 const& qk,
                                              IntersectionResult const& result,
                                              Point const& ip,
                                              overlay::method_type method,
                                              TurnInfo & tp,
                                              OutputIterator out)
    {
        overlay::side_calculator<Point1, Point2> side_calc(pi, pj, pk, qi, qj, qk);
        overlay::collinear<TurnInfo>::apply(pi, pj, pk, qi, qj, qk,
            tp, result.template get<0>(), result.template get<1>(), side_calc);
        tp.method = method;
        geometry::convert(ip, tp.point);
        AssignPolicy::apply(tp, pi, qi, result.template get<0>(), result.template get<1>());
        *out++ = tp;
    }

    template<typename Point,
             typename Geometry,
             typename SegId,
             typename IntersectionResult,
             typename SegmentPred>
    static inline std::pair<bool, std::size_t>
    find_if(Point const& point,
            Geometry const& geometry,
            SegId const& seg_id,
            IntersectionResult const& result,
            SegmentPred segment_pred)
    {
        const bool is = segment_pred(geometry, seg_id);
        const std::size_t size = result.template get<0>().count;

        std::pair<bool, std::size_t> res(is, size);

        if ( is )
        {
            for ( std::size_t i = 0 ; i < size ; ++i )
            {
                if ( equals::equals_point_point(point, result.template get<0>().intersections[i]) )
                {
                    res.second = i;
                    break;
                }
            }
        }

        return res;
    }

    struct is_first_segment
    {
        template <typename Geometry, typename SegId>
        inline bool operator()(Geometry const&, SegId const& seg_id)
        {
            return seg_id.segment_index == 0;
        }
    };
    
    struct is_last_segment
    {
        template <typename Geometry, typename SegId>
        inline bool operator()(Geometry const& g, SegId const& seg_id)
        {
            const size_t count = geometry::num_points(sub_geometry::get(g, seg_id));
            BOOST_ASSERT(count >= 2);
            return seg_id.segment_index == count - 2;
        }
    };
};

// GET_TURN_INFO_TYPE

template <typename Tag,
          bool IsLinear = boost::is_base_of<linear_tag, Tag>::value,
          bool IsAreal = boost::is_base_of<areal_tag, Tag>::value>
struct tag_base : not_implemented<Tag>
{};

template <typename Tag>
struct tag_base<Tag, true, false>
{
    typedef linear_tag type;
};

template <typename Tag>
struct tag_base<Tag, false, true>
{
    typedef areal_tag type;
};

template <typename Geometry1, typename Geometry2, typename AssignPolicy = overlay::calculate_distance_policy,
          typename Tag1 = typename tag<Geometry1>::type, typename Tag2 = typename tag<Geometry2>::type,
          typename TagBase1 = typename tag_base<Tag1>::type, typename TagBase2 = typename tag_base<Tag2>::type>
struct get_turn_info
    : overlay::get_turn_info<AssignPolicy>
{};

template <typename Geometry1, typename Geometry2, typename AssignPolicy, typename Tag1, typename Tag2>
struct get_turn_info<Geometry1, Geometry2, AssignPolicy, Tag1, Tag2, linear_tag, linear_tag>
    : get_turn_info_linear_linear<AssignPolicy>
{};

// GET_TURNS

template <typename Geometry1,
          typename Geometry2,
          typename GetTurnPolicy
            = get_turn_info<Geometry1, Geometry2, overlay::calculate_distance_policy> >
struct get_turns
{
    typedef typename geometry::point_type<Geometry1>::type point1_type;

    typedef overlay::turn_info
        <
            point1_type,
            turn_operation_with_distance<point1_type>
        > turn_info;

    template <typename Turns>
    static inline void apply(Turns & turns, Geometry1 const& geometry1, Geometry2 const& geometry2)
    {
        detail::get_turns::no_interrupt_policy interrupt_policy;

        static const bool reverse1 = detail::overlay::do_reverse<geometry::point_order<Geometry1>::value>::value;
        static const bool reverse2 = detail::overlay::do_reverse<geometry::point_order<Geometry2>::value>::value;

        dispatch::get_turns
            <
                typename bg::tag<Geometry1>::type, typename bg::tag<Geometry2>::type,
                Geometry1, Geometry2, reverse1, reverse2,
                GetTurnPolicy
            >::apply(0, geometry1, 1, geometry2, bg::detail::no_rescale_policy(), turns, interrupt_policy);
    }
};

// TURNS SORTING AND SEARCHING

struct operation_order_uibc
{
    template <typename Op> static inline
    int apply(Op const& op)
    {
        switch(op.operation)
        {
        case detail::overlay::operation_opposite : return 0;
        case detail::overlay::operation_none : return 0;
        case detail::overlay::operation_union : return 1;
        case detail::overlay::operation_intersection : return 2;
        case detail::overlay::operation_blocked : return 3;
        case detail::overlay::operation_continue : return 4;
        }
        return -1;
    };
};

// sort turns by G1 - source_index == 0 by:
// seg_id -> distance -> operation
template <typename OperationOrder>
struct less_seg_dist_op
{
    template <typename Op> static inline
    bool use_operation(Op const& left, Op const& right)
    {
        return OperationOrder::apply(left) < OperationOrder::apply(right);
    }

    template <typename Op> static inline
    bool use_other_multi_ring_id(Op const& left, Op const& right)
    {
        //return left.other_id.ring_index < right.other_id.ring_index;
        
        if ( left.other_id.ring_index == -1 )
        {
            if ( right.other_id.ring_index == -1 )
                return use_operation(left, right); // sort by operation
            else
                return true; // right always greater
        }
        else // left.other_id.ring_index != -1
        {
            if ( right.other_id.ring_index == -1 )
                return false; // left always greater

            // here both ring_indexes are greater than -1
            // so first, sort also by multi_index
            return left.other_id.multi_index < right.other_id.multi_index || (
                       left.other_id.multi_index == right.other_id.multi_index && (
                       left.other_id.ring_index < right.other_id.ring_index || (
                           left.other_id.ring_index == right.other_id.ring_index &&
                           use_operation(left, right) )
                    )
                );
        }
    }

    template <typename Op> static inline
    bool use_distance(Op const& left, Op const& right)
    {
        return left.enriched.distance < right.enriched.distance || (
                    geometry::math::equals(left.enriched.distance, right.enriched.distance) &&
                    use_other_multi_ring_id(left, right)
                );
    }

    template <typename Turn>
    inline bool operator()(Turn const& left, Turn const& right) const
    {
        segment_identifier const& sl = left.operations[0].seg_id;
        segment_identifier const& sr = right.operations[0].seg_id;

        return sl < sr || ( sl == sr && use_distance(left.operations[0], right.operations[0]) );
    }
};

template <typename Turn> inline
bool is_valid_method(Turn const& turn)
{
    return turn.method != detail::overlay::method_none
        && turn.method != detail::overlay::method_disjoint
        && turn.method != detail::overlay::method_error;
}

template <typename Turn> inline
bool is_valid_operation(Turn const& turn)
{
    BOOST_ASSERT(!turn.has(detail::overlay::operation_opposite));
    return !turn.both(detail::overlay::operation_none);
}

template <typename Turn> inline
bool is_valid_turn(Turn const& turn)
{
    return is_valid_method(turn) && is_valid_operation(turn);
}

template <bool IsCyclic, typename TurnIt, typename Cond> inline
TurnIt find_next_if(TurnIt first, TurnIt current, TurnIt last, Cond cond)
{
    if ( first == last )
        return last;

    if ( current != last )
    {
        TurnIt it = current;
        ++it;

        for ( ; it != last ; ++it )
            if ( cond(*it) )
                return it;
    }

    if ( IsCyclic )
    {
        for ( TurnIt it = first ; it != current ; ++it )
            if ( cond(*it) )
                return it;
    }

    return last;
}

template <bool IsCyclic, typename TurnIt, typename Cond> inline
TurnIt find_previous_if(TurnIt first, TurnIt current, TurnIt last, Cond cond)
{
    typedef std::reverse_iterator<TurnIt> Rit;
    Rit rlast = Rit(first);
    if ( current == last )
        return last;
    ++current;
    Rit res = find_next_if<IsCyclic>(Rit(last), Rit(current), rlast, cond);
    if ( res == rlast )
        return last;
    else
        return --res.base();
}

template <typename TurnIt, typename Cond> inline
TurnIt find_first_if(TurnIt first, TurnIt last, Cond cond)
{
    return std::find_if(first, last, cond);
}

template <typename TurnIt, typename Cond> inline
TurnIt find_last_if(TurnIt first, TurnIt last, Cond cond)
{
    typedef std::reverse_iterator<TurnIt> Rit;
    Rit rlast = Rit(first);
    Rit res = std::find_if(Rit(last), rlast, cond);
    if ( res == rlast )
        return last;
    else
        return --res.base();
}

}}} // namespace detail::relate::turns
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_RELATE_TURNS_HPP
