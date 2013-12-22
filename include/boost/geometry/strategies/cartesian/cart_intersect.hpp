// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2013 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_INTERSECTION_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_INTERSECTION_HPP

#include <algorithm>

#include <boost/geometry/core/exception.hpp>

#include <boost/geometry/geometries/concepts/point_concept.hpp>
#include <boost/geometry/geometries/concepts/segment_concept.hpp>

#include <boost/geometry/arithmetic/determinant.hpp>
#include <boost/geometry/algorithms/detail/assign_values.hpp>
#include <boost/geometry/algorithms/detail/disjoint/point_point.hpp>
#include <boost/geometry/algorithms/detail/overlay/segment_ratio.hpp>

#include <boost/geometry/util/math.hpp>
#include <boost/geometry/util/select_calculation_type.hpp>

// Temporary / will be Strategy as template parameter
#include <boost/geometry/strategies/side.hpp>
#include <boost/geometry/strategies/cartesian/side_by_triangle.hpp>

#include <boost/geometry/strategies/side_info.hpp>
#include <boost/geometry/strategies/intersection_result.hpp>

#if defined(BOOST_GEOMETRY_DEBUG_ROBUSTNESS)
#  include <boost/geometry/io/wkt/write.hpp>
#endif


namespace boost { namespace geometry
{


namespace strategy { namespace intersection
{


/*!
    \see http://mathworld.wolfram.com/Line-LineIntersection.html
 */
template <typename Policy, typename CalculationType = void>
struct relate_cartesian_segments
{
    typedef typename Policy::return_type return_type;
    typedef typename Policy::segment_type1 segment_type1;
    typedef typename Policy::segment_type2 segment_type2;

    //typedef typename point_type<segment_type1>::type point_type;
    //BOOST_CONCEPT_ASSERT( (concept::Point<point_type>) );

    BOOST_CONCEPT_ASSERT( (concept::ConstSegment<segment_type1>) );
    BOOST_CONCEPT_ASSERT( (concept::ConstSegment<segment_type2>) );

    typedef typename select_calculation_type
        <segment_type1, segment_type2, CalculationType>::type coordinate_type;

#if defined(BOOST_GEOMETRY_DEBUG_ROBUSTNESS)
    static inline void debug_segments(std::string const& header, segment_type1 const& a, segment_type2 const& b)
    {
        std::cout << "Robustness issue: " << header << std::endl;
        std::cout
            << "A: " << wkt(a) << std::endl
            << "B: " << wkt(b) << std::endl
            ;
    }
#endif

    template <typename T, typename ResultType>
    static inline void cramers_rule(T const& dx_a, T const& dy_a,
        T const& dx_b, T const& dy_b, T const& wx, T const& wy,
        // out:
        ResultType& d, ResultType& da)
    {
        // Cramers rule
        d = geometry::detail::determinant<ResultType>(dx_a, dy_a, dx_b, dy_b);
        da = geometry::detail::determinant<ResultType>(dx_b, dy_b, wx, wy);
        // Ratio is da/d , collinear if d == 0, intersecting if 0 <= r <= 1
        // IntersectionPoint = (x1 + r * dx_a, y1 + r * dy_a)
    }


    // Relate segments a and b
//    static inline return_type apply(segment_type1 const& a, segment_type2 const& b)
//    {
//        // TODO: rescale this and then calculate
//        return apply(a, b, ...);
//    }

