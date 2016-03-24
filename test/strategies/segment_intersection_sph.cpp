// Boost.Geometry
// Unit Test

// Copyright (c) 2016, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <geometry_test_common.hpp>

#include <boost/geometry/algorithms/equals.hpp>

#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/segment.hpp>

#include <boost/geometry/io/wkt/read.hpp>
#include <boost/geometry/io/wkt/write.hpp>

#include <boost/geometry/policies/relate/direction.hpp>
#include <boost/geometry/policies/relate/intersection_points.hpp>
#include <boost/geometry/policies/relate/tupled.hpp>

#include <boost/geometry/strategies/spherical/intersection.hpp>

template <typename P1, typename P2>
bool equals_pp(P1 const& p1, P2 const& p2)
{
    typedef typename bg::select_coordinate_type<P1, P2>::type calc_t;
    calc_t p10 = bg::get<0>(p1);
    calc_t p11 = bg::get<1>(p1);
    calc_t p20 = bg::get<0>(p2);
    calc_t p21 = bg::get<1>(p2);
    calc_t relaxed_eps = std::numeric_limits<calc_t>::epsilon() * 100000;
    return bg::math::abs(p10 - p20) <= relaxed_eps && bg::math::abs(p11 - p21) <= relaxed_eps;
}

template <typename S1, typename S2, typename P>
void test_spherical_strategy_one(S1 const& s1, S2 const& s2, char m, std::size_t expected_count, P const& ip0 = P(), P const& ip1 = P())
{
    typedef typename bg::coordinate_type<P>::type coord_t;
    typedef bg::policies::relate::segments_tupled
                <
                    bg::policies::relate::segments_intersection_points
                        <
                            bg::segment_intersection_points<P, bg::segment_ratio<coord_t> >
                        >,
                    bg::policies::relate::segments_direction
                > policy_t;
    typedef bg::strategy::intersection::relate_spherical_segments
        <
            policy_t
        > strategy_t;
    typedef typename strategy_t::return_type return_type;

    // NOTE: robust policy is currently ignored
    return_type res = strategy_t::apply(s1, s2, 0);

    size_t const res_count = boost::get<0>(res).count;
    char const res_method = boost::get<1>(res).how;

    BOOST_CHECK_MESSAGE(res_method == m,
                        "IP method: " << res_method << " different than expected: " << m
                            << " for " << bg::wkt(s1) << " and " << bg::wkt(s2));

    BOOST_CHECK_MESSAGE(res_count == expected_count,
                        "IP count: " << res_count << " different than expected: " << expected_count
                            << " for " << bg::wkt(s1) << " and " << bg::wkt(s2));

    if (res_count > 0 && expected_count > 0)
    {
        P const& res_i0 = boost::get<0>(res).intersections[0];
        BOOST_CHECK_MESSAGE(equals_pp(res_i0, ip0),
                            "IP0: " << bg::wkt(res_i0) << " different than expected: " << bg::wkt(ip0)
                                << " for " << bg::wkt(s1) << " and " << bg::wkt(s1));
    }
    if (res_count > 1 && expected_count > 1)
    {
        P const& res_i1 = boost::get<0>(res).intersections[1];
        BOOST_CHECK_MESSAGE(equals_pp(res_i1, ip1),
                            "IP1: " << bg::wkt(res_i1) << " different than expected: " << bg::wkt(ip1)
                                << " for " << bg::wkt(s1) << " and " << bg::wkt(s1));
    }
}

template <typename T>
T translated(T v, double t)
{
    v += T(t);
    if (v > 180)
        v -= 360;
    return v;
}

template <typename S1, typename S2, typename P>
void test_spherical_strategy(S1 const& s1, S2 const& s2, char m, std::size_t expected_count, P const& ip0 = P(), P const& ip1 = P())
{
    S1 s1t = s1;
    S2 s2t = s2;
    P ip0t = ip0;
    P ip1t = ip1;

    double t = 0;
    for (int i = 0; i < 4; ++i, t += 90.0)
    {
        bg::set<0, 0>(s1t, translated(bg::get<0, 0>(s1), t));
        bg::set<1, 0>(s1t, translated(bg::get<1, 0>(s1), t));
        bg::set<0, 0>(s2t, translated(bg::get<0, 0>(s2), t));
        bg::set<1, 0>(s2t, translated(bg::get<1, 0>(s2), t));
        if (expected_count > 0)
            bg::set<0>(ip0t, translated(bg::get<0>(ip0), t));
        if (expected_count > 1)
            bg::set<0>(ip1t, translated(bg::get<0>(ip1), t));

        test_spherical_strategy_one(s1t, s2t, m, expected_count, ip0t, ip1t);
    }
}

template <typename S1, typename S2, typename P>
void test_spherical_strategy(std::string const& s1_wkt, std::string const& s2_wkt,
                             char m, std::size_t expected_count,
                             std::string const& ip0_wkt = "", std::string const& ip1_wkt = "")
{
    S1 s1;
    S2 s2;
    P ip0, ip1;

    bg::read_wkt(s1_wkt, s1);
    bg::read_wkt(s2_wkt, s2);
    if (! ip0_wkt.empty())
        bg::read_wkt(ip0_wkt, ip0);
    if (!ip1_wkt.empty())
        bg::read_wkt(ip1_wkt, ip1);

    test_spherical_strategy(s1, s2, m, expected_count, ip0, ip1);
}

