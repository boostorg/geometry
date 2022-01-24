// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2010-2017 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2022.
// Modifications copyright (c) 2022, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//#define BOOST_GEOMETRY_DEBUG_ENRICH
//#define BOOST_GEOMETRY_DEBUG_SEGMENT_IDENTIFIER

#include <geometry_test_common.hpp>

#include <boost/geometry/extensions/algorithms/dissolve.hpp>
#include <boost/geometry/extensions/multi/algorithms/dissolve.hpp>

// To check results
#include <boost/geometry/algorithms/correct_closure.hpp>
#include <boost/geometry/algorithms/length.hpp>
#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/is_valid.hpp>
#include <boost/geometry/algorithms/num_points.hpp>
#include <boost/geometry/algorithms/unique.hpp>

#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/point_xy.hpp>

#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/io/wkt/wkt.hpp>
#include <boost/geometry/multi/io/wkt/wkt.hpp>

#include <boost/geometry/multi/algorithms/for_each.hpp>

#include <boost/geometry/multi/geometries/multi_linestring.hpp>
#include <boost/geometry/multi/geometries/multi_polygon.hpp>


#if defined(TEST_WITH_SVG)
#  include <boost/geometry/io/svg/svg_mapper.hpp>
#  include <boost/geometry/io/svg/write_svg_multi.hpp>
#  include <boost/geometry/algorithms/detail/overlay/debug_turn_info.hpp>
#endif


namespace
{
    // Simplex
    std::string const dissolve_1 = "POLYGON((0 0,0 4,1.5 2.5,2.5 1.5,4 0,0 0))";

    // Self intersecting
    std::string const dissolve_2 = "POLYGON((1 2,1 1,2 1,2 2.25,3 2.25,3 0,0 0,0 3,3 3,2.75 2,1 2))";

    // Self intersecting in last segment
    std::string const dissolve_3 = "POLYGON((0 2,2 4,2 0,4 2,0 2))";

    // Self tangent - polygons are now included twice
    std::string const dissolve_4 = "POLYGON((0 0,0 4,4 4,4 0,2 4,0 0))";

    // Self tangent in corner - polygons are now included twice
    std::string const dissolve_5 = "POLYGON((0 0,0 4,4 4,4 0,0 4,2 0,0 0))";

    // With spike
    std::string const dissolve_6 = "POLYGON((0 0,0 4,4 4,4 2,6 2,4 2,4 0,0 0))";

    // Many intersections
    std::string const dissolve_7 = "POLYGON((1 3,0 9,9 5,1 7,9 8,2 5,10 10,9 2,1 3))";

    // Pentagram
    std::string const dissolve_8 = "POLYGON((5 0,2.5 9,9.5 3.5,0.5 3.5,7.5 9,5 0))";
    // CCW pentagram
    std::string const dissolve_9 = "POLYGON((5 0,7.5 9,0.5 3.5,9.5 3.5,2.5 9,5 0))";
    // CW, one keyhole
    std::string const dissolve_10 = "POLYGON((2 8,8 8,8 0,0 0,0 6,4 6,4 4,2 4,2 8))";
    // CCW, one keyhole
    std::string const dissolve_11 = "POLYGON((2 8,2 4,4 4,4 6,0 6,0 0,8 0,8 8,2 8))";

    // More pentagrams
    // Source: http://upload.wikimedia.org/wikipedia/commons/8/83/Acute_heptagram.svg
    std::string const dissolve_12 = "POLYGON((409 5,229 793.631528,733.348792 161.198146,4.543671 512.172194,813.456329 512.172194,84.651208 161.198146,589 793.631528))";
    // Source: http://upload.wikimedia.org/wikipedia/commons/a/a7/Obtuse_heptagram.svg
    std::string const dissolve_13 = "POLYGON((409 5,813.456329 512.172194,229 793.631528,84.651208 161.198146,733.348792 161.198146,589 793.631528,4.543671 512.172194))";

    std::string const dissolve_14 = "POLYGON((0 0,0 2,2 0,4 2,4 0,2 2,0 0))";
    std::string const dissolve_15 = "POLYGON((0 0,2 2,4 0,4 2,2 0,0 2,0 0))";

    std::string const dissolve_16 = "POLYGON((1 3,4 5,7 3,4 1,1 3),(2 2,4 4,6 2,6 4,4 2,2 4,2 2))";

    // Contains two types of turns
    std::string const dissolve_17 = "POLYGON((0 1,0 5,3 5,3 2,2 3,4 3,4 0,5 1,0 1))";

    // Same but with one more, creating a positive turn too
    std::string const dissolve_18 = "POLYGON((0 1,0 5,3 5,3 2,2 3,4 3,4 0,6 2,6 1,0 1))";

    // Non intersection, but with duplicate
    std::string const dissolve_d1 = "POLYGON((0 0,0 4,4 0,4 0,0 0))";

    // With many duplicate points
    std::string const dissolve_d2 = "POLYGON((0 0,0 1,0 1,0 1,0 2,0 2,0 3,0 3,0 3,0 3,0 4,2 4,2 4,4 4,4 0,4 0,3 0,3 0,3 0,3 0,3 0,0 0))";

    // Case with touching reversed interior ring inside, which should be removed
    std::string const dissolve_h1_a = "POLYGON((0 0,0 4,4 4,4 0,0 0),(1 2,2 4,3 2,1 2))";

    // Case with correct interior ring inside, which should be stay
    std::string const dissolve_h1_b = "POLYGON((0 0,0 4,4 4,4 0,0 0),(1 2,3 2,2 4,1 2))";

    // Hole: interior intersecting exterior
    std::string const dissolve_h2 = "POLYGON((0 0,0 4,4 4,4 0,0 0),(1 1,1 3,5 4,1 1))";

