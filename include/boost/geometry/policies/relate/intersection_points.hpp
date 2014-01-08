// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_GEOMETRY_POLICIES_RELATE_INTERSECTION_POINTS_HPP
#define BOOST_GEOMETRY_GEOMETRY_POLICIES_RELATE_INTERSECTION_POINTS_HPP


#include <algorithm>
#include <string>

#include <boost/concept_check.hpp>
#include <boost/numeric/conversion/cast.hpp>

#include <boost/geometry/arithmetic/determinant.hpp>
#include <boost/geometry/algorithms/detail/assign_indexed_point.hpp>
#include <boost/geometry/algorithms/detail/overlay/segment_ratio.hpp>
#include <boost/geometry/core/access.hpp>
#include <boost/geometry/strategies/side_info.hpp>
#include <boost/geometry/util/select_calculation_type.hpp>
#include <boost/geometry/util/select_most_precise.hpp>


namespace boost { namespace geometry
{

namespace policies { namespace relate
{


template <typename S1, typename S2, typename ReturnType, typename CalculationType = void>
struct segments_intersection_points
{
    typedef ReturnType return_type;
    typedef S1 segment_type1;
    typedef S2 segment_type2;

    typedef typename select_calculation_type
        <
            S1, S2, CalculationType
        >::type coordinate_type;

    template <typename SegmentIntersectionInfo>
    static inline return_type segments_crosses(side_info const&,
                    SegmentIntersectionInfo const& sinfo,
                    S1 const& s1, S2 const& s2)
    {
        typedef typename geometry::coordinate_type
            <
                typename return_type::point_type
            >::type return_coordinate_type;
        typedef typename SegmentIntersectionInfo::promoted_type promoted_type;


        return_type result;
        result.count = 1;

        promoted_type const s1x = get<0, 0>(s1);
        promoted_type const s1y = get<0, 1>(s1);
        promoted_type const dx = sinfo.dx_a;
        promoted_type const dy = sinfo.dy_a;
        // We now always use the robust-ratio because next check was not enough:
        // if (sinfo.r < 0 || sinfo.r > 1)
        // It also would need this check:
        // if (sinfo.rB < 0 || sinfo.rB > 1) for the other segment
        // TODO: these comments can be removed.
        // NOTE: in case of integer, the robust one is identical to the original one (but more precise)
        // in case of float, the robust one is nearly always as precise (or more) than the FP one
        // It does not change the result of the floating-point intersection point
        {
            // Because we calculate side/info test from rescaled coordinates, we now
            // use the ratio based on rescaled too. This is in 99.999% cases exactly the same.
            // Where it is not the same, the FP one is off. Sometimes it is outside
            // the range, so we have to use it...
            // For now we only use that if the FP r is off.
            assert(sinfo.robust_ra.denominator() != 0);
            promoted_type const num = sinfo.robust_ra.numerator();
            promoted_type const den = sinfo.robust_ra.denominator();
            set<0>(result.intersections[0],
                boost::numeric_cast<return_coordinate_type>(s1x + num * dx / den));
            set<1>(result.intersections[0],
                boost::numeric_cast<return_coordinate_type>(s1y + num * dy / den));
        }

        result.fractions[0].assign(sinfo);

#ifdef BOOST_GEOMETRY_CHECK_RATIO
        {
            promoted_type const s2x = get<0, 0>(s2);
            promoted_type const s2y = get<0, 1>(s2);
            promoted_type const dx = sinfo.dx_b;
            promoted_type const dy = sinfo.dy_b;
            set<0>(result.intersections_check[0],
                boost::numeric_cast<return_coordinate_type>(s2x + sinfo.rb * dx));
            set<1>(result.intersections_check[0],
                boost::numeric_cast<return_coordinate_type>(s2y + sinfo.rb * dy));
        }
#else
        boost::ignore_unused_variable_warning(s2);
#endif

        return result;
    }

    template <typename Segment1, typename Segment2, typename Ratio>
    static inline return_type segments_collinear(
        Segment1 const& a, Segment2 const& b,
        Ratio const& ra_from_wrt_b, Ratio const& ra_to_wrt_b,
        Ratio const& rb_from_wrt_a, Ratio const& rb_to_wrt_a)
    {
        return_type result;
        int index = 0, count_a = 0, count_b = 0;
        Ratio on_a[2];

        // IMPORTANT: the order of conditions is different as in direction.hpp
        if (ra_from_wrt_b.on_segment())
        {
            //     a1--------->a2
            // b1----->b2
            //
            // ra1 (relative to b) is between 0/1:
            // -> First point of A is intersection point
            assert(index < 2);
            detail::assign_point_from_index<0>(a, result.intersections[index]);
            result.fractions[index].assign(Ratio::zero(), ra_from_wrt_b);
            on_a[index] = Ratio::zero();
            index++;
            count_a++;
        }
        if (rb_from_wrt_a.in_segment())
        {
            // We take the first intersection point of B
            // a1--------->a2
            //         b1----->b2
            // But only if it is not located on A
            // a1--------->a2
            // b1----->b2      rb_from_wrt_a == 0/1 -> a already taken

            assert(index < 2);
            detail::assign_point_from_index<0>(b, result.intersections[index]);
            result.fractions[index].assign(rb_from_wrt_a, Ratio::zero());
            on_a[index] = rb_from_wrt_a;
            index++;
            count_b++;
        }

        if (ra_to_wrt_b.on_segment())
        {
            // Similarly, second IP (here a2)
            // a1--------->a2
            //         b1----->b2
            assert(index < 2);
            detail::assign_point_from_index<1>(a, result.intersections[index]);
            result.fractions[index].assign(Ratio::one(), ra_to_wrt_b);
            on_a[index] = Ratio::one();
            index++;
            count_a++;
        }
        if (rb_to_wrt_a.in_segment())
        {
            assert(index < 2);
            detail::assign_point_from_index<1>(b, result.intersections[index]);
            result.fractions[index].assign(rb_to_wrt_a, Ratio::one());
            on_a[index] = rb_to_wrt_a;
            index++;
            count_b++;
        }

        // TEMPORARY
        // If both are from b, and b is reversed w.r.t. a, we swap IP's
        // to align them w.r.t. a
        // get_turn_info still relies on some order (in some collinear cases)
        if (index == 2 && on_a[1] < on_a[0])
        {
            std::swap(result.fractions[0], result.fractions[1]);
            std::swap(result.intersections[0], result.intersections[1]);
        }

        result.count = index;

        return result;
    }

    static inline return_type disjoint()
    {
        return return_type();
    }
    static inline return_type error(std::string const&)
    {
        return return_type();
    }

    static inline return_type collinear_disjoint()
    {
        return return_type();
    }

    static inline return_type degenerate(S1 const& s, bool)
    {
        return_type result;
        result.count = 1;
        set<0>(result.intersections[0], get<0, 0>(s));
        set<1>(result.intersections[0], get<0, 1>(s));
        return result;
    }
};


}} // namespace policies::relate

}} // namespace boost::geometry

#endif // BOOST_GEOMETRY_GEOMETRY_POLICIES_RELATE_INTERSECTION_POINTS_HPP
