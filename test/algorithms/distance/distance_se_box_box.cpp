// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2016, Oracle and/or its affiliates.

// Contributed and/or modified by Vissarion Fysikopoulos, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#include <iostream>

#ifndef BOOST_TEST_MODULE
#define BOOST_TEST_MODULE test_distance_spherical_equatorial_box_box
#endif

#include <boost/range.hpp>
#include <boost/type_traits/is_same.hpp>

#include <boost/test/included/unit_test.hpp>
#include <boost/geometry/util/condition.hpp>
#include <boost/geometry/strategies/strategies.hpp>

#include "test_distance_se_common.hpp"


typedef bg::cs::spherical_equatorial<bg::degree> cs_type;
typedef bg::model::point<double, 2, cs_type> point_type;
typedef bg::model::segment<point_type> segment_type;
typedef bg::model::box<point_type> box_type;

namespace distance = bg::strategy::distance;
namespace services = distance::services;
typedef bg::default_distance_result<point_type, point_type>::type return_type;

typedef distance::cross_track_box_box<> box_box_strategy;
typedef distance::cross_track_box_box
<
void, distance::comparable::cross_track<>
> comparable_box_box_strategy;

//===========================================================================

inline bg::distance_result
<
point_type, point_type, distance::haversine<double>
>::type
distance_pp(std::string const& wkt1,
            std::string const& wkt2,
            double radius)
{
    point_type p1, p2;
    bg::read_wkt(wkt1, p1);
    bg::read_wkt(wkt2, p2);

    distance::haversine<double> strategy(radius);
    return bg::distance(p1, p2, strategy);
}

inline bg::default_comparable_distance_result<point_type>::type
comparable_distance_pp(std::string const& wkt1,
                       std::string const& wkt2)
{
    point_type p1, p2;
    bg::read_wkt(wkt1, p1);
    bg::read_wkt(wkt2, p2);
    return bg::comparable_distance(p1, p2);
}

inline bg::distance_result
<
point_type, point_type, distance::cross_track<>
>::type
distance_ps(std::string const& wkt_point,
            std::string const& wkt_segment,
            double radius)
{
    point_type point;
    segment_type segment;
    bg::read_wkt(wkt_point, point);
    bg::read_wkt(wkt_segment, segment);

    distance::cross_track<> strategy(radius);
    return bg::distance(point, segment, strategy);
}

inline bg::default_comparable_distance_result<point_type>::type
comparable_distance_ps(std::string const& wkt_point,
                       std::string const& wkt_segment)
{
    point_type point;
    segment_type segment;
    bg::read_wkt(wkt_point, point);
    bg::read_wkt(wkt_segment, segment);
    return bg::comparable_distance(point, segment);
}

enum features_type { pp, ps };

template <typename Geometry1, typename Geometry2>
struct test_distances
{
    template <typename Strategy>
    static inline void apply(std::string const& case_id,
                             std::string const& wkt1,
                             std::string const& wkt2,
                             double expected_distance,
                             double expected_comparable_distance,
                             Strategy const& strategy)
    {
        typedef test_distance_of_geometries<Geometry1, Geometry2> tester;

        bool const is_comparable = boost::is_same
                <
                Strategy,
                typename services::comparable_type<Strategy>::type
                >::value;

        if (BOOST_GEOMETRY_CONDITION(is_comparable))
        {
            tester::apply(case_id, wkt1, wkt2,
                          expected_comparable_distance,
                          expected_comparable_distance,
                          strategy);
        }
        else
        {
            tester::apply(case_id, wkt1, wkt2,
                          expected_distance,
                          expected_comparable_distance,
                          strategy);
        }
    }

    template <typename Strategy>
    static inline void apply(std::string const& case_id,
                             std::string const& wkt1,
                             std::string const& wkt2,
                             std::string const& feature1,
                             std::string const& feature2,
                             features_type ftype,
                             Strategy const& strategy)
    {
        double const radius = strategy.radius();
        double expected_distance, expected_comparable_distance;

        if (ftype == pp)
        {
            expected_distance = distance_pp(feature1, feature2, radius);
            expected_comparable_distance
                    = comparable_distance_pp(feature1, feature2);
        }
        else
        {
            expected_distance = distance_ps(feature1, feature2, radius);
            expected_comparable_distance
                    = comparable_distance_ps(feature1, feature2);
        }

        apply(case_id, wkt1, wkt2,
              expected_distance, expected_comparable_distance,
              strategy);
    }
};

template <typename T, typename U>
T to_comparable(T const& value, U const& radius)
{
    T x = sin(value / (radius * 2.0));
    return x * x;
}


//===========================================================================


