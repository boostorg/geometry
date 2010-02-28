#ifndef BOOST_GEOMETRY_TEST_OVERLAY_CASES_HPP
#define BOOST_GEOMETRY_TEST_OVERLAY_CASES_HPP

// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <string>

// See powerpoint

static std::string case_1[2] = {
        "POLYGON((0 1,2 5,5 3,0 1))",
        "POLYGON((3 0,0 3,4 5,3 0))" };

static std::string case_2[2] = {
        "POLYGON((0 2,2 5,5 3,3 0,0 2))",
        "POLYGON((0 4,4 5,5 1,1 0,0 4))" };

static std::string case_3[2] = {
        "POLYGON((1 2,2 4,4 3,3 1,1 2))",
        "POLYGON((1 1,1 4,4 4,4 1,1 1))" };


static std::string case_4[2] = {
        "POLYGON((0 2,2 5,5 3,3 0,0 2))",
        "POLYGON((0 4,4 5,5 1,1 0,0 4),(1 1,2 1,2 2,1 2,1 1),(3 3,4 3,4 4,3 4,3 3))" };

static std::string case_5[2] = {
        "POLYGON((0 4,1 5,2.5 4,3.5 5,5 4,4 3,5 2,4 1,3.5 0,3 1,2.5 0,2 1,1.5 0,1 3.5,0.25 3,1 2.5,0 2,0 4))",
        "POLYGON((0.5 0.5,0.5 4.5,4.5 4.5,4.5 0.5,0.5 0.5))"
        };

static std::string case_6[2] = {
        "POLYGON((0 1,2 5,5 2,0 1))",
        "POLYGON((1 3,2 5,5 2,1 3))" };

static std::string case_7[2] = {
        "POLYGON((0 1,1 3,5 2,0 1))",
        "POLYGON((1 3,2 5,5 2,1 3))" };

static std::string case_8[2] = {
        "POLYGON((0 0,0 4,4 0,0 0))",
        "POLYGON((1 3,4 4,3 1,1 3))" };

static std::string case_9[2] = {
        "POLYGON((0 0,0 4,4 0,0 0))",
        "POLYGON((2 2,2 4,5 4,2 2))" };

static std::string case_10[2] = {
        "POLYGON((0 0,2 4,4 0,0 0))",
        "POLYGON((1 2,2 5,3 2,2 4,1 2))" };

static std::string case_11[2] = {
        "POLYGON((0 0,0 4,4 0,0 0))",
        "POLYGON((1 2,2 2,2 1,1 1))" };

static std::string case_12[2] = {
        "POLYGON((0 1,1 4,2 2,3 4,4 1,0 1))",
        "POLYGON((0 3,2 4,4 3,0 3))" };

static std::string case_13[2] = {
        "POLYGON((0 1,1 2,3 0,0 1))",
        "POLYGON((0 3,2 3,2 1,0 3))" };

static std::string case_14[2] = {
        "POLYGON((0 0,0 4,2 2,4 0,0 0))",
        "POLYGON((1 3,4 4,3 1,1 3))" };

static std::string case_15[2] = {
        "POLYGON((0 0,0 4,1.5 2.5,2.5 1.5,4 0,0 0))",
        "POLYGON((1 3,4 4,3 1,1 3))" };

static std::string case_16[2] = {
        "POLYGON((1 2,3 4,2 1,1 2))",
        "POLYGON((1 0,1 2,2 1,3 4,2 3,3 5,4 0,1 0))" };

static std::string case_17[2] = {
        "POLYGON((0 0,0 4,4 0,0 0))",
        "POLYGON((1 1,1 3,3 1,1 1))" };

static std::string case_18[2] = {
        "POLYGON((0 0,0 4,1.5 2.5,2.5 1.5,4 0,0 0))",
        "POLYGON((1 1,1 3,3 1,1 1))" };