    // Hole: two intersecting holes
    std::string const dissolve_h3 = "POLYGON((0 0,0 4,4 4,4 0,0 0),(1 1,1 3,3 3,3 1,1 1),(2 2,2 3.5,3.5 3.5,3.5 2,2 2))";

    // Hole: self-intersecting hole
    std::string const dissolve_h4 = "POLYGON((0 0,0 4,4 4,4 0,0 0),(1 1,3 3,3 2.5,1 3.5,1.5 3.5,1 1))";

    // Star having an extra thingy
    std::string const dissolve_star_a = "POLYGON((20.1493053436279300 3.3291947841644287,13.9365568161010740 3.7241046428680420,18.7846546173095700 1.0315386056900024,18.7846546173095700 0.9956377148628235,17.9586830139160160 4.6934285163879395,15.1575593948364260 1.2828447818756104,20.1493053436279300 3.3291947841644287))";
    // Star with larger extra thingy
    std::string const dissolve_star_b = "POLYGON((20.1493053436279300 3.3291947841644287,13.9365568161010740 3.7241046428680420,18.7846546173095700 1.0315386056900024,18.7846546173095700 0.500000000000000,17.9586830139160160 4.6934285163879395,15.1575593948364260 1.2828447818756104,20.1493053436279300 3.3291947841644287))";
    // Star without extra thingy
    std::string const dissolve_star_c = "POLYGON((20.1493053436279300 3.3291947841644287,13.9365568161010740 3.7241046428680420,18.7846546173095700 1.0315386056900024,17.9586830139160160 4.6934285163879395,15.1575593948364260 1.2828447818756104,20.1493053436279300 3.3291947841644287))";

    std::string const multi_three_triangles = "MULTIPOLYGON(((1 1,5 5,8 0,1 1)),((4 2,0 8,5 9,4 2)),((5 3,4 8,10 4,5 3)))";
    std::string const multi_simplex_two = "MULTIPOLYGON(((0 0,1 4,4 1,0 0)),((2 2,3 6,6 3,2 2)))";
    std::string const multi_simplex_three = "MULTIPOLYGON(((0 0,1 4,4 1,0 0)),((2 2,3 6,6 3,2 2)),((3 4,5 6,6 2,3 4)))";
    std::string const multi_simplex_four = "MULTIPOLYGON(((0 0,1 4,4 1,0 0)),((2 2,3 6,6 3,2 2)),((3 4,5 6,6 2,3 4)),((5 5,7 7,8 4,5 5)))";
    std::string const multi_disjoint = "MULTIPOLYGON(((0 0,1 4,4 1,0 0)),((1 6,2 10,5 7,1 6)),((3 4,5 6,6 2,3 4)),((6 5,8 7,9 4,6 5)))";
    std::string const multi_new_interior = "MULTIPOLYGON(((0 0,1 4,4 1,0 0)),((2 2,3 6,6 3,2 2)),((3 4,5 6,6 2,3 4)),((3 1,5 4,8 4,3 1)))";

    // Testcases sent on GGL mailing list - report Javier - 2011, March 7
    std::string const ggl_list_20110307_javier_01_a = "MULTIPOLYGON(((560 -400, 600 -400, 600 -440, 560 -440, 560 -400)), ((480 -400, 520 -400, 520 -440, 480 -440, 480 -400)), ((600 -320, 640 -320, 640 -360, 600 -360, 600 -320)), ((520 -400, 560 -400, 560 -440, 520 -440, 520 -400)))";
    std::string const ggl_list_20110307_javier_01_b = "POLYGON((0 0, 2000 0, 2000 -2000, 0 -2000, 0 0), (560 -400, 560 -440, 600 -440, 600 -400, 560 -400), (480 -400, 480 -440, 520 -440, 520 -400, 480 -400), (600 -320, 600 -360, 640 -360, 640 -320, 600 -320), (520 -400, 520 -440, 560 -440, 560 -400, 520 -400))";

    std::string const ggl_list_denis = "POLYGON((55 10, 141 237, 249 23, 21 171, 252 169, 24 89, 266 73, 55 10))";

    // Testcases sent by Johan Doré at September 24 / October 26 / October 30, 2017
    std::string const dissolve_mail_2017_09_24_a = "POLYGON((0 1, 1 0, 1 1, 0 0, 0 1))"; // two triangles
    std::string const dissolve_mail_2017_09_24_b = "POLYGON((1 0, 0 0, 0 4, 4 4, 4 0))"; // input is not closed
    std::string const dissolve_mail_2017_09_24_c = "POLYGON((0 0, 1 0, 0 -1, 0.0001 1))"; // spike and not closed
    std::string const dissolve_mail_2017_09_24_d = "POLYGON((0 0, 1 0, 0 -1, 0 1))"; // spike and not closed

    std::string const dissolve_mail_2017_10_26_a = "POLYGON((0 3, 3 3, 3 1, 2 1, 2 2, 1 2, 1 1, 2 1, 2 0, 0 0, 0 3))"; // should form interior ring
    std::string const dissolve_mail_2017_10_26_b = "POLYGON((0 0, 0 4, 4 4, 4 0, 1 0, 1 3, 3 3, 3 0))"; // should NOT form interior ring and (maybe) should remove two not necessary intersection points (to be decided)
    std::string const dissolve_mail_2017_10_26_c = "POLYGON((0 2, 2 1, 3 1, 1 1, 2 1, 4 2, 4 0, 0 0))"; // contains cluster and should ignore count_left/count_right

    // Testcase sent by Johan Doré, giving a problem on Windows indicating that input (geometry1/geometry2) for union should NOT be reference to output collection. Fixed by copying it.
    std::string const dissolve_mail_2018_08_19 = "POLYGON((13.78 -2.18, 7.04 -2.33, 7.08 -2.33,16.05 -0.55,16.46 -6.08,16.99 -5.82, 2.92 -1.99,15.14 -1.65,17.41 -4.42,13.78 -2.18))";

