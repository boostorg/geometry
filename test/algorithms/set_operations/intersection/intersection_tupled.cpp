// Boost.Geometry (aka GGL, Generic Geometry Library)

// Copyright (c) 2020, Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Licensed under the Boost Software License version 1.0.
// http://www.boost.org/users/license.html

#include <geometry_test_common.hpp>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <tuple>

#include <boost/geometry/algorithms/correct.hpp>
#include <boost/geometry/algorithms/detail/overlay/isolated_point_output.hpp>
#include <boost/geometry/algorithms/difference.hpp>
#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/algorithms/length.hpp>
#include <boost/geometry/algorithms/reverse.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/io/wkt/wkt.hpp>
#include <boost/geometry/strategies/cartesian/intersection.hpp>
#include <boost/geometry/strategies/cartesian/point_in_poly_winding.hpp>
#include <boost/geometry/strategies/cartesian/point_in_point.hpp>

#include <boost/tuple/tuple.hpp>

// TEMP
#include <boost/geometry/strategies/cartesian.hpp>
#include <boost/geometry/strategies/geographic.hpp>
#include <boost/geometry/strategies/spherical.hpp>


typedef bg::model::point<double, 2, bg::cs::cartesian> Pt;
typedef bg::model::linestring<Pt> Ls;
typedef bg::model::polygon<Pt> Po;
typedef bg::model::ring<Pt> R;
typedef bg::model::multi_point<Pt> MPt;
typedef bg::model::multi_linestring<Ls> MLs;
typedef bg::model::multi_polygon<Po> MPo;

template <typename Coordinate, bool Clockwise, bool Closed>
void test_coincident_exits()
{
    using point = bg::model::d2::point_xy<Coordinate>;
    using line = bg::model::linestring<point>;
    using lines = bg::model::multi_linestring<line>;
    using polygon = bg::model::polygon<point, Clockwise, Closed>;
    using polygons = bg::model::multi_polygon<polygon>;
    using points = bg::model::multi_point<point>;

    for (bool reverse_line : {false, true})
    for (bool reverse_polygons : {false, true})
    {
        lines a;
        polygons b;
        bg::read_wkt("MULTILINESTRING((160 105,160 135))", a);
        bg::read_wkt("MULTIPOLYGON(((180 135,160 120,200 120,180 135)),"
            "((120 120,160 90,180 105,140 135,120 120)))", b);
        bg::correct(b);
        if (reverse_line) std::reverse(a.front().begin(), a.front().end());
        if (reverse_polygons) std::reverse(b.begin(), b.end());

        lines intersection, difference;
        std::tuple<points, lines, polygons> tuple;
        bg::intersection(a, b, intersection);
        bg::intersection(a, b, tuple);
        bg::difference(a, b, difference);
        BOOST_CHECK_EQUAL(bg::length(intersection), 15);
        BOOST_CHECK_EQUAL(bg::length(std::get<1>(tuple)), 15);
        BOOST_CHECK_EQUAL(bg::length(difference), 15);
    }
}

template <typename G>
inline void check(std::string const& wkt1,
                  std::string const& wkt2,
                  G const& g,
                  std::string const& expected)
{
    G expect;
    bg::read_wkt(expected, expect);
    bg::correct(expect);
    if (! boost::empty(g) || ! boost::empty(expect))
    {
        BOOST_CHECK_MESSAGE(
            // Commented out becasue the output in reversed case may be slightly different
            //   e.g. different number of duplicated points in MultiPoint
            //boost::size(g) == boost::size(expect) &&
            bg::equals(g, expect),
            wkt1 << " x " << wkt2 << " -> " << bg::wkt(g)
                 << " different than expected: " << expected
        );
    }
}

inline void check(std::string const& wkt1,
                  std::string const& wkt2,
                  boost::tuple<MPt, MLs, MPo> const& tup,
                  std::string const& out_p_str,
                  std::string const& out_l_str,
                  std::string const& out_a_str)
{
    check(wkt1, wkt2, boost::get<0>(tup), out_p_str);
    check(wkt1, wkt2, boost::get<1>(tup), out_l_str);
    check(wkt1, wkt2, boost::get<2>(tup), out_a_str);
}

inline void check(std::string const& wkt1,
                  std::string const& wkt2,
                  std::pair<MPt, MLs> const& pair,
                  std::string const& out_p_str,
                  std::string const& out_l_str,
                  std::string const& )
{
    check(wkt1, wkt2, pair.first, out_p_str);
    check(wkt1, wkt2, pair.second, out_l_str);
}

inline void check(std::string const& wkt1,
                  std::string const& wkt2,
                  std::tuple<MPt, MLs, MPo> const& tup,
                  std::string const& out_p_str,
                  std::string const& out_l_str,
                  std::string const& out_a_str)
{
    check(wkt1, wkt2, std::get<0>(tup), out_p_str);
    check(wkt1, wkt2, std::get<1>(tup), out_l_str);
    check(wkt1, wkt2, std::get<2>(tup), out_a_str);
}