static std::string case_19[2] = {
        "POLYGON((2 0,1 3,5 2,2 0))",
        "POLYGON((1 3,2 5,5 2,1 3))" };

static std::string case_20[2] = {
        "POLYGON((2 0,1 3,5 2,2 0))",
        "POLYGON((2 0,1 3,5 2,2 0))" };

static std::string case_21[2] = {
        "POLYGON((1 0,0 5,5 2,1 0),(2 1,3 2,1 3,2 1))",
        "POLYGON((2 1,1 3,3 2,2 1))" };

static std::string case_22[2] = {
        "POLYGON((1 0,0 3,4 2,1 0))",
        "POLYGON((2 4,5 5,4 2,2 4))" };

static std::string case_23[2] = {
        "POLYGON((1 0,0 3,4 2,1 0))",
        "POLYGON((3 3,4 2,1 1,3 3))" };

static std::string case_24[2] = {
        "POLYGON((1 0,0 3,4 2,1 0))",
        "POLYGON((2 2,4 2,1 1,2 2))" };

static std::string case_25[2] = {
        "POLYGON((1 0,0 3,4 2,1 0))",
        "POLYGON((3 1,4 2,5 0,3 1))" };

static std::string case_26[2] = {
        "POLYGON((1 0,0 3,4 2,1 0))",
        "POLYGON((3 1,4 2,5 4,4 0,3 1))" };

static std::string case_27[2] = {
        "POLYGON((1 0,0 3,4 2,1 0))",
        "POLYGON((1 1,4 2,5 0,1 1))" };

static std::string case_28[2] = {
        "POLYGON((1 0,0 3,4 2,1 0))",
        "POLYGON((1 1,4 2,5 4,5 0,1 1))" };

static std::string case_29[2] = {
        "POLYGON((1 0,0 3,4 2,1 0))",
        "POLYGON((4 4,5 1,4 2,1 1,4 4))" };

static std::string case_30[2] = {
        "POLYGON((1 0,0 3,2 2,3 3,1 0))",
        "POLYGON((1 2,2 2,2 4,4 3,3 1,1 2))" };

static std::string case_31[2] = {
        "POLYGON((0 0,0 3,2 2,0 0))",
        "POLYGON((4 1,2 2,3 3,4 1))" };

static std::string case_32[2] = {
        "POLYGON((0 0,0 3,2 2,0 0))",
        "POLYGON((4 1,3 0,2 2,4 1))" };

static std::string case_33[2] = {
        "POLYGON((0 0,0 3,2 2,0 0))",
        "POLYGON((3 3,3 0,2 2,3 3))" };

static std::string case_34[2] = {
        "POLYGON((2 0,0 3,4 2,2 0))",
        "POLYGON((2 1,4 2,5 3,4 1,2 1))" };

static std::string case_35[2] = {
        "POLYGON((1 0,0 3,5 3,4 2,5 1,1 0))",
        "POLYGON((2 2,4 2,4 1,2 2))" };

static std::string case_36[2] = {
        "POLYGON((1 0,0 3,4 2,1 0))",
        "POLYGON((1 5,5 5,4 2,3 3,2 1,1 2,1 5))" };

static std::string case_37[2] = {
        "POLYGON((4 0,2 2,5 5,4 0))",
        "POLYGON((0 2,4 3,2 2,4 1,0 2))" };

static std::string case_38[2] = {
        "POLYGON((4 0,3 3,2 2,4 5,5 2,4 0))",
        "POLYGON((1 1,1 3,4 4,3 3,4 2,1 1))" };

static std::string case_39[2] = {
        "POLYGON((0 4,4 8,7 7,8 4,5 3,4 0,0 4))",
        "POLYGON((0 4,4 8,5 5,8 4,7 1,4 0,0 4))" };

static std::string case_40[2] = {
        "POLYGON((0 1,1 2,2 1,3 2,4 1,5 2,4 0,2 0,0 1))",
        "POLYGON((0 3,2 4,4 4,5 2,4 3,3 2,2 3,1 2,0 3))" };