    // This case needs rescaling for <float>. For <double> it runs fine without.
    std::string const dissolve_mail_2017_10_30_a = "POLYGON((12.7069120407104490 -2.3525938987731934, 12.6983022689819340 -2.3552336692810059, 12.6984634399414060 -2.3553242683410645, 12.6980066299438480 -2.3553242683410645, 12.6983022689819340 -2.3552336692810059, 12.6911554336547850 -2.3512287139892578, 12.7025737762451170 -2.3398520946502686))";

    // Testcases sent by Artem Pavlenko via gitter
    // https://gitter.im/boostorg/geometry?at=58ef46408e4b63533dc49b48
    std::string const gitter_2013_04_a = "POLYGON((36.9121 2.03883,26.2052 54.353,60.0781 64.2202,96.2171 55.9826,71.1506 39.8365,5.72552 94.1523,4.06819 13.9054,59.7155 44.5877,60.9243 16.4597,48.8696 93.039,36.9121 2.03883))";
    std::string const gitter_2013_04_b = "POLYGON((337 176,602 377,294 372,581 166,453 449,337 176))";

    // Large polygon with self-intersections
    std::string const dissolve_mail_2017_09_24_e = "POLYGON((25.21475410461420 -1.19600892066955, 25.36818313598630 -1.20732820034027, 25.36953926086420 -1.20868659019470, 25.37180328369140 -1.20959210395812, 25.37451934814450 -1.21049761772155, 25.37632942199700 -1.21185600757598, 25.37904357910150 -1.21276152133941, 25.38085556030270 -1.21321427822113, 25.38221168518060 -1.21366703510284, 25.38311767578120 -1.21411991119384, 25.38447570800780 -1.21411991119384, 25.38447570800780 -1.21185600757598, 25.38492774963370 -1.21140325069427, 25.38492774963370 -1.21095049381256, 25.38945388793940 -1.21095049381256, 25.38990592956540 -1.21049761772155, 25.39035987854000 -1.21004486083984, 25.39081192016600 -1.21004486083984, 25.39081192016600 -1.20959210395812, 25.39126396179190 -1.20913934707641, 25.39171600341790 -1.20913934707641, 25.39262199401850 -1.20868659019470, 25.39262199401850 -1.20823383331298, 25.39352798461910 -1.20823383331298, 25.39398002624510 -1.20778107643127, 25.39443206787100 -1.20778107643127, 25.39443206787100 -1.20732820034027, 25.39488410949700 -1.20687544345855, 25.39443206787100 -1.20642268657684, 25.39420700073240 -1.20596992969512, 25.39386749267570 -1.20551717281341, 25.39352798461910 -1.20551717281341, 25.39352798461910 -1.20506441593170, 25.39386749267570 -1.20551717281341, 25.39398002624510 -1.20551717281341, 25.39420700073240 -1.20596992969512, 25.39488410949700 -1.20687544345855, 25.39533805847160 -1.20778107643127, 25.39579010009760 -1.20823383331298, 25.39579010009760 -1.20868659019470, 25.39624214172360 -1.20913934707641, 25.39669609069820 -1.20959210395812, 25.39669609069820 -1.21049761772155, 25.39714813232420 -1.21095049381256, 25.39760017395010 -1.21185600757598, 25.39805221557610 -1.21321427822113, 25.39850616455070 -1.21411991119384, 25.39895820617670 -1.21547818183898, 25.39895820617670 -1.21638369560241, 25.39986419677730 -1.21819484233856, 25.40031623840330 -1.21955311298370, 25.40076828002920 -1.22045862674713, 25.40076828002920 -1.22181701660156, 25.40122032165520 -1.22226977348327, 25.40122032165520 -1.22408080101013, 25.40212631225580 -1.22498643398284, 25.40212631225580 -1.22634470462799, 25.40257835388180 -1.22725021839141, 25.40257835388180 -1.22770309448242, 25.40303230285640 -1.22860860824584, 25.40303230285640 -1.23041963577270, 25.40348434448240 -1.23041963577270, 25.21475410461420 -1.19600892066955))";

    // Idem but moving closing point eastwards to view self-intersections better
    std::string const dissolve_mail_2017_09_24_f = "POLYGON((25.35475410461420 -1.20600892066955, 25.36818313598630 -1.20732820034027, 25.36953926086420 -1.20868659019470, 25.37180328369140 -1.20959210395812, 25.37451934814450 -1.21049761772155, 25.37632942199700 -1.21185600757598, 25.37904357910150 -1.21276152133941, 25.38085556030270 -1.21321427822113, 25.38221168518060 -1.21366703510284, 25.38311767578120 -1.21411991119384, 25.38447570800780 -1.21411991119384, 25.38447570800780 -1.21185600757598, 25.38492774963370 -1.21140325069427, 25.38492774963370 -1.21095049381256, 25.38945388793940 -1.21095049381256, 25.38990592956540 -1.21049761772155, 25.39035987854000 -1.21004486083984, 25.39081192016600 -1.21004486083984, 25.39081192016600 -1.20959210395812, 25.39126396179190 -1.20913934707641, 25.39171600341790 -1.20913934707641, 25.39262199401850 -1.20868659019470, 25.39262199401850 -1.20823383331298, 25.39352798461910 -1.20823383331298, 25.39398002624510 -1.20778107643127, 25.39443206787100 -1.20778107643127, 25.39443206787100 -1.20732820034027, 25.39488410949700 -1.20687544345855, 25.39443206787100 -1.20642268657684, 25.39420700073240 -1.20596992969512, 25.39386749267570 -1.20551717281341, 25.39352798461910 -1.20551717281341, 25.39352798461910 -1.20506441593170, 25.39386749267570 -1.20551717281341, 25.39398002624510 -1.20551717281341, 25.39420700073240 -1.20596992969512, 25.39488410949700 -1.20687544345855, 25.39533805847160 -1.20778107643127, 25.39579010009760 -1.20823383331298, 25.39579010009760 -1.20868659019470, 25.39624214172360 -1.20913934707641, 25.39669609069820 -1.20959210395812, 25.39669609069820 -1.21049761772155, 25.39714813232420 -1.21095049381256, 25.39760017395010 -1.21185600757598, 25.39805221557610 -1.21321427822113, 25.39850616455070 -1.21411991119384, 25.39895820617670 -1.21547818183898, 25.39895820617670 -1.21638369560241, 25.39986419677730 -1.21819484233856, 25.40031623840330 -1.21955311298370, 25.40076828002920 -1.22045862674713, 25.40076828002920 -1.22181701660156, 25.40122032165520 -1.22226977348327, 25.40122032165520 -1.22408080101013, 25.40212631225580 -1.22498643398284, 25.40212631225580 -1.22634470462799, 25.40257835388180 -1.22725021839141, 25.40257835388180 -1.22770309448242, 25.40303230285640 -1.22860860824584, 25.40303230285640 -1.23041963577270, 25.40348434448240 -1.23041963577270, 25.35475410461420 -1.20600892066955))";

