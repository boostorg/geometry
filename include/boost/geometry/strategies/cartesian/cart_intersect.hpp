// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2007-2012 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2013 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_INTERSECTION_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_INTERSECTION_HPP

#include <algorithm>

#include <boost/rational.hpp>


#include <boost/geometry/core/exception.hpp>

#include <boost/geometry/geometries/concepts/point_concept.hpp>
#include <boost/geometry/geometries/concepts/segment_concept.hpp>

#include <boost/geometry/arithmetic/determinant.hpp>
#include <boost/geometry/algorithms/detail/assign_values.hpp>
#include <boost/geometry/algorithms/detail/disjoint/point_point.hpp>

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


#ifndef DOXYGEN_NO_DETAIL
namespace detail
{

#if ! defined(BOOST_GEOMETRY_RESCALE_TO_ROBUST)
template <std::size_t Dimension, typename Segment, typename T>
static inline void segment_arrange(Segment const& s, T& s_1, T& s_2, bool& swapped)
{
    s_1 = get<0, Dimension>(s);
    s_2 = get<1, Dimension>(s);
    if (s_1 > s_2)
    {
        std::swap(s_1, s_2);
        swapped = true;
    }
}

template <std::size_t Index, typename Segment>
inline typename geometry::point_type<Segment>::type get_from_index(
            Segment const& segment)
{
    typedef typename geometry::point_type<Segment>::type point_type;
    point_type point;
    geometry::detail::assign::assign_point_from_index
        <
            Segment, point_type, Index, 0, dimension<Segment>::type::value
        >::apply(segment, point);
    return point;
}

// this will go too
template <std::size_t Dimension, typename Point, typename T>
static inline void point_arrange(Point const& p1, Point const& p2, T& s_1, T& s_2, bool& swapped)
{
    s_1 = get<Dimension>(p1);
    s_2 = get<Dimension>(p2);
    if (s_1 > s_2)
    {
        std::swap(s_1, s_2);
        swapped = true;
    }
}
#endif

}
#endif // DOXYGEN_NO_DETAIL

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

#if ! defined(BOOST_GEOMETRY_RESCALE_TO_ROBUST)
        robustness_verify_collinear(a, b, a_is_point, b_is_point, sides, collinear);
        robustness_verify_meeting(a, b, sides, collinear, collinear_use_first);