    // The main entry-routine, calculating intersections of segments a / b
    template <typename RobustPoint>
    static inline return_type apply(segment_type1 const& a, segment_type2 const& b,
            RobustPoint const& robust_a1, RobustPoint const& robust_a2,
            RobustPoint const& robust_b1, RobustPoint const& robust_b2)
    {
        using geometry::detail::equals::equals_point_point;
        bool const a_is_point = equals_point_point(robust_a1, robust_a2);
        bool const b_is_point = equals_point_point(robust_b1, robust_b2);

        typedef side::side_by_triangle<coordinate_type> side;

#if ! defined(BOOST_GEOMETRY_RESCALE_TO_ROBUST)
        coordinate_type const zero = 0;
#endif

        if(a_is_point && b_is_point)
        {
            return equals_point_point(robust_a1, robust_b2)
                ? Policy::degenerate(a, true)
                : Policy::disjoint()
                ;
        }

        side_info sides;
        sides.set<0>(side::apply(robust_b1, robust_b2, robust_a1),
                     side::apply(robust_b1, robust_b2, robust_a2));
        sides.set<1>(side::apply(robust_a1, robust_a2, robust_b1),
                     side::apply(robust_a1, robust_a2, robust_b2));

        bool collinear = sides.collinear();


        if (sides.same<0>() || sides.same<1>())
        {
            // Both points are at same side of other segment, we can leave
            return Policy::disjoint();
        }

        // Degenerate cases: segments of single point, lying on other segment, are not disjoint
        if (a_is_point)
        {
            return Policy::degenerate(a, true);
        }
        if (b_is_point)
        {
            return Policy::degenerate(b, false);
        }

        typedef typename select_most_precise
            <
                coordinate_type, double
            >::type promoted_type;
        typedef typename geometry::coordinate_type
            <
                RobustPoint
            >::type robust_coordinate_type;

        segment_intersection_info<coordinate_type, promoted_type, robust_coordinate_type> sinfo;

        sinfo.dx_a = get<1, 0>(a) - get<0, 0>(a); // distance in x-dir
        sinfo.dx_b = get<1, 0>(b) - get<0, 0>(b);
        sinfo.dy_a = get<1, 1>(a) - get<0, 1>(a); // distance in y-dir
        sinfo.dy_b = get<1, 1>(b) - get<0, 1>(b);

        robust_coordinate_type const robust_dx_a = get<0>(robust_a2) - get<0>(robust_a1);
        robust_coordinate_type const robust_dx_b = get<0>(robust_b2) - get<0>(robust_b1);
        robust_coordinate_type const robust_dy_a = get<1>(robust_a2) - get<1>(robust_a1);
        robust_coordinate_type const robust_dy_b = get<1>(robust_b2) - get<1>(robust_b1);

        // r: ratio 0-1 where intersection divides A/B
        // (only calculated for non-collinear segments)
        if (! collinear)
        {
            // Calculate determinants - Cramers rule
            coordinate_type const wx = get<0, 0>(a) - get<0, 0>(b);
            coordinate_type const wy = get<0, 1>(a) - get<0, 1>(b);

            promoted_type d, da;
            cramers_rule(sinfo.dx_a, sinfo.dy_a, sinfo.dx_b, sinfo.dy_b,
                wx, wy, d, da);

            robust_coordinate_type const robust_wx = get<0>(robust_a1) - get<0>(robust_b1);
            robust_coordinate_type const robust_wy = get<1>(robust_a1) - get<1>(robust_b1);
            robust_coordinate_type robust_da0, robust_da;
            cramers_rule(robust_dx_a, robust_dy_a, robust_dx_b, robust_dy_b,
                robust_wx, robust_wy, robust_da0, robust_da);

            robust_coordinate_type robust_db0, robust_db;
            cramers_rule(robust_dx_b, robust_dy_b, robust_dx_a, robust_dy_a,
                get<0>(robust_b1) - get<0>(robust_a1),
                get<1>(robust_b1) - get<1>(robust_a1),
                robust_db0, robust_db);

            if (robust_da0 == 0)
            {
                // This is still a collinear case (because of FP imprecision this could, in the past, occur here)
                // Not it should NOT occur anymore
                // sides.debug();
                sides.set<0>(0,0);
                sides.set<1>(0,0);
                collinear = true;
            }
            else
            {
                sinfo.r = da / d;

                sinfo.robust_ra.assign(robust_da, robust_da0);
                sinfo.robust_rb.assign(robust_db, robust_db0);

#ifdef BOOST_GEOMETRY_CHECK_RATIO
                promoted_type db0, db;
                cramers_rule(sinfo.dx_b, sinfo.dy_b, sinfo.dx_a, sinfo.dy_a,
                    get<0, 0>(b) - get<0, 0>(a), // wx = d(x) between 2 segments
                    get<0, 1>(b) - get<0, 1>(a), // wy = d(y) between 2 segments
                    db0, db);
                sinfo.rb = db0 != 0 ? db / db0 : -9999;

//                std::cout << "CHECK: " << std::setprecision(16) << sinfo.r << " " << sinfo.rb
//                    << " " << boost::rational_cast<double>(sinfo.robust_ra)
//                    << " " << boost::rational_cast<double>(sinfo.robust_rb)
//                    << std::endl;
//                std::cout << "  robust: "
//                        << double(sinfo.dx_b) / double(sinfo.dx_a)
//                        << " " << double(sinfo.dy_b) / double(sinfo.dy_a)
//                        << " " << double(wx) / double(wy)
//                        << " " << double(robust_dx_b) / double(robust_dx_a)
//                        << " " << double(robust_dy_b) / double(robust_dy_a)
//                        << " " << double(robust_wx) / double(robust_wy)
//                        << std::endl;

#endif

                verify_r(sinfo.r);
            }
        }

        if(collinear)
        {
            bool collinear_use_first
                    = geometry::math::abs(robust_dx_a) + geometry::math::abs(robust_dx_b)
                    >= geometry::math::abs(robust_dy_a) + geometry::math::abs(robust_dy_b);
            if (collinear_use_first)
            {
                return relate_collinear<0>(a, b, robust_a1, robust_a2, robust_b1, robust_b2);
            }
            else
            {
                // Y direction contains larger segments (maybe dx is zero)
                return relate_collinear<1>(a, b, robust_a1, robust_a2, robust_b1, robust_b2);
            }
        }

        return Policy::segments_crosses(sides, sinfo, a, b);
    }

private :

