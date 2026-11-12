// Boost.Geometry
// Unit Test

// Copyright (c) 2022 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <test_geometries/const_point.hpp>
#include <test_geometries/custom_cartesian_point.hpp>

#include <boost/geometry/algorithms/covered_by.hpp>
#include <boost/geometry/algorithms/crosses.hpp>
#include <boost/geometry/algorithms/disjoint.hpp>
#include <boost/geometry/algorithms/intersects.hpp>
#include <boost/geometry/algorithms/overlaps.hpp>
#include <boost/geometry/algorithms/relate.hpp>
#include <boost/geometry/algorithms/relation.hpp>
#include <boost/geometry/algorithms/touches.hpp>
#include <boost/geometry/algorithms/within.hpp>

#include <boost/geometry/geometries/geometries.hpp>


#include <geometry_test_common.hpp>


template <typename Geometry1, typename Geometry2>
void test_relate_const(Geometry1 const &geometry1, Geometry2 const &geometry2,
    bool exp_cov, bool exp_crosses, bool exp_disjoint, bool exp_intersects,
    bool exp_overlaps, bool exp_touches, bool exp_relate, bool exp_within)
{
    namespace bg = boost::geometry;

    BOOST_CHECK_EQUAL(exp_cov, bg::covered_by(geometry1, geometry2));
    BOOST_CHECK_EQUAL(exp_crosses, bg::crosses(geometry1, geometry2));
    BOOST_CHECK_EQUAL(exp_disjoint, bg::disjoint(geometry1, geometry2));
    BOOST_CHECK_EQUAL(exp_intersects, bg::intersects(geometry1, geometry2));
    BOOST_CHECK_EQUAL(exp_overlaps, bg::overlaps(geometry1, geometry2));
    BOOST_CHECK_EQUAL(exp_touches, bg::touches(geometry1, geometry2));
    BOOST_CHECK_EQUAL(exp_relate, bg::relate(geometry1, geometry2, bg::de9im::mask("F0F******")));
    BOOST_CHECK_EQUAL(exp_within, bg::within(geometry1, geometry2));
}

int test_main(int, char* [])
{
    ring_of_const_point const rectangle{{2, 2}, {2, 4}, {4, 4}, {4, 2}, {2, 2}};
    ring_of_const_point const triangle{{1, 2}, {4, 5}, {5, 4}, {1, 2}};
    linestring_of_const_point const diagonal{{0, 0}, {6, 6}};
    linestring_of_const_point const horizontal{{0, 3}, {6, 3}};

    // areal/areal
    test_relate_const(rectangle, triangle, false, false, false, true, true, false, false, false);

    // point/areal
    test_relate_const(rectangle.front(), triangle, false, false, true, false, false, false, false, false);

    // point/linear
    test_relate_const(rectangle.front(), diagonal, true, false, false, true, false, false, false, true);

    // linear/linear
    test_relate_const(horizontal, diagonal, false, true, false, true, false, false, false, false);

    // A read-only point model can supply a fractional intersection to an
    // integer multi-linestring's boundary checker.
    linestring_of_const_point const fractional{{-1, 0.5}, {1, 0.5}};
    bg::model::multi_linestring<bg::model::linestring<bg::model::d2::point_xy<int>>>
        const integer{{{0, 0}, {0, 2}}};
    test_relate_const(fractional, integer, false, true, false, true, false, false, false, false);
    test_relate_const(integer, fractional, false, true, false, true, false, false, false, false);
    BOOST_CHECK_EQUAL(bg::relation(fractional, integer).str(), "0F1FF0102");
    BOOST_CHECK_EQUAL(bg::relation(integer, fractional).str(), "0F1FF0102");

    custom_cartesian_line l;
    custom_cartesian_polygon poly;
    bg::covered_by(l, poly);
    bg::crosses(l, poly);
    bg::disjoint(l, poly);
    bg::intersects(l, poly);
    bg::overlaps(l, poly);
    bg::touches(l, poly);
    bg::relate(l, poly, bg::de9im::mask("F0F******"));
    bg::within(l, poly);

    return 0;
}
