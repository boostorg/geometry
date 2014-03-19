// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2011-2014 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_INTERSECTION_SIDE_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_INTERSECTION_SIDE_HPP

#include <boost/geometry/algorithms/detail/recalculate.hpp>
#include <boost/geometry/algorithms/detail/assign_indexed_point.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/strategies/intersection_result.hpp>
#include <boost/geometry/strategies/side.hpp>
#include <boost/geometry/strategies/cartesian/cart_intersect.hpp>
#include <boost/geometry/policies/robustness/segment_ratio_type.hpp>

namespace boost { namespace geometry
{

namespace detail { namespace intersection_side
{


//! Calculates the side (left=1,right=-1,collinear=0) of the intersection point
//! of p/q (if any) with respect to segment s.
//! This is not done by calculating IP p/q and calling side, because that is
//! (even if robust_policy is used) not robust: the IP can be rounded off.
//! So it is done by investigating the sides of the segments, and calculating
//! the segment_ratios of the IP and check which side it is.
//! Return value is conform geometry::side
//! Currently all points are of same type
//! Even if there is no IP at all, a side is returned. IP is not calculated on
//! itself
template <typename Point, typename RobustPolicy>
inline int intersection_side(Point const& pi, Point const& pj,
            Point const& qi, Point const& qj,
            Point const& si, Point const& sj,
            RobustPolicy const& robust_policy)
{
    typedef typename geometry::robust_point_type
    <
        Point,
        RobustPolicy
    >::type robust_point_type;
    typedef typename geometry::segment_ratio_type
    <
        Point,
        RobustPolicy
    >::type segment_ratio_type;

    robust_point_type pi_rob, pj_rob, qi_rob, qj_rob, si_rob, sj_rob;
    geometry::recalculate(pi_rob, pi, robust_policy);
    geometry::recalculate(pj_rob, pj, robust_policy);
    geometry::recalculate(qi_rob, qi, robust_policy);
    geometry::recalculate(qj_rob, qj, robust_policy);
    geometry::recalculate(si_rob, si, robust_policy);
    geometry::recalculate(sj_rob, sj, robust_policy);

    typedef typename geometry::strategy::side::services::default_strategy
        <
            typename geometry::cs_tag<robust_point_type>::type
        >::type side_strategy;

    int const side_pi = side_strategy::apply(pi_rob, si_rob, sj_rob);
    int const side_pj = side_strategy::apply(pj_rob, si_rob, sj_rob);
    if (side_pi == side_pj)
    {
        return side_pi;
    }

    int const side_qi = side_strategy::apply(qi_rob, si_rob, sj_rob);
    int const side_qj = side_strategy::apply(qj_rob, si_rob, sj_rob);
    if (side_qi == side_qj)
    {
        // Same here, see comment above
        return side_qi;
    }

    typedef model::referring_segment<const Point> segment_type;

    segment_type p(pi, pj);
    segment_type q(qi, qj);
    segment_type s(si, sj);

    // No segment lies on the left or right or completely collinear
    // Relate segments p // q and p // s, don't calculate IP's but just return
    // the ratios. Both ratios called "robust_ra" are measured on p and can be
    // related.
    fraction_type<segment_ratio_type> ratio_p_wrt_q
        = strategy::intersection::relate_cartesian_segments
        <
            policies::relate::segments_intersection_ratios
                <
                    fraction_type<segment_ratio_type>
                >
        >::apply(p, q, robust_policy, pi_rob, pj_rob, qi_rob, qj_rob);

    fraction_type<segment_ratio_type> ratio_p_wrt_s
        = strategy::intersection::relate_cartesian_segments
        <
            policies::relate::segments_intersection_ratios
                <
                    fraction_type<segment_ratio_type>
                >
        >::apply(p, s, robust_policy, pi_rob, pj_rob, si_rob, sj_rob);

    return

        // Closer to the segment start point of p -> side of pi
        ratio_p_wrt_q.robust_ra < ratio_p_wrt_s.robust_ra ? side_pi

        // Ratios are equal, IP must be exactly on the segment
        : ratio_p_wrt_q.robust_ra == ratio_p_wrt_s.robust_ra ? 0

        // ratio p wrt q is larger than ratio p wrt s
        // So: close to the segment end point of p -> side of pj
        : side_pj;
}


}} // namespace detail::intersection_side

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_INTERSECTION_SIDE_HPP
