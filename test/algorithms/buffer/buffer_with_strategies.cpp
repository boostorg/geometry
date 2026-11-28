// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2015-2019 Barend Gehrels, Amsterdam, the Netherlands.
// Copyright (c) 2022 Adam Wulkiewicz, Lodz, Poland.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "geometry_test_common.hpp"

#include <boost/geometry/algorithms/buffer.hpp>

#include <boost/geometry/strategies/strategies.hpp>
#include <boost/geometry/strategies/relate/cartesian_exact.hpp>
#include <boost/geometry/strategies/relate/simple_cartesian.hpp>

#include <boost/geometry/geometries/geometries.hpp>

// For test
#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/correct.hpp>
#include <boost/geometry/algorithms/is_valid.hpp>
#include <boost/geometry/algorithms/num_points.hpp>
#include <boost/geometry/io/wkt/read.hpp>


// This unit test tests boost::geometry::buffer (overload with strategies)
// More detailed tests are, per geometry type, available in buffer_<TYPE>.cpp
// (testing boost::geometry::buffer_inserter)

// SVG's are not created (they are in detailed tests)

static std::string const polygon_simplex = "POLYGON ((0 0,1 5,6 1,0 0))";
static std::string const polygon_empty = "POLYGON()";
static std::string const multi_polygon_empty = "MULTIPOLYGON()";
static std::string const multi_polygon_simplex
    = "MULTIPOLYGON(((0 1,2 5,5 3,0 1)),((1 1,5 2,5 0,1 1)))";


template
<
    typename Geometry,
    typename GeometryOut,
    typename JoinStrategy,
    typename EndStrategy,
    typename SideStrategy,
    typename PointStrategy,
    typename DistanceStrategy
>
void test_with_strategies(std::string const& caseid,
        std::string const& wkt,
        JoinStrategy const& join_strategy,
        EndStrategy const& end_strategy,
        SideStrategy const& side_strategy,
        PointStrategy const& point_strategy,
        DistanceStrategy const& distance_strategy,
        double expected_area,
        std::size_t expected_numpoints,
        double tolerance = 0.01)
{
    namespace bg = boost::geometry;
    Geometry g;
    bg::read_wkt(wkt, g);
    bg::correct(g);

    GeometryOut result;

    bg::buffer(g, result,
                distance_strategy, side_strategy,
                join_strategy, end_strategy, point_strategy);

    BOOST_CHECK_MESSAGE
        (
            bg::num_points(result) == expected_numpoints,
            "Buffer " << caseid
            << " numpoints expected: " << expected_numpoints
            << " detected: " << bg::num_points(result)
        );

    auto const area = bg::area(result);
    auto const difference = area - expected_area;

    BOOST_CHECK_MESSAGE
        (
            bg::math::abs(difference) < tolerance,
            "Buffer " << caseid
            << std::setprecision(12)
            << " area expected: " << expected_area
            << " detected: " << area
        );
}


template <bool Clockwise, typename Point>
void test_all()
{
    typedef bg::model::polygon<Point, Clockwise> polygon;
    typedef bg::model::multi_polygon<polygon> multi_polygon;

    bg::strategy::buffer::join_round join(160);
    bg::strategy::buffer::end_round end(160);
    bg::strategy::buffer::point_circle circle(160);
    bg::strategy::buffer::side_straight side;

    typedef bg::strategy::buffer::distance_symmetric
    <
        typename bg::coordinate_type<Point>::type
    > distance;

    test_with_strategies<multi_polygon, multi_polygon>(
        "multi_polygon_empty", multi_polygon_empty,
        join, end, side, circle, distance(1.0),
        0.0, 0);


    // PostGIS: 34.2550669294223 216 (40 / qcircle)
    // SQL Server: 34.2550419903829 220 (default options)
    test_with_strategies<multi_polygon, multi_polygon>(
        "multi_polygon_simplex", multi_polygon_simplex,
        join, end, side, circle, distance(1.0),
        34.2551, 219);

    test_with_strategies<polygon, multi_polygon>(
        "polygon_empty", polygon_empty,
        join, end, side, circle, distance(1.0),
        0.0, 0);

    //
    // PostGIS: 35.2256914798762 164 (40 / qcircle)
    // SQL Server: 35.2252355201605 153 (default options)
    test_with_strategies<polygon, multi_polygon>(
        "polygon_simplex", polygon_simplex,
        join, end, side, circle, distance(1.0),
        35.2257, 166);
}