#endif

        if (sides.same<0>() || sides.same<1>())
        {
            // Both points are at same side of other segment, we can leave
#if ! defined(BOOST_GEOMETRY_RESCALE_TO_ROBUST)
            if (robustness_verify_same_side(a, b, sides))
#endif
            {
                return Policy::disjoint();
            }
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
#if 0

            promoted_type const d = geometry::detail::determinant<promoted_type>(sinfo.dx_a, sinfo.dy_a, sinfo.dx_b, sinfo.dy_b);
            promoted_type const da = geometry::detail::determinant<promoted_type>(sinfo.dx_b, sinfo.dy_b, wx, wy);

            // Calculate them robustly as well, for various comparisons (TODO - conditionally)
            robust_coordinate_type const robust_da0 = geometry::detail::determinant<robust_coordinate_type>
                  (
                    robust_dx_a, robust_dy_a,
                    robust_dx_b, robust_dy_b
                  );
            robust_coordinate_type const robust_db0 = geometry::detail::determinant<robust_coordinate_type>
                  (
                    robust_dx_b, robust_dy_b,
                    robust_dx_a, robust_dy_a
                  );

            robust_coordinate_type const robust_wx1 = get<0>(robust_a1) - get<0>(robust_b1);
            robust_coordinate_type const robust_wy1 = get<1>(robust_a1) - get<1>(robust_b1);
            robust_coordinate_type const robust_wx2 = get<0>(robust_a2) - get<0>(robust_b2);
            robust_coordinate_type const robust_wy2 = get<1>(robust_a2) - get<1>(robust_b2);
            robust_coordinate_type const robust_da = geometry::detail::determinant<robust_coordinate_type>(robust_dx_b, robust_dy_b, robust_wx1, robust_wy1);
            robust_coordinate_type const robust_db = geometry::detail::determinant<robust_coordinate_type>(robust_dx_a, robust_dy_a, robust_wx2, robust_wy2);
#endif
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

                sinfo.robust_ra = boost::rational<boost::long_long_type>(robust_da, robust_da0);
                sinfo.robust_rb = boost::rational<boost::long_long_type>(robust_db, robust_db0);

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

#if defined(BOOST_GEOMETRY_RESCALE_TO_ROBUST)
                verify_r(sinfo.r);
#else
                if (! robustness_verify_r(a, b, r))
                {
                    // Also this should NOT occur anymore
                    return Policy::disjoint();
                }
                if (robustness_verify_disjoint_at_one_collinear(a, b, sides))
                {
                    return Policy::disjoint();
                }
#endif
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

#if defined(BOOST_GEOMETRY_RESCALE_TO_ROBUST)
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

#else

    // Ratio should lie between 0 and 1
    // Also these three conditions might be of FP imprecision, the segments were actually (nearly) collinear
    template <typename T>
    static inline bool robustness_verify_r(
                segment_type1 const& a, segment_type2 const& b,
                T& r)
    {
        T const zero = 0;
        T const one = 1;
        if (r < zero || r > one)
        {
            if (verify_disjoint<0>(a, b) || verify_disjoint<1>(a, b))
            {
                // Can still be disjoint (even if not one is left or right from another)
                // This is e.g. in case #snake4 of buffer test.
                return false;
            }

            //std::cout << "ROBUSTNESS: correction of r " << r << std::endl;
            // sides.debug();

            // ROBUSTNESS: the r value can in epsilon-cases much larger than 1, while (with perfect arithmetic)
            // it should be one. It can be 1.14 or even 1.98049 or 2 (while still intersecting)

            // If segments are crossing (we can see that with the sides)
            // and one is inside the other, there must be an intersection point.
            // We correct for that.
            // This is (only) in case #ggl_list_20110820_christophe in unit tests

            // If segments are touching (two sides zero), of course they should intersect
            // This is (only) in case #buffer_rt_i in the unit tests)

            // If one touches in the middle, they also should intersect (#buffer_rt_j)

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
        return true;
    }
#endif

    template <std::size_t Dimension>
    static inline bool analyse_equal(segment_type1 const& a, segment_type2 const& b)
    {
        coordinate_type const a_1 = geometry::get<0, Dimension>(a);
        coordinate_type const a_2 = geometry::get<1, Dimension>(a);
        coordinate_type const b_1 = geometry::get<0, Dimension>(b);
        coordinate_type const b_2 = geometry::get<1, Dimension>(b);
        return math::equals(a_1, b_1)
            || math::equals(a_2, b_1)
            || math::equals(a_1, b_2)
            || math::equals(a_2, b_2)
            ;
    }

#if ! defined(BOOST_GEOMETRY_RESCALE_TO_ROBUST)
all this stuff can be removed completely
    static inline void robustness_verify_collinear(
                segment_type1 const& a, segment_type2 const& b,
                bool a_is_point, bool b_is_point,
                side_info& sides,
                bool& collinear)
    {
        bool only_0_collinear = sides.zero<0>() && ! b_is_point && ! sides.zero<1>();
        bool only_1_collinear = sides.zero<1>() && ! a_is_point && ! sides.zero<0>();
        if (only_0_collinear || only_1_collinear)
        {
            typename geometry::point_type<segment_type1>::type a0 = detail::get_from_index<0>(a);
            typename geometry::point_type<segment_type1>::type a1 = detail::get_from_index<1>(a);
            typename geometry::point_type<segment_type2>::type b0 = detail::get_from_index<0>(b);
            typename geometry::point_type<segment_type2>::type b1 = detail::get_from_index<1>(b);
            bool ae = false, be = false;

            // If one of the segments is collinear, the other is probably so too.
            side_info check;
            coordinate_type factor = 1;
            int iteration = 0;
            bool collinear_consistent = false;
            do
            {
                typedef side::side_by_triangle<coordinate_type> side;

                // We have a robustness issue. We keep increasing epsilon until we have a consistent set
                coordinate_type const two = 2;
                factor *= two;
                coordinate_type epsilon = math::relaxed_epsilon<coordinate_type>(factor);
                check.set<0>
                    (
                        side::apply_with_epsilon(b0, b1, a0, epsilon),
                        side::apply_with_epsilon(b0, b1, a1, epsilon)
                    );
                check.set<1>
                    (
                        side::apply_with_epsilon(a0, a1, b0, epsilon),
                        side::apply_with_epsilon(a0, a1, b1, epsilon)
                    );
                ae = point_equals_with_epsilon(a0, a1, epsilon);
                be = point_equals_with_epsilon(b0, b1, epsilon);

                collinear_consistent = true;
                if (   (check.zero<0>() && ! be && ! check.zero<1>())
                    || (check.zero<1>() && ! ae && ! check.zero<0>())
                   )
                {
                    collinear_consistent = false;
                }

#if defined(BOOST_GEOMETRY_DEBUG_ROBUSTNESS)
                std::cout
                    << "*** collinear_consistent: "
                    << iteration << std::boolalpha
                    << " consistent: " << collinear_consistent
                    << " equals: " << ae << "," << be
                    << " epsilon: " << epsilon
                    << "  ";
                check.debug();
#endif


            } while (! collinear_consistent && iteration++ < 10);

            sides = check;
            collinear = sides.collinear();
        }
    }

    static inline void robustness_verify_meeting(
                segment_type1 const& a, segment_type2 const& b,
                side_info& sides,
                bool& collinear, bool& collinear_use_first)
    {
        if (sides.meeting())
        {
            // If two segments meet each other at their segment-points, two sides are zero,
            // the other two are not (unless collinear but we don't mean those here).
            // However, in near-epsilon ranges it can happen that two sides are zero
            // but they do not meet at their segment-points.
            // In that case they are nearly collinear and handled as such.

            if (! point_equals
                    (
                        select(sides.zero_index<0>(), a),
                        select(sides.zero_index<1>(), b)
                    )
                )
            {

                typename geometry::point_type<segment_type1>::type a0 = detail::get_from_index<0>(a);
                typename geometry::point_type<segment_type1>::type a1 = detail::get_from_index<1>(a);
                typename geometry::point_type<segment_type2>::type b0 = detail::get_from_index<0>(b);
                typename geometry::point_type<segment_type2>::type b1 = detail::get_from_index<1>(b);

                side_info check;
                coordinate_type factor = 1;
                coordinate_type epsilon = math::relaxed_epsilon<coordinate_type>(factor);
                int iteration = 1;
                bool points_meet = false;
                bool meeting_consistent = false;
                do
                {
                    typedef side::side_by_triangle<coordinate_type> side;

                    // We have a robustness issue. We keep increasing epsilon until we have a consistent set
                    coordinate_type const two = 2;
                    factor *= two;
                    epsilon = math::relaxed_epsilon<coordinate_type>(factor);
                    check.set<0>
                        (
                            side::apply_with_epsilon(b0, b1, a0, epsilon),
                            side::apply_with_epsilon(b0, b1, a1, epsilon)
                        );
                    check.set<1>
                        (
                            side::apply_with_epsilon(a0, a1, b0, epsilon),
                            side::apply_with_epsilon(a0, a1, b1, epsilon)
                        );

                    meeting_consistent = true;
                    if (check.meeting())
                    {
                        points_meet = point_equals_with_epsilon
                                (
                                    select(check.zero_index<0>(), a),
                                    select(check.zero_index<1>(), b),
                                    epsilon
                                );
                        if (! points_meet)
                        {
                            meeting_consistent = false;

                        }
                    }

#if defined(BOOST_GEOMETRY_DEBUG_ROBUSTNESS)
                    std::cout
                        << "*** meeting_consistent: "
                        << iteration << std::boolalpha
                        << " consistent: " << meeting_consistent
                        << " epsilon: " << epsilon
                        << "  ";
                    check.debug();
#endif


                } while (! meeting_consistent && iteration++ < 10);


                sides = check;

                if (! sides.meeting()
                     && ((sides.zero<0>() && !sides.zero<1>())
                            || (! sides.zero<0>() && sides.zero<1>())
                        )
                    )
                {
                    // Set sides to zero
                    sides.set<0>(0,0);
                    sides.set<1>(0,0);
#if defined(BOOST_GEOMETRY_DEBUG_ROBUSTNESS)
                    std::cout << "ADAPTED New side info: " << std::endl;
                    sides.debug();
#endif
                }

                collinear = sides.collinear();

                if (collinear_use_first && analyse_equal<0>(a, b))
                {
                    collinear_use_first = false;
#if defined(BOOST_GEOMETRY_DEBUG_ROBUSTNESS)
                    std::cout << "Use [1] to check collinearity" << std::endl;
#endif
                }
                else if (! collinear_use_first && analyse_equal<1>(a, b))
                {
                    collinear_use_first = true;
#if defined(BOOST_GEOMETRY_DEBUG_ROBUSTNESS)
                    std::cout << "Use [0] to check collinearity" << std::endl;
#endif
                }
            }
        }
    }

    // Verifies and if necessary correct missed touch because of robustness
    // This is the case at multi_polygon_buffer unittest #rt_m
    static inline bool robustness_verify_same_side(
                segment_type1 const& a, segment_type2 const& b,
                side_info& sides)
    {
        int corrected = 0;
        if (sides.one_touching<0>())
        {
            if (point_equals(
                        select(sides.zero_index<0>(), a),
                        select(0, b)
                    ))
            {
                sides.correct_to_zero<1, 0>();
                corrected = 1;
            }
            if (point_equals
                    (
                        select(sides.zero_index<0>(), a),
                        select(1, b)
                    ))
            {
                sides.correct_to_zero<1, 1>();
                corrected = 2;
            }
        }
        else if (sides.one_touching<1>())
        {
            if (point_equals(
                        select(sides.zero_index<1>(), b),
                        select(0, a)
                    ))
            {
                sides.correct_to_zero<0, 0>();
                corrected = 3;
            }
            if (point_equals
                    (
                        select(sides.zero_index<1>(), b),
                        select(1, a)
                    ))
            {
                sides.correct_to_zero<0, 1>();
                corrected = 4;
            }
        }

        return corrected == 0;
    }

    static inline bool robustness_verify_disjoint_at_one_collinear(
                segment_type1 const& a, segment_type2 const& b,
                side_info const& sides)
    {
        if (sides.one_of_all_zero())
        {
            if (verify_disjoint<0>(a, b) || verify_disjoint<1>(a, b))
            {
                return true;
            }
        }
        return false;
    }

    template <typename Segment>
    static inline typename point_type<Segment>::type select(int index, Segment const& segment)
    {
        return index == 0
            ? detail::get_from_index<0>(segment)
            : detail::get_from_index<1>(segment)
            ;
    }

    // We cannot use geometry::equals here. Besides that this will be changed
    // to compare segment-coordinate-values directly (not necessary to retrieve point first)
    template <typename Point1, typename Point2>
    static inline bool point_equals(Point1 const& point1, Point2 const& point2)
    {
        return math::equals(get<0>(point1), get<0>(point2))
            && math::equals(get<1>(point1), get<1>(point2))
            ;
    }

    template <typename Point1, typename Point2, typename T>
    static inline bool point_equals_with_epsilon(Point1 const& point1, Point2 const& point2, T const& epsilon)
    {
        // Check if difference is within espilon range (epsilon can be 0 for integer)
        return math::abs(geometry::get<0>(point1) - geometry::get<0>(point2)) <= epsilon
            && math::abs(geometry::get<1>(point1) - geometry::get<1>(point2)) <= epsilon
            ;
    }


    // We cannot use geometry::equals here. Besides that this will be changed
    // to compare segment-coordinate-values directly (not necessary to retrieve point first)
    template <typename Point1, typename Point2>
    static inline bool point_equality(Point1 const& point1, Point2 const& point2,
                    bool& equals_0, bool& equals_1)
    {
        equals_0 = math::equals(get<0>(point1), get<0>(point2));
        equals_1 = math::equals(get<1>(point1), get<1>(point2));
        return equals_0 && equals_1;
    }

    template <std::size_t Dimension>
    static inline bool verify_disjoint(segment_type1 const& a,
                    segment_type2 const& b)
    {
        coordinate_type a_1, a_2, b_1, b_2;
        bool a_swapped = false, b_swapped = false;
        detail::segment_arrange<Dimension>(a, a_1, a_2, a_swapped);
        detail::segment_arrange<Dimension>(b, b_1, b_2, b_swapped);
        return math::smaller(a_2, b_1) || math::larger(a_1, b_2);
    }
#endif

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

        typedef boost::rational<boost::long_long_type> ratio_type;
        ratio_type const ra_from(oa_1 - ob_1, length_b);
        ratio_type const ra_to(oa_2 - ob_1, length_b);
        ratio_type const rb_from(ob_1 - oa_1, length_a);
        ratio_type const rb_to(ob_2 - oa_1, length_a);

        static ratio_type const zero(0, 1);
        static ratio_type const one(1, 1);
        bool const sanity_check_b_disjoint = ((rb_from < zero && rb_to < zero) || (rb_from > one && rb_to > one));
        if ((ra_from < zero && ra_to < zero) || (ra_from > one && ra_to > one))
        {
            assert(sanity_check_b_disjoint); // this will go
            return Policy::disjoint();
        }
        assert(! sanity_check_b_disjoint);

        return Policy::segments_collinear(a, b, ra_from, ra_to, rb_from, rb_to);
    }
};


}} // namespace strategy::intersection

}} // namespace boost::geometry


#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_INTERSECTION_HPP
