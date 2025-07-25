// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2007-2016 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2008-2016 Bruno Lalande, Paris, France.
// Copyright (c) 2009-2016 Mateusz Loskot, London, UK.

// This file was modified by Oracle on 2016-2021.
// Modifications copyright (c) 2016-2021, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Parts of Boost.Geometry are redesigned from Geodan's Geographic Library
// (geolib/GGL), copyright (c) 1995-2010 Geodan, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <string>

#include "test_union.hpp"
#include <algorithms/test_overlay.hpp>

#include <algorithms/overlay/overlay_cases.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/util/constexpr.hpp>


#define TEST_UNION(caseid, clips, holes, points, area) \
    (test_one<Polygon, Polygon, Polygon>) \
    ( #caseid, caseid[0], caseid[1], clips, holes, points, area)

#define TEST_UNION_WITH(caseid, clips, holes, points, area) \
    (test_one<Polygon, Polygon, Polygon>) \
    ( #caseid, caseid[0], caseid[1], clips, holes, points, area, settings)

#define TEST_UNION_REV(caseid, clips, holes, points, area) \
    (test_one<Polygon, Polygon, Polygon>) \
    ( #caseid "_rev", caseid[1], caseid[0], clips, holes, points, area)


template <typename Ring, typename Polygon>
void test_areal()
{
    typedef typename bg::coordinate_type<Polygon>::type ct;

    constexpr bool is_ccw = bg::point_order<Polygon>::value == bg::counterclockwise;

    ut_settings ignore_validity_for_float;
    if (BOOST_GEOMETRY_CONDITION((std::is_same<ct, float>::value)) )
    {
        ignore_validity_for_float.set_test_validity(false);
    }

    test_one<Polygon, Polygon, Polygon>("simplex_normal",
        simplex_normal[0], simplex_normal[1],
        1, 0, 13, 11.526367);

    test_one<Polygon, Polygon, Polygon>("simplex_with_empty_1",
        simplex_normal[0], polygon_empty,
        1, 0, 4, 8.0);
    test_one<Polygon, Polygon, Polygon>("simplex_with_empty_2",
        polygon_empty, simplex_normal[0],
        1, 0, 4, 8.0);

    test_one<Polygon, Polygon, Polygon>("star_ring", example_star, example_ring,
        1, 0, 23, 5.67017141);

    // This sample was selected because of the border case.
    test_one<Polygon, Polygon, Polygon>("star_poly", example_star, example_polygon,
        1, 1, -1, 5.647949);

    // Pseudo-box as Polygon
    // (note, internally, the intersection points is different, so yes,
    // it has to be tested)
    test_one<Polygon, Polygon, Polygon>("box_poly3", "POLYGON((1.5 1.5 , 1.5 2.5 , 4.5 2.5 , 4.5 1.5 , 1.5 1.5))",
            "POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2,3.7 1.6,3.4 2,4.1 2.5,5.3 2.5,5.4 1.2,4.9 0.8,2.9 0.7,2 1.3))",
                1, 1, 15, 5.93625);

    test_one<Polygon, Polygon, Polygon>("first_within_second",
        first_within_second[0], first_within_second[1],
        1, 0, 5, 25.0);

    test_one<Polygon, Polygon, Polygon>("second_within_first",
        first_within_second[1], first_within_second[0],
        1, 0, 5, 25.0);

    test_one<Polygon, Polygon, Polygon>("first_within_hole_of_second",
        first_within_hole_of_second[0], first_within_hole_of_second[1],
        2, 1, 15, 17.0);

    test_one<Polygon, Polygon, Polygon>("new_hole",
        new_hole[0], new_hole[1],
        1, 1, 14, 23.0);

    test_one<Polygon, Polygon, Polygon>("side_side",
        side_side[0], side_side[1], 1, 0, 7, 2.0);

    test_one<Polygon, Polygon, Polygon>("identical",
        identical[0], identical[1], 1, 0, 5, 1.0);

    test_one<Polygon, Polygon, Polygon>("disjoint",
        disjoint[0], disjoint[1], 2, 0, 10, 2.0);

    test_one<Polygon, Polygon, Polygon>("intersect_holes_intersect",
        intersect_holes_intersect[0], intersect_holes_intersect[1],
        1, 1, 14, 39.75);

    test_one<Polygon, Polygon, Polygon>("intersect_holes_intersect_and_disjoint",
        intersect_holes_intersect_and_disjoint[0], intersect_holes_intersect_and_disjoint[1],
        1, 1, 14, 39.75);

    test_one<Polygon, Polygon, Polygon>("intersect_holes_intersect_and_touch",
        intersect_holes_intersect_and_touch[0], intersect_holes_intersect_and_touch[1],
        1, 1, 14, 39.75);

    test_one<Polygon, Polygon, Polygon>("intersect_holes_new_ring",
        intersect_holes_new_ring[0], intersect_holes_new_ring[1],
        1, 2, 15, 253.8961);

    test_one<Polygon, Polygon, Polygon>("intersect_holes_disjoint",
        intersect_holes_disjoint[0],
        intersect_holes_disjoint[1],
        1, 0, 9, 40.0);

    test_one<Polygon, Polygon, Polygon>("within_holes_disjoint",
        within_holes_disjoint[0], within_holes_disjoint[1],
        1, 0, 5, 49.0);

    test_one<Polygon, Polygon, Polygon>("winded",
        winded[0], winded[1],
        1, 5, 30, 114.0);

    test_one<Polygon, Polygon, Polygon>("two_bends",
        two_bends[0], two_bends[1],
        1, 0, 7, 40.0);

    test_one<Polygon, Polygon, Polygon>("star_comb_15",
        star_comb_15[0], star_comb_15[1],
        1, 27, 204, 898.09693338);

    test_one<Polygon, Polygon, Polygon>("equal_holes_disjoint",
        equal_holes_disjoint[0], equal_holes_disjoint[1],
        1, 1, 10, 81 - 3 * 7);

    test_one<Polygon, Polygon, Polygon>("only_hole_intersections1",
        only_hole_intersections[0], only_hole_intersections[1],
        1, 2, 13, 190.9090909);
    test_one<Polygon, Polygon, Polygon>("only_hole_intersections2",
        only_hole_intersections[0], only_hole_intersections[2],
        1, 2, 13, 190.9090909);

    test_one<Polygon, Polygon, Polygon>("intersect_exterior_and_interiors_winded",
        intersect_exterior_and_interiors_winded[0], intersect_exterior_and_interiors_winded[1],
        1, 1, 26, 66.5333333);

    test_one<Polygon, Polygon, Polygon>("crossed",
        crossed[0], crossed[1],
        1, 3, 17, 23.5); // Area from SQL Server - was somehow wrong before

    test_one<Polygon, Polygon, Polygon>("fitting",
        fitting[0], fitting[1],
        1, 0, 5, 25);

    test_one<Polygon, Polygon, Polygon>("distance_zero",
        distance_zero[0], distance_zero[1],
        1, 0, -1, 9.0098387);

    test_one<Polygon, Polygon, Polygon>("wrapped_a",
        wrapped[0], wrapped[1],
        1, 1, 16, 16);
    test_one<Polygon, Polygon, Polygon>("wrapped_b",
        wrapped[0], wrapped[2],
        1, 1, 16, 16);

    test_one<Polygon, Polygon, Polygon>("9",
                case_9[0], case_9[1], 2, 0, 8, 11);
    test_one<Polygon, Polygon, Polygon>("22",
                case_22[0], case_22[1], 2, 0, 8, 9.5);
    test_one<Polygon, Polygon, Polygon>("25",
                case_25[0], case_25[1], 2, 0, 8, 7);
    test_one<Polygon, Polygon, Polygon>("26",
                case_26[0], case_26[1], 2, 0, 9, 7.5);
    test_one<Polygon, Polygon, Polygon>("31",
                case_31[0], case_31[1], 2, 0, 8, 4.5);
    test_one<Polygon, Polygon, Polygon>("32",
                case_32[0], case_32[1], 2, 0, 8, 4.5);
    test_one<Polygon, Polygon, Polygon>("33",
                case_33[0], case_33[1], 2, 0, 8, 4.5);
    test_one<Polygon, Polygon, Polygon>("36",
                case_36[0], case_36[1], 1, 1, 10, 14.375);
    test_one<Polygon, Polygon, Polygon>("40",
                case_40[0], case_40[1], 2, 0, 18, 11);

    // Test rings which should be generated by assemble
    test_one<Polygon, Polygon, Polygon>("53_iet",
                case_53[0], case_53[2], 1, 1, 16, 16);
    test_one<Polygon, Polygon, Polygon>("58_iet",
                case_58[0], case_58[2], 1, 3, 20, 12.16666);

    test_one<Polygon, Polygon, Polygon>("59_iet",
                case_59[0], case_59[2], 1, 1, 14, 17.20833);

    test_one<Polygon, Polygon, Polygon>("80",
                case_80[0], case_80[1], 2, 2, 18, 129.0);

    test_one<Polygon, Polygon, Polygon>("81",
                case_81[0], case_81[1], 1, 2, 15, 163.5);

    test_one<Polygon, Polygon, Polygon>("82",
                case_82[0], case_82[1], 1, 1, 21, 19.75);

    test_one<Polygon, Polygon, Polygon>("83",
                case_83[0], case_83[1], 1, 0, 7, 20.0);

    test_one<Polygon, Polygon, Polygon>("90",
                case_90[0], case_90[1], 1, 2, 17, 221.3688);

    test_one<Polygon, Polygon, Polygon>("91",
                case_91[0], case_91[1], 1, 1, 9, 147.5);

    test_one<Polygon, Polygon, Polygon>("92",
                case_92[0], case_92[1], 2, 0, 9, 175.0);

    test_one<Polygon, Polygon, Polygon>("93",
                case_93[0], case_93[1], 1, 2, 12, 172.9167);

    test_one<Polygon, Polygon, Polygon>("94",
                case_94[0], case_94[1], 2, 0, 12, 170.0);

    // Has two separate but touching interior rings
    test_one<Polygon, Polygon, Polygon>("95",
                case_95[0], case_95[1], 1, 2, 15, 1320.0);

    test_one<Polygon, Polygon, Polygon>("96",
                case_96[0], case_96[1], 1, 1, 10, 1500.0);

    test_one<Polygon, Polygon, Polygon>("97",
                case_97[0], case_97[1], 1, 0, 10, 286.799);

    test_one<Polygon, Polygon, Polygon>("98",
                case_98[0], case_98[1], 1, 1, 14, 653.067);

    test_one<Polygon, Polygon, Polygon>("99",
                case_99[0], case_99[1], 1, 0, 5, 1600.0);

    test_one<Polygon, Polygon, Polygon>("100",
                case_100[0], case_100[1], 1, 1, 13, 19.125);

    test_one<Polygon, Polygon, Polygon>("101",
                case_101[0], case_101[1], 1, 0, 9, 21.0);

    test_one<Polygon, Polygon, Polygon>("102",
                case_102[0], case_102[1], 1, 1, 17, 8.75);

    test_one<Polygon, Polygon, Polygon>("103",
                case_103[0], case_103[1],
                1, 0, 5, 1.0);
    test_one<Polygon, Polygon, Polygon>("104",
                case_104[0], case_104[1],
                1, 0, 5, 100.0);

    test_one<Polygon, Polygon, Polygon>("105",
                case_105[0], case_105[1], 1, 0, 5, 100);

    test_one<Polygon, Polygon, Polygon>("108",
                case_108[0], case_108[1], 1, 0, 13, 5.0);

    TEST_UNION(case_precision_1, 1, 0, -1, 22.0);
    TEST_UNION(case_precision_2, 1, 0, -1, 22.0);
    TEST_UNION(case_precision_3, 1, 0, -1, 22.0);
    TEST_UNION(case_precision_4, 1, 0, -1, 22.0);
    TEST_UNION(case_precision_5, 1, 0, -1, 22.0);
    TEST_UNION(case_precision_6, 1, 0, -1, 71.0);
    TEST_UNION(case_precision_7, 1, 0, -1, 22.0);
    TEST_UNION(case_precision_8, 1, 1, -1, 73.0);
    TEST_UNION(case_precision_9, 1, 1, -1, 73.0);
    TEST_UNION(case_precision_10, 1, 1, -1, 73.0);
    TEST_UNION(case_precision_11, 1, 1, -1, 73.0);
    TEST_UNION(case_precision_12, 1, 0, -1, 14.0);
    TEST_UNION(case_precision_13, 1, 0, -1, 14.0);
    TEST_UNION(case_precision_14, 1, 0, -1, 22.0);
    TEST_UNION(case_precision_15, 1, 1, -1, 73.0);
    TEST_UNION(case_precision_16, 1, 1, -1, 73.0);
    TEST_UNION(case_precision_17, 1, 1, -1, 73.0);
    TEST_UNION(case_precision_18, 1, 1, -1, 73.0);
    TEST_UNION(case_precision_19, 1, 1, -1, 73.0);
    TEST_UNION(case_precision_20, 1, 0, -1, 22.0);
    TEST_UNION(case_precision_21, 1, 0, -1, 22.0);
    TEST_UNION(case_precision_22, 1, 1, -1, 73.0);
    TEST_UNION(case_precision_23, 1, 1, -1, 73.0);
    TEST_UNION(case_precision_24, 1, 0, -1, 22.0);
    TEST_UNION(case_precision_25, 1, 0, -1, 22.0);
    TEST_UNION(case_precision_26, 1, 1, -1, 73.0);

    TEST_UNION_REV(case_precision_1, 1, 0, -1, 22.0);
    TEST_UNION_REV(case_precision_2, 1, 0, -1, 22.0);
    TEST_UNION_REV(case_precision_3, 1, 0, -1, 22.0);
    TEST_UNION_REV(case_precision_4, 1, 0, -1, 22.0);
    TEST_UNION_REV(case_precision_5, 1, 0, -1, 22.0);
    TEST_UNION_REV(case_precision_6, 1, 0, -1, 71.0);
    TEST_UNION_REV(case_precision_7, 1, 0, -1, 22.0);
    TEST_UNION_REV(case_precision_8, 1, 1, -1, 73.0);
    TEST_UNION_REV(case_precision_9, 1, 1, -1, 73.0);
    TEST_UNION_REV(case_precision_10, 1, 1, -1, 73.0);
    TEST_UNION_REV(case_precision_11, 1, 1, -1, 73.0);
    TEST_UNION_REV(case_precision_12, 1, 0, -1, 14.0);
    TEST_UNION_REV(case_precision_13, 1, 0, -1, 14.0);
    TEST_UNION_REV(case_precision_14, 1, 0, -1, 22.0);
    TEST_UNION_REV(case_precision_15, 1, 1, -1, 73.0);
    TEST_UNION_REV(case_precision_16, 1, 1, -1, 73.0);
    TEST_UNION_REV(case_precision_17, 1, 1, -1, 73.0);
    TEST_UNION_REV(case_precision_18, 1, 1, -1, 73.0);
    TEST_UNION_REV(case_precision_19, 1, 1, -1, 73.0);
    TEST_UNION_REV(case_precision_20, 1, 0, -1, 22.0);
    TEST_UNION_REV(case_precision_21, 1, 0, -1, 22.0);
    TEST_UNION_REV(case_precision_22, 1, 1, -1, 73.0);
    TEST_UNION_REV(case_precision_23, 1, 1, -1, 73.0);
    TEST_UNION_REV(case_precision_24, 1, 0, -1, 22.0);
    TEST_UNION_REV(case_precision_25, 1, 0, -1, 22.0);
    TEST_UNION_REV(case_precision_26, 1, 1, -1, 73.0);

    /*
    test_one<Polygon, Polygon, Polygon>(102,
        simplex_normal[0], simplex_reversed[1],
        1, 0, 7, X);

    test_one<Polygon, Polygon, Polygon>(103,
        simplex_reversed[0], simplex_normal[1],
        1, 0, 7, 24.0);

    test_one<Polygon, Polygon, Polygon>(104,
        simplex_reversed[0], simplex_reversed[1],
        1, 0, 7, 24.0);

    test_one<Polygon, Polygon, Polygon>(100,
        star_15, comb_15,
        1, 10, 7, 24.0);
    */

    // test some other input/output types

    // 1 input Ring
    test_one<Polygon, Polygon, Ring>("identical_pr", identical[0], identical[1], 1, 0, 5, 1.0);
    test_one<Polygon, Ring, Polygon>("identical_rp", identical[0], identical[1], 1, 0, 5, 1.0);

    // 2 input rings
    test_one<Polygon, Ring, Ring>("identical_rr", identical[0], identical[1], 1, 0, 5, 1.0);

    // output is also Ring
    test_one<Ring, Ring, Ring>("identical_rrr", identical[0], identical[1], 1, 0, 5, 1.0);

    // "new hole", tested with Ring -> the newly formed hole will be omitted
    test_one<Ring, Ring, Ring>("new_hole_discarded", new_hole[0], new_hole[1], 1, 0, 9, 24.0);

    test_one<Polygon, Polygon, Polygon>("ggl_list_20110306_javier",
        ggl_list_20110306_javier[0], ggl_list_20110306_javier[1],
        1, 1, 16, 80456.4904910401);

    test_one<Polygon, Polygon, Polygon>("ggl_list_20110307_javier",
        ggl_list_20110307_javier[0], ggl_list_20110307_javier[1],
        1, 1, 13, 20016.4);

    test_one<Polygon, Polygon, Polygon>("ggl_list_20110627_phillip",
        ggl_list_20110627_phillip[0], ggl_list_20110627_phillip[1],
        1, 0, 8, 14729.07145);

    test_one<Polygon, Polygon, Polygon>("ggl_list_20110716_enrico",
        ggl_list_20110716_enrico[0], ggl_list_20110716_enrico[1],
        1, 1, 15, 129904.197692871);

    TEST_UNION(ggl_list_20110820_christophe, count_set(1, 2), 0, -1, 67.3550722317627);

    // SQL Server gives: 313.360374193241
    // PostGIS gives:    313.360364623393
    TEST_UNION(isovist, 1, 0, -1, 313.36036462);

    TEST_UNION(ggl_list_20190307_matthieu_1, 1, 1, -1, 0.83773);
    TEST_UNION(ggl_list_20190307_matthieu_2, 1, 0, -1, 16.0);

    // Ticket 5103 https://svn.boost.org/trac/boost/ticket/5103
    // This ticket was actually reported for Boost.Polygon
    // We check it for Boost.Geometry as well.
    // SQL Server gives:     2515271331437.69
    // PostGIS gives:        2515271327070.52
    // Boost.Geometry gives: 2515271327070.5237746891 (ttmath)
    //                       2515271327070.5156 (double)
    //                       2515271320603.0000 (int)
    // Note the int-test was tested outside of this unit test.
    // It is in two points 0.37 off (logical for an int).
    // Because of the width of the polygon (400000 meter)
    // this causes a substantial difference.
    TEST_UNION(ticket_5103, 1, 0, 25, 2515271327070.5);

    TEST_UNION(ticket_8310a, 1, 0, -1, 10.5000019595);
    TEST_UNION(ticket_8310b, 1, 0, -1, 10.5000019595);
    TEST_UNION(ticket_8310c, 1, 0, -1, 10.5000019595);
    TEST_UNION_REV(ticket_8310a, 1, 0, -1, 10.5000019595);
    TEST_UNION_REV(ticket_8310b, 1, 0, -1, 10.5000019595);
    TEST_UNION_REV(ticket_8310c, 1, 0, -1, 10.5000019595);

    TEST_UNION(ticket_9081_15, 1, 0, -1, 0.0403425433);

    TEST_UNION(ticket_9563, 1, 0, -1, 150.0);

    TEST_UNION(ticket_9756, 1, 0, 10, expectation_limits(1289.0835, 1290.40625));

    // Can generate one polygon, or two splitted, both is OK
    TEST_UNION(ticket_10108_a, count_set(1, 2), 0, 8, 0.0435229);
    TEST_UNION(ticket_10108_b, count_set(1, 2), 0, 10, 2424.3449);

    TEST_UNION(ticket_10866, 1, 0, 14, 332760303.5);

    TEST_UNION(ticket_11725, 1, 1, 10, 7.5);

    TEST_UNION(issue_548, 1, 0, -1, 617382720000);

    TEST_UNION(issue_566_a, 1, 0, -1, 214.3728);
    TEST_UNION(issue_566_b, 1, 0, -1, 214.3728);
    TEST_UNION_REV(issue_566_a, 1, 0, -1, 214.3728);
    TEST_UNION_REV(issue_566_b, 1, 0, -1, 214.3728);

    TEST_UNION(issue_690, 2, 0, -1, 25492.0505);

    TEST_UNION(issue_838, 1, 0, -1, expectation_limits(1.3333, 1.33785));
    TEST_UNION_REV(issue_838, 1, 0, -1, expectation_limits(1.3333, 1.33785));

    TEST_UNION(issue_893, 1, 0, -1, 97686917.29298662);
    TEST_UNION_REV(issue_893, 1, 0, -1, 97686917.29298662);

    TEST_UNION(issue_1076, 1, 0, -1, 1225.0);
    TEST_UNION_REV(issue_1076, 1, 0, -1, 1225.0);

    TEST_UNION(issue_1081a, 1, 2, -1, 1600.56);
    TEST_UNION_REV(issue_1081a, 1, 2, -1, 1600.56);

    TEST_UNION(issue_1081b, 1, 2, -1, 1600.56);
    TEST_UNION_REV(issue_1081b, 1, 2, -1, 1600.56);

    TEST_UNION(issue_1081c, 1, 1, -1, 2338.08);
    TEST_UNION_REV(issue_1081c, 1, 1, -1, 2338.08);

    TEST_UNION(issue_1100, 1, 0, -1, 1.46181);
    TEST_UNION_REV(issue_1100, 1, 0, -1, 1.46181);

    TEST_UNION(issue_1108, 1, 0, -1, 12.1742);
    TEST_UNION_REV(issue_1108, 1, 0, -1, 12.1742);

    TEST_UNION(issue_1183, 1, 0, -1, 607.6507);
    TEST_UNION_REV(issue_1183, 1, 0, -1, 607.6507);

    TEST_UNION(issue_1186, 1, 1, -1, 21.6189);
    TEST_UNION_REV(issue_1186, 1, 1, -1, 21.6189);

    TEST_UNION(issue_1229, 1, 0, -1, 384869.166);
    TEST_UNION_REV(issue_1229, 1, 0, -1, 384869.166);

    TEST_UNION(geos_1, 1, 0, -1, expectation_limits(3458.0, 3461.3203125));
    TEST_UNION(geos_2, 1, 0, -1, expectation_limits(349.0625, 350.55102539));
    TEST_UNION(geos_3, 1, 0, -1, 29391548.4998779);
    TEST_UNION(geos_4, 1, 0, -1, 2304.4163115);

    // Robustness issues, followed out buffer-robustness-tests, test them also reverse
    {
        // Area can vary depending on joining point of nearly parallel lines
        ut_settings settings;
        settings.percentage = 0.01;
        test_one<Polygon, Polygon, Polygon>("buffer_rt_a", buffer_rt_a[0], buffer_rt_a[1],
                    1, 0, -1, 19.28, settings);
        test_one<Polygon, Polygon, Polygon>("buffer_rt_a_rev", buffer_rt_a[1], buffer_rt_a[0],
                    1, 0, -1, 19.28, settings);
    }
    test_one<Polygon, Polygon, Polygon>("buffer_rt_f", buffer_rt_f[0], buffer_rt_f[1],
                1, 0, -1, 4.60853);
    test_one<Polygon, Polygon, Polygon>("buffer_rt_f_rev", buffer_rt_f[1], buffer_rt_f[0],
                1, 0, -1, 4.60853);
    test_one<Polygon, Polygon, Polygon>("buffer_rt_g", buffer_rt_g[0], buffer_rt_g[1],
                1, 0, -1, 16.571);
    test_one<Polygon, Polygon, Polygon>("buffer_rt_g_rev", buffer_rt_g[1], buffer_rt_g[0],
                1, 0, -1, 16.571);
    test_one<Polygon, Polygon, Polygon>("buffer_rt_i", buffer_rt_i[0], buffer_rt_i[1],
                1, 0, -1, 13.6569);
    test_one<Polygon, Polygon, Polygon>("buffer_rt_i_rev", buffer_rt_i[1], buffer_rt_i[0],
                    1, 0, -1, 13.6569);
    test_one<Polygon, Polygon, Polygon>("buffer_rt_j", buffer_rt_j[0], buffer_rt_j[1],
                1, 0, -1, 16.5711);
    test_one<Polygon, Polygon, Polygon>("buffer_rt_j_rev", buffer_rt_j[1], buffer_rt_j[0],
                1, 0, -1, 16.5711);

    test_one<Polygon, Polygon, Polygon>("buffer_rt_l", buffer_rt_l[0], buffer_rt_l[1],
                1, 0, -1, 19.3995);
    test_one<Polygon, Polygon, Polygon>("buffer_rt_l_rev", buffer_rt_l[1], buffer_rt_l[0],
                1, 0, -1, 19.3995);

    test_one<Polygon, Polygon, Polygon>("buffer_rt_m1", buffer_rt_m1[0], buffer_rt_m1[1],
                1, 0, -1, 19.4852);
    test_one<Polygon, Polygon, Polygon>("buffer_rt_m1_rev", buffer_rt_m1[1], buffer_rt_m1[0],
                1, 0, -1, 19.4852);

    test_one<Polygon, Polygon, Polygon>("buffer_rt_m2", buffer_rt_m2[0], buffer_rt_m2[1],
                1, 0, -1, 21.4853);
    test_one<Polygon, Polygon, Polygon>("buffer_rt_m2_rev", buffer_rt_m2[1], buffer_rt_m2[0],
                1, 0, -1, 21.4853);

    test_one<Polygon, Polygon, Polygon>("buffer_rt_q", buffer_rt_q[0], buffer_rt_q[1],
                1, 0, -1, 18.5710);
    test_one<Polygon, Polygon, Polygon>("buffer_rt_q_rev", buffer_rt_q[1], buffer_rt_q[0],
                1, 0, -1, 18.5710);
    test_one<Polygon, Polygon, Polygon>("buffer_rt_r", buffer_rt_r[0], buffer_rt_r[1],
                1, 0, -1, 21.07612);
    test_one<Polygon, Polygon, Polygon>("buffer_rt_r_rev", buffer_rt_r[1], buffer_rt_r[0],
                1, 0, -1, 21.07612);
    test_one<Polygon, Polygon, Polygon>("buffer_rt_t", buffer_rt_t[0], buffer_rt_t[1],
                1, 0, -1, 15.6569);
    test_one<Polygon, Polygon, Polygon>("buffer_rt_t_rev", buffer_rt_t[1], buffer_rt_t[0],
                1, 0, -1, 15.6569, ignore_validity_for_float);

    test_one<Polygon, Polygon, Polygon>("buffer_mp1", buffer_mp1[0], buffer_mp1[1],
                1, 0, -1, 22.815);

    {
        // Contains a self-intersection invalidity for ccw
        ut_settings settings;
        settings.set_test_validity(! is_ccw);
        test_one<Polygon, Polygon, Polygon>("buffer_mp2",
            buffer_mp2[0], buffer_mp2[1],
            1, -1, 217, 36.752837, settings);
    }

    test_one<Polygon, Polygon, Polygon>("mysql_21964079_1",
        mysql_21964079_1[0], mysql_21964079_1[1],
        2, 1, -1, 234.5);
    test_one<Polygon, Polygon, Polygon>("mysql_21964079_2",
        mysql_21964079_2[0], mysql_21964079_2[1],
        2, 1, -1, 112.0);

    test_one<Polygon, Polygon, Polygon>("mysql_23023665_1",
        mysql_23023665_1[0], mysql_23023665_1[1],
        2, 1, -1, 92.0 + 142.5);
    test_one<Polygon, Polygon, Polygon>("mysql_23023665_2",
        mysql_23023665_2[0], mysql_23023665_2[1],
        2, 1, -1, 96.0 + 16.0);
    test_one<Polygon, Polygon, Polygon>("mysql_23023665_3",
        mysql_23023665_3[0], mysql_23023665_3[1],
        2, 1, -1, 225.0 + 66.0);
    test_one<Polygon, Polygon, Polygon>("mysql_21964049",
        mysql_21964049[0], mysql_21964049[1],
        1, 1, -1, 220.5);
}

template <typename P, bool ClockWise>
void test_all()
{
    using polygon = bg::model::polygon<P, ClockWise>;
    using ring = bg::model::ring<P>;
    using box = bg::model::box<P>;

    test_areal<ring, polygon>();

#if ! defined(BOOST_GEOMETRY_TEST_ONLY_ONE_TYPE)
    // Open
    test_areal<bg::model::ring<P, true, false>, bg::model::polygon<P, true, false> >();

    // Counter clockwise
    test_areal<bg::model::ring<P, false>, bg::model::polygon<P, false> >();

    // Counter clockwise and open
    test_areal<bg::model::ring<P, false, false>, bg::model::polygon<P, false, false> >();
#endif

    test_one<polygon, box, polygon>("box_ring", example_box, example_ring,
        1, 1, 15, 6.38875);

    test_one<polygon, box, polygon>("box_poly", example_box, example_polygon,
        1, 3, 23, 6.30983);


    test_one<polygon, box, polygon>("box_poly1", example_box,
            "POLYGON((3.4 2,4.1 3,5.3 2.6,5.4 1.2,4.9 0.8,2.9 0.7,2 1.3,2.4 1.7,2.8 1.8,3.4 1.2,3.7 1.6,3.4 2))",
                1, 1, 15, 6.38875);
    test_one<polygon, box, polygon>("box_poly2", example_box,
            "POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2,3.7 1.6,3.4 2,4.1 2.5,5.3 2.5,5.4 1.2,4.9 0.8,2.9 0.7,2 1.3))",
                1, 1, 15, 5.93625);

    // 3: see areal

    test_one<polygon, box, polygon>("box_poly4", example_box,
            "POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2,3.7 1.6,3.4 2,4.1 2.5,4.5 2.5,4.5 1.2,4.9 0.8,2.9 0.7,2 1.3))",
                1, 1, 15, 4.651245);

    test_one<polygon, box, polygon>("box_poly5", example_box,
            "POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2,3.7 1.6,3.4 2,4.1 2.5,4.5 2.5,4.5 2.3,5.0 2.3,5.0 2.1,4.5 2.1,4.5 1.9,4.0 1.9,4.5 1.2,4.9 0.8,2.9 0.7,2 1.3))",
                1, 1, 21, 4.7191);

    test_one<polygon, box, polygon>("box_poly6", example_box,
            "POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2,3.7 1.6,3.4 2,4.1 2.5,4.5 1.2,2.9 0.7,2 1.3))",
                1, 1, 15, 4.2174);

    test_one<polygon, box, polygon>("box_poly7", example_box,
            "POLYGON((2 1.3,2.4 1.7,2.8 1.8,3.4 1.2,3.7 1.6,3.4 2,4.0 3.0,5.0 2.0,2.9 0.7,2 1.3))",
                1, 1, 17, 4.270554);

    test_one<polygon, box, polygon>("box_poly8", "box(0 0, 3 3)",
            "POLYGON((2 2, 1 4, 2 4, 3 3, 2 2))",
                1, 0, 8, 10.25);
}


int test_main(int, char* [])
{
    BoostGeometryWriteTestConfiguration();
    test_all<bg::model::d2::point_xy<default_test_type>, true>();

#if ! defined(BOOST_GEOMETRY_TEST_ONLY_ONE_ORDER)
    test_all<bg::model::d2::point_xy<default_test_type>, false>();
#endif

#if ! defined(BOOST_GEOMETRY_TEST_ONLY_ONE_TYPE)
    test_all<bg::model::d2::point_xy<float>, true>();
    test_all<bg::model::d2::point_xy<long double>, true>();
    test_all<bg::model::d2::point_xy<mp_test_type>, true>();
#endif

    return 0;
}