template <typename In1, typename In2, typename Tup>
inline void test_one(std::string const& in1_str,
                     std::string const& in2_str,
                     std::string const& out_p_str = "MULTIPOINT()",
                     std::string const& out_l_str = "MULTILINESTRING()",
                     std::string const& out_a_str = "MULTIPOLYGON()")
{
    In1 in1;
    bg::read_wkt(in1_str, in1);
    bg::correct(in1);

    In2 in2;
    bg::read_wkt(in2_str, in2);
    bg::correct(in2);

    {
        Tup result;
        bg::intersection(in1, in2, result);
        check(in1_str, in2_str, result, out_p_str, out_l_str, out_a_str);
    }
    {
        Tup result;
        bg::intersection(in2, in1, result);
        check(in1_str, in2_str, result, out_p_str, out_l_str, out_a_str);
    }
}

// Point-set equality alone does not detect repeated isolated contacts.
template <typename Linear, typename Areal, typename Tup>
void test_unique_contacts(std::string const& linear_wkt,
                          std::string const& areal_wkt,
                          std::string const& points_wkt,
                          std::string const& lines_wkt = "MULTILINESTRING()")
{
    Linear linear;
    Areal areal;
    MPt points;
    MLs lines;
    bg::read_wkt(linear_wkt, linear);
    bg::read_wkt(areal_wkt, areal);
    bg::correct(areal);
    bg::read_wkt(points_wkt, points);
    bg::read_wkt(lines_wkt, lines);

    for (bool reverse_linear : {false, true})
    {
        if (reverse_linear)
        {
            bg::reverse(linear);
        }
        for (bool reverse_operands : {false, true})
        {
            Tup result;
            if (reverse_operands)
            {
                bg::intersection(areal, linear, result);
            }
            else
            {
                bg::intersection(linear, areal, result);
            }
            check(linear_wkt, areal_wkt, result, points_wkt,
                  lines_wkt, "MULTIPOLYGON()");
            BOOST_CHECK_EQUAL(boost::size(bg::tuples::get<0>(result)), boost::size(points));
            BOOST_CHECK_EQUAL(boost::size(bg::tuples::get<1>(result)), boost::size(lines));
        }
    }
}

template <typename Tup>
void test_unique_linear_areal_contacts()
{
    // Regression cases from issue #1406, using valid polygon orientation.
    test_unique_contacts<Ls, Po, Tup>("LINESTRING(1.5 1.5,0.5 1.5)",
        "POLYGON((1 1.5,2.5 4.5,4 6,3.5 4.5,1 1.5))", "MULTIPOINT(1 1.5)");
    test_unique_contacts<Ls, Po, Tup>("LINESTRING(0 0,1 1)",
        "POLYGON((1 1,3 5,4 6,3 4,1 1))", "MULTIPOINT(1 1)");

    for (auto const& triangles : {
        "MULTIPOLYGON(((0 0,-2 1,-2 2,0 0)),((0 0,2 2,2 1,0 0)))",
        "MULTIPOLYGON(((0 0,2 2,2 1,0 0)),((0 0,-2 1,-2 2,0 0)))"})
    {
        test_unique_contacts<Ls, MPo, Tup>("LINESTRING(-1 0,1 0)", triangles,
                                          "MULTIPOINT(0 0)");
        test_unique_contacts<Ls, MPo, Tup>("LINESTRING(-1 0,0 0)", triangles,
                                          "MULTIPOINT(0 0)");
        test_unique_contacts<Ls, MPo, Tup>("LINESTRING(-1 0,1 0,-1 0,1 0)", triangles,
                                          "MULTIPOINT(0 0)");
        test_unique_contacts<Ls, MPo, Tup>("LINESTRING(-1 0,0 0,0 0,1 0)", triangles,
                                          "MULTIPOINT(0 0)");
        test_unique_contacts<MLs, MPo, Tup>("MULTILINESTRING((-1 0,1 0),(1 0,-1 0))",
                                           triangles, "MULTIPOINT(0 0)");
    }

    test_unique_contacts<Ls, MPo, Tup>("LINESTRING(-1 0,5 0)",
        "MULTIPOLYGON(((0 0,-2 1,-2 2,0 0)),((0 0,2 2,2 1,0 0)),"
        "((4 0,3 1,5 1,4 0)))", "MULTIPOINT(0 0,4 0)");
    test_unique_contacts<Ls, MPo, Tup>("LINESTRING(-1 -1,1 1)",
        "MULTIPOLYGON(((0.25 0.25,-2 1,-2 2,0.25 0.25)),"
        "((0.25 0.25,2 3,2 2.5,0.25 0.25)))", "MULTIPOINT(0.25 0.25)");

    // A collapsed interval and a touch at the same position share one point.
    test_unique_contacts<Ls, MPo, Tup>("LINESTRING(4 2,6 2)",
        "MULTIPOLYGON(((0 0,4 0,4 4,0 4,0 0),(2 1,2 3,4 2,2 1)),"
        "((4 2,5 3,6 3,4 2)))", "MULTIPOINT(4 2)");
    test_unique_contacts<Ls, Po, Tup>("LINESTRING(4 2,6 2)",
        "POLYGON((0 0,4 0,4 4,0 4,0 0),(2 1,2 3,4 2,2 1))",
        "MULTIPOINT(4 2)");

    // A touch at an interval's endpoint belongs to the linear output.
    test_unique_contacts<Ls, MPo, Tup>("LINESTRING(-1 0,1 0)",
        "MULTIPOLYGON(((-2 -1,-2 0,0 0,0 -1,-2 -1)),((0 0,2 2,2 1,0 0)))",
        "MULTIPOINT()", "MULTILINESTRING((-1 0,0 0))");

    // Calls append their own unique contacts, preserving all earlier entries.
    Ls linear;
    MPo areal;
    Tup result;
    bg::read_wkt("LINESTRING(-1 0,1 0)", linear);
    bg::read_wkt("MULTIPOLYGON(((0 0,-2 1,-2 2,0 0)),((0 0,2 2,2 1,0 0)))", areal);
    bg::read_wkt("MULTIPOINT(9 9,0 0,9 9)", bg::tuples::get<0>(result));
    bg::read_wkt("MULTILINESTRING((9 9,10 10))", bg::tuples::get<1>(result));
    for (std::size_t count : {4, 5})
    {
        bg::intersection(linear, areal, result);
        auto const& points = bg::tuples::get<0>(result);
        BOOST_CHECK_EQUAL(boost::size(points), count);
        BOOST_CHECK_EQUAL(bg::get<0>(points[0]), 9);
        BOOST_CHECK_EQUAL(bg::get<0>(points[1]), 0);
        BOOST_CHECK_EQUAL(bg::get<0>(points[2]), 9);
        BOOST_CHECK_EQUAL(bg::get<0>(points[count - 1]), 0);
        BOOST_CHECK_EQUAL(boost::size(bg::tuples::get<1>(result)), 1);
    }
}

