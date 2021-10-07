// Boost.Geometry
// Unit Test

// Copyright (c) 2021, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE test_algorithms_closest_points_pointlike_linear
#endif

#include "common.hpp"
//#include "test_empty_geometry.hpp"
#include <boost/geometry/algorithms/length.hpp>

namespace bg = boost::geometry;

//===========================================================================

template <typename Point, typename Strategies>
void test_closest_points_point_segment(Strategies const& strategies)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG_CLOSEST_POINTS
    std::cout << std::endl;
    std::cout << "point/segment closest_points tests" << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef test_geometry<Point, Segment, Segment> tester;

    tester::apply("POINT(1 1)",
                  "SEGMENT(0 1,1 0)",
                  "SEGMENT(1 1,0.5 0.5)",
                  "SEGMENT(1 1,0.499962 0.500095)",
                  "SEGMENT(1 1,0.503314 0.496737)",
                  strategies);

    tester::apply("POINT(0 1)",
                  "SEGMENT(0 1,1 0)",
                  "SEGMENT(0 1,0 1)",
                  strategies);

    //default strategies
    tester::apply("POINT(1 1)",
                  "SEGMENT(0 1,1 0)",
                  "SEGMENT(1 1,0.5 0.5)",
                  "SEGMENT(1 1,0.499962 0.500095)",
                  "SEGMENT(1 1,0.503314 0.496737)",
                  strategies, true, true);
}

//===========================================================================

template <typename Point, typename Strategies>
void test_closest_points_point_segment_diff_spheroid(Strategies const& strategies)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG_CLOSEST_POINTS
    std::cout << std::endl;
    std::cout << "point/segment closest_points tests non-default spheroid"
              << std::endl;
#endif

    typedef bg::model::segment<Point> Segment;
    typedef test_geometry<Point, Segment, Segment> tester;

    tester::apply("POINT(1 1)",
                  "SEGMENT(0 1,1 0)",
                  "SEGMENT(1 1,0.311318 0.673915)",
                  strategies);

    tester::apply("POINT(0 1)",
                  "SEGMENT(0 1,1 0)",
                  "SEGMENT(0 1,0 1)",
                  strategies);
}

//===========================================================================

template <typename Point, typename Strategies>
void test_closest_points_point_linestring(Strategies const& strategies)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG_CLOSEST_POINTS
    std::cout << std::endl;
    std::cout << "point/linestring closest_points tests" << std::endl;
#endif

    typedef bg::model::linestring<Point> Linestring;
    typedef bg::model::segment<Point> Segment;
    typedef test_geometry<Point, Linestring, Segment> tester;

    tester::apply("POINT(1 1)",
                  "LINESTRING(3 2, 1 3, 0 3, 0 1, 1 0, 0 0)",
                  "SEGMENT(1 1,0.5 0.5)",
                  "SEGMENT(1 1,0.499962 0.500095)",
                  "SEGMENT(1 1,0.503314 0.496737)",
                  strategies);

    tester::apply("POINT(3 2)",
                  "LINESTRING(3 2, 1 3, 0 3, 0 1, 1 0, 0 0)",
                  "SEGMENT(3 2,3 2)",
                  strategies);

    //default strategies
    tester::apply("POINT(3 2)",
                  "LINESTRING(3 2, 1 3, 0 3, 0 1, 1 0, 0 0)",
                  "SEGMENT(3 2,3 2)",
                  strategies, true, true);
}

//===========================================================================

template <typename Point, typename Strategies>
void test_closest_points_point_multi_linestring(Strategies const& strategies)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG_CLOSEST_POINTS
    std::cout << std::endl;
    std::cout << "point/multi_linestring closest_points tests" << std::endl;
#endif

    typedef bg::model::linestring<Point> Linestring;
    typedef bg::model::multi_linestring<Linestring> MultiLinestring;
    typedef bg::model::segment<Point> Segment;
    typedef test_geometry<Point, MultiLinestring, Segment> tester;

    tester::apply("POINT(1 1)",
                  "MULTILINESTRING((3 2, 1 3, 0 3, 0 1, 1 0, 0 0),(2 0,3 1,4 0))",
                  "SEGMENT(1 1,0.5 0.5)",
                  "SEGMENT(1 1,0.499962 0.500095)",
                  "SEGMENT(1 1,0.503314 0.496737)",
                  strategies);

    tester::apply("POINT(1 3)",
                  "MULTILINESTRING((3 2, 1 3, 0 3, 0 1, 1 0, 0 0),(2 0,3 1,4 0))",
                  "SEGMENT(1 3,1 3)",
                  strategies);

    //default strategies
    tester::apply("POINT(1 3)",
                  "MULTILINESTRING((3 2, 1 3, 0 3, 0 1, 1 0, 0 0),(2 0,3 1,4 0))",
                  "SEGMENT(1 3,1 3)",
                  strategies, true, true);
}

//===========================================================================

template <typename Point, typename Strategies>
void test_closest_points_multi_point_segment(Strategies const& strategies)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG_CLOSEST_POINTS
    std::cout << std::endl;
    std::cout << "multi point/segment closest_points tests" << std::endl;
