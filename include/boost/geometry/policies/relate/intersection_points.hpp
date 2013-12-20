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
    typedef boost::rational<boost::long_long_type> robust_type;

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

        coordinate_type const s1x = get<0, 0>(s1);
        coordinate_type const s1y = get<0, 1>(s1);

        return_type result;
        result.count = 1;
        typedef double R; // TODO fix this
        set<0>(result.intersections[0],
            boost::numeric_cast<return_coordinate_type>(R(s1x) + sinfo.r * R(sinfo.dx_a)));
        set<1>(result.intersections[0],
            boost::numeric_cast<return_coordinate_type>(R(s1y) + sinfo.r * R(sinfo.dy_a)));

        result.fractions[0].assign(sinfo);

#ifdef BOOST_GEOMETRY_CHECK_RATIO
        coordinate_type const s2x = get<0, 0>(s2);
        coordinate_type const s2y = get<0, 1>(s2);
        set<0>(result.intersections_check[0],
            boost::numeric_cast<return_coordinate_type>(R(s2x) + sinfo.rb * R(sinfo.dx_b)));
        set<1>(result.intersections_check[0],
            boost::numeric_cast<return_coordinate_type>(R(s2y) + sinfo.rb * R(sinfo.dy_b)));
#else
        boost::ignore_unused_variable_warning(s2);
#endif

        return result;
    }

    template <int Dim, typename Segment, typename Point>
    static inline void assign_point(Segment const& segment, Point& point)
    {
        detail::assign_point_from_index<Dim>(segment, point);
    }

    // TODO: this is copied. Will be removed by using new class segment_ratio
    static inline boost::rational<boost::long_long_type> zero()
    {
        static const boost::rational<boost::long_long_type> result(0, 1);
        return result;
    }
    static inline boost::rational<boost::long_long_type> one()
    {
        static const boost::rational<boost::long_long_type> result(1, 1);
        return result;
    }
    template <typename Ratio>
    static inline bool on_segment(Ratio const& r)
    {
        return r >= zero() && r <= one();
    }
    template <typename Ratio>
    static inline bool in_segment(Ratio const& r)
    {
        return r > zero() && r < one();
    }
    // END TODO


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
        if (on_segment(ra_from_wrt_b))
        {
            //     a1--------->a2
            // b1----->b2
            //
            // ra1 (relative to b) is between 0/1:
            // -> First point of A is intersection point
            assert(index < 2);
            assign_point<0>(a, result.intersections[index]);
            result.fractions[index].assign(zero(), ra_from_wrt_b);
            on_a[index] = zero();
            index++;
            count_a++;
        }
        if (in_segment(rb_from_wrt_a))
        {
            // We take the first intersection point of B
            // a1--------->a2
            //         b1----->b2
            // But only if it is not located on A
            // a1--------->a2
            // b1----->b2      rb_from_wrt_a == 0/1 -> a already taken

            assert(index < 2);
            assign_point<0>(b, result.intersections[index]);
            result.fractions[index].assign(rb_from_wrt_a, zero());
            on_a[index] = rb_from_wrt_a;
            index++;
            count_b++;
        }

        if (on_segment(ra_to_wrt_b))
        {
            // Similarly, second IP (here a2)
            // a1--------->a2
            //         b1----->b2
            assert(index < 2);
            assign_point<1>(a, result.intersections[index]);
            result.fractions[index].assign(one(), ra_to_wrt_b);
            on_a[index] = one();
            index++;
            count_a++;
        }
        if (in_segment(rb_to_wrt_a))
        {
            assert(index < 2);
            assign_point<1>(b, result.intersections[index]);
            result.fractions[index].assign(rb_to_wrt_a, one());
            on_a[index] = rb_to_wrt_a;
            index++;
            count_b++;
        }

        // TEMPORARY
        // If both are from b, and b is reversed w.r.t. a, we swap IP's
        // to align them w.r.t. a
        // get_turn_info still relies on some order (in some collinear cases)
        if (index == 2 && on_a[0] > on_a[1])
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