void test_unique_contact_iterators()
{
    Ls linear;
    MPo areal;
    bg::read_wkt("LINESTRING(5 0,-1 0)", linear);
    bg::read_wkt("MULTIPOLYGON(((0 0,-2 1,-2 2,0 0)),((0 0,2 2,2 1,0 0)),"
                 "((4 0,3 1,5 1,4 0)))", areal);
    Pt points[8];
    Ls lines[1];
    Po polygons[1];
    auto const out = bg::detail::intersection::intersection_insert<std::tuple<Pt, Ls, Po>>(
        linear, areal, std::make_tuple(points + 0, lines + 0, polygons + 0));
    BOOST_CHECK(std::get<0>(out) == points + 2);
    BOOST_CHECK(std::get<1>(out) == lines);
    BOOST_CHECK(std::get<2>(out) == polygons);
    BOOST_CHECK_EQUAL(bg::get<0>(points[0]), 4);
    BOOST_CHECK_EQUAL(bg::get<0>(points[1]), 0);

    // Distinct source coordinates can coincide after output conversion.
    using integer_point = bg::model::point<int, 2, bg::cs::cartesian>;
    std::pair<bg::model::multi_point<integer_point>, MLs> result;
    bg::read_wkt("LINESTRING(0 0,1 0)", linear);
    bg::read_wkt("MULTIPOLYGON(((0.25 0,0.2 1,0.3 1,0.25 0)),"
                 "((0.75 0,0.7 1,0.8 1,0.75 0)))", areal);
    bg::intersection(linear, areal, result, bg::strategies::relate::cartesian<>());
    BOOST_CHECK_EQUAL(result.first.size(), 1);
    BOOST_CHECK_EQUAL(bg::get<0>(result.first.front()), 0);
    BOOST_CHECK_EQUAL(bg::get<1>(result.first.front()), 0);
    BOOST_CHECK(result.second.empty());
}

template <typename CoordinateSystem, typename OutputCoordinate = double, typename Strategy>
void test_unique_angular_contacts(Strategy const& strategy)
{
    using point = bg::model::point<double, 2, CoordinateSystem>;
    using line = bg::model::linestring<point>;
    using lines = bg::model::multi_linestring<line>;
    using polygon = bg::model::polygon<point>;
    using polygons = bg::model::multi_polygon<polygon>;
    using output_point = bg::model::point<OutputCoordinate, 2, CoordinateSystem>;
    using points = bg::model::multi_point<output_point>;

    lines linear;
    polygons areal;
    bg::read_wkt("MULTILINESTRING((-1 0,1 0),(1 0,-1 0))", linear);
    bg::read_wkt("MULTIPOLYGON(((0 0,-2 1,-2 2,0 0)),((0 0,2 2,2 1,0 0)))", areal);
    for (bool reverse_linear : {false, true})
    {
        if (reverse_linear)
        {
            bg::reverse(linear);
        }
        for (bool reverse_operands : {false, true})
        {
            std::tuple<points, lines, polygons> result;
            if (reverse_operands)
            {
                bg::intersection(areal, linear, result, strategy);
            }
            else
            {
                bg::intersection(linear, areal, result, strategy);
            }
            BOOST_CHECK_EQUAL(std::get<0>(result).size(), 1);
            BOOST_CHECK_EQUAL(bg::get<0>(std::get<0>(result).front()), 0);
            BOOST_CHECK_EQUAL(bg::get<1>(std::get<0>(result).front()), 0);
            BOOST_CHECK(std::get<1>(result).empty());
            BOOST_CHECK(std::get<2>(result).empty());
        }
    }
}