#endif

    typedef bg::model::multi_point<Point> MultiPoint;
    typedef bg::model::segment<Point> Segment;
    typedef test_geometry<MultiPoint, Segment, Segment> tester;

    tester::apply("MULTIPOINT((2 0),(1 1),(1 2))",
                  "SEGMENT(0 1,1 0)",
                  "SEGMENT(1 1,0.5 0.5)",
                  "SEGMENT(1 1,0.499962 0.500095)",
                  "SEGMENT(1 1,0.503314 0.496737)",
                  strategies);

    tester::apply("MULTIPOINT((0 1),(1 1),(1 2))",
                  "SEGMENT(0 1,1 0)",
                  "SEGMENT(0 1,0 1)",
                  strategies);

    //default strategies
    tester::apply("MULTIPOINT((0 1),(1 1),(1 2))",
                  "SEGMENT(0 1,1 0)",
                  "SEGMENT(0 1,0 1)",
                  strategies, true, true);
}

//===========================================================================

template <typename Point, typename Strategies>
void test_closest_points_multi_point_linestring(Strategies const& strategies)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG_CLOSEST_POINTS
    std::cout << std::endl;
    std::cout << "multi point/linestring closest_points tests" << std::endl;
#endif

    typedef bg::model::multi_point<Point> MultiPoint;
    typedef bg::model::linestring<Point> Linestring;
    typedef bg::model::segment<Point> Segment;
    typedef test_geometry<MultiPoint, Linestring, Segment> tester;

    tester::apply("MULTIPOINT((2 0),(1 1),(3 0))",
                  "LINESTRING(4 2,3 2, 1 3, 0 3, 0 1, 1 0, 0 0)",
                  "SEGMENT(1 1,0.5 0.5)",
                  "SEGMENT(1 1,0.499962 0.500095)",
                  "SEGMENT(1 1,0.503314 0.496737)",
                  strategies);

    tester::apply("MULTIPOINT((3 2),(1 1),(3 0))",
                  "LINESTRING(4 2,3 2, 1 3, 0 3, 0 1, 1 0, 0 0)",
                  "SEGMENT(3 2,3 2)",
                  strategies);

    //default strategies
    tester::apply("MULTIPOINT((3 2),(1 1),(3 0))",
                  "LINESTRING(4 2,3 2, 1 3, 0 3, 0 1, 1 0, 0 0)",
                  "SEGMENT(3 2,3 2)",
                  strategies, true, true);
}

//===========================================================================

template <typename Point, typename Strategies>
void test_closest_points_multi_point_multi_linestring(Strategies const& strategies)
{

#ifdef BOOST_GEOMETRY_TEST_DEBUG_CLOSEST_POINTS
    std::cout << std::endl;
    std::cout << "multi point/multi linestring closest_points tests" << std::endl;
#endif

    typedef bg::model::multi_point<Point> MultiPoint;
    typedef bg::model::linestring<Point> Linestring;
    typedef bg::model::multi_linestring<Linestring> MultiLinestring;
    typedef bg::model::segment<Point> Segment;
    typedef test_geometry<MultiPoint, MultiLinestring, Segment> tester;

    tester::apply("MULTIPOINT((5 0),(1 1),(6 0))",
                  "MULTILINESTRING((3 2, 1 3, 0 3, 0 1, 1 0, 0 0),(2 0,3 1,4 0))",
                  "SEGMENT(1 1,0.5 0.5)",
                  "SEGMENT(1 1,0.499962 0.500095)",
                  "SEGMENT(1 1,0.503314 0.496737)",
                  strategies);

    tester::apply("MULTIPOINT((5 0),(1 3),(6 0))",
                  "MULTILINESTRING((3 2, 1 3, 0 3, 0 1, 1 0, 0 0),(2 0,3 1,4 0))",
                  "SEGMENT(1 3,1 3)",
                  strategies);

    //default strategies
    tester::apply("MULTIPOINT((5 0),(1 3),(6 0))",
                  "MULTILINESTRING((3 2, 1 3, 0 3, 0 1, 1 0, 0 0),(2 0,3 1,4 0))",
                  "SEGMENT(1 3,1 3)",
                  strategies, true, true);
}


//===========================================================================
//===========================================================================
//===========================================================================

template <typename Point, typename Strategies>
void test_all_pl_l(Strategies strategies)
{
    test_closest_points_point_segment<Point>(strategies);
    test_closest_points_point_linestring<Point>(strategies);
    test_closest_points_point_multi_linestring<Point>(strategies);

    //test_closest_points_multi_point_segment<Point>(ps_strategy);
    //test_closest_points_multi_point_linestring<Point>(ps_strategy);
    //test_closest_points_multi_point_multi_linestring<Point>(ps_strategy);

    //test_more_empty_input_pointlike_linear<Point>(ps_strategy);
}

