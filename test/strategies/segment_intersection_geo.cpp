// Boost.Geometry
// Unit Test

// Copyright (c) 2016, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include "segment_intersection_sph.hpp"


#include <boost/geometry/strategies/geographic/intersection.hpp>


template <typename S, typename P>
void test_default_strategy(std::string const& s1_wkt, std::string const& s2_wkt,
                           char m, std::size_t expected_count,
                           std::string const& ip0_wkt = "", std::string const& ip1_wkt = "")
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

    typename bg::strategy::intersection::services::default_strategy
        <
            bg::geographic_tag,
            policy_t,
            void
        >::type strategy;

    test_strategy<S, S, P>(s1_wkt, s2_wkt, strategy, m, expected_count, ip0_wkt, ip1_wkt);
}

template <typename S, typename P>
void test_great_elliptic_strategy(std::string const& s1_wkt, std::string const& s2_wkt,
                                  char m, std::size_t expected_count,
                                  std::string const& ip0_wkt = "", std::string const& ip1_wkt = "")
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

    bg::strategy::intersection::relate_great_elliptic_segments
        <
            policy_t
        > strategy;

    test_strategy<S, S, P>(s1_wkt, s2_wkt, strategy, m, expected_count, ip0_wkt, ip1_wkt);
}

template <typename S, typename P>
void test_experimental_elliptic_strategy(std::string const& s1_wkt, std::string const& s2_wkt,
                                         char m, std::size_t expected_count,
                                         std::string const& ip0_wkt = "", std::string const& ip1_wkt = "")
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

    bg::strategy::intersection::relate_experimental_elliptic_segments
        <
            policy_t
        > strategy;

    test_strategy<S, S, P>(s1_wkt, s2_wkt, strategy, m, expected_count, ip0_wkt, ip1_wkt);
}

template <typename S, typename P>
void test_geographic_strategies(std::string const& s1_wkt, std::string const& s2_wkt,
                                char m, std::size_t expected_count,
                                std::string const& ip0_wkt = "", std::string const& ip1_wkt = "")
{
    test_default_strategy<S, P>(s1_wkt, s2_wkt, m, expected_count, ip0_wkt, ip1_wkt);
    test_great_elliptic_strategy<S, P>(s1_wkt, s2_wkt, m, expected_count, ip0_wkt, ip1_wkt);
    test_experimental_elliptic_strategy<S, P>(s1_wkt, s2_wkt, m, expected_count, ip0_wkt, ip1_wkt);
}