void test_unique_large_cartesian_contact_keys()
{
    using point = bg::model::point<std::int64_t, 2, bg::cs::cartesian>;
    using line = bg::model::linestring<point>;
    using strategy = bg::strategies::relate::cartesian<>;
    using collector = bg::detail::overlay::following::isolated_point_output
        <std::pair<point, line>, strategy, true>;
    std::int64_t const first = std::int64_t(1) << 53;
    point contacts[] = {point(first, 0), point(first + 1, 0), point(first + 2, 0)};
    std::size_t order[] = {0, 1, 2};
    do
    {
        collector unique_points;
        bg::model::multi_point<point> result;
        auto out = std::back_inserter(result);
        for (std::size_t index : order)
        {
            unique_points.apply(contacts[index], out, strategy());
        }
        BOOST_REQUIRE_EQUAL(result.size(), 3);
        for (std::size_t i = 0; i < result.size(); ++i)
        {
            BOOST_CHECK_EQUAL(bg::get<0>(result[i]), first + order[i]);
            BOOST_CHECK_EQUAL(bg::get<1>(result[i]), 0);
            unique_points.apply(contacts[order[i]], out, strategy());
        }
        BOOST_CHECK_EQUAL(result.size(), 3);
    }
    while (std::next_permutation(std::begin(order), std::end(order)));
}

// A set must order equivalent poles consistently relative to other points.
template <typename CoordinateSystem, typename Coordinate = double, typename Strategy>
void test_unique_angular_contact_keys(Strategy const& strategy,
                                      double half_period,
                                      double pole_latitude,
                                      double near_pole_direction,
                                      double equator_latitude)
{
    using point = bg::model::point<Coordinate, 2, CoordinateSystem>;
    using line = bg::model::linestring<point>;
    using output = std::pair<point, line>;
    using collector = bg::detail::overlay::following::isolated_point_output<output, Strategy, true>;
    double const degree = half_period / 180;
    point contacts[4];
    bg::set<0>(contacts[0], half_period);
    bg::set<1>(contacts[0], pole_latitude);
    bg::set<0>(contacts[1], half_period - degree);
    bg::set<1>(contacts[1], pole_latitude + near_pole_direction * degree);
    bg::set<0>(contacts[2], half_period - 2 * degree);
    bg::set<1>(contacts[2], pole_latitude + 2 * near_pole_direction * degree);
    bg::set<0>(contacts[3], 0);
    bg::set<1>(contacts[3], pole_latitude);
    point antimeridian;
    bg::set<0>(antimeridian, -half_period);
    bg::set<1>(antimeridian, equator_latitude);

    std::size_t order[] = {0, 1, 2, 3};
    do
    {
        collector unique_points;
        bg::model::multi_point<point> result;
        auto out = std::back_inserter(result);
        unique_points.apply(antimeridian, out, strategy);
        bool seen[] = {false, false, false};
        std::size_t count = 1;
        for (std::size_t index : order)
        {
            unique_points.apply(contacts[index], out, strategy);
            std::size_t const group = index == 3 ? 0 : index;
            if (!seen[group])
            {
                seen[group] = true;
                BOOST_REQUIRE_EQUAL(result.size(), ++count);
                BOOST_CHECK_EQUAL(bg::get<0>(result.back()), bg::get<0>(contacts[index]));
                BOOST_CHECK_EQUAL(bg::get<1>(result.back()), bg::get<1>(contacts[index]));
            }
            else
            {
                BOOST_CHECK_EQUAL(result.size(), count);
            }
        }
        point positive_antimeridian = antimeridian;
        bg::set<0>(positive_antimeridian, half_period);
        unique_points.apply(positive_antimeridian, out, strategy);
        BOOST_CHECK_EQUAL(result.size(), 4);
        BOOST_CHECK_EQUAL(bg::get<0>(result.front()), -half_period);
        BOOST_CHECK_EQUAL(bg::get<1>(result.front()), equator_latitude);
    }
    while (std::next_permutation(std::begin(order), std::end(order)));
}

template <typename Tup>
inline void test_pp()
{
    test_one<Pt, Pt, Tup>(
        "POINT(0 0)",
        "POINT(0 0)",
        "MULTIPOINT(0 0)");

    test_one<Pt, MPt, Tup>(
        "POINT(0 0)",
        "MULTIPOINT(0 0, 1 1)",
        "MULTIPOINT(0 0)");

    test_one<MPt, MPt, Tup>(
        "MULTIPOINT(0 0, 1 1, 2 2)",
        "MULTIPOINT(1 1, 2 2, 3 3)",
        "MULTIPOINT(1 1, 2 2)");
}