template <typename Umbrella>
void test_piece_strategy(Umbrella const& umbrella)
{
    using point = bg::model::point<double, 2, bg::cs::cartesian>;
    using polygon = bg::model::polygon<point, false>;
    using multi_polygon = bg::model::multi_polygon<polygon>;
    using lines = bg::model::multi_linestring<bg::model::linestring<point>>;

    auto const input = bg::from_wkt<lines>("MULTILINESTRING((0 0,8 0),(2 -4,2 4))");
    auto const side = bg::strategy::buffer::side_straight{};
    auto const join = bg::strategy::buffer::join_miter{};
    auto const end = bg::strategy::buffer::end_flat{};
    auto const circle = bg::strategy::buffer::point_circle{36};
    multi_polygon result;
    bg::buffer(input, result, bg::strategy::buffer::distance_symmetric<double>{1},
               side, join, end, circle, umbrella);
    BOOST_CHECK(bg::is_valid(result));
    BOOST_CHECK_EQUAL(result.size(), 1u);
    BOOST_CHECK_EQUAL(bg::area(result), 28.0);

    auto const square = bg::from_wkt<polygon>("POLYGON((0 0,8 0,8 8,0 8,0 0))");
    bg::buffer(square, result, bg::strategy::buffer::distance_symmetric<double>{-1},
               side, join, end, circle, umbrella);
    BOOST_CHECK(bg::is_valid(result));
    BOOST_CHECK_EQUAL(result.size(), 1u);
    BOOST_CHECK_EQUAL(bg::area(result), 36.0);
}

void test_winding_scan_axis()
{
    using point = bg::model::point<double, 2, bg::cs::cartesian>;
    using ring = bg::model::ring<point>;
    using simple = bg::strategies::relate::simple_cartesian;
    using distance = bg::strategy::buffer::distance_symmetric<double>;
    using collection = bg::detail::buffer::buffered_piece_collection<ring, simple, distance>;

    // Use enough vertices to activate pruning. An x-section test drops both
    // vertical sides for an interior query even though the winding scans y.
    ring rectangle;
    constexpr int size = 32;
    for (int i = 0; i < size; ++i) rectangle.emplace_back(0, i);
    for (int i = 0; i < size; ++i) rectangle.emplace_back(i, size);
    for (int i = size; i > 0; --i) rectangle.emplace_back(size, i);
    for (int i = size; i > 0; --i) rectangle.emplace_back(i, 0);
    rectangle.push_back(rectangle.front());

    auto const generation = simple::buffer_generation_strategy();
    collection::original_ring original(rectangle, false, false, generation);
    BOOST_REQUIRE(!original.m_sections.empty());
    BOOST_REQUIRE_GE(original.m_ring.size() - original.m_sections.size(), 16u);

    auto const check = [&](auto const& winding)
    {
        using bg::detail::buffer::point_in_original;
        BOOST_CHECK_EQUAL(point_in_original(point(16, 16), original, winding), 1);
        BOOST_CHECK_EQUAL(point_in_original(point(32, 16), original, winding), 0);
        BOOST_CHECK_EQUAL(point_in_original(point(33, 16), original, winding), -1);
    };
    check(simple::relate(point{}, rectangle));
    check(bg::strategies::detail::exact_cartesian::winding<double>{});

    bg::sectionalize<false, std::index_sequence<0>>(rectangle, original.m_sections, generation);
    check(bg::strategies::detail::simple_cartesian::winding<void, 0>{});
    check(bg::strategies::detail::exact_cartesian::winding<double, 0>{});

    // The out-and-back edges touch the query, but do not survive reduction.
    // The prepared scan belongs to the original boundary, not its residual.
    rectangle.emplace_back(-2, -2);
    rectangle.push_back(rectangle.front());
    namespace ec = bg::strategies::detail::exact_cartesian::ring_topology_detail;
    auto const boundary = ec::make_boundary<double>(rectangle);
    ec::predicates<double> const predicates;
    ec::winding_edge_scan<double> const scan;
    BOOST_CHECK_EQUAL(ec::algorithms::locate_raw(point(-1, -1), boundary, predicates, scan), 0);
    BOOST_CHECK_EQUAL(ec::algorithms::locate(point(-1, -1), boundary, predicates, scan), -1);
    BOOST_CHECK_EQUAL(ec::algorithms::locate(point(16, 0), boundary, predicates, scan), 0);
}

int test_main(int, char* [])
{
    BoostGeometryWriteTestConfiguration();

    test_all<true, bg::model::point<default_test_type, 2, bg::cs::cartesian> >();
    test_piece_strategy(bg::strategies::relate::simple_cartesian{});
    test_piece_strategy(bg::strategies::relate::cartesian_exact
        <double, bg::strategies::relate::bounded_ulp_output<4>>{});
    test_winding_scan_axis();

    return 0;
}