    // As d but with other spike
    std::string const dissolve_mail_2017_09_24_g = "POLYGON((1 0, 0 -1, 0 1, 0 0,1 0))";
    std::string const dissolve_mail_2017_09_24_h = "POLYGON((0 0, 0 1, 2 1, 1 1,0 0))";

    std::string const dissolve_ticket17 = "POLYGON ((-122.28139163 37.37319149,-122.28100699 37.37273669,-122.28002186 37.37303123,-122.27979681 37.37290072,-122.28007349 37.37240493,-122.27977334 37.37220360,-122.27819720 37.37288580,-122.27714184 37.37275161,-122.27678628 37.37253167,-122.27766437 37.37180973,-122.27804382 37.37121453,-122.27687664 37.37101354,-122.27645829 37.37203386,-122.27604423 37.37249110,-122.27632234 37.37343339,-122.27760980 37.37391082,-122.27812478 37.37800320,-122.26117222 37.39121007,-122.25572289 37.39566631,-122.25547269 37.39564971,-122.25366304 37.39552993,-122.24919976 37.39580268,-122.24417933 37.39366907,-122.24051443 37.39094143,-122.23246277 37.38100418,-122.23606766 37.38141338,-122.24001587 37.37738940,-122.23666848 37.37609347,-122.23057450 37.37882170,-122.22679803 37.37807143,-122.22525727 37.37448817,-122.22523229 37.37443000,-122.23083199 37.37609347,-122.23033486 37.37777891,-122.23169030 37.37732117,-122.23229178 37.37709687,-122.23237761 37.37631249,-122.23297776 37.37438834,-122.23872850 37.37165986,-122.24044511 37.36934068,-122.24671067 37.36865847,-122.24825570 37.36981819,-122.25151719 37.36947713,-122.25357721 37.36756706,-122.26001451 37.36579354,-122.25615213 37.36545239,-122.25486458 37.36245083,-122.25357721 37.36108651,-122.25194642 37.36013139,-122.24885652 37.35958557,-122.24911401 37.35849399,-122.25357721 37.35808470,-122.25675286 37.35897159,-122.25855539 37.35753887,-122.26181687 37.35828939,-122.26713837 37.35897159,-122.26782510 37.36108651,-122.26662339 37.36456559,-122.27288911 37.36722601,-122.27366159 37.36531602,-122.27168740 37.36470213,-122.27391900 37.36374701,-122.27074326 37.36245083,-122.27134408 37.35951742,-122.27426240 37.36135926,-122.27709482 37.36115474,-122.27966974 37.36231438,-122.27958391 37.36463382,-122.27572152 37.36463382,-122.27563569 37.36524779,-122.27700899 37.36593000,-122.27709482 37.36763529,-122.27554978 37.36838573,-122.27667254 37.36931478,-122.27677932 37.36932073,-122.27769362 37.36853987,-122.27942490 37.36830803,-122.28178776 37.36677917,-122.28509559 37.36443500,-122.28845129 37.36413744,-122.29194403 37.36695946,-122.29382577 37.36726817,-122.29600414 37.36898512,-122.29733083 37.36995398,-122.29593239 37.37141436,-122.29416649 37.37075898,-122.29325026 37.37108436,-122.29652910 37.37311697,-122.29584237 37.37374461,-122.29537583 37.37573372,-122.29487677 37.37752502,-122.30923212 37.37593011,-122.31122484 37.38230086,-122.31467994 37.38092472,-122.31715663 37.38252181,-122.32307970 37.38166978,-122.31985618 37.37667694,-122.32210304 37.37580220,-122.32581446 37.37589532,-122.32401730 37.37331839,-122.32960417 37.37189020,-122.33465527 37.37331906,-122.33425328 37.37623680,-122.33620676 37.37726132,-122.33397986 37.37822382,-122.33358918 37.38036590,-122.33202637 37.37986918,-122.33147954 37.38101784,-122.33394080 37.38198017,-122.33545239 37.38587943,-122.33478058 37.38785697,-122.33386050 37.38723721,-122.33350041 37.38571137,-122.33122003 37.38548891,-122.33140008 37.38650606,-122.33366042 37.38817490,-122.33244019 37.39157602,-122.33298157 37.39419201,-122.33164013 37.39477028,-122.33202017 37.39518351,-122.33358038 37.39499282,-122.33376050 37.39597811,-122.33550067 37.39734478,-122.33556069 37.39481797,-122.33344040 37.39292676,-122.33638094 37.38892189,-122.34240644 37.38852719,-122.34906293 37.38726898,-122.35072321 37.39338769,-122.34910291 37.39445252,-122.34796272 37.39410291,-122.34449043 37.39640534,-122.34500223 37.39729709,-122.34936291 37.39670910,-122.35098322 37.39531066,-122.35364623 37.39554510,-122.35434369 37.39612111,-122.35798429 37.39600988,-122.35768430 37.39478621,-122.36334519 37.39206871,-122.36604726 37.39203267,-122.36778592 37.39335592,-122.36518870 37.40022011,-122.36554552 37.40247752,-122.36370519 37.40331974,-122.36270506 37.40530591,-122.36320512 37.40670418,-122.36149849 37.40851392,-122.36730580 37.41054938,-122.37263720 37.41378932,-122.37161871 37.42076600,-122.36566153 37.42006292,-122.36520547 37.42742106,-122.37165953 37.43661157,-122.35943972 37.44459022,-122.35356359 37.44600810,-122.33792254 37.45796329,-122.35228518 37.47478091,-122.35127080 37.48181199,-122.34867342 37.48487322,-122.34359717 37.48801082,-122.33388431 37.48677650,-122.33142321 37.48429747,-122.32929580 37.48473149,-122.32609609 37.48291144,-122.32344850 37.48228229,-122.31924364 37.48410234,-122.31677299 37.48114051,-122.31431751 37.47848973,-122.31259201 37.47682190,-122.31515972 37.47568196,-122.31691389 37.47360309,-122.31292494 37.46960081,-122.31130153 37.46937743,-122.30889894 37.47124987,-122.30612839 37.47011613,-122.30149630 37.46568378,-122.30064277 37.46363784,-122.29283821 37.45922376,-122.28630141 37.45415497,-122.28883099 37.44629920,-122.28316717 37.44197138,-122.27554148 37.42297597,-122.25597410 37.40553692,-122.25196579 37.40129593,-122.25012043 37.40049143,-122.24823207 37.39897758,-122.24754551 37.39740941,-122.24778582 37.39621607,-122.24934787 37.39599102,-122.25005170 37.39871849,-122.25222328 37.39863668,-122.25342491 37.39737529,-122.25520162 37.39667289,-122.25528737 37.39522726,-122.27747460 37.37809616,-122.27977493 37.37858717,-122.28157729 37.37920106,-122.28322534 37.37952846,-122.28416939 37.38092656,-122.28621223 37.37984219,-122.28638389 37.37613857,-122.28382607 37.37843722,-122.27930278 37.37718220,-122.28196361 37.37652740,-122.28295058 37.37568167,-122.28216101 37.37523148,-122.28114822 37.37543608,-122.27934569 37.37528613,-122.27996369 37.37448121,-122.28104521 37.37454944,-122.28185197 37.37422883,-122.28290767 37.37474038,-122.28376597 37.37467224,-122.28428104 37.37399012,-122.28402346 37.37338989,-122.28610922 37.37364914,-122.28651264 37.37327388,-122.28672722 37.37207343,-122.28628398 37.37205448,-122.28574460 37.37166682,-122.28479711 37.37200981,-122.28327731 37.37137228,-122.28285511 37.37100700,-122.28279409 37.37125669,-122.28315527 37.37173756,-122.28321872 37.37220569,-122.28187007 37.37231918,-122.28193109 37.37294908,-122.28139163 37.37319149))";
    std::string const dissolve_reallife = "POLYGON((170718 605997,170718 605997,170776 606016,170773 606015,170786 606020,170778 606016,170787 606021,170781 606017,170795 606028,170795 606028,170829 606055,170939 606140,170933 605968,170933 605968,170932 605908,170929 605834,170920 605866,170961 605803,170739 605684,170699 605749,170691 605766,170693 605762,170686 605775,170688 605771,170673 605794,170676 605790,170668 605800,170672 605796,170651 605818,170653 605816,170639 605829,170568 605899,170662 605943,170633 605875,170603 605961,170718 605997))";