template <typename Tup>
inline void test_pl()
{
    test_one<Pt, Ls, Tup>(
        "POINT(0 0)",
        "LINESTRING(0 0, 1 1)",
        "MULTIPOINT(0 0)");

    test_one<Pt, MLs, Tup>(
        "POINT(1 1)",
        "MULTILINESTRING((0 0, 1 1),(1 1, 2 2),(4 4, 5 5))",
        "MULTIPOINT(1 1)");

    test_one<MPt, Ls, Tup>(
        "MULTIPOINT(0 0, 1 1, 2 2, 3 3)",
        "LINESTRING(0 0, 1 1)",
        "MULTIPOINT(0 0, 1 1)");

    test_one<MPt, MLs, Tup>(
        "MULTIPOINT(0 0, 1 1, 2 2, 3 3)",
        "MULTILINESTRING((0 0, 1 1),(1 1, 2 2),(4 4, 5 5))",
        "MULTIPOINT(0 0, 1 1, 2 2)");
}

template <typename Tup>
inline void test_pa()
{
    test_one<Pt, R, Tup>(
        "POINT(0 0)",
        "POLYGON((0 0, 0 5, 5 5, 5 0, 0 0))",
        "MULTIPOINT(0 0)");

    test_one<Pt, Po, Tup>(
        "POINT(0 0)",
        "POLYGON((0 0, 0 5, 5 5, 5 0, 0 0),(0 0, 4 1, 4 4, 1 4, 0 0))",
        "MULTIPOINT(0 0)");

    test_one<Pt, MPo, Tup>(
        "POINT(0 0)",
        "MULTIPOLYGON(((0 0, 0 5, 5 5, 5 0, 0 0),(0 0, 4 1, 4 4, 1 4, 0 0)),((0 0, 1 2, 2 2, 2 1, 0 0)))",
        "MULTIPOINT(0 0)");

    test_one<MPt, R, Tup>(
        "MULTIPOINT(0 0, 1 1, 2 2, 3 3, 4 4, 5 5, 6 6)",
        "POLYGON((0 0, 0 5, 5 5, 5 0, 0 0))",
        "MULTIPOINT(0 0, 1 1, 2 2, 3 3, 4 4, 5 5)");

    test_one<MPt, Po, Tup>(
        "MULTIPOINT(0 0, 1 1, 2 2, 3 3, 4 4, 5 5, 6 6)",
        "POLYGON((0 0, 0 5, 5 5, 5 0, 0 0),(0 0, 4 1, 4 4, 1 4, 0 0))",
        "MULTIPOINT(0 0, 4 4, 5 5)");

    test_one<MPt, MPo, Tup>(
        "MULTIPOINT(0 0, 0 0, 1 1, 2 2, 3 3, 4 4, 5 5, 6 6)",
        "MULTIPOLYGON(((0 0, 0 5, 5 5, 5 0, 0 0),(0 0, 4 1, 4 4, 1 4, 0 0)),((0 0, 1 2, 2 2, 2 1, 0 0)))",
        "MULTIPOINT(0 0, 0 0, 1 1, 2 2, 4 4, 5 5)"); // one (0 0) could be filtered out
}

template <typename Tup>
inline void test_ll()
{
    test_one<Ls, Ls, Tup>(
        "LINESTRING(0 0, 1 0, 2 1, 3 0)",
        "LINESTRING(0 0, 1 0, 3 0, 4 0)",
        "MULTIPOINT(3 0)",
        "MULTILINESTRING((0 0, 1 0))");

    test_one<Ls, MLs, Tup>(
        "LINESTRING(0 0, 1 0, 2 1, 3 0)",
        "MULTILINESTRING((0 0, 1 0, 3 0),(2 1, 2 2))",
        "MULTIPOINT(3 0, 2 1)",
        "MULTILINESTRING((0 0, 1 0))");

    test_one<MLs, MLs, Tup>(
        "MULTILINESTRING((0 0, 1 0, 2 1),(2 1, 3 0))",
        "MULTILINESTRING((0 0, 1 0, 3 0),(2 1, 2 2))",
        "MULTIPOINT(3 0, 2 1, 2 1)", // (2 1) could be filtered out
        "MULTILINESTRING((0 0, 1 0))");


    test_one<Ls, Ls, Tup>(
        "LINESTRING(0 0, 0 5, 5 5, 5 0, 0 0)",
        "LINESTRING(0 0, 0 1, 6 1, 5 2, 5 5, 5 6, 4 5, 4 7, 7 7, 7 0, 0 0)",
        "MULTIPOINT(4 5, 5 1, 0 0)", // (0 0) could be filtered out
        "MULTILINESTRING((0 0, 0 1), (5 5, 5 2), (5 0, 0 0))");

    test_one<MLs, MLs, Tup>(
        "MULTILINESTRING((0 0, 0 5, 5 5, 5 0, 0 0),(0 0, 4 1, 4 4, 1 4, 0 0))",
        "MULTILINESTRING((0 0, 1 4, 5 4, 5 1, 4 1, 0 0),(0 0, 2 1, 2 2, 1 2, 0 0))",
        // all (0 0) could be filtered out
        "MULTIPOINT(0 0, 0 0, 0 0, 0 0, 0 0, 0 0, 0 0, 0 0, 0 0, 0 0, 0 0, 0 0)",
        "MULTILINESTRING((5 4,5 1),(0 0,4 1),(4 4,1 4,0 0))");
}