template <typename T>
void test_geographic()
{
    typedef bg::model::point<double, 2, bg::cs::geographic<bg::degree> > point_t;
    typedef bg::model::segment<point_t> segment_t;

    // crossing   X
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-45 -45, 45 45)", "SEGMENT(-45 45, 45 -45)", 'i', 1, "POINT(0 0)");
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-45 -45, 45 45)", "SEGMENT(45 -45, -45 45)", 'i', 1, "POINT(0 0)");
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(45 45, -45 -45)", "SEGMENT(-45 45, 45 -45)", 'i', 1, "POINT(0 0)");
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(45 45, -45 -45)", "SEGMENT(45 -45, -45 45)", 'i', 1, "POINT(0 0)");
    // crossing   X
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-1 1, 1 -1)", 'i', 1, "POINT(0 0)");
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(1 -1, -1 1)", 'i', 1, "POINT(0 0)");
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(1 1, -1 -1)", "SEGMENT(-1 1, 1 -1)", 'i', 1, "POINT(0 0)");
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(1 1, -1 -1)", "SEGMENT(1 -1, -1 1)", 'i', 1, "POINT(0 0)");

    // equal
    //   //
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-45 -45, 45 45)", "SEGMENT(-45 -45, 45 45)", 'e', 2, "POINT(-45 -45)", "POINT(45 45)");
    //   //
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-45 -45, 45 45)", "SEGMENT(45 45, -45 -45)", 'e', 2, "POINT(-45 -45)", "POINT(45 45)");

    // starting outside s1
    //    /
    //   |
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-2 -2, -1 -1)", 'a', 1, "POINT(-1 -1)");
    //   /
    //  /|
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-2 -2, 0 0)", 'm', 1, "POINT(0 0)");
    //   /|
    //  / |
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-2 -2, 1 1)", 't', 1, "POINT(1 1)");
    //   |/
    //  /|
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-2 -2, 2 2)", 'i', 1, "POINT(0 0)");
    //       ------
    // ------
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-2 0, -1 0)", 'a', 1, "POINT(-1 0)");
    //    ------
    // ------
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-2 0, 0 0)", 'c', 2, "POINT(-1 0)", "POINT(0 0)");
    //    ------
    // ---------
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-2 0, 1 0)", 'c', 2, "POINT(-1 0)", "POINT(1 0)");
    //    ------
    // ------------
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-2 0, 2 0)", 'c', 2, "POINT(-1 0)", "POINT(1 0)");

    // starting at s1
    //  /
    // //
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-1 -1, 0 0)", 'c', 2, "POINT(-1 -1)", "POINT(0 0)");
    //  //
    // //
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-1 -1, 1 1)", 'e', 2, "POINT(-1 -1)", "POINT(1 1)");
    // | /
    // |/
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-1 -1, 2 2)", 'f', 1, "POINT(-1 -1)");
    // ------
    // ---
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-1 0, 0 0)", 'c', 2, "POINT(-1 0)", "POINT(0 0)");
    // ------
    // ------
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-1 0, 1 0)", 'e', 2, "POINT(-1 0)", "POINT(1 0)");
    // ------
    // ---------
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-1 0, 2 0)", 'c', 2, "POINT(-1 0)", "POINT(1 0)");
    
    // starting inside
    //   //
    //  /
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(0 0, 1 1)", 'c', 2, "POINT(0 0)", "POINT(1 1)");
    //   |/
    //   /
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(0 0, 2 2)", 's', 1, "POINT(0 0)");
    // ------
    //    ---
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(0 0, 1 0)", 'c', 2, "POINT(0 0)", "POINT(1 0)");
    // ------
    //    ------
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(0 0, 2 0)", 'c', 2, "POINT(0 0)", "POINT(1 0)");
    
    // starting at p2
    //    |
    //   /
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(1 1, 2 2)", 'a', 1, "POINT(1 1)");
    // ------
    //       ---
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(1 0, 2 0)", 'a', 1, "POINT(1 0)");

    // disjoint, crossing
    //     /
    //  |
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-3 -3, -2 -2)", 'd', 0);
    //     |
    //  /
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(2 2, 3 3)", 'd', 0);
    // disjoint, collinear
    //          ------
    // ------
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-3 0, -2 0)", 'd', 0);
    // ------
    //           ------
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(2 0, 3 0)", 'd', 0);

    // degenerated
    //    /
    // *
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-2 -2, -2 -2)", 'd', 0);
    //    /
    //   *
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(-1 -1, -1 -1)", '0', 1, "POINT(-1 -1)");
    //    /
    //   *
    //  /
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(0 0, 0 0)", '0', 1, "POINT(0 0)");
    //    *
    //   /
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(1 1, 1 1)", '0', 1, "POINT(1 1)");
    //       *
    //   /
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 -1, 1 1)", "SEGMENT(2 2, 2 2)", 'd', 0);
    // similar to above, collinear
    // *   ------
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-2 0, -2 0)", 'd', 0);
    //    *------
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(-1 0, -1 0)", '0', 1, "POINT(-1 0)");
    //    ---*---
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(0 0, 0 0)", '0', 1, "POINT(0 0)");
    //    ------*
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(1 0, 1 0)", '0', 1, "POINT(1 0)");
    //    ------   *
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 0, 1 0)", "SEGMENT(2 0, 2 0)", 'd', 0);

    // Northern hemisphere
    // ---   ------
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(-3 50, -2 50)", 'd', 0);
    //    ------
    // ---
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(-2 50, -1 50)", 'a', 1, "POINT(-1 50)");
    //  \/
    //  /\                   (avoid multi-line comment)
    test_default_strategy<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(-2 50, 0 50)", 'i', 1, "POINT(-0.5 50.0032229484023)");
    test_great_elliptic_strategy<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(-2 50, 0 50)", 'i', 1, "POINT(-0.5 50.0032229484023)");
    test_experimental_elliptic_strategy<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(-2 50, 0 50)", 'i', 1, "POINT(-0.5 50.0032319221120)");
    //  ________
    // /   _____\            (avoid multi-line comment)
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(-2 50, 1 50)", 't', 1, "POINT(1 50)");
    //  _________
    // /  _____  \           (avoid multi-line comment)
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(-2 50, 2 50)", 'd', 0);
    //  ______
    // /___   \              (avoid multi-line comment)
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(-1 50, 0 50)", 'f', 1, "POINT(-1 50)");
    // ------
    // ------
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(-1 50, 1 50)", 'e', 2, "POINT(-1 50)", "POINT(1 50)");
    //  ________
    // /_____   \            (avoid multi-line comment)
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(-1 50, 2 50)", 'f', 1, "POINT(-1 50)");
    //  ______
    // /   ___\              (avoid multi-line comment)
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(0 50, 1 50)", 't', 1, "POINT(1 50)");
    //  \/
    //  /\                   (avoid multi-line comment)
    test_default_strategy<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(0 50, 2 50)", 'i', 1, "POINT(0.5 50.0032229484023)");
    test_great_elliptic_strategy<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(0 50, 2 50)", 'i', 1, "POINT(0.5 50.0032229484023)");
    test_experimental_elliptic_strategy<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(0 50, 2 50)", 'i', 1, "POINT(0.5 50.0032319221120)");
    // ------
    //       ---
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(1 50, 2 50)", 'a', 1, "POINT(1 50)");
    // ------   ---
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(-1 50, 1 50)", "SEGMENT(2 50, 3 50)", 'd', 0);

    // ___|
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(0 0, 1 0)", "SEGMENT(1 0, 1 1)", 'a', 1, "POINT(1 0)");
    // ___|
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(1 0, 1 1)", "SEGMENT(0 0, 1 0)", 'a', 1, "POINT(1 0)");

    //   |/
    //  /|
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(10 -1, 20 1)", "SEGMENT(12.5 -1, 12.5 1)", 'i', 1, "POINT(12.5 -0.50051443471392)");
    //   |/
    //  /|
    test_geographic_strategies<segment_t, point_t>(
        "SEGMENT(10 -1, 20 1)", "SEGMENT(17.5 -1, 17.5 1)", 'i', 1, "POINT(17.5 0.50051443471392)");
}

int test_main(int, char* [])
{
    //test_geographic<float>();
    test_geographic<double>();

    return 0;
}