static std::string case_41[2] = {
        "POLYGON((3 1,1 4,3 3.5,5 3,3 1))",
        "POLYGON((3 1,1 4,5 3,4 2,3 1))" };

static std::string case_42[2] = {
        "POLYGON((3 1,1 4,3 3.5,5 3,5 3,3 1))",
        "POLYGON((3 1,1 4,1 4,5 3,4 2,3 1))" };

// keyholing
static std::string case_43[2] = {
        "POLYGON((1 2,1 1,2 1,2 2.25,3 2.25,3 0,0 0,0 3,3 3,2.75 2,1 2))",
        "POLYGON((1.5 1.5,1.5 2.5,2.5 2.5,2.5 1.5,1.5 1.5))" };
// self-tangency

static std::string case_44[2] = {
        "POLYGON((0 0,0 3,3 3,3 2,0 1.5,3 1,3 0,0 0))",
        "POLYGON((1 1,1 2,2 2,2 1,1 1))" };
// self-intersection

static std::string case_45[2] = {
        "POLYGON((0.5 0,0.5 3,3 3,3 2,0 1.5,3 1,3 0,0.5 0))",
        "POLYGON((1 1,1 2,2 2,2 1,1 1))" };
// self-tangent

static std::string case_46[2] = {
        "POLYGON((2 0,2 3,5 3,5 2,2 1.5,5 1,5 0,2 0))",
        "POLYGON((1.5 1,1.5 2,2.5 2,2.5 1,1.5 1))" };
// self-tangent

static std::string case_47[2] = {
        "POLYGON((2.5 0,2.5 3,5 3,5 2,2 1.5,5 1,5 0,2.5 0))",
        "POLYGON((2 1,2 2,3 2,3 1,2 1))" };

static std::string case_49[2] = {
        "POLYGON((1 0,1 1,2 2,3 3,4 3,4 0,1 0))",
        "POLYGON((0 2,2 5,5 2,5 0,2 0,2 2,0 2))" };


// 50, collinear opposite
static std::string case_50[2] = {
        "POLYGON((0 0,0 3,2 3,4 3,4 0,0 0))", 
        "POLYGON((0 3,0 5,5 5,5 0,4 0,4 3,2 3,0 3))" };

// 51, fit
static std::string case_51[2] = {
        "POLYGON((0 0,0 5,5 5,5 0,0 0),(4 1,3 4,1 2,4 1))", 
        "POLYGON((1 2,3 4,4 1,1 2))" };

// 52, partially equal, touching each other
static std::string case_52[2] = {
        "POLYGON((0 0,1 4,3 1,4 4,1 4,5 5,5 0,0 0))",
        "POLYGON((0 0,1 4,3 1,4 4,1 4,5 5,4 1,0 0))" };

// 53, wrapping a box, self-touching 
static std::string case_53[3] = {
        "POLYGON((2 2,2 3,3 3,3 2,2 2))",
        // a: self-tangent ext. ring, ST_IsValid=false
        "POLYGON((0 2,0 5,5 5,5 0,2 0,2 2,3 2,3 1,4 1,4 4,1 4,1 3,2 3,2 2,0 2))", 
        // b: tangent ext/int ring, ST_IsValid=true
        "POLYGON((0 2,0 5,5 5,5 0,2 0,2 2,0 2),(1 3,2 3,2 2,3 2,3 1,4 1,4 4,1 4,1 3))" 
    };

static std::string case_54[4] = {
        // a: self-tangent ext. ring
        "POLYGON((2 2,2 3,4 3,4 0,1 0,1 2,2 2,2 1,3 1,3 2,2 2))", 
        // b: tangent ext/int ring
        "POLYGON((2 2,2 3,4 3,4 0,1 0,1 2,2 2),(2 1,3 1,3 2,2 2,2 1))", 
        case_53[1],
        case_53[2]
        // other like in case53
    };

