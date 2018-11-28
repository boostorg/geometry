// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2013, 2014, 2015, 2017.
// Modifications copyright (c) 2013-2017 Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_GET_TURN_INFO_HELPERS_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_GET_TURN_INFO_HELPERS_HPP

#include <boost/geometry/policies/robustness/no_rescale_policy.hpp>
#include <boost/geometry/algorithms/detail/direction_code.hpp>

namespace boost { namespace geometry {

#ifndef DOXYGEN_NO_DETAIL
namespace detail { namespace overlay {

enum turn_position { position_middle, position_front, position_back };

template <typename Point, typename SegmentRatio>
struct turn_operation_linear
    : public turn_operation<Point, SegmentRatio>
{
    turn_operation_linear()
        : position(position_middle)
        , is_collinear(false)
    {}

    turn_position position;
    bool is_collinear; // valid only for Linear geometry
};

template
<
    typename TurnPointCSTag, typename PointP, typename PointQ,
    typename RetrievePolicy1, typename RetrievePolicy2,
    typename SideStrategy
>
struct side_calculator
{
    inline side_calculator(PointP const& pi, PointP const& pj,
                           PointQ const& qi, PointQ const& qj,
                           RetrievePolicy1 const& retrieve_policy_p,
                           RetrievePolicy2 const& retrieve_policy_q,
                           SideStrategy const& side_strategy)
        : m_pi(pi), m_pj(pj)
        , m_qi(qi), m_qj(qj)
        , m_side_strategy(side_strategy)
        , m_retrieve_policy_p(retrieve_policy_p)
        , m_retrieve_policy_q(retrieve_policy_q)
    {}

    inline int pk_wrt_p1() const { return m_side_strategy.apply(m_pi, m_pj, get_pk()); }
    inline int pk_wrt_q1() const { return m_side_strategy.apply(m_qi, m_qj, get_pk()); }
    inline int qk_wrt_p1() const { return m_side_strategy.apply(m_pi, m_pj, get_qk()); }
    inline int qk_wrt_q1() const { return m_side_strategy.apply(m_qi, m_qj, get_qk()); }

    inline int pk_wrt_q2() const { return m_side_strategy.apply(m_qj, get_qk(), get_pk()); }
    inline int qk_wrt_p2() const { return m_side_strategy.apply(m_pj, get_pk(), get_qk()); }

    inline PointP const& get_pk() const { return m_retrieve_policy_p.get(); }
    inline PointQ const& get_qk() const { return m_retrieve_policy_q.get(); }

    PointP const& m_pi;
    PointP const& m_pj;
    PointQ const& m_qi;
    PointQ const& m_qj;

    SideStrategy const& m_side_strategy;
    RetrievePolicy1 const& m_retrieve_policy_p;
    RetrievePolicy2 const& m_retrieve_policy_q;
};

template<typename Point, typename RetrievePolicy, typename RobustPolicy>
struct robust_retriever
{
    robust_retriever(RetrievePolicy const& retrieve_policy,
                     RobustPolicy const& robust_policy)

        : m_retrieve_policy(retrieve_policy)
        , m_robust_policy(robust_policy)
        , m_retrieved(false)
    {}

    Point const& get() const
    {
        if (! m_retrieved)
        {
            geometry::recalculate(m_result, m_retrieve_policy.get(), m_robust_policy);
            m_retrieved = true;
        }
        return m_result;
    }

    RetrievePolicy const& m_retrieve_policy;
    RobustPolicy const& m_robust_policy;
    mutable Point m_result;
    mutable bool m_retrieved;
};

template
<
    typename Point1, typename Point2,
    typename RetrievePolicy1, typename RetrievePolicy2,
    typename RobustPolicy
>
struct robust_points
{
    typedef typename geometry::robust_point_type
        <
            Point1, RobustPolicy
        >::type robust_point1_type;

    typedef robust_point1_type robust_point2_type;