template <typename Tup>
inline void test_la()
{
    test_one<MLs, MPo, Tup>("MULTILINESTRING((180 30,180 60))",
        "MULTIPOLYGON(((160 60,160 45,200 45,200 60,160 60)),"
        "((160 30,200 0,200 30,180 45,160 30)))",
        "MULTIPOINT()", "MULTILINESTRING((180 30,180 60))");

    test_one<MLs, MPo, Tup>("MULTILINESTRING((6 4,6 8))",
        "MULTIPOLYGON(((0 0,8 0,8 8,0 8,0 0),(2 2,2 6,6 4,2 2)),"
        "((3 3,6 4,3 5,3 3)))", "MULTIPOINT()", "MULTILINESTRING((6 4,6 8))");
    for (auto const& area : {
        "MULTIPOLYGON(((80 90,60 75,80 60,80 90)),"
        "((60 75,40 90,0 60,40 30,60 45,60 75),(60 75,40 60,40 75,60 75)))",
        "MULTIPOLYGON(((60 75,40 90,0 60,40 30,60 45,60 75),"
        "(60 75,40 60,40 75,60 75)),((80 90,60 75,80 60,80 90)))"})
    {
        test_one<MLs, MPo, Tup>("MULTILINESTRING((80 60,60 75,60 90))", area,
            "MULTIPOINT()", "MULTILINESTRING((80 60,60 75))");
        test_one<MLs, MPo, Tup>("MULTILINESTRING((60 90,60 75,80 60))", area,
            "MULTIPOINT()", "MULTILINESTRING((60 75,80 60))");
    }

    test_one<MLs, MPo, Tup>("MULTILINESTRING((4 4,4 8))",
        "MULTIPOLYGON(((0 0,8 0,8 8,0 8,0 0),(2 2,2 6,6 6,6 2,2 2)),"
        "((4 4,6 4,4 6,4 4)))", "MULTIPOINT()", "MULTILINESTRING((4 4,4 8))");

    test_one<Ls, Po, Tup>("LINESTRING(4 2,6 2)",
        "POLYGON((0 0,4 0,4 4,0 4,0 0),(2 1,2 3,4 2,2 1))", "MULTIPOINT(4 2)");
    test_one<Ls, Po, Tup>("LINESTRING(6 2,4 2)",
        "POLYGON((0 0,4 0,4 4,0 4,0 0),(2 1,2 3,4 2,2 1))", "MULTIPOINT(4 2)");

    test_one<MLs, MPo, Tup>(
        "MULTILINESTRING((180 60,160 75,140 75,120 75))",
        "MULTIPOLYGON(((160 75,120 75,140 60,160 60,160 75)),"
        "((140 75,140 90,120 90,140 75)))",
        "MULTIPOINT()", "MULTILINESTRING((160 75,120 75))");
    test_one<Ls, Po, Tup>("LINESTRING(0 1,2 1)",
        "POLYGON((1 1,2 2,0 2,1 1))", "MULTIPOINT(1 1)");
    test_one<Ls, Po, Tup>("LINESTRING(1 1,2 0)",
        "POLYGON((1 1,2 2,0 2,1 1))", "MULTIPOINT(1 1)");
    test_one<Ls, Po, Tup>("LINESTRING(2 0,1 1)",
        "POLYGON((1 1,2 2,0 2,1 1))", "MULTIPOINT(1 1)");

    test_one<Ls, R, Tup>(
        "LINESTRING(0 2, -4 1, 0 0, 5 0, 9 1, 5 2, 9 3, 5 5, 4 9, 4 5, 3 3, 2 5, 2 9, 0 5)",
        "POLYGON((0 0, 0 5, 5 5, 5 0, 0 0))",
        "MULTIPOINT(0 2, 5 2, 5 5, 0 5)",
        "MULTILINESTRING((0 0, 5 0),(4 5, 3 3, 2 5))");

    test_one<Ls, Po, Tup>(
        "LINESTRING(1 4, -4 1, 0 0, 5 0, 9 1, 5 2, 9 3, 5 5, 4 9, 4 5, 3 3, 2 5, 2 9, 0 5)",
        "POLYGON((0 0, 0 5, 5 5, 5 0, 0 0),(0 0, 4 1, 4 4, 1 4, 0 0))",
        "MULTIPOINT(5 2, 5 5, 0 5)",
        "MULTILINESTRING((1 4, 0 3.4)(0 0, 5 0),(4 5, 3.5 4),(2.5 4, 2 5))");

    test_one<MLs, R, Tup>(
        "MULTILINESTRING((0 2, -4 1, 0 0, 5 0, 9 1, 5 2, 9 3, 5 5, 4 9), (4 9, 4 5, 3 3, 2 5, 2 9, 0 5))",
        "POLYGON((0 0, 0 5, 5 5, 5 0, 0 0))",
        "MULTIPOINT(0 2, 5 2, 5 5, 0 5)",
        "MULTILINESTRING((0 0, 5 0),(4 5, 3 3, 2 5))");

    test_one<MLs, Po, Tup>(
        "MULTILINESTRING((1 4, -4 1, 0 0, 5 0, 9 1, 5 2, 9 3, 5 5, 4 9), (4 9, 4 5, 3 3, 2 5, 2 9, 0 5))",
        "POLYGON((0 0, 0 5, 5 5, 5 0, 0 0),(0 0, 4 1, 4 4, 1 4, 0 0))",
        "MULTIPOINT(5 2, 5 5, 0 5)",
        "MULTILINESTRING((1 4, 0 3.4)(0 0, 5 0),(4 5, 3.5 4),(2.5 4, 2 5))");

    test_one<MLs, MPo, Tup>(
        "MULTILINESTRING((1 4, -4 1, 0 0, 5 0, 9 1, 5 2, 9 3, 5 5, 4 9), (4 9, 4 5, 4 4, 2 2, 2 5, 1 9, 0 5))",
        "MULTIPOLYGON(((0 0, 0 5, 5 5, 5 0, 0 0),(0 0, 4 1, 4 4, 1 4, 0 0)),((0 0, 1 2, 2 2, 2 1, 0 0)))",
        "MULTIPOINT(5 2, 5 5, 2 2, 0 5)",
        "MULTILINESTRING((1 4, 0 3.4)(0 0, 5 0),(4 5, 4 4),(2 4, 2 5))");


    test_one<Ls, R, Tup>(
        "LINESTRING(0 0, 0 5, 5 5, 5 0, 0 0)",
        "POLYGON((0 0, 0 1, 6 1, 5 2, 5 5, 5 6, 4 5, 4 7, 7 7, 7 0, 0 0))",
        "MULTIPOINT(4 5)",
        "MULTILINESTRING((0 0, 0 1), (5 5, 5 2), (5 1, 5 0, 0 0))");

    test_one<MLs, Po, Tup>(
        "MULTILINESTRING((0 0, 0 5, 5 5, 5 0, 0 0),(0 0, 4 1, 4 4, 1 4, 0 0))",
        "POLYGON((0 0, 1 4, 5 4, 5 1, 4 1, 0 0),(0 0, 2 1, 2 2, 1 2, 0 0))",
        "MULTIPOINT(0 0)", // (0 0) could be filtered out
        "MULTILINESTRING((5 4, 5 1),(0 0, 4 1, 4 4, 1 4, 0 0))");
}