static std::string case_55[2] = {
        //"POLYGON((2 2,3 3,4 3,4 0,1 0,1 1,2 2),(2 1,3 1,3 2,2 2,2 1))", 
        //"POLYGON((0 2,0 5,5 5,5 0,2 0,2 2,0 2),(1 3,2 2,3 1,4 1,4 4,1 4,1 3))" 
        // B, not a 4-fold intersection
        //"POLYGON((2 2,3 3,4 3,4 0,1 0,1 1,2 2),(2 1,3 1,3 2,2.0 1.9,2 1))", 
        //"POLYGON((0 2,0 5,5 5,5 0,2 0,2 2,0 2),(1 3,2.1 2.1,3 1,4 1,4 4,1 4,1 3))" 
        // C, not a 4-fold intersection, 
        "POLYGON((2 2,3 3,4 3,4 0,1 0,1 1,2 2),(2 1,3 1,3 2,2.0 1.9,2 1))", 
        "POLYGON((0 2,0 5,5 5,5 0,2 0,2 2,0 2),(1 3,1.9 1.9,3 1,4 1,4 4,1 4,1 3))" 
    };

static std::string case_56[2] = {
        "POLYGON((0 0,0 1,2 3,3 0,4 2,5 0,0 0))", 
        //"POLYGON((0 -1,0 1,2 3,3 0,4 2,5 -1,0 -1))",
        //"POLYGON((0 1,0 4,5 4,5 0,3 0,4 1,4 3,2 3,2 1,3 0,0 0))" 
        "POLYGON((1 0,1 4,5 4,5 0,3 0,4 1,4 3,2 3,2 1,3 0,1 0))" 
    };

static std::string case_57[2] = {
        case_56[0],
        //"POLYGON((0 2,4 5,5 1,0 2))" 
        //"POLYGON((0 -1,0 1,2 3,3 0,4 2,5 -1,0 -1))",
        //"POLYGON((0 0,0 1,2 3,3 0,4 2,6 0,0 0))", 
        "POLYGON((0 0,4 5,5 0,0 0))" 

    };


static std::string case_many_situations[2] = {
        "POLYGON((2 6,2 14,10 18,18 14,18 6,16 5,14 4,12 3,10 2,8 3,6 4,4 5,2 6))",
        "POLYGON((2 6,2 7,2 8,2 9,2 10,2 11,2 12,1 14"
            ",4 14,4 15,7 15,7 18,8 17,9 20,10 18"
            ",10 15.5,11 17.5,12 17,11 15,12 14.5,13 16.5,14 18.5,15 18,14 16,15 15.5,16 17.5,18 14"
            ",19 13,17 11,19 9,19 7"
            ",17 5,16 5,15 4.5,13 3.5,11 2.5,10 2"
            ",8 3,6 4,4 5,2 6))" };

static std::string collinear_overlaps[2] = {
        "POLYGON((0 2,0 4,2 4,2 5,0 5,0 7,2 7,2 8,6 8,6 7,8 7,8 5,6 5,6 4,8 4,8 2,6 2,6 1,2 1,2 2,0 2))",
        "POLYGON((3 0,3 2,1 2,1 3,3 3,3 4,1 4,1 5,3 5,3 6,1 6,1 7,3 7,3 9,5 9,5 7,7 7,7 6,5 6,5 5,7 5,7 4,5 4,5 3,7 3,7 2,5 2,5 0,3 0))" };


static std::string line_line1[2] = {
        "LINESTRING(0 1,2 5,5 3)", "LINESTRING(3 0,0 3,4 5)"};



