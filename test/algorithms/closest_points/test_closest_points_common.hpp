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



//========================================================================


template <typename Point, typename Geometry1, typename Geometry2,
          typename Strategy>
void test_empty_input(Geometry1 const& geometry1,
                      Geometry2 const& geometry2,
                      Strategy const& strategy)
{
    bg::model::segment<Point> out_seg;
/*
    try
    {
        bg::closest_points(geometry1, geometry2, out_seg);
    }
    catch(bg::empty_input_exception const& )
    {
        return;
    }
    BOOST_CHECK_MESSAGE(false,
                        "A empty_input_exception should have been thrown");

    try
    {
        bg::closest_points(geometry2, geometry1, out_seg);
    }
    catch(bg::empty_input_exception const& )
    {
        return;
    }
    BOOST_CHECK_MESSAGE(false,
                        "A empty_input_exception should have been thrown");
*/
    try
    {
        bg::closest_points(geometry1, geometry2, out_seg, strategy);
    }
    catch(bg::empty_input_exception const& )
    {
        return;
    }
    BOOST_CHECK_MESSAGE(false,
                        "A empty_input_exception should have been thrown");

    try
    {
        bg::closest_points(geometry2, geometry1, out_seg, strategy);
    }
    catch(bg::empty_input_exception const& )
    {
        return;
    }
    BOOST_CHECK_MESSAGE(false,
                        "A empty_input_exception should have been thrown");
}

template <typename Point, typename Strategy>
void test_more_empty_input_pointlike_linear(Strategy const& strategy)
{
#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "testing on empty inputs... " << std::flush;
#endif
    bg::model::linestring<Point> line_empty;
    bg::model::multi_point<Point> multipoint_empty;
    bg::model::multi_linestring<bg::model::linestring<Point> > multiline_empty;

    Point point = from_wkt<Point>("POINT(0 0)");
    bg::model::linestring<Point> line =
        from_wkt<bg::model::linestring<Point> >("LINESTRING(0 0,1 1,2 2)");

    // 1st geometry is empty
    //test_empty_input<Point>(multipoint_empty, line, strategy);

    // 2nd geometry is empty
    //test_empty_input<Point>(line, multipoint_empty, strategy);
    test_empty_input<Point>(point, line_empty, strategy);
    //test_empty_input<Point>(point, multiline_empty, strategy);

    // both geometries are empty
    //test_empty_input<Point>(multipoint_empty, line_empty, strategy);
    //test_empty_input<Point>(multipoint_empty, multiline_empty, strategy);

#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << "done!" << std::endl;
#endif
}

template <typename Point, typename Strategy>
void test_more_empty_input_linear_linear(Strategy const& strategy)
{
#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << std::endl;
    std::cout << "testing on empty inputs... " << std::flush;
#endif
    bg::model::linestring<Point> line_empty;
    bg::model::multi_linestring<bg::model::linestring<Point> > multiline_empty;

    bg::model::linestring<Point> line =
        from_wkt<bg::model::linestring<Point> >("LINESTRING(0 0,1 1)");
/*
    // 1st geometry is empty
    test_empty_input(line_empty, line, strategy);
    test_empty_input(multiline_empty, line, strategy);

    // 2nd geometry is empty
    test_empty_input(line, line_empty, strategy);
    test_empty_input(line, multiline_empty, strategy);

    // both geometries are empty
    test_empty_input(line_empty, line_empty, strategy);
    test_empty_input(multiline_empty, line_empty, strategy);
    test_empty_input(multiline_empty, multiline_empty, strategy);
*/
#ifdef BOOST_GEOMETRY_TEST_DEBUG
    std::cout << "done!" << std::endl;
#endif
}

#endif // BOOST_GEOMETRY_TEST_CLOSEST_POINTS_COMMON_HPP
