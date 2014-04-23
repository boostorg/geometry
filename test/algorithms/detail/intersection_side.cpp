// Boost.Geometry (aka GGL, Generic Geometry Library)
//
// Copyright (c) 2007-2014 Barend Gehrels, Amsterdam, the Netherlands.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_GEOMETRY_DEFINE_STREAM_OPERATOR_SEGMENT_RATIO

#include <geometry_test_common.hpp>

#include <boost/geometry/geometries/point_xy.hpp>

#include <boost/geometry/strategies/cartesian/cart_intersect.hpp>
#include <boost/geometry/strategies/intersection_result.hpp>
#include <boost/geometry/policies/relate/intersection_points.hpp>
#include <boost/geometry/policies/relate/intersection_ratios.hpp>
#include <boost/geometry/policies/robustness/no_rescale_policy.hpp>
#include <boost/geometry/algorithms/detail/intersection_side.hpp>
#include <boost/geometry/algorithms/make.hpp>

#if defined(TEST_WITH_SVG)
# include <boost/geometry/io/svg/svg_mapper.hpp>
#endif


template <typename Point, typename T>
void test_one(std::string const& caseid,
                T pi_x, T pi_y, T pj_x, T pj_y, // line p to intersect
                T qi_x, T qi_y, T qj_x, T qj_y, // line q to intersect
                T si_x, T si_y, T sj_x, T sj_y, // subject line to find side on of IP p/q
                int expected_side)
{
    typedef bg::detail::no_rescale_policy robust_policy_type;
    robust_policy_type robust_policy;

    Point pi = bg::make<Point>(pi_x, pi_y);
    Point pj = bg::make<Point>(pj_x, pj_y);
    Point qi = bg::make<Point>(qi_x, qi_y);
    Point qj = bg::make<Point>(qj_x, qj_y);
    Point si = bg::make<Point>(si_x, si_y);
    Point sj = bg::make<Point>(sj_x, sj_y);

    typedef bg::model::referring_segment<const Point> segment_type;

    segment_type p(pi, pj);
    segment_type q(qi, qj);
    segment_type s(si, sj);

    int detected_side = bg::detail::intersection_side::intersection_side(pi, pj,
                            qi, qj, si, sj, robust_policy);

    BOOST_CHECK_EQUAL(expected_side, detected_side);

#if defined(TEST_WITH_SVG)
    {
        std::ostringstream filename;
        filename << "intersection_side_" << caseid
            << "_" << string_from_type<typename bg::coordinate_type<Point>::type>::name()
            << ".svg";

        std::ofstream svg(filename.str().c_str());

        bg::svg_mapper<Point> mapper(svg, 500, 500);
        mapper.add(p);
        mapper.add(q);
        mapper.add(s);

        mapper.map(p, "opacity:0.7;stroke:rgb(0,192,0);stroke-width:3");
        mapper.map(q, "opacity:0.7;stroke:rgb(0,0,255);stroke-width:3");
        mapper.map(s, "opacity:0.7;stroke:rgb(255,0,0);stroke-width:2");

        std::string style =  ";font-family='Verdana';font-weight:bold";
        std::string align = ";text-anchor:end;text-align:end";
        int offset = 8;

        mapper.text(pi, "pi", "fill:rgb(0,192,0)" + style, offset, offset);
        mapper.text(pj, "pj", "fill:rgb(0,192,0)" + style, offset, offset);

        mapper.text(qi, "qi", "fill:rgb(0,0,255)" + style + align, -offset, offset);
        mapper.text(qj, "qj", "fill:rgb(0,0,255)" + style + align, -offset, offset);

        mapper.text(si, "si", "fill:rgb(255,0,0)" + style + align, -offset, offset);
        mapper.text(sj, "sj", "fill:rgb(255,0,0)" + style + align, -offset, offset);

        // Map the intersection point on the SVG
        {
            typedef typename bg::segment_ratio_type
            <
                Point,
                robust_policy_type
            >::type segment_ratio_type;

            // Get the intersection point (or two points)
            bg::segment_intersection_points<Point, segment_ratio_type> is
                = bg::strategy::intersection::relate_cartesian_segments
                <
                    bg::policies::relate::segments_intersection_points
                        <
                            bg::segment_intersection_points<Point, segment_ratio_type>
                        >
                >::apply(p, q, robust_policy);

            bg::fraction_type<segment_ratio_type> rs
                = bg::strategy::intersection::relate_cartesian_segments
                <
                    bg::policies::relate::segments_intersection_ratios
                        <
                            bg::fraction_type<segment_ratio_type>
                        >
                >::apply(p, q, robust_policy);

            for (int i = 0; i < is.count; i++)
            {
                mapper.map(is.intersections[i], "opacity:0.8;stroke:rgb(255,128,0);stroke-width:3", 3);
                std::ostringstream out;
                out << detected_side << " " << is.fractions[i].robust_ra << " / " << is.fractions[i].robust_rb
                    << std::endl
                    << rs.robust_ra << " / " << rs.robust_rb;
                mapper.text(is.intersections[i], out.str(), "fill:rgb(255,128,0)" + style + align, -2 * offset, offset);
            }
        }
    }

#endif

}


template <typename P>
void test_all()
{
    test_one<P, double>("simplex_left",
            0, 0,   4, 4, // p
            -1, 4,   3, 0, // q
            2, 0,   2, 4, // subject
            1);
    test_one<P, double>("simplex_right",
            0, 0,   4, 4, // p
            1, 4,   5, 0, // q
            2, 0,   2, 4, // subject
            -1);
    test_one<P, double>("simplex_collinear",
            0, 0,   4, 4, // p
            0, 4,   4, 0, // q
            2, 0,   2, 4, // subject
            0);

    test_one<P, double>("p_left",
            0, 0,   0, 4, // p
            1, 4,   5, 0, // q
            2, 0,   2, 4, // subject
            1);

    test_one<P, double>("p_right",
            4, 0,   4, 4, // p
            1, 4,   5, 0, // q
            2, 0,   2, 4, // subject
            -1);
    test_one<P, double>("p_left",
            1, 0,   1, 4, // p
            1, 4,   5, 0, // q
            2, 0,   2, 4, // subject
            1);

    test_one<P, double>("p_collinear",
            2, -1,   2, 5, // p
            1, 4,   5, 0, // q
            2, 0,   2, 4, // subject
            0);

    test_one<P, double>("q_left",
            0, 0,   4, 4, // p
            1, 4,   1, 0, // q
            2, 0,   2, 4, // subject
            1);

    test_one<P, double>("q_right",
            0, 0,   4, 4, // p
            3, 4,   3, 0, // q
            2, 0,   2, 4, // subject
            -1);

    test_one<P, double>("q_collinear",
            0, 0,   4, 4, // p
            2, 1,   2, 3, // q
            2, 0,   2, 4, // subject
            0);
}


int test_main( int , char* [] )
{
    test_all<bg::model::d2::point_xy<double> >();

    return 0;
}