// Pies
static std::string pie_23_16_16[2] = {
    "POLYGON((2500 2500,2500 3875,2855 3828,3187 3690,3472 3472,3690 3187,3828 2855,3875 2500,3828 2144,3690 1812,3472 1527,3187 1309,2855 1171,2499 1125,2144 1171,1812 1309,1527 1527,1309 1812,1171 2144,1125 2499,1171 2855,1309 3187,1527 3472,1812 3690,2500 2500))",
    "POLYGON((2500 2500,1525 1937,1413 2208,1375 2499,1413 2791,1525 3062,1704 3295,1937 3474,2208 3586,2499 3625,2791 3586,3062 3474,3295 3295,3474 3062,3586 2791,3625 2500,3586 2208,2500 2500))"
    };

static std::string pie_16_4_12[2] = {
    "POLYGON((2500 2500,2500 3875,2855 3828,3187 3690,3472 3472,3690 3187,3828 2855,3875 2500,3828 2144,3690 1812,3472 1527,3187 1309,2855 1171,2499 1125,2144 1171,1812 1309,1527 1527,2500 2500))",
    "POLYGON((2500 2500,2500 1375,2208 1413,1937 1525,1704 1704,2500 2500))"
    };

static std::string pie_4_13_15[2] = {
    "POLYGON((2500 2500,2500 3875,2855 3828,3187 3690,3472 3472,2500 2500))",
    "POLYGON((2500 2500,1704 1704,1525 1937,1413 2208,1375 2499,1413 2791,1525 3062,1704 3295,1937 3474,2208 3586,2499 3625,2791 3586,3062 3474,3295 3295,2500 2500))"
    };

static std::string pie_16_2_15_0[2] = {
    "POLYGON((2500 2500,2500 3875,2855 3828,3187 3690,3472 3472,3690 3187,3828 2855,3875 2500,3828 2144,3690 1812,3472 1527,3187 1309,2855 1171,2499 1125,2144 1171,1812 1309,1527 1527,2500 2500))",
    "POLYGON((2500 2500,1704 1704,1525 1937,2500 2500))"
    };


// Shifted pies
static std::string pie_20_20_7_100[2] = {
    "POLYGON((2500 2500,2500 3875,2855 3828,3187 3690,3472 3472,3690 3187,3828 2855,3875 2500,3828 2144,3690 1812,3472 1527,3187 1309,2855 1171,2499 1125,2144 1171,1812 1309,1527 1527,1309 1812,1171 2144,1125 2499,1171 2855,2500 2500))",
    "POLYGON((2500 2600,3586 2308,3474 2037,3295 1804,3062 1625,2791 1513,2499 1475,2208 1513,1937 1625,1704 1804,1525 2037,1413 2308,1375 2600,1413 2891,1525 3162,1704 3395,1937 3574,2208 3686,2499 3725,2791 3686,3062 3574,2500 2600))"
    };

static std::string pie_23_23_3_2000[2] = {
    "POLYGON((2500 2500,2500 3875,2855 3828,3187 3690,3472 3472,3690 3187,3828 2855,3875 2500,3828 2144,3690 1812,3472 1527,3187 1309,2855 1171,2499 1125,2144 1171,1812 1309,1527 1527,1309 1812,1171 2144,1125 2499,1171 2855,1309 3187,1527 3472,1812 3690,2500 2500))",
    "POLYGON((2500 3500,3295 4295,3474 4062,3586 3791,3625 3500,3586 3208,3474 2937,3295 2704,3062 2525,2791 2413,2499 2375,2208 2413,1937 2525,1704 2704,1525 2937,1413 3208,1375 3500,1413 3791,1525 4062,1704 4295,1937 4474,2208 4586,2499 4625,2791 4586,2500 3500))"
    };

static std::string pie_23_21_12_500[2] = {
    "POLYGON((2500 2500,2500 3875,2855 3828,3187 3690,3472 3472,3690 3187,3828 2855,3875 2500,3828 2144,3690 1812,3472 1527,3187 1309,2855 1171,2499 1125,2144 1171,1812 1309,1527 1527,1309 1812,1171 2144,1125 2499,1171 2855,1309 3187,1527 3472,1812 3690,2500 2500))",
    "POLYGON((2500 3000,2500 1875,2208 1913,1937 2025,1704 2204,1525 2437,1413 2708,1375 3000,1413 3291,1525 3562,1704 3795,1937 3974,2208 4086,2499 4125,2791 4086,3062 3974,3295 3795,3474 3562,3586 3291,3625 3000,3586 2708,3474 2437,2500 3000))"
    };