template <typename Tup>
inline void test_aa()
{
    test_one<R, R, Tup>(
        "POLYGON((0 0, 0 5, 5 5, 5 0, 0 0))",
        "POLYGON((0 0, 0 1, 6 1, 5 2, 5 5, 5 6, 4 5, 4 7, 7 7, 7 0, 0 0))",
        "MULTIPOINT(4 5)",
        "MULTILINESTRING((5 2, 5 5))",
        "MULTIPOLYGON(((0 0, 0 1, 5 1, 5 0, 0 0)))");

    test_one<R, MPo, Tup>(
        "POLYGON((0 0, 0 5, 5 5, 5 0, 0 0))",
        "MULTIPOLYGON(((0 0, 0 1, 6 1, 6 0, 0 0)),"
                     "((6 1, 5 2, 5 5, 5 6, 4 5, 4 7, 7 7, 7 1, 6 1)))",
        "MULTIPOINT(4 5)",
        "MULTILINESTRING((5 2, 5 5))",
        "MULTIPOLYGON(((0 0, 0 1, 5 1, 5 0, 0 0)))");

    test_one<Po, Po, Tup>(
        "POLYGON((0 0, 0 5, 5 5, 5 0, 0 0),(0 0, 4 1, 4 4, 1 4, 0 0))",
        "POLYGON((0 0, 1 4, 5 4, 5 1, 4 1, 0 0),(0 0, 2 1, 2 2, 1 2, 0 0))",
        // all (0 0) could be filtered out
        "MULTIPOINT(0 0, 0 0, 0 0, 0 0, 0 0, 0 0, 0 0, 0 0, 0 0, 0 0, 0 0, 0 0)",
        "MULTILINESTRING((0 0, 1 4, 4 4),(0 0, 4 1))",
        "MULTIPOLYGON(((4 1, 4 4, 5 4, 5 1, 4 1)))");

    test_one<Po, MPo, Tup>(
        "POLYGON((0 0, 0 5, 5 5, 5 0, 0 0),(0 0, 4 1, 4 4, 1 4, 0 0))",
        "MULTIPOLYGON(((0 0, 1 4, 5 4, 5 1, 4 1, 0 0),(0 0, 2 1, 2 2, 1 2, 0 0)),"
                     "((5 0, 5 1, 6 1, 6 4, 5 4, 3 6, 2 5, 2 7, 7 7, 7 0 5 0)))",
        // all (0 0) and (5 0) could be filtered out
        "MULTIPOINT(2 5, 5 0, 0 0, 0 0, 0 0, 0 0, 0 0, 0 0, 0 0, 0 0, 0 0, 0 0, 0 0, 0 0)",
        "MULTILINESTRING((0 0, 1 4, 4 4),(0 0, 4 1),(5 1, 5 0))",
        "MULTIPOLYGON(((4 1, 4 4, 5 4, 5 1, 4 1)),((5 4, 4 5, 5 5, 5 4)))");

    test_one<MPo, MPo, Tup>(
        "MULTIPOLYGON(((0 0, 0 5, 5 5, 5 0, 0 0),(0 0, 4 1, 4 4, 1 4, 0 0)),"
                     "((2 6, 2 8, 8 8, 8 5, 7 5, 7 6, 2 6)))",
        "MULTIPOLYGON(((0 0, 1 4, 5 4, 5 1, 4 1, 0 0),(0 0, 2 1, 2 2, 1 2, 0 0)),"
                     "((5 0, 5 1, 6 1, 6 4, 5 4, 3 6, 2 5, 2 7, 7 7, 7 0 5 0)))",
        // all (0 0) and (5 0) could be filtered out
        "MULTIPOINT(2 5, 5 0, 0 0, 0 0, 0 0, 0 0, 0 0, 0 0, 0 0, 0 0, 0 0, 0 0, 0 0, 0 0)",
        "MULTILINESTRING((0 0, 1 4, 4 4),(0 0, 4 1),(5 1, 5 0),(7 5, 7 6))",
        "MULTIPOLYGON(((4 1, 4 4, 5 4, 5 1, 4 1)),"
                     "((5 4, 4 5, 5 5, 5 4)),"
                     "((2 6, 2 7, 7 7, 7 6, 2 6)))");
}