    template <typename T>
    static inline void verify_r(T& r)
    // If so (<0 or >1) we might use the robust fraction instead, or use it anyway
    {
        T const zero = 0;
        T const one = 1;
        if (r < zero || r > one)
        {
            // Note that even for ttmath r is occasionally > 1, e.g. 1.0000000000000000000000036191231203575
#if defined(BOOST_GEOMETRY_DEBUG_ROBUSTNESS)
            debug_segments("correcting r", a, b);
            std::cout << " --> r=" << r;
            if (r > 1.00000000000001 || r < -0.00000000000001)
            {
                std::cout << " !!!";
            }
            std::cout << std::endl << std::endl;
#endif

            if (r > one)
            {
                r = one;
            }
            else if (r < zero)
            {
                r = zero;
            }
        }
    }

    template <std::size_t Dimension, typename RobustPoint>
    static inline return_type relate_collinear(segment_type1 const& a,
            segment_type2 const& b,
            RobustPoint const& robust_a1, RobustPoint const& robust_a2,
            RobustPoint const& robust_b1, RobustPoint const& robust_b2)
    {
        return relate_collinear(a, b,
                                get<Dimension>(robust_a1),
                                get<Dimension>(robust_a2),
                                get<Dimension>(robust_b1),
                                get<Dimension>(robust_b2));
    }

    /// Relate segments known collinear
    template <typename RobustType>
    static inline return_type relate_collinear(segment_type1 const& a
            , segment_type2 const& b
            , RobustType oa_1, RobustType oa_2
            , RobustType ob_1, RobustType ob_2
            )
    {
        // Calculate the ratios where a starts in b, b starts in a
        //         a1--------->a2         (2..7)
        //                b1----->b2      (5..8)
        // length_a: 7-2=5
        // length_b: 8-5=3
        // b1 is located w.r.t. a at ratio: (5-2)/5=3/5 (on a)
        // b2 is located w.r.t. a at ratio: (8-2)/5=6/5 (right of a)
        // a1 is located w.r.t. b at ratio: (2-5)/3=-3/3 (left of b)
        // a2 is located w.r.t. b at ratio: (7-5)/3=2/3 (on b)
        // A arrives (a2 on b), B departs (b1 on a)

        // If both are reversed:
        //         a2<---------a1         (7..2)
        //                b2<-----b1      (8..5)
        // length_a: 2-7=-5
        // length_b: 5-8=-3
        // b1 is located w.r.t. a at ratio: (8-7)/-5=-1/5 (before a starts)
        // b2 is located w.r.t. a at ratio: (5-7)/-5=2/5 (on a)
        // a1 is located w.r.t. b at ratio: (7-8)/-3=1/3 (on b)
        // a2 is located w.r.t. b at ratio: (2-8)/-3=6/3 (after b ends)

        // If both one is reversed:
        //         a1--------->a2         (2..7)
        //                b2<-----b1      (8..5)
        // length_a: 7-2=+5
        // length_b: 5-8=-3
        // b1 is located w.r.t. a at ratio: (8-2)/5=6/5 (after a ends)
        // b2 is located w.r.t. a at ratio: (5-2)/5=3/5 (on a)
        // a1 is located w.r.t. b at ratio: (2-8)/-3=6/3 (after b ends)
        // a2 is located w.r.t. b at ratio: (7-8)/-3=1/3 (on b)
        RobustType const length_a = oa_2 - oa_1; // no abs, see above
        RobustType const length_b = ob_2 - ob_1;

        typedef geometry::segment_ratio<boost::long_long_type> ratio_type;
        ratio_type const ra_from(oa_1 - ob_1, length_b);
        ratio_type const ra_to(oa_2 - ob_1, length_b);
        ratio_type const rb_from(ob_1 - oa_1, length_a);
        ratio_type const rb_to(ob_2 - oa_1, length_a);

        if ((ra_from.left() && ra_to.left()) || (ra_from.right() && ra_to.right()))
        {
            return Policy::disjoint();
        }

        return Policy::segments_collinear(a, b, ra_from, ra_to, rb_from, rb_to);
    }
};


}} // namespace strategy::intersection

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_INTERSECTION_HPP