    // https://svn.boost.org/trac/boost/ticket/10713
    std::string const dissolve_ticket10713 = "POLYGON((-0.7189743518829346 4.1308121681213379, 0.0831791982054710 4.1034231185913086, 0.1004156470298767 4.1107301712036133, 0.1044322624802589 4.1026973724365234, 0.0831791982054710 4.1034231185913086, -0.7711903452873230 3.7412264347076416, -0.7189743518829346 4.1308121681213379))";
}


#if defined(TEST_WITH_SVG)
template <typename Mapper>
struct map_visitor
{
    map_visitor(Mapper& mapper)
        : m_mapper(mapper)
    {}

    void print(char const* header)
    {}

    template <typename Turns>
    void print(char const* header, Turns const& turns, int turn_index)
    {
        std::string style = "fill:rgb(0,0,0);font-family:Arial;font-size:6px";
        stream(turns, turns[turn_index], turns[turn_index].operations[0], header, style);
    }

    template <typename Turns>
    void print(char const* header, Turns const& turns, int turn_index, int op_index)
    {
        std::string style = "fill:rgb(0,0,0);font-family:Arial;font-size:6px";
        stream(turns, turns[turn_index], turns[turn_index].operations[op_index], header, style);
    }

    template <typename Turns>
    void visit_turns(int phase, Turns const& turns)
    {
        typedef typename boost::range_value<Turns>::type turn_type;
        std::size_t index = 0;
        for (turn_type const& turn : turns)
        {
            switch (phase)
            {
                case 2 : // after self_turns and enrich
                    if (turn.discarded)
                    {
                        m_mapper.map(turn.point, "fill:rgb(255,128,0);", 2);
                    }
                    else
                    {
                        m_mapper.map(turn.point, "fill:rgb(255,128,0);"
                                "stroke:rgb(0,0,0);stroke-width:1", 4);
                    }
                    break;
                case 3 : // after enrich/traverse
                    label_turn(index, turn, -2, "fill:rgb(0,0,128);");
                    break;
            }
            index++;
        }
    }

