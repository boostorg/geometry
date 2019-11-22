// Boost.Geometry
// Unit Test

// Copyright (c) 2019, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_GEOMETRY_TEST_CLOSEST_POINTS_COMMON_HPP
#define BOOST_GEOMETRY_TEST_CLOSEST_POINTS_COMMON_HPP

#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/algorithms/closest_points.hpp>
#include <boost/geometry/strategies/strategies.hpp>

#include <boost/test/included/unit_test.hpp>

#include <from_wkt.hpp>

namespace bg = boost::geometry;

//===========================================================================

// spherical strategies

// pt-pt
typedef bg::strategy::closest_points::spherical
                      <double> spherical_pp;

//===========================================================================

// geographic strategies

// pt-pt
typedef bg::strategy::closest_points::geographic
                      <bg::strategy::andoyer> andoyer_pp;
typedef bg::strategy::closest_points::geographic
                      <bg::strategy::thomas> thomas_pp;
typedef bg::strategy::closest_points::geographic
                      <bg::strategy::vincenty> vincenty_pp;

// pt-seg
typedef bg::strategy::closest_points::geographic_cross_track
                      <bg::strategy::andoyer> andoyer_cp;
typedef bg::strategy::closest_points::geographic_cross_track
                      <bg::strategy::thomas> thomas_cp;
typedef bg::strategy::closest_points::geographic_cross_track
                      <bg::strategy::vincenty> vincenty_cp;

//pt-box
typedef bg::strategy::closest_points::geographic_cross_track_point_box
                      <bg::strategy::andoyer> andoyer_pb;
typedef bg::strategy::closest_points::geographic_cross_track_point_box
                      <bg::strategy::thomas> thomas_pb;
typedef bg::strategy::closest_points::geographic_cross_track_point_box
                      <bg::strategy::vincenty> vincenty_pb;

//box-box
typedef bg::strategy::closest_points::geographic_cross_track_box_box
                      <bg::strategy::andoyer> andoyer_bb;
typedef bg::strategy::closest_points::geographic_cross_track_box_box
                      <bg::strategy::thomas> thomas_bb;
typedef bg::strategy::closest_points::geographic_cross_track_box_box
                      <bg::strategy::vincenty> vincenty_bb;

//segment-box
typedef bg::strategy::closest_points::geographic_segment_box
                      <bg::strategy::andoyer> andoyer_sb;
typedef bg::strategy::closest_points::geographic_segment_box
                      <bg::strategy::thomas> thomas_sb;
typedef bg::strategy::closest_points::geographic_segment_box
                      <bg::strategy::vincenty> vincenty_sb;

//===========================================================================

template <typename Segment>
static inline Segment swap(Segment const& s)
{
    Segment s_swaped;

    bg::set<0, 0>(s_swaped, bg::get<1, 0>(s));
    bg::set<0, 1>(s_swaped, bg::get<1, 1>(s));
    bg::set<1, 0>(s_swaped, bg::get<0, 0>(s));
    bg::set<1, 1>(s_swaped, bg::get<0, 1>(s));

    return s_swaped;
}

template <int i, int j, typename Segment>
void compare_result_with_expected(Segment const& expected_resulting_segment,
                                  Segment const& resulting_segment)
{
    double expected = bg::get<i, j>(expected_resulting_segment);
    double resulting = bg::get<i, j>(resulting_segment);
    BOOST_CHECK_CLOSE(expected, resulting, 0.01);
}

template <typename Segment>
void compare_result_with_expected(Segment const& exp_resulting_segment,
                                  Segment const& resulting_segment)
{
    compare_result_with_expected<0,0>(exp_resulting_segment, resulting_segment);
    compare_result_with_expected<1,0>(exp_resulting_segment, resulting_segment);
    compare_result_with_expected<0,1>(exp_resulting_segment, resulting_segment);
    compare_result_with_expected<1,1>(exp_resulting_segment, resulting_segment);
}

template
<
    typename Geometry1,
    typename Geometry2,
    typename Segment,
    typename Strategy
>
void test_closest_points(Geometry1 const& geometry1,
                         Geometry2 const& geometry2,
                         Segment const& exp_resulting_segment,
                         Strategy const& strategy)
{
#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << bg::wkt(geometry1) << " --- " << bg::wkt(geometry2)
              << std::endl;
#endif
    Segment resulting_segment;
    bg::closest_points(geometry1, geometry2, resulting_segment, strategy);

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << "closest_points : " << bg::wkt(resulting_segment)
              << std::endl << std::endl;
#endif

    compare_result_with_expected(exp_resulting_segment, resulting_segment);
}

template
<
    typename Geometry1,
    typename Geometry2,
    typename Segment
>
void test_closest_points(Geometry1 const& geometry1,
                         Geometry2 const& geometry2,
                         Segment const& exp_resulting_segment)
{
#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << bg::wkt(geometry1) << " --- " << bg::wkt(geometry2)
              << std::endl;
#endif
    Segment resulting_segment;
    bg::closest_points(geometry1, geometry2, resulting_segment);

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << "closest_points : " << bg::wkt(resulting_segment) << std::endl;
#endif

    compare_result_with_expected(exp_resulting_segment, resulting_segment);
}


template <typename Geometry1, typename Geometry2, typename OutputSegment>
struct test_geometry
{
    template <typename Strategy>
    inline static void apply(std::string const& wkt1,
                             std::string const& wkt2,
                             std::string const& wkt_expected_resulting_segment,
                             Strategy const& strategy,
                             bool swap_geometries = true)
    {
        Geometry1 geometry1;
        bg::read_wkt(wkt1, geometry1);
        Geometry2 geometry2;
        bg::read_wkt(wkt2, geometry2);

        OutputSegment expected_resulting_segment;
        bg::read_wkt(wkt_expected_resulting_segment,
                     expected_resulting_segment);

        test_closest_points(geometry1, geometry2, expected_resulting_segment,
                            strategy);

        if (swap_geometries)
        {
            // swap input geometries and expected segment
            test_closest_points(geometry2, geometry1,
                                swap(expected_resulting_segment),
                                strategy);
        }
    }

    inline static void apply(std::string const& wkt1,
                             std::string const& wkt2,
                             std::string const& wkt_expected_resulting_segment)
    {
        Geometry1 geometry1;
        bg::read_wkt(wkt1, geometry1);
        Geometry2 geometry2;
        bg::read_wkt(wkt2, geometry2);

        OutputSegment expected_resulting_segment;
        bg::read_wkt(wkt_expected_resulting_segment,
                     expected_resulting_segment);

        test_closest_points(geometry1, geometry2, expected_resulting_segment);
    }
};

#endif // BOOST_GEOMETRY_TEST_CLOSEST_POINTS_COMMON_HPP