template <typename S, typename P>
void test_spherical_strategy(std::string const& s1_wkt, std::string const& s2_wkt,
                             char m, std::size_t expected_count,
                             std::string const& ip0_wkt = "", std::string const& ip1_wkt = "")
{
    test_spherical_strategy<S, S, P>(s1_wkt, s2_wkt, m, expected_count, ip0_wkt, ip1_wkt);
}

template <typename T>
void test_spherical()
{
    typedef bg::model::point<double, 2, bg::cs::spherical_equatorial<bg::degree> > point_t;
    typedef bg::model::segment<point_t> segment_t;

    // crossing
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-45 -45, 45 45)", "SEGMENT(-45 45, 45 -45)", 'i', 1, "POINT(0 0)");
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-45 -45, 45 45)", "SEGMENT(45 -45, -45 45)", 'i', 1, "POINT(0 0)");
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(45 45, -45 -45)", "SEGMENT(-45 45, 45 -45)", 'i', 1, "POINT(0 0)");
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(45 45, -45 -45)", "SEGMENT(45 -45, -45 45)", 'i', 1, "POINT(0 0)");
    // crossing
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-1 1, 1 -1)", 'i', 1, "POINT(0 0)");
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(1 -1, -1 1)", 'i', 1, "POINT(0 0)");
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(1 1, -1 -1)", "SEGMENT(-1 1, 1 -1)", 'i', 1, "POINT(0 0)");
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(1 1, -1 -1)", "SEGMENT(1 -1, -1 1)", 'i', 1, "POINT(0 0)");

    // collinear, equal
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-45 -45, 45 45)", "SEGMENT(-45 -45, 45 45)", 'e', 2, "POINT(-45 -45)", "POINT(45 45)");
    // collinear, equal, opposite
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-45 -45, 45 45)", "SEGMENT(45 45, -45 -45)", 'e', 2, "POINT(-45 -45)", "POINT(45 45)");

    // starting outside
    // great circles crossing
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-2 -2, -1 -1)", 'a', 1, "POINT(-1 -1)"); // touching at the end of S2
    // great circles crossing
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-2 -2, 0 0)", 'm', 1, "POINT(0 0)"); // touching at the end of S2
    // great circles crossing
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-2 -2, 1 1)", 't', 1, "POINT(1 1)"); // touching at the end of S2
    // great circles crossing
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-2 -2, 2 2)", 'i', 1, "POINT(0 0)"); // intersecting in the middle of both
    // similar to above, all collinear
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-2 0, -1 0)", 'a', 1, "POINT(-1 0)");
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-2 0, 0 0)", 'c', 2, "POINT(-1 0)", "POINT(0 0)");
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-2 0, 1 0)", 'c', 2, "POINT(-1 0)", "POINT(1 0)");
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-2 0, 2 0)", 'c', 2, "POINT(-1 0)", "POINT(1 0)");

    // starting at p1
    // collinear
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-1 -1, 0 0)", 'c', 2, "POINT(-1 -1)", "POINT(0 0)");
    // collinear
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-1 -1, 1 1)", 'e', 2, "POINT(-1 -1)", "POINT(1 1)");
    // great circles crossing
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-1 -1, 2 2)", 'f', 1, "POINT(-1 -1)"); // touching at the beginning of S2
    // similar to above, all collinear
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-1 0, 0 0)", 'c', 2, "POINT(-1 0)", "POINT(0 0)");
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-1 0, 1 0)", 'e', 2, "POINT(-1 0)", "POINT(1 0)");
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-1 0, 2 0)", 'c', 2, "POINT(-1 0)", "POINT(1 0)");
    
    // starting inside
    // collinear
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(0 0, 1 1)", 'c', 2, "POINT(0 0)", "POINT(1 1)");
    // great circles crossing
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(0 0, 2 2)", 's', 1, "POINT(0 0)"); // touching at the beginning of S2
    // similar to above, all collinear
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(0 0, 1 0)", 'c', 2, "POINT(0 0)", "POINT(1 0)");
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(0 0, 2 0)", 'c', 2, "POINT(0 0)", "POINT(1 0)");
    
    // starting at p2
    // crossing
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(1 1, 2 2)", 'a', 1, "POINT(1 1)"); // touching at the beginning of S2
    // similar to above, collinear
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(1 0, 2 0)", 'a', 1, "POINT(1 0)");

    // great circles crossing, disjoint
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-3 -3, -2 -2)", 'd', 0);
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(2 2, 3 3)", 'd', 0);
    // similar to above, collinear
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-3 0, -2 0)", 'd', 0);
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(2 0, 3 0)", 'd', 0);

    // crossing, disjoint
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-2 -2, -2 -2)", 'd', 0);
    // collinear, degenerated
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-1 -1, -1 -1)", '0', 1, "POINT(-1 -1)");
    // collinear, degenerated
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(0 0, 0 0)", '0', 1, "POINT(0 0)");
    // collinear, degenerated
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(1 1, 1 1)", '0', 1, "POINT(1 1)");
    // crossing, disjoint
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(2 2, 2 2)", 'd', 0);
    // similar to above, collinear
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-2 0, -2 0)", 'd', 0);
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-1 0, -1 0)", '0', 1, "POINT(-1 0)");
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(0 0, 0 0)", '0', 1, "POINT(0 0)");
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(1 0, 1 0)", '0', 1, "POINT(1 0)");
    test_spherical_strategy<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(2 0, 2 0)", 'd', 0);
}

int test_main(int, char* [])
{
    test_spherical<float>();
    test_spherical<double>();

    return 0;
}