static std::string ticket_17[2] = {
    "POLYGON ((-122.28139163 37.37319149,-122.28100699 37.37273669,-122.28002186 37.37303123,-122.27979681 37.37290072,-122.28007349 37.37240493,-122.27977334 37.37220360,-122.27819720 37.37288580,-122.27714184 37.37275161,-122.27678628 37.37253167,-122.27766437 37.37180973,-122.27804382 37.37121453,-122.27687664 37.37101354,-122.27645829 37.37203386,-122.27604423 37.37249110,-122.27632234 37.37343339,-122.27760980 37.37391082,-122.27812478 37.37800320,-122.26117222 37.39121007,-122.25572289 37.39566631,-122.25547269 37.39564971,-122.25366304 37.39552993,-122.24919976 37.39580268,-122.24417933 37.39366907,-122.24051443 37.39094143,-122.23246277 37.38100418,-122.23606766 37.38141338,-122.24001587 37.37738940,-122.23666848 37.37609347,-122.23057450 37.37882170,-122.22679803 37.37807143,-122.22525727 37.37448817,-122.22523229 37.37443000,-122.23083199 37.37609347,-122.23033486 37.37777891,-122.23169030 37.37732117,-122.23229178 37.37709687,-122.23237761 37.37631249,-122.23297776 37.37438834,-122.23872850 37.37165986,-122.24044511 37.36934068,-122.24671067 37.36865847,-122.24825570 37.36981819,-122.25151719 37.36947713,-122.25357721 37.36756706,-122.26001451 37.36579354,-122.25615213 37.36545239,-122.25486458 37.36245083,-122.25357721 37.36108651,-122.25194642 37.36013139,-122.24885652 37.35958557,-122.24911401 37.35849399,-122.25357721 37.35808470,-122.25675286 37.35897159,-122.25855539 37.35753887,-122.26181687 37.35828939,-122.26713837 37.35897159,-122.26782510 37.36108651,-122.26662339 37.36456559,-122.27288911 37.36722601,-122.27366159 37.36531602,-122.27168740 37.36470213,-122.27391900 37.36374701,-122.27074326 37.36245083,-122.27134408 37.35951742,-122.27426240 37.36135926,-122.27709482 37.36115474,-122.27966974 37.36231438,-122.27958391 37.36463382,-122.27572152 37.36463382,-122.27563569 37.36524779,-122.27700899 37.36593000,-122.27709482 37.36763529,-122.27554978 37.36838573,-122.27667254 37.36931478,-122.27677932 37.36932073,-122.27769362 37.36853987,-122.27942490 37.36830803,-122.28178776 37.36677917,-122.28509559 37.36443500,-122.28845129 37.36413744,-122.29194403 37.36695946,-122.29382577 37.36726817,-122.29600414 37.36898512,-122.29733083 37.36995398,-122.29593239 37.37141436,-122.29416649 37.37075898,-122.29325026 37.37108436,-122.29652910 37.37311697,-122.29584237 37.37374461,-122.29537583 37.37573372,-122.29487677 37.37752502,-122.30923212 37.37593011,-122.31122484 37.38230086,-122.31467994 37.38092472,-122.31715663 37.38252181,-122.32307970 37.38166978,-122.31985618 37.37667694,-122.32210304 37.37580220,-122.32581446 37.37589532,-122.32401730 37.37331839,-122.32960417 37.37189020,-122.33465527 37.37331906,-122.33425328 37.37623680,-122.33620676 37.37726132,-122.33397986 37.37822382,-122.33358918 37.38036590,-122.33202637 37.37986918,-122.33147954 37.38101784,-122.33394080 37.38198017,-122.33545239 37.38587943,-122.33478058 37.38785697,-122.33386050 37.38723721,-122.33350041 37.38571137,-122.33122003 37.38548891,-122.33140008 37.38650606,-122.33366042 37.38817490,-122.33244019 37.39157602,-122.33298157 37.39419201,-122.33164013 37.39477028,-122.33202017 37.39518351,-122.33358038 37.39499282,-122.33376050 37.39597811,-122.33550067 37.39734478,-122.33556069 37.39481797,-122.33344040 37.39292676,-122.33638094 37.38892189,-122.34240644 37.38852719,-122.34906293 37.38726898,-122.35072321 37.39338769,-122.34910291 37.39445252,-122.34796272 37.39410291,-122.34449043 37.39640534,-122.34500223 37.39729709,-122.34936291 37.39670910,-122.35098322 37.39531066,-122.35364623 37.39554510,-122.35434369 37.39612111,-122.35798429 37.39600988,-122.35768430 37.39478621,-122.36334519 37.39206871,-122.36604726 37.39203267,-122.36778592 37.39335592,-122.36518870 37.40022011,-122.36554552 37.40247752,-122.36370519 37.40331974,-122.36270506 37.40530591,-122.36320512 37.40670418,-122.36149849 37.40851392,-122.36730580 37.41054938,-122.37263720 37.41378932,-122.37161871 37.42076600,-122.36566153 37.42006292,-122.36520547 37.42742106,-122.37165953 37.43661157,-122.35943972 37.44459022,-122.35356359 37.44600810,-122.33792254 37.45796329,-122.35228518 37.47478091,-122.35127080 37.48181199,-122.34867342 37.48487322,-122.34359717 37.48801082,-122.33388431 37.48677650,-122.33142321 37.48429747,-122.32929580 37.48473149,-122.32609609 37.48291144,-122.32344850 37.48228229,-122.31924364 37.48410234,-122.31677299 37.48114051,-122.31431751 37.47848973,-122.31259201 37.47682190,-122.31515972 37.47568196,-122.31691389 37.47360309,-122.31292494 37.46960081,-122.31130153 37.46937743,-122.30889894 37.47124987,-122.30612839 37.47011613,-122.30149630 37.46568378,-122.30064277 37.46363784,-122.29283821 37.45922376,-122.28630141 37.45415497,-122.28883099 37.44629920,-122.28316717 37.44197138,-122.27554148 37.42297597,-122.25597410 37.40553692,-122.25196579 37.40129593,-122.25012043 37.40049143,-122.24823207 37.39897758,-122.24754551 37.39740941,-122.24778582 37.39621607,-122.24934787 37.39599102,-122.25005170 37.39871849,-122.25222328 37.39863668,-122.25342491 37.39737529,-122.25520162 37.39667289,-122.25528737 37.39522726,-122.27747460 37.37809616,-122.27977493 37.37858717,-122.28157729 37.37920106,-122.28322534 37.37952846,-122.28416939 37.38092656,-122.28621223 37.37984219,-122.28638389 37.37613857,-122.28382607 37.37843722,-122.27930278 37.37718220,-122.28196361 37.37652740,-122.28295058 37.37568167,-122.28216101 37.37523148,-122.28114822 37.37543608,-122.27934569 37.37528613,-122.27996369 37.37448121,-122.28104521 37.37454944,-122.28185197 37.37422883,-122.28290767 37.37474038,-122.28376597 37.37467224,-122.28428104 37.37399012,-122.28402346 37.37338989,-122.28610922 37.37364914,-122.28651264 37.37327388,-122.28672722 37.37207343,-122.28628398 37.37205448,-122.28574460 37.37166682,-122.28479711 37.37200981,-122.28327731 37.37137228,-122.28285511 37.37100700,-122.28279409 37.37125669,-122.28315527 37.37173756,-122.28321872 37.37220569,-122.28187007 37.37231918,-122.28193109 37.37294908,-122.28139163 37.37319149))",
    "BOX(-122.280 37.377,-122.277 37.379)"};