template <typename Tup>
inline void test_pair()
{
    test_pp<Tup>();
    test_pl<Tup>();
    test_pa<Tup>();
    test_ll<Tup>();
    test_la<Tup>();
}

template <typename Tup>
inline void test_tuple()
{
    test_pp<Tup>();
    test_pl<Tup>();
    test_pa<Tup>();
    test_ll<Tup>();
    test_la<Tup>();
    test_aa<Tup>();
}

int test_main(int, char* [])
{
    test_unique_linear_areal_contacts<std::pair<MPt, MLs>>();
    test_unique_linear_areal_contacts<boost::tuple<MPt, MLs, MPo>>();
    test_unique_linear_areal_contacts<std::tuple<MPt, MLs, MPo>>();
    test_unique_contact_iterators();
    test_unique_angular_contacts<bg::cs::spherical_equatorial<bg::degree>>(
        bg::strategies::relate::spherical<>());
    test_unique_angular_contacts<bg::cs::geographic<bg::degree>>(
        bg::strategies::relate::geographic<>());
    test_unique_angular_contacts<bg::cs::spherical_equatorial<bg::degree>, std::int16_t>(
        bg::strategies::relate::spherical<>());
    test_unique_angular_contacts<bg::cs::geographic<bg::degree>, std::int16_t>(
        bg::strategies::relate::geographic<>());
    test_unique_large_cartesian_contact_keys();
    test_unique_angular_contact_keys<bg::cs::spherical_equatorial<bg::degree>>(
        bg::strategies::relate::spherical<>(), 180, 90, -1, 0);
    test_unique_angular_contact_keys<bg::cs::geographic<bg::degree>>(
        bg::strategies::relate::geographic<>(), 180, 90, -1, 0);
    test_unique_angular_contact_keys<bg::cs::spherical<bg::degree>>(
        bg::strategies::relate::spherical<>(), 180, 0, 1, 90);
    test_unique_angular_contact_keys<bg::cs::spherical_equatorial<bg::degree>, std::int64_t>(
        bg::strategies::relate::spherical<>(), 180, 90, -1, 0);
    test_unique_angular_contact_keys<bg::cs::geographic<bg::degree>, std::int64_t>(
        bg::strategies::relate::geographic<>(), 180, 90, -1, 0);
    double const pi = bg::math::pi<double>();
    test_unique_angular_contact_keys<bg::cs::spherical_equatorial<bg::radian>>(
        bg::strategies::relate::spherical<>(), pi, pi / 2, -1, 0);
    test_unique_angular_contact_keys<bg::cs::geographic<bg::radian>>(
        bg::strategies::relate::geographic<>(), pi, pi / 2, -1, 0);

    test_coincident_exits<std::int_least64_t, true, true>();
    test_coincident_exits<std::int_least64_t, false, false>();
    test_coincident_exits<double, true, true>();
    test_coincident_exits<double, false, false>();

    test_pair<std::pair<MPt, MLs> >();
    test_tuple<boost::tuple<MPt, MLs, MPo> >();
    test_tuple<std::tuple<MPt, MLs, MPo> >();

    return 0;
}