BOOST_AUTO_TEST_CASE( test_all_pointlike_linear )
{
    test_all_pl_l<car_point>(cartesian());

    //test_all_pl_l<sph_point>(spherical_ps());
    //test_all_pl_l<sph_point>(spherical_ps(bg::formula::mean_radius
    //                                      <double>(bg::srs::spheroid<double>())));

    //test_all_pl_l<geo_point>(andoyer_ps());
    //test_all_pl_l<geo_point>(thomas_ps());
    //test_all_pl_l<geo_point>(vincenty_ps());

    //typedef bg::srs::spheroid<double> stype;

    //test_closest_points_point_segment_diff_spheroid<geo_point>
    //        (andoyer_ps(stype(5000000,6000000)));
}

/*

// tests from https://github.com/boostorg/geometry/pull/707#issuecomment-786650747

template <typename CT>
void closest_path_tester(std::string point_wkt,
                         std::string segment_wkt,
                         double error)
{
    using point_type = bg::model::point
        <
            CT,
            2,
            bg::cs::geographic<bg::degree>
        >;
    using segment_type = bg::model::segment<point_type>;

    bg::strategies::closest_points::geographic_cross_track<>
        closest_point_strategy;
    bg::strategies::distance::geographic_cross_track<>
        cross_track_strategy;
    bg::strategies::distance::geographic<> distance_strategy;

    point_type point;
    segment_type segment;

    bg::read_wkt(point_wkt, point);
    bg::read_wkt(segment_wkt, segment);

    const auto distance = bg::distance(point, segment, cross_track_strategy);

    segment_type projection;
    bg::closest_points(point, segment, projection, closest_point_strategy);

    auto p0 = point_type(bg::get<0,0>(projection), bg::get<0,1>(projection));
    auto p1 = point_type(bg::get<1,0>(projection), bg::get<1,1>(projection));

    const auto dist1 = bg::distance(p0, p1);//bg::length(projection);
    const auto dist2 = bg::distance(p0, segment);//should be equal to dist1
    const auto dist3 = bg::distance(p1, segment);//should be 0


#ifdef BOOST_GEOMETRY_TEST_DEBUG_CLOSEST_POINTS_CLOSEST_POINTS
    std::cout << std::setprecision(30)
              << "Point: " << bg::wkt(point) << "\n"
              << "Segment: " << bg::wkt(segment) << "\n"
              << "Distance: " << distance << "\n"
              << "Projection: " << bg::wkt(projection) << "\n"
              << "Distance 1: " << dist1 << "\n"
              << "Distance 2: " << dist2 << "\n"
              << "Distance 3: " << dist3 << std::endl;
#endif

    BOOST_CHECK_CLOSE_FRACTION(distance, dist1, error);
}

BOOST_AUTO_TEST_CASE(closest_path_test_1)
{
    auto point_wkt = "POINT(11.845747600604916272 50.303247769986953131)";
    auto segment_wkt = "SEGMENT(11.8449176 50.3030635,11.8458063 50.3032608)";

    closest_path_tester<double>(point_wkt, segment_wkt, 1e-20);
    closest_path_tester<long double>(point_wkt, segment_wkt, 1e-20);
}

BOOST_AUTO_TEST_CASE(clostest_path_test_2)
{
    auto point_wkt = "POINT(11.921418190002441406 50.316425323486328125)";
    auto segment_wkt = "SEGMENT(11.9214920 50.3161678,11.9212341 50.3161381)";

    closest_path_tester<double>(point_wkt, segment_wkt, 1e-20);
    closest_path_tester<long double>(point_wkt, segment_wkt, 1e-8);
}

BOOST_AUTO_TEST_CASE(clostest_path_test_3)
{
    auto point_wkt = "POINT(11.904624124918561169 50.317349861000025692)";
    auto segment_wkt = "SEGMENT(11.9046808 50.3173523,11.9045925 50.3173485)";

    closest_path_tester<double>(point_wkt, segment_wkt, 1e-20);
    closest_path_tester<long double>(point_wkt, segment_wkt, 1e-20);
}

BOOST_AUTO_TEST_CASE(clostest_path_test_4)
{
    auto point_wkt = "POINT(11.907328887553041017 50.311933736642272308)";
    auto segment_wkt = "SEGMENT(11.9072659 50.3119291,11.9074099 50.3119397)";

    closest_path_tester<double>(point_wkt, segment_wkt, 1e-20);
    closest_path_tester<long double>(point_wkt, segment_wkt, 1e-20);
}

BOOST_AUTO_TEST_CASE(clostest_path_test_5)
{
    auto point_wkt = "POINT(11.894846916198730469 50.316379547119140625)";
    auto segment_wkt = "SEGMENT(11.8958402 50.3155918,11.8953426 50.3155504)";

    closest_path_tester<double>(point_wkt, segment_wkt, 1e-6);
    closest_path_tester<long double>(point_wkt, segment_wkt, 1e-20);
}

BOOST_AUTO_TEST_CASE(clostest_path_test_6)
{
    auto point_wkt = "POINT(11.914519782008024862 50.319138765234583843)";
    auto segment_wkt = "SEGMENT(11.9145157 50.3191017,11.9145257 50.3191925)";

    closest_path_tester<double>(point_wkt, segment_wkt, 1e-6);
    closest_path_tester<long double>(point_wkt, segment_wkt, 1e-20);
}

*/