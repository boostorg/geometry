// Boost.Geometry

// Copyright (c) 2025 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_BUFFER_CASES_HPP
#define BOOST_GEOMETRY_TEST_BUFFER_CASES_HPP

#include <string>

// The "w" series of recursive_polygons_buffer, which are or were failing for traversing through the graph
// using the biconnected components.

// Needs aligning position codes in assign_clustered_counts
static std::string const rt_w1
    = "MULTIPOLYGON(((2 0,2 1,3 1,3 0,2 0)),((4 2,4 3,5 2,4 2)),((1 6,0 6,0 7,1 6)),((1 6,2 5,1 5,1 6)))";

// Caused by a wrong block_q in get_turn_info (resulting in ix i/o iu)
static std::string const rt_w2
    = "MULTIPOLYGON(((6 3,6 4,7 4,6 3)),((6 3,5 2,5 3,6 3)))";

// Goes wrong either way
static std::string const rt_w3
    = "MULTIPOLYGON(((8 3,8 4,9 4,9 3,8 3)),((7 5,7 6,8 6,7 5)),((5 5,4 4,4 5,5 5)),((5 5,6 5,5 4,5 5)),((0 7,0 8,1 8,0 7)),((2 2,2 3,3 3,2 2)),((2 8,3 8,3 7,3 6,2 6,2 8)))";

// Needs fix in arrival handling - it had weird turn info near node 3, ux, while both continue.
static std::string const rt_w4
    = "MULTIPOLYGON(((4 6,4 7,5 6,4 6)),((8 0,8 1,9 1,8 0)),((1 6,1 7,2 7,1 6)),((0 2,0 3,1 2,0 2)),((5 2,6 3,6 2,5 2)),((5 2,5 1,4 1,4 2,5 2)),((2 9,3 8,2 8,1 8,1 9,2 9)))";

// Having a big clusters with all position codes, and two interior rings
static std::string const rt_w5
    = "MULTIPOLYGON(((2 1,2 2,3 2,3 1,2 1)),((2 6,2 7,3 7,3 6,2 6)),((3 4,3 5,4 5,4 4,3 4)),((5 3,5 2,4 2,5 3)),((5 3,5 4,6 3,5 3)),((0 2,1 1,0 0,0 2)),((0 8,1 8,1 7,0 7,0 8)),((0 5,1 5,2 5,1 4,0 3,0 5)),((7 1,6 1,6 2,7 1)),((7 1,8 0,7 0,7 1)),((8 6,7 6,7 8,8 8,8 6)),((6 8,6 7,5 7,5 8,6 8)),((6 1,5 0,5 1,6 1)),((7 8,7 9,8 9,7 8)),((6 4,7 3,6 3,6 4)),((6 4,6 5,7 5,7 4,6 4)),((7 6,6 6,6 7,7 6)),((5 8,4 7,4 8,5 8)),((5 8,5 9,6 9,5 8)),((7 4,8 3,7 3,7 4)),((8 3,8 4,9 4,9 3,8 3)))";

// Fixed regression
static std::string const rt_w6
    = "MULTIPOLYGON(((7 0,7 1,8 0,7 0)),((8 6,8 7,9 7,9 6,8 6)),((2 8,3 9,3 8,2 8)),((4 3,4 4,5 4,4 3)),((6 5,6 6,7 6,6 5)),((1 7,2 7,1 6,0 6,1 7)),((1 5,0 4,0 5,1 5)),((1 5,2 6,2 5,1 5)),((0 4,1 4,0 3,0 4)),((2 1,1 0,0 0,0 1,2 1)),((7 4,6 3,6 4,7 4)),((7 4,7 5,8 5,7 4)),((6 3,7 3,6 2,6 3)))";

// Fixed by first handling non-clustered turns
static std::string const rt_w7
    = "MULTIPOLYGON(((6 0,6 1,7 1,6 0)),((6 6,7 7,7 6,6 6)),((8 0,8 1,9 1,8 0)),((2 7,2 8,3 7,2 7)),((1 2,1 3,2 3,1 2)),((8 2,8 3,9 2,8 2)),((4 5,4 4,3 4,4 5)),((4 5,5 6,5 5,4 5)))";

//  Fixed select_collinear_target_edge
static std::string const rt_w8
    = "MULTIPOLYGON(((0 4,0 5,1 5,0 4)),((1 3,1 4,2 4,1 3)),((2 8,2 9,3 8,2 8)),((6 3,6 4,7 3,6 3)),((7 1,7 2,8 2,8 1,7 1)),((0 2,0 3,1 2,0 2)),((0 2,1 1,0 1,0 2)),((3 2,3 1,2 1,3 2)),((3 2,2 2,3 3,3 2)),((9 6,9 5,8 5,9 6)),((9 6,8 6,8 7,9 7,9 6)),((5 2,5 1,4 1,4 2,5 2)),((5 2,5 3,6 2,5 2)),((5 6,4 6,4 7,5 6)),((5 6,6 5,5 5,5 6)),((5 6,6 7,6 6,5 6)))";

// fixed by setting operations to blocked
static std::string const rt_w9
   = "MULTIPOLYGON(((3 8,4 9,4 8,3 8)),((4 2,4 4,5 3,5 2,4 2)),((4 2,4 1,3 1,3 2,4 2)),((2 4,2 5,3 5,3 4,3 3,2 3,2 2,1 2,1 3,2 4)),((8 8,7 8,7 9,8 8)),((8 8,9 8,8 7,8 8)),((1 2,0 2,0 3,1 2)),((4 6,3 6,3 7,4 6)),((4 6,5 6,5 5,4 5,4 6)),((8 7,8 6,7 6,8 7)),((8 6,9 7,9 6,8 6)))";

// fixed by position_code=3 in assign_clustered_counts
static std::string const rt_w10
    = "MULTIPOLYGON(((3 2,3 3,4 3,3 2)),((2 8,2 9,3 9,3 8,2 8)),((7 8,7 9,8 9,7 8)),((4 4,5 5,5 4,4 4)),((5 7,5 8,6 7,5 7)),((6 2,7 3,7 2,6 2)),((0 3,1 4,1 3,0 2,0 3)),((8 5,8 4,7 4,8 5)),((8 5,8 6,9 5,8 5)),((7 4,6 4,6 5,7 5,7 4)),((3 6,4 7,4 6,3 6)),((3 6,3 5,2 5,3 6)),((8 2,8 1,7 1,8 2)),((8 2,8 3,9 2,8 2)),((1 3,2 2,1 2,1 3)))";

// same as rt_w1 - also its configuration is similar.
static std::string const rt_w11
    = "MULTIPOLYGON(((2 8,2 9,3 9,3 8,2 8)),((6 7,6 8,7 8,7 7,6 7)),((5 0,5 1,6 1,6 0,5 0)),((4 2,4 3,5 2,4 2)),((4 2,4 1,3 1,3 2,4 2)),((1 6,0 6,0 7,1 6)),((1 6,2 5,1 5,1 6)),((4 1,4 0,3 0,4 1)))";

static std::string const rt_w12
    = "MULTIPOLYGON(((0 7,0 8,1 7,0 7)),((4 3,4 4,5 3,4 3)),((5 2,6 3,6 2,5 2)),((5 2,4 1,4 2,5 2)))";

// fixed by using continue with count-left 0
static std::string const rt_w13
    = "MULTIPOLYGON(((6 2,6 3,7 2,6 2)),((4 4,4 3,3 3,3 4,4 4)),((4 4,4 5,5 4,4 4)),((4 3,4 2,3 2,4 3)))";

// Caused by a wrong block_q in get_turn_info (resulting in ix i/o iu)
static std::string const rt_w14
    = "MULTIPOLYGON(((6 0,6 1,7 0,6 0)),((1 4,2 5,2 4,1 4)),((1 2,1 3,2 3,1 2)),((3 7,3 8,4 7,3 7)),((2 1,3 2,3 1,2 1)),((3 5,3 6,4 6,4 5,3 5)),((7 4,6 4,6 5,7 4)),((7 4,8 3,7 3,7 4)))";

// Would need calling assign_count before enrich. But it is now harmonized
// to be the same as union/intersection
static std::string const rt_w15
    = "MULTIPOLYGON(((3 8,3 9,4 9,4 8,3 8)),((0 8,1 9,1 8,0 8)),((1 6,1 7,2 7,2 6,1 6)),((7 7,8 8,8 7,7 7)),((4 3,3 3,3 4,4 3)),((4 3,5 2,4 2,4 3)),((1 5,2 5,2 4,2 3,1 3,1 4,0 4,1 5)),((6 1,7 0,6 0,6 1)),((6 1,5 1,6 2,6 1)),((7 0,7 1,8 0,7 0)),((2 1,3 1,3 0,1 0,2 1)),((2 1,2 2,3 2,2 1)))";

// fixed by ahead_side in cc
static std::string const rt_w16
    = "MULTIPOLYGON(((6 4,6 5,7 5,6 4)),((4 0,4 1,5 0,4 0)),((4 2,4 3,5 3,4 2)),((1 2,1 3,2 3,2 2,1 2)))";

// fixed by keeping set_both_verified
static std::string const rt_w17
    = "MULTIPOLYGON(((3 1,4 2,4 1,3 1)),((5 3,6 4,6 3,5 3)),((5 0,5 1,6 1,6 0,5 0)),((8 5,9 6,9 5,8 5)),((8 5,7 4,7 5,8 5)))";

static std::string const rt_w18
    = "MULTIPOLYGON(((4 4,4 5,5 4,4 4)),((5 6,6 7,6 6,5 6)),((5 1,4 1,4 2,5 3,5 1)),((7 6,7 7,8 7,7 6)),((0 6,1 6,1 5,1 4,0 4,0 6)),((1 8,2 7,1 7,1 8)),((1 8,2 9,2 8,1 8)),((1 6,1 7,2 6,1 6)),((7 3,7 2,6 2,7 3)),((7 3,7 4,8 4,8 3,7 3)),((3 2,3 1,2 1,3 2)),((3 2,2 2,2 3,3 3,3 2)),((5 8,5 7,4 7,4 8,5 8)),((5 8,6 9,6 8,5 8)))";


// Contains a cc turn (1) located wrongly.
// Reported at 1/1, but should be 0/1 - or it should have a segment id belonging to previous segment.
// Fixed by removing specific arriving handling.
static std::string const rt_w19
    = "MULTIPOLYGON(((3 8,3 9,4 9,3 8)),((3 4,4 5,4 4,3 4)),((7 0,7 1,8 1,7 0)),((8 5,9 6,9 5,8 5)),((2 7,3 7,3 6,2 6,2 7)),((7 4,8 3,7 3,6 3,7 4)))";

// REPORTED as #1359 - but somehow now fixed - issue was: start turn discarded while it should not be.
static std::string const rt_w20
    = "MULTIPOLYGON(((2 8,2 9,3 8,2 8)),((7 1,7 2,8 1,7 1)),((5 2,5 3,6 2,5 2)),((2 2,2 3,3 3,2 2)),((1 3,1 4,2 4,1 3)),((6 4,6 5,7 4,6 4)),((9 8,8 8,9 9,9 8)),((9 8,9 7,8 7,9 8)))";

// reproduction of wrong order (i/u)
static std::string const rt_w21
    = "MULTIPOLYGON(((2 8,2 9,3 8,2 8)),((4 6,4 7,5 6,4 6)),((4 2,5 2,4 1,4 2)))";

// Needs a spike. Also, it misses a small triangle, so area is not completely correct
static std::string const rt_w22
    = "MULTIPOLYGON(((6 3,6 4,7 4,7 3,6 3)),((6 1,6 2,7 2,6 1)),((2 8,2 9,3 9,2 8)),((4 7,5 8,5 7,4 7)),((2 1,2 0,1 0,2 1)),((2 1,2 2,3 2,3 1,2 1)),((3 3,2 2,2 3,3 3)),((3 3,3 4,4 3,3 3)),((0 5,1 4,1 3,0 3,0 5)),((8 6,9 6,9 5,8 5,8 6)),((8 6,7 6,6 6,7 7,8 6)),((1 3,1 2,0 2,1 3)),((4 3,5 3,4 2,4 3)),((4 2,5 1,4 1,4 2)))";

// Caused by spike handling.
static std::string const rt_w23
    = "MULTIPOLYGON(((4 0,4 1,5 1,5 0,4 0)),((1 3,1 4,2 4,2 3,1 3)),((6 3,6 4,7 4,7 3,6 3)),((6 8,7 9,7 8,6 8)),((4 8,5 9,5 8,4 8)),((4 4,4 5,5 5,5 4,4 4)),((8 1,8 0,7 0,7 1,8 1)))";

// Case with suspicious UX turn, causing a wrong graph, and more. Old algorithm could cope with it.
// This should be fixed in get_turn_info.
static std::string const rt_w24
    = "MULTIPOLYGON(((2 3,2 4,3 3,2 3)),((8 5,8 6,9 6,8 5)),((3 6,4 5,3 5,3 6)),((3 6,4 7,4 6,3 6)),((5 2,5 3,6 3,5 2)),((5 8,6 9,6 8,5 8)),((0 5,1 6,1 5,0 5)),((0 1,0 2,1 1,0 1)))";

// Case with wrong order (i/u), corrected to cc, but it also needs adapting the left counts.
static std::string const rt_w25
    = "MULTIPOLYGON(((0 5,0 6,1 5,0 5)),((6 1,7 1,7 0,6 0,6 1)),((1 6,1 7,2 6,1 6)),((8 7,8 8,9 8,9 7,8 7)),((8 1,9 2,9 1,8 1)),((0 1,0 2,1 2,1 1,0 1)),((6 3,6 4,7 3,6 3)),((4.5 8.5,5 8,4 8,4 9,5 9,4.5 8.5)),((2 4,3 5,4 5,4 4,2 4)),((3 7,4 8,4 7,3 7)),((3 7,2 7,2 8,3 7)))";

// Needs skip-count to be translated to travels_to_ip_index as -1, for a cc turn.
static std::string const rt_w26
    = "MULTIPOLYGON(((6 6,6 7,7 6,6 6)),((0 0,0 1,1 1,1 0,0 0)),((3 6,2 5,2 7,2.5 6.5,3 7,4 6,5 5,3 5,3 6)),((3 2,2 2,2 3,1 3,3 5,3 2)),((3 1,4 0,3 0,3 1)),((3 1,2 0,2 1,3 1)),((3 7,2 7,2 8,3 7)),((1 8,0 8,0 9,1 9,1 8)),((1 8,2 7,1 7,1 8)),((2 5,1 5,1 6,2 6,1.5 5.5,2 5)),((5 0,5 1,6 1,6 0,5 0)),((1 3,1 2,0 2,1 3)),((4 6,4 7,5 7,5 6,4 6)),((1 6,0 6,0 7,1 6)),((1 5,1 4,0 4,0 5,1 5)),((6 1,6 2,7 2,7 1,6 1)),((6 2,6 3,7 3,6 2)),((7 1,8 1,8 0,7 0,7 1)))";

// Fixed by fixing by suspicious UX turn.
static std::string const rt_w27
    = "MULTIPOLYGON(((3 6,4 7,4 6,3 6)),((4 3,4 4,5 4,5 3,4 3)),((5 8,6 9,6 8,5 8)),((2 7,2 8,3 8,3 7,2 7)))";

// Fixed by fixing by suspicious UX turn.
static std::string const rt_w28
    = "MULTIPOLYGON(((8 8,8 9,9 9,8 8)),((1 2,1 3,2 3,1 2)),((3 8,3 9,4 9,3 8)),((1 1,2 1,1 0,1 1)),((1 1,0 1,0 2,1 1)),((3 0,2 0,2 1,3 0)),((1 8,2 7,1 7,1 8)),((1 8,0 8,0 9,2 9,1 8)),((2.5 5.5,3 5,2 5,2 6,3 6,2.5 5.5)),((5 5,6 5,6 4,5 4,5 5)),((7 3,8 4,8 3,7 3)),((7 3,6 3,6 4,7 4,7 3)),((7 4,7 5,7 6,8 6,8 4,7 4)),((7 8,7 7,6 7,6 8,7 8)),((4 8,5 9,6 8,4 8)),((4 8,4 7,3 7,4 8)),((2 7,2 6,1 6,2 7)),((1 6,2 5,1 5,1 6)),((5 4,5 3,3 3,3 4,4 4,4 5,5 4)),((2 5,2 4,1 4,2 5)),((8 4,9 5,9 4,8 4)),((6 7,6 6,5 6,5 7,6 7)))";

// Needs original arrival behaviour
static std::string const rt_w29
    = "MULTIPOLYGON(((3 7,3 8,4 7,3 7)),((6 5,7 4,5 4,6 5)),((5 4,5 3,4 3,5 4)))";

// Needs to skip the "target ahead" check in is_cc_target_for_union
static std::string const rt_w30
    = "MULTIPOLYGON(((6 6,7 7,7 6,6 6)),((8 3,9 3,8.5 2.5,9 2,8 2,8 3)),((4 0,4 1,5 0,4 0)),((0 5,1 6,1 5,0 5)),((3 8,4 9,4 8,3 8)),((7 4,7 5,8 5,7 4)),((7 4,6 3,6 4,7 4)))";

// Like rt_w30, needs to skip the "target ahead" check
static std::string const rt_w31
    = "MULTIPOLYGON(((0 1,0 2,1 2,1 1,0 1)),((0 4,0 5,1 5,0 4)),((3 8,4 8,3 7,2 7,3 8)),((3 2,3 3,4 3,3 2)),((0 6,0 7,1 6,0 6)),((0 8,0 9,1 8,0 8)),((4 5,4 4,3 4,3 5,4 5)),((4 5,4 6,5 6,4 5)),((9 3,9 2,8 2,8 3,9 3)),((7 4,7 5,8 5,7 4)),((7 4,6 3,6 4,7 4)),((6 8,7 8,6.5 7.5,7 7,6 7,6 8)),((5 6,5 7,6 6,5 6)))";

#endif