    template <typename Clusters, typename Turns>
    void visit_clusters(Clusters const& , Turns const& ) {}

    template <typename Turns, typename Turn, typename Operation>
    void visit_traverse(Turns const& , Turn const& , Operation const& , char const*)
    {}

    template <typename Turns, typename Turn, typename Operation>
    void visit_traverse_reject(Turns const& , Turn const& , Operation const& ,
                               bg::detail::overlay::traverse_error_type )
    {}

    template <typename Rings>
    void visit_generated_rings(Rings const& rings)
    {
        typedef typename boost::range_value<Rings>::type ring_type;
        for (ring_type const& ring : rings)
        {
            double const area = bg::area(ring);
            std::string const color = area < 0 ? "rgb(255,0,0)" : "rgb(0,0,255)";
            std::string const style = "stroke:" + color
                    + ";stroke-width:0.1;fill-opacity:0.1;fill:" + color;
            m_mapper.map(ring, style);
        }
    }


private :

    template <typename Turn>
    bool label_operation(Turn const& turn, std::size_t index, std::ostream& os)
    {
        os << bg::operation_char(turn.operations[index].operation);
        bool result = false;
        if (! turn.discarded)
        {
            os << "->" << turn.operations[index].enriched.get_next_turn_index();
            result = true;
        }
        if (turn.operations[index].enriched.prefer_start)
        {
            os << "$";
        }
        if (! turn.operations[index].enriched.startable)
        {
            os << "@";
        }

        return result;
    }

    template <typename Turn>
    void label_turn(std::size_t index, Turn const& turn, int y_offset, std::string const& color)
    {
        std::ostringstream out;
        out << index;
        if (turn.cluster_id != -1)
        {
            out << " c=" << turn.cluster_id << " ";
        }
        bool lab1 = label_operation(turn, 0, out);
        out << " / ";
        bool lab2 = label_operation(turn, 1, out);
        if (turn.discarded)
        {
            out << "!";
        }

        std::string font8 = "font-family:Arial;font-size:6px";
        std::string font6 = "font-family:Arial;font-size:4px";
        std::string style =  color + ";" + font8;
        if (turn.discarded)
        {
            style =  "fill:rgb(92,92,92);" + font6;
        }
        else if (turn.cluster_id != -1)
        {
            style =  color + ";" + font8;
        }
        else if (! lab1 || ! lab2)
        {
            style =  color + ";" + font6;
        }

        m_mapper.text(turn.point, out.str(), style, 5, y_offset, 6);
    }

    Mapper& m_mapper;
};

#endif

//! Unittest settings
struct ut_settings
{
    double percentage;
    bool test_validity;

    explicit ut_settings(double p = 0.001, bool tv = true)
        : percentage(p)
        , test_validity(tv)
    {}

};

template <typename Geometry>
std::string as_wkt(Geometry const& geometry)
{
    std::ostringstream out;
    out << bg::wkt(geometry);
    return out.str();
}