// Cases for relative location of box2 wrt to box1
//
//           |         |
//      11   |    7    |   4
//           |         |
//    --10---+---------+---3---
//           |         |
//       9   |    6    |   2
//           |         |
//    -------+---------+-------
//           |         |
//       8   |    5    |   1
//           |         |
//
// case 6 includes all possible intersections
// The picture assumes northern hemisphere location
// southern hemisphere picture is mirrored wrt the equator

template <typename Strategy>
void test_distance_box_box(Strategy const& strategy)
{
    typedef test_distances<box_type, box_type> tester;

    std::string const box1 = "BOX(10 10,20 20)";

    // case 1
    tester::apply("bb1", box1, "BOX(30 0,40 5)",
                  "POINT(20 10)", "POINT(30 5)", pp,
                  strategy);

    // case 2
    tester::apply("bb2-a", box1, "BOX(30 12, 40 17)",
                  "POINT(30 17)", "SEGMENT(20 10,20 20)", ps,
                  strategy);

    tester::apply("bb2-b", box1, "BOX(30 10, 40 17)",
                  "POINT(30 17)", "SEGMENT(20 10,20 20)", ps,
                  strategy);

    tester::apply("bb2-c", box1, "BOX(30 8, 40 17)",
                  "POINT(30 17)", "SEGMENT(20 10,20 20)", ps,
                  strategy);


    // case 3
    tester::apply("bb3-a", box1, "BOX(30 15, 40 25)",
                  "POINT(20 20)", "SEGMENT(30 15,30 25)", ps,
                  strategy);

    tester::apply("bb3-b", box1, "BOX(30 20, 40 40)",
                  "POINT(20 20)", "SEGMENT(30 20,30 40)", ps,
                  strategy);

    // case 4
    tester::apply("bb4", box1, "BOX(30 25, 40 40)",
                  "POINT(20 20)", "POINT(30 25)", pp,
                  strategy);

    // case 5
    tester::apply("bb5", box1, "BOX(12 2, 17 7)",
                  "POINT(17 7)", "POINT(17 10)", pp,
                  strategy);

    // case 6, boxes intersect thus distance is 0
    tester::apply("bb6-a", box1, "BOX(12 2, 17 10)",
                  "POINT(0 0)", "POINT(0 0)", pp,
                  strategy);

    tester::apply("bb6-b", box1, "BOX(12 2, 17 17)",
                  "POINT(0 0)", "POINT(0 0)", pp,
                  strategy);

    tester::apply("bb6-c", box1, "BOX(20 2, 30 10)",
                  "POINT(0 0)", "POINT(0 0)", pp,
                  strategy);

    tester::apply("bb6-d", box1, "BOX(20 11, 30 15)",
                  "POINT(0 0)", "POINT(0 0)", pp,
                  strategy);

    tester::apply("bb6-e", box1, "BOX(20 20, 30 30)",
                  "POINT(0 0)", "POINT(0 0)", pp,
                  strategy);

    tester::apply("bb6-f", box1, "BOX(15 20, 17 30)",
                  "POINT(0 0)", "POINT(0 0)", pp,
                  strategy);

    tester::apply("bb6-g", box1, "BOX(8 20, 10 25)",
                  "POINT(0 0)", "POINT(0 0)", pp,
                  strategy);

    tester::apply("bb6-h", box1, "BOX(8 15 , 10 17)",
                  "POINT(0 0)", "POINT(0 0)", pp,
                  strategy);

    tester::apply("bb6-i", box1, "BOX(8 8, 10 10)",
                  "POINT(0 0)", "POINT(0 0)", pp,
                  strategy);

    tester::apply("bb6-j", box1, "BOX(15 8, 17 10)",
                  "POINT(0 0)", "POINT(0 0)", pp,
                  strategy);

    // case 7
    tester::apply("bb7", box1, "BOX(12 22, 17 27)",
                  "POINT(17 20)", "POINT(17 22)", pp,
                  strategy);

    // case 8
    tester::apply("bb8", box1, "BOX(4 4, 8 8)",
                  "POINT(8 8)", "POINT(10 10)", pp,
                  strategy);

    // case 9
    tester::apply("bb9-a", box1, "BOX(4 14, 8 18)",
                  "POINT(8 18)", "SEGMENT(10 10, 10 20)", ps,
                  strategy);

    tester::apply("bb9-b", box1, "BOX(4 10, 8 18)",
                  "POINT(8 18)", "SEGMENT(10 10, 10 20)", ps,
                  strategy);

    tester::apply("bb9-c", box1, "BOX(4 8, 8 18)",
                  "POINT(8 18)", "SEGMENT(10 10, 10 20)", ps,
                  strategy);

    // case 10
    tester::apply("bb10", box1, "BOX(4 18, 8 22)",
                  "POINT(10 20)", "SEGMENT(8 18, 8 22)", ps,
                  strategy);

    tester::apply("bb10", box1, "BOX(4 20, 8 22)",
                  "POINT(10 20)", "SEGMENT(8 20, 8 22)", ps,
                  strategy);

    // case 11
    tester::apply("bb11", box1, "BOX(4 22, 8 24)",
                  "POINT(8 22)", "POINT(10 20)", pp,
                  strategy);

    // far away boxes
    tester::apply("bb-far", "BOX(150 15, 170 25)", box1,
                  "POINT(20 20)", "SEGMENT(150 15, 150 25)", ps,
                  strategy);

    // crosses antimeridian
    tester::apply("bb-anti1", "BOX(170 15, -160 25)", box1,
                  "POINT(20 20)", "SEGMENT(170 15, 170 25)", ps,
                  strategy);

    tester::apply("bb-anti2", "BOX(170 15, -160 25)", "BOX(160 10, -170 20)",
                  "POINT(20 20)", "POINT(20 20)", pp,
                  strategy);

    tester::apply("bb-anti3", "BOX(170 15, -160 25)", "BOX(160 10, 170 20)",
                  "POINT(20 20)", "POINT(20 20)", pp,
                  strategy);

    tester::apply("bb-anti4", "BOX(170 10, -160 20)", "BOX(160 30, -170 40)",
                  "POINT(180 20)", "POINT(180 30)", pp,
                  strategy);

    // South hemisphere

    tester::apply("bb-south1", "BOX(10 -20, 20 -10)", "BOX(30 -15, 40 -12)",
                  "POINT(30 -15)", "SEGMENT(20 -10, 20 -20)", ps,
                  strategy);

    tester::apply("bb-south2", "BOX(10 -20, 20 -10)", "BOX(30 -30, 40 -25)",
                  "POINT(30 -25)", "POINT(20 -20)", pp,
                  strategy);

    tester::apply("bb-south3", "BOX(10 -20, 20 -10)", "BOX(30 -25, 40 -15)",
                  "POINT(20 -20)", "SEGMENT(30 -15, 30 -25)", ps,
                  strategy);

    tester::apply("bb-south4", "BOX(10 -20, 20 -10)", "BOX(5 -30, 30 -25)",
                  "POINT(10 -25)", "POINT(10 -20)", pp,
                  strategy);

    tester::apply("bb-south4", "BOX(10 -20, 20 -10)", "BOX(5 -7, 30 -5)",
                  "POINT(10 -7)", "POINT(10 -10)", pp,
                  strategy);


    // Crosses equator

    tester::apply("bb-eq1", "BOX(30 -15, 40 30)", "BOX(10 -20, 20 25)",
                  "POINT(20 25)", "SEGMENT(30 -15, 30 30)", ps,
                  strategy);

    tester::apply("bb-eq2", "BOX(30 -15, 40 20)", "BOX(10 -20, 20 25)",
                  "POINT(30 20)", "SEGMENT(20 -20, 20 25)", ps,
                  strategy);

    tester::apply("bb-eq3", "BOX(30 5, 40 20)", "BOX(10 -20, 20 25)",
                  "POINT(30 20)", "SEGMENT(20 -20, 20 25)", ps,
                  strategy);

    tester::apply("bb-eq4", "BOX(5 -30, 40 -25)", "BOX(10 -20, 20 25)",
                  "POINT(10 -25)", "POINT(10 -20)", pp,
                  strategy);

    tester::apply("bb-eq5", "BOX(30 5, 40 20)", "BOX(10 -20, 50 25)",
                  "POINT(30 20)", "POINT(30 20)", pp,
                  strategy);

    tester::apply("bb-eq6", "BOX(30 5, 40 20)", "BOX(10 -20, 35 25)",
                  "POINT(30 20)", "POINT(30 20)", pp,
                  strategy);

    // One box in the north and one in the south hemisphere

    tester::apply("bb-ns1", "BOX(30 15, 40 20)", "BOX(10 -20, 20 -15)",
                  "POINT(30 15)", "POINT(20 -15)", pp,
                  strategy);

    tester::apply("bb-ns2", "BOX(30 15, 40 20)", "BOX(25 -20, 50 -15)",
                  "POINT(30 15)", "POINT(30 -15)", pp,
                  strategy);
}