/*
static std::string isovist[2] =
    {
    "POLYGON((37.29449462890625 1.7902572154998779,  46.296027072709599 -2.4984308554828116,  45.389434814453125 -4.5143837928771973,  47.585065917176543 -6.1314922196594779,  46.523914387974358 -8.5152102535033496,  42.699958801269531 -4.4278755187988281,  42.577877044677734 -4.4900407791137695,  42.577911376953125 -4.4901103973388672,  40.758884429931641 -5.418975830078125,  40.6978759765625 -5.4500408172607422,  41.590042114257813 -7.2021245956420898,  57.297810222148939 -37.546793343968417,  50.974888957147442 -30.277285722290763,  37.140213012695313 1.3446992635726929,  37.000419616699219 1.664225697517395,  37.29449462890625 1.7902572154998779))",
    "POLYGON((43.644271850585938 0.96149998903274536,43.764598846435547 0.93951499462127686,49.071769542946825 0.61489892713413252,48.43512638981781 -0.81299959072453376,47.830955505371094 -0.69758313894271851,47.263670054709685 -1.784876824891044,46.695858001708984 -1.6093428134918213,45.389434814453125 -4.5143837928771973,47.604561877161387 -6.087697464505224,46.559533858616469 -8.435196445683264,42.699958801269531 -4.4278755187988281,42.577877044677734 -4.4900407791137695,42.577911376953125 -4.4901103973388672,40.758884429931641 -5.418975830078125,40.6978759765625 -5.4500408172607422,41.590042114257813 -7.2021245956420898,57.524304765518266 -37.807195733984784,41.988733475572282 -19.945838749437218,41.821544647216797 -19.211688995361328,40.800632476806641 -17.208097457885742,39.966808319091797 -17.625011444091797,38.823680877685547 -16.296066284179688,37.326129913330078 -17.190576553344727,35.963497161865234 -15.476018905639648,35.656356811523438 -15.66030216217041,34.931102752685547 -16.223842620849609,34.634240447128811 -15.85007183479255,34.886280059814453 -14.120697975158691,34.658355712890625 -13.81736946105957,34.328716278076172 -13.992490768432617,33.598796844482422 -14.546377182006836,33.164891643669634 -14.000060288415174,33.566280364990234 -12.450697898864746,33.339523315429688 -12.147735595703125,32.998821258544922 -12.323249816894531,32.274600982666016 -12.879127502441406,31.682494778186321 -12.133624901803865,32.226280212402344 -10.790698051452637,32.000633239746094 -10.488097190856934,31.669155120849609 -10.653837203979492,30.947774887084961 -11.208560943603516,30.207040612748258 -10.275926149505661,30.896280288696289 -9.1206979751586914,30.670633316040039 -8.8180980682373047,30.339155197143555 -8.9838371276855469,29.619997024536133 -9.5368013381958008,29.135100397190627 -8.9262827849488211,32.718830108642578 -4.3281683921813965,32.708168029785156 -2.3611698150634766,32.708126068115234 -2.3611700534820557,32.708126068115234 -2.3611266613006592,30.501169204711914 -2.3718316555023193,27.069889344709196 -4.2926591211028242,26.472516656201325 -3.5380830513658776,36.954700469970703 1.2597870826721191,37.140213012695313 1.3446992635726929,37.000419616699219 1.664225697517395,37.29449462890625 1.7902572154998779,37.43402099609375 1.470055103302002,51.370888500897557 7.4163459734570729,51.20102152843122 7.1738039562841562,42.721500396728516 3.6584999561309814,42.721500396728516 2.2342472076416016,42.399410247802734 1.4956772327423096,43.644271850585938 0.96149998903274536))"
    };
*/

#endif // BOOST_GEOMETRY_TEST_OVERLAY_CASES_HPP