template <typename GeometryOut, typename Geometry>
void test_dissolve(std::string const& caseid, Geometry const& geometry,
        double expected_area,
        std::size_t expected_clip_count,
        std::size_t expected_hole_count,
        std::size_t expected_point_count,
        ut_settings const& settings)
{
    using coordinate_type = typename bg::coordinate_type<Geometry>::type;

    static const bool is_line = bg::geometry_id<GeometryOut>::type::value == 2;

    //std::cout << bg::area(geometry) << std::endl;

    using multi_polygon = bg::model::multi_polygon<GeometryOut>;
    multi_polygon dissolved1;

    // Check dispatch::dissolve
    {
        using strategy_type = typename bg::strategies::relate::services::default_strategy
            <
                Geometry, Geometry
            >::type;

        using rescale_policy_type = typename bg::rescale_policy_type
            <
                typename bg::point_type<Geometry>::type
            >::type;

        rescale_policy_type robust_policy
                = bg::get_rescale_policy<rescale_policy_type>(geometry);

        // This will optionally also create SVG with turn-debug information
        strategy_type strategy;


#if ! defined(TEST_WITH_SVG)
        bg::detail::overlay::overlay_null_visitor visitor;
#else
        std::ostringstream filename;
        filename << "dissolve_" << caseid << "_"
            << string_from_type<coordinate_type>::name()
            << ".svg";

        std::ofstream svg(filename.str().c_str());

        using mapper_type = bg::svg_mapper
            <
                typename bg::point_type<Geometry>::type
            >;

        mapper_type mapper(svg, 500, 500);
        mapper.add(geometry);

        mapper.map(geometry, "fill-opacity:0.5;fill:rgb(153,204,0);"
            "stroke:rgb(153,204,0);stroke-width:2;fill-rule:nonzero;");

        map_visitor<mapper_type> visitor(mapper);
#endif

        bg::dispatch::dissolve
            <
                Geometry,
                GeometryOut,
                false
            >::apply(geometry, robust_policy, std::back_inserter(dissolved1),
                     strategy, visitor);

#if defined(TEST_WITH_SVG)
        for (GeometryOut& dissolved : dissolved1)
        {
           mapper.map(dissolved, "fill-opacity:0.1;fill:rgb(255,0,0);"
                      "stroke-opacity:0.4;stroke:rgb(255,0,255);stroke-width:3;"
                                 "fill-rule:nonzero;");
        }
#endif

    }

    if (settings.test_validity)
    {
        std::string message;
        bool const valid = bg::is_valid(dissolved1, message);
        BOOST_CHECK_MESSAGE(valid,
                "dissolve: " << caseid
                << " geometry is not valid: " << message);
    }

    // Make output unique (TODO: this should probably be moved to dissolve itself)
    for (GeometryOut& dissolved : dissolved1)
    {
        bg::unique(dissolved);
    }

    typename bg::default_area_result<Geometry>::type length_or_area = 0;
    std::size_t holes = 0;
    std::size_t count = 0;

    for (GeometryOut& dissolved : dissolved1)
    {
        length_or_area +=
            is_line ? bg::length(dissolved) : bg::area(dissolved);

        holes += bg::num_interior_rings(dissolved);
        count += bg::num_points(dissolved);
    }

    BOOST_CHECK_MESSAGE(count == expected_point_count,
            "dissolve: " << caseid
            << " #points expected: " << expected_point_count
            << " detected: " << count
            << " type: " << string_from_type<coordinate_type>::name()
            );

    BOOST_CHECK_MESSAGE(dissolved1.size() == expected_clip_count,
            "dissolve: " << caseid
            << " #clips expected: " << expected_clip_count
            << " detected: " << dissolved1.size()
            << " type: " << string_from_type<coordinate_type>::name()
            );

    BOOST_CHECK_EQUAL(holes, expected_hole_count);
    BOOST_CHECK_CLOSE(length_or_area, expected_area, settings.percentage);

    // Check dissolve_inserter
    std::vector<GeometryOut> dissolved2;
    bg::dissolve_inserter<GeometryOut>(geometry, std::back_inserter(dissolved2));

    // Check dissolve and validity, assuming GeometryOut is a single polygon
    multi_polygon dissolved3;
    bg::dissolve(geometry, dissolved3);

    for (GeometryOut& dissolved : dissolved2)
    {
        bg::unique(dissolved);
    }
    for (GeometryOut& dissolved : dissolved3)
    {
        bg::unique(dissolved);
    }

    BOOST_CHECK_EQUAL(dissolved1.size(), dissolved2.size());
    BOOST_CHECK_EQUAL(dissolved1.size(), dissolved3.size());

    if (dissolved1.size() == dissolved2.size()
            && dissolved1.size() == dissolved3.size())
    {
        for (std::size_t i = 0; i < dissolved1.size(); i++)
        {
            std::string const wkt1 = as_wkt(dissolved1[i]);
            std::string const wkt2 = as_wkt(dissolved2[i]);
            std::string const wkt3 = as_wkt(dissolved3[i]);
            BOOST_CHECK_MESSAGE(wkt1 == wkt2, caseid << " : output differs: " << wkt1 << " VERSUS " << wkt2);
            BOOST_CHECK_MESSAGE(wkt1 == wkt3, caseid << " : output differs: " << wkt1 << " VERSUS " << wkt3);
        }
    }
}


template <typename Geometry, typename GeometryOut>
void test_one(std::string caseid, std::string const& wkt,
        double expected_area,
        std::size_t expected_clip_count,
        std::size_t expected_hole_count,
        std::size_t expected_point_count,
        ut_settings const& settings)
{
    Geometry geometry;
    bg::read_wkt(wkt, geometry);

    // If defined as closed, it should be closed. The algorithm itself
    // cannot close it without making a copy.
    bg::correct_closure(geometry);

    test_dissolve<GeometryOut>(caseid, geometry,
        expected_area,
        expected_clip_count, expected_hole_count, expected_point_count,
        settings);

    // Verify if reversed version is indeed identical (it should, because each
    // ring is now corrected within dissolve itself
    bg::reverse(geometry);

    caseid += "_rev";
    test_dissolve<GeometryOut>(caseid, geometry,
        expected_area,
        expected_clip_count, expected_hole_count, expected_point_count,
        settings);

#ifdef BOOST_GEOMETRY_TEST_MULTI_PERMUTATIONS
    // Test different combinations of a multi-polygon

    int n = geometry.size();

    // test them in all orders
    std::vector<int> indices;
    for (int i = 0; i < n; i++)
    {
        indices.push_back(i);
    }
    int permutation = 0;
    do
    {
        std::ostringstream out;
        out << caseid;
        Geometry geometry2;
        for (int i = 0; i < n; i++)
        {
            int index = indices[i];
            out << "_" << index;
            geometry2.push_back(geometry[index]);
        }
        test_dissolve<GeometryOut>(out.str(), geometry2, expected_area,
                expected_clip_count, expected_hole_count, expected_point_count, settings);
    } while (std::next_permutation(indices.begin(), indices.end()));
#endif

}