template <typename Strategy>
void test_distance_box_box_negative(Strategy const& strategy)
{
    typedef test_distances<box_type, box_type> tester;
    std::string const box1neg = "BOX(-20 10,-10 20)";

    // case 1
    tester::apply("bb1", box1neg, "BOX(-40 0,-30 5)",
                  "POINT(-20 10)", "POINT(-30 5)", pp,
                  strategy);

    // case 2
    tester::apply("bb2-a", box1neg, "BOX(-40 12, -30 17)",
                  "POINT(-30 17)", "SEGMENT(-20 10,-20 20)", ps,
                  strategy);

    tester::apply("bb2-b", box1neg, "BOX(-40 10, -30 17)",
                  "POINT(-30 17)", "SEGMENT(-20 10,-20 20)", ps,
                  strategy);

    tester::apply("bb2-c", box1neg, "BOX(-40 8, -30 17)",
                  "POINT(-30 17)", "SEGMENT(-20 10,-20 20)", ps,
                  strategy);


    // case 3
    tester::apply("bb3-a", box1neg, "BOX(-40 15, -30 25)",
                  "POINT(-20 20)", "SEGMENT(-30 15,-30 25)", ps,
                  strategy);

    tester::apply("bb3-b", box1neg, "BOX(-40 20, -30 40)",
                  "POINT(-20 20)", "SEGMENT(-30 20,-30 40)", ps,
                  strategy);

    // case 4
    tester::apply("bb4", box1neg, "BOX(-40 25, -30 40)",
                  "POINT(-20 20)", "POINT(-30 25)", pp,
                  strategy);

    // case 5
    tester::apply("bb5", box1neg, "BOX(-17 2,-12 7)",
                  "POINT(-17 7)", "POINT(-17 10)", pp,
                  strategy);

    // case 6, boxes intersect thus distance is 0
    tester::apply("bb6-a", box1neg, "BOX(-17 2, -12 10)",
                  "POINT(0 0)", "POINT(0 0)", pp,
                  strategy);

    tester::apply("bb6-b", box1neg, "BOX(-17 2, -12 17)",
                  "POINT(0 0)", "POINT(0 0)", pp,
                  strategy);

    tester::apply("bb6-c", box1neg, "BOX(-30 2, -20 10)",
                  "POINT(0 0)", "POINT(0 0)", pp,
                  strategy);

    tester::apply("bb6-d", box1neg, "BOX(-30 11, -20 15)",
                  "POINT(0 0)", "POINT(0 0)", pp,
                  strategy);

    tester::apply("bb6-e", box1neg, "BOX(-30 20, -20 30)",
                  "POINT(0 0)", "POINT(0 0)", pp,
                  strategy);

    tester::apply("bb6-f", box1neg, "BOX(-17 20, -15 30)",
                  "POINT(0 0)", "POINT(0 0)", pp,
                  strategy);

    tester::apply("bb6-g", box1neg, "BOX(-10 20, -8 25)",
                  "POINT(0 0)", "POINT(0 0)", pp,
                  strategy);

    tester::apply("bb6-h", box1neg, "BOX(-10 15 , -8 17)",
                  "POINT(0 0)", "POINT(0 0)", pp,
                  strategy);

    tester::apply("bb6-i", box1neg, "BOX(-10 8, -8 10)",
                  "POINT(0 0)", "POINT(0 0)", pp,
                  strategy);

    tester::apply("bb6-j", box1neg, "BOX(-17 8, -15 10)",
                  "POINT(0 0)", "POINT(0 0)", pp,
                  strategy);

    // case 7
    tester::apply("bb7", box1neg, "BOX(-17 22, -12 27)",
                  "POINT(-17 20)", "POINT(-17 22)", pp,
                  strategy);

    // case 8
    tester::apply("bb8", box1neg, "BOX(-8 4, -4 8)",
                  "POINT(-8 8)", "POINT(-10 10)", pp,
                  strategy);

    // case 9
    tester::apply("bb9-a", box1neg, "BOX(-8 14, -4 18)",
                  "POINT(-8 18)", "SEGMENT(-10 10, -10 20)", ps,
                  strategy);

    tester::apply("bb9-b", box1neg, "BOX(-8 10, -4 18)",
                  "POINT(-8 18)", "SEGMENT(-10 10, -10 20)", ps,
                  strategy);

    tester::apply("bb9-c", box1neg, "BOX(-8 8, -4 18)",
                  "POINT(-8 18)", "SEGMENT(-10 10, -10 20)", ps,
                  strategy);

    // case 10
    tester::apply("bb10", box1neg, "BOX(-8 18, -4 22)",
                  "POINT(-10 20)", "SEGMENT(-8 18, -8 22)", ps,
                  strategy);

    tester::apply("bb10", box1neg, "BOX(-8 20, -4 22)",
                  "POINT(-10 20)", "SEGMENT(-8 20, -8 22)", ps,
                  strategy);

    // case 11
    tester::apply("bb11", box1neg, "BOX(-8 22, -4 24)",
                  "POINT(-8 22)", "POINT(-10 20)", pp,
                  strategy);
}

BOOST_AUTO_TEST_CASE( test_box_box )
{
    test_distance_box_box(box_box_strategy());
    test_distance_box_box_negative(box_box_strategy());
}