    inline robust_points(Point1 const& pi, Point1 const& pj, Point1 const& pk,
                         Point2 const& qi, Point2 const& qj, Point2 const& qk,
                         RobustPolicy const& robust_policy)
    {
        geometry::recalculate(m_rpi, pi, robust_policy);
        geometry::recalculate(m_rpj, pj, robust_policy);
        geometry::recalculate(m_rpk, pk, robust_policy);
        geometry::recalculate(m_rqi, qi, robust_policy);
        geometry::recalculate(m_rqj, qj, robust_policy);
        geometry::recalculate(m_rqk, qk, robust_policy);
    }

    robust_point1_type m_rpi, m_rpj, m_rpk;
    robust_point2_type m_rqi, m_rqj, m_rqk;
};

template
<
    typename Point1, typename Point2,
    typename RetrievePolicy1, typename RetrievePolicy2,
    typename TurnPoint, typename IntersectionStrategy, typename RobustPolicy>
class intersection_info_base
    : private robust_points<Point1, Point2, RetrievePolicy1, RetrievePolicy2, RobustPolicy>
{
    typedef robust_points<Point1, Point2, RetrievePolicy1, RetrievePolicy2, RobustPolicy> base;

public:
    typedef Point1 point1_type;
    typedef Point2 point2_type;

    typedef typename base::robust_point1_type robust_point1_type;
    typedef typename base::robust_point2_type robust_point2_type;

    typedef robust_retriever<robust_point1_type, RetrievePolicy1, RobustPolicy> robust_retriever_type1;
    typedef robust_retriever<robust_point2_type, RetrievePolicy2, RobustPolicy> robust_retriever_type2;

    typedef typename cs_tag<TurnPoint>::type cs_tag;

    typedef typename IntersectionStrategy::side_strategy_type side_strategy_type;
    typedef side_calculator<cs_tag, robust_point1_type, robust_point2_type,
        robust_retriever_type1, robust_retriever_type2, side_strategy_type> side_calculator_type;

    typedef side_calculator
        <
            cs_tag, robust_point2_type, robust_point1_type,
            robust_retriever_type2, robust_retriever_type1,
            side_strategy_type
        > robust_swapped_side_calculator_type;

    intersection_info_base(Point1 const& pi, Point1 const& pj,
                           Point2 const& qi, Point2 const& qj,
                           RetrievePolicy1 const& retrieve_policy_p,
                           RetrievePolicy2 const& retrieve_policy_q,
                           IntersectionStrategy const& intersection_strategy,
                           RobustPolicy const& robust_policy)
        : base(pi, pj, retrieve_policy_p.get(),
               qi, qj, retrieve_policy_q.get(), robust_policy)
        , m_robust_retrieve_policy_p(retrieve_policy_p, robust_policy)
        , m_robust_retrieve_policy_q(retrieve_policy_q, robust_policy)
        , m_side_calc(base::m_rpi, base::m_rpj,
                      base::m_rqi, base::m_rqj,
                      m_robust_retrieve_policy_p, m_robust_retrieve_policy_q,
                      intersection_strategy.get_side_strategy())
        , m_pi(pi), m_qi(qi)
    {}

    inline Point1 const& pi() const { return m_pi; }
    inline Point2 const& qi() const { return m_qi; }

    inline robust_point1_type const& rpi() const { return base::m_rpi; }
    inline robust_point1_type const& rpj() const { return base::m_rpj; }
    inline robust_point1_type const& rpk() const { return base::m_rpk; }

    inline robust_point2_type const& rqi() const { return base::m_rqi; }
    inline robust_point2_type const& rqj() const { return base::m_rqj; }
    inline robust_point2_type const& rqk() const { return base::m_rqk; }

    inline side_calculator_type const& sides() const { return m_side_calc; }
    
    robust_swapped_side_calculator_type get_swapped_sides() const
    {
        robust_swapped_side_calculator_type result(rqi(), rqj(), rpi(), rpj(),
                            m_robust_retrieve_policy_q, m_robust_retrieve_policy_p,
                            m_side_calc.m_side_strategy);
        return result;
    }

private:
    robust_retriever_type1 m_robust_retrieve_policy_p;
    robust_retriever_type2 m_robust_retrieve_policy_q;
    side_calculator_type m_side_calc;

    point1_type const& m_pi;
    point2_type const& m_qi;
};

template
<
    typename Point1, typename Point2,
    typename RetrievePolicy1, typename RetrievePolicy2,
    typename TurnPoint, typename IntersectionStrategy
>
class intersection_info_base<Point1, Point2, RetrievePolicy1, RetrievePolicy2,
        TurnPoint, IntersectionStrategy, detail::no_rescale_policy>
{
public:
    typedef Point1 point1_type;
    typedef Point2 point2_type;

    typedef Point1 robust_point1_type;
    typedef Point2 robust_point2_type;

    typedef typename cs_tag<TurnPoint>::type cs_tag;

    typedef typename IntersectionStrategy::side_strategy_type side_strategy_type;
    typedef side_calculator<cs_tag, Point1, Point2,
        RetrievePolicy1, RetrievePolicy2, side_strategy_type> side_calculator_type;

    typedef side_calculator
        <
            cs_tag, point2_type, point1_type,
            RetrievePolicy2, RetrievePolicy1,
            side_strategy_type
        > swapped_side_calculator_type;
    
    intersection_info_base(Point1 const& pi, Point1 const& pj,
                           Point2 const& qi, Point2 const& qj,
                           RetrievePolicy1 const& retrieve_policy_p,
                           RetrievePolicy2 const& retrieve_policy_q,
                           IntersectionStrategy const& intersection_strategy,
                           no_rescale_policy const& /*robust_policy*/)
        : m_retrieve_policy_p(retrieve_policy_p)
        , m_retrieve_policy_q(retrieve_policy_q)
        , m_side_calc(pi, pj, qi, qj,
                      retrieve_policy_p, retrieve_policy_q,
                      intersection_strategy.get_side_strategy())
    {}

    inline Point1 const& pi() const { return m_side_calc.m_pi; }
    inline Point2 const& qi() const { return m_side_calc.m_qi; }

    inline Point1 const& rpi() const { return m_side_calc.m_pi; }
    inline Point1 const& rpj() const { return m_side_calc.m_pj; }
    inline Point1 const& rpk() const { return m_side_calc.get_pk(); }

    inline Point2 const& rqi() const { return m_side_calc.m_qi; }
    inline Point2 const& rqj() const { return m_side_calc.m_qj; }
    inline Point2 const& rqk() const { return m_side_calc.get_qk(); }

    inline side_calculator_type const& sides() const { return m_side_calc; }

    swapped_side_calculator_type get_swapped_sides() const
    {
        swapped_side_calculator_type result(m_side_calc.m_qi, m_side_calc.m_qj,
            m_side_calc.m_pi, m_side_calc.m_pj,
            m_retrieve_policy_q, m_retrieve_policy_p,
            m_side_calc.m_side_strategy);
        return result;
    }

private:
    RetrievePolicy1 const& m_retrieve_policy_p;
    RetrievePolicy2 const& m_retrieve_policy_q;
    side_calculator_type m_side_calc;
};


template
<
    typename Point1, typename Point2,
    typename RetrievePolicy1, typename RetrievePolicy2,
    typename TurnPoint,
    typename IntersectionStrategy,
    typename RobustPolicy
>
class intersection_info
    : public intersection_info_base<Point1, Point2, RetrievePolicy1, RetrievePolicy2,
        TurnPoint, IntersectionStrategy, RobustPolicy>
{
    typedef intersection_info_base<Point1, Point2, RetrievePolicy1, RetrievePolicy2,
        TurnPoint, IntersectionStrategy, RobustPolicy> base;

public:
    typedef segment_intersection_points
    <
        TurnPoint,
        typename geometry::segment_ratio_type
            <
                TurnPoint, RobustPolicy
            >::type
    > intersection_point_type;

    // NOTE: formerly defined in intersection_strategies
    typedef policies::relate::segments_tupled
        <
            policies::relate::segments_intersection_points
                <
                    intersection_point_type
                >,
            policies::relate::segments_direction
        > intersection_policy_type;

    typedef IntersectionStrategy intersection_strategy_type;
    typedef typename IntersectionStrategy::side_strategy_type side_strategy_type;

    typedef model::referring_segment<Point1 const> segment_type1;
    typedef model::referring_segment<Point2 const> segment_type2;
    typedef typename base::side_calculator_type side_calculator_type;
    
    typedef typename intersection_policy_type::return_type result_type;
    typedef typename boost::tuples::element<0, result_type>::type i_info_type; // intersection_info
    typedef typename boost::tuples::element<1, result_type>::type d_info_type; // dir_info

    intersection_info(Point1 const& pi, Point1 const& pj,
                      Point2 const& qi, Point2 const& qj,
                      RetrievePolicy1 const& retrieve_policy_p,
                      RetrievePolicy2 const& retrieve_policy_q,
                      IntersectionStrategy const& intersection_strategy,
                      RobustPolicy const& robust_policy)
        : base(pi, pj, qi, qj,
               retrieve_policy_p, retrieve_policy_q,
               intersection_strategy, robust_policy)
        , m_result(intersection_strategy.apply(
                        segment_type1(pi, pj),
                        segment_type2(qi, qj),
                        intersection_policy_type(),
                        robust_policy,
                        base::rpi(), base::rpj(),
                        base::rqi(), base::rqj()))
        , m_intersection_strategy(intersection_strategy)
        , m_robust_policy(robust_policy)
    {}

    inline result_type const& result() const { return m_result; }
    inline i_info_type const& i_info() const { return m_result.template get<0>(); }
    inline d_info_type const& d_info() const { return m_result.template get<1>(); }

    inline side_strategy_type get_side_strategy() const
    {
        return m_intersection_strategy.get_side_strategy();
    }

    // TODO: it's more like is_spike_ip_p
    inline bool is_spike_p() const
    {
        if (base::sides().pk_wrt_p1() == 0)
        {
            // p:  pi--------pj--------pk
            // or: pi----pk==pj

            if (! is_ip_j<0>())
            {
                return false;
            }

            int const qk_p1 = base::sides().qk_wrt_p1();
            int const qk_p2 = base::sides().qk_wrt_p2();

            if (qk_p1 == -qk_p2)
            {
                if (qk_p1 == 0)
                {
                    // qk is collinear with both p1 and p2,
                    // verify if pk goes backwards w.r.t. pi/pj
                    return direction_code(base::rpi(), base::rpj(), base::rpk()) == -1;
                }

                // qk is at opposite side of p1/p2, therefore
                // p1/p2 (collinear) are opposite and form a spike
                return true;
            }
        }
        
        return false;
    }

    inline bool is_spike_q() const
    {
        // See comments at is_spike_p
        if (base::sides().qk_wrt_q1() == 0)
        {
            if (! is_ip_j<1>())
            {
                return false;
            }

            int const pk_q1 = base::sides().pk_wrt_q1();
            int const pk_q2 = base::sides().pk_wrt_q2();
                
            if (pk_q1 == -pk_q2)
            {
                if (pk_q1 == 0)
                {
                    return direction_code(base::rqi(), base::rqj(), base::rqk()) == -1;
                }
                        
                return true;
            }
        }
        
        return false;
    }

private:
    template <std::size_t OpId>
    bool is_ip_j() const
    {
        int arrival = d_info().arrival[OpId];
        bool same_dirs = d_info().dir_a == 0 && d_info().dir_b == 0;

        if (same_dirs)
        {
            if (i_info().count == 2)
            {
                return arrival != -1;
            }
            else
            {
                return arrival == 0;
            }
        }
        else
        {
            return arrival == 1;
        }
    }

    result_type m_result;
    IntersectionStrategy const& m_intersection_strategy;
    RobustPolicy const& m_robust_policy;
};

}} // namespace detail::overlay
#endif // DOXYGEN_NO_DETAIL

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_OVERLAY_GET_TURN_INFO_HELPERS_HPP