#define TEST_DISSOLVE(caseid, area, clips, holes, points) { \
    ut_settings settings; \
    (test_one<polygon, polygon>) ( #caseid, caseid, area, clips, holes, points, settings); }

#define TEST_DISSOLVE_WITH(caseid, area, clips, holes, points, settings) { \
    (test_one<polygon, polygon>) ( #caseid, caseid, area, clips, holes, points, settings); }

#define TEST_DISSOLVE_IGNORE(caseid, area, clips, holes, points) { \
    ut_settings settings; settings.test_validity = false; \
    (test_one<polygon, polygon>) ( #caseid, caseid, area, clips, holes, points, settings); }

#define TEST_MULTI(caseid, area, clips, holes, points) { \
    ut_settings settings; \
    (test_one<multi_polygon, polygon>) ( #caseid, caseid, area, clips, holes, points, settings); }

template <typename P, bool Clockwise>
void test_all(ut_settings const& settings_for_sensitive_cases)
{
    typedef bg::model::polygon<P, Clockwise> polygon;
    typedef bg::model::multi_polygon<polygon> multi_polygon;

    TEST_DISSOLVE(dissolve_1, 8.0, 1, 0, 4);
    TEST_DISSOLVE(dissolve_2, 7.9296875, 1, 1, 12);
    TEST_DISSOLVE(dissolve_3, 4.0, 2, 0, 8);
    TEST_DISSOLVE(dissolve_4, 8.0, 2, 0, 8);
    TEST_DISSOLVE(dissolve_5, 12.0, 2, 0, 8);
    TEST_DISSOLVE(dissolve_6, 16.0, 1, 0, 5);

    TEST_DISSOLVE(dissolve_7, 50.48056402439, 1, 0, 7);
    TEST_DISSOLVE(dissolve_8, 25.6158412, 1, 0, 11);

    // CCW polygons should turn CW after dissolve
    TEST_DISSOLVE(dissolve_9, 25.6158412, 1, 0, 11);
    TEST_DISSOLVE(dissolve_10, 60.0, 1, 0, 7);
    TEST_DISSOLVE(dissolve_11, 60.0, 1, 0, 7);

    // More pentagrams
    TEST_DISSOLVE(dissolve_12, 186556.84077318, 1, 0, 15);
    TEST_DISSOLVE(dissolve_13, 361733.91651, 1, 0, 15);

    TEST_DISSOLVE(dissolve_14, 4.0, 3, 0, 13);
    TEST_DISSOLVE(dissolve_15, 4.0, 3, 0, 13);
    TEST_DISSOLVE(dissolve_16, 8.2667, 8, 0, 38);

    TEST_DISSOLVE(dissolve_17, 14.5, 2, 0, 11);
    TEST_DISSOLVE(dissolve_18, 15.0, 3, 0, 15);

    TEST_DISSOLVE(dissolve_d1, 8.0, 1, 0, 4);
    TEST_DISSOLVE(dissolve_d2, 16.0, 1, 0, 5);

    TEST_DISSOLVE(dissolve_h1_a, 14.0, 1, 1, 9);
    TEST_DISSOLVE(dissolve_h1_b, 14.0, 1, 1, 9);
    TEST_DISSOLVE(dissolve_h2, 12.5, 2, 0, 13);
    TEST_DISSOLVE(dissolve_h3, 10.75, 1, 1, 14);
    TEST_DISSOLVE(dissolve_h4, 14.3447, 1, 3, 17);

    TEST_DISSOLVE(dissolve_star_a, 7.38821, 2, 0, 15);
    TEST_DISSOLVE(dissolve_star_b, 7.28259, 2, 0, 15);
    TEST_DISSOLVE(dissolve_star_c, 7.399696, 1, 0, 11);

    TEST_DISSOLVE(dissolve_mail_2017_09_24_a, 0.5, 2, 0, 8);

    TEST_DISSOLVE(dissolve_mail_2017_09_24_b, 16.0, 1, 0, 5);
    TEST_DISSOLVE(dissolve_mail_2017_09_24_c, 0.5, 2, 0, 8);
    TEST_DISSOLVE(dissolve_mail_2017_09_24_d, 0.5, 1, 0, 4);
    TEST_DISSOLVE(dissolve_mail_2017_09_24_e, 0.001801138128, 5, 0, 69);
    TEST_DISSOLVE_WITH(dissolve_mail_2017_09_24_f, 0.000361308800, 5, 0, 69,
                       settings_for_sensitive_cases);
    TEST_DISSOLVE(dissolve_mail_2017_09_24_g, 0.5, 1, 0, 4);
    TEST_DISSOLVE(dissolve_mail_2017_09_24_h, 0.5, 1, 0, 4);

    TEST_DISSOLVE(dissolve_mail_2017_10_26_a, 7.0, 1, 1, 12);
    TEST_DISSOLVE(dissolve_mail_2017_10_26_b, 16.0, 1, 0, 5);
    TEST_DISSOLVE(dissolve_mail_2017_10_26_c, 6.0, 1, 0, 6);

    TEST_DISSOLVE(dissolve_mail_2017_10_30_a, 0.0001241171, 2, 0, 9);

    TEST_DISSOLVE(dissolve_ticket10713, 0.157052766, 2, 0, 8);

    TEST_MULTI(multi_three_triangles, 42.614078674948232, 1, 1, 13);
    TEST_MULTI(multi_simplex_two, 14.7, 1, 0, 8);
    TEST_MULTI(multi_simplex_three, 16.7945, 1, 0, 14);
    TEST_MULTI(multi_simplex_four, 20.7581, 1, 0, 18);
    TEST_MULTI(multi_disjoint, 24.0, 4, 0, 16);
    TEST_MULTI(multi_new_interior, 19.5206, 1, 1, 18);
    TEST_MULTI(ggl_list_20110307_javier_01_a, 6400.0, 2, 0, 11);

    TEST_DISSOLVE(ggl_list_20110307_javier_01_b, 3993600.0, 1, 2, 16);
    TEST_DISSOLVE_WITH(dissolve_ticket17, 0.00920834633689, 1, 1, 228,
                       settings_for_sensitive_cases);
    TEST_DISSOLVE_WITH(dissolve_reallife, 91756.916526794434, 1, 0, 25,
                       settings_for_sensitive_cases);

#if defined(BOOST_GEOMETRY_TEST_FAILURES)
    TEST_DISSOLVE(gitter_2013_04_a, 3043.9181, 3, 0, 21);
#endif

    TEST_DISSOLVE(gitter_2013_04_b, 31210.429356259738, 1, 0, 11);

    TEST_DISSOLVE(ggl_list_denis, 21123.3281, 2, 0, 22);

    TEST_DISSOLVE(dissolve_mail_2018_08_19, 26.211, 2, 1, 15);
}


int test_main(int, char* [])
{
    test_all<bg::model::d2::point_xy<float>, true >(ut_settings(0.01));
    test_all<bg::model::d2::point_xy<double>, true >(ut_settings());
    // Counter clockwise input does not work correctly in all cases, it is
    // partly a problem of the test itself
    return 0;
}
