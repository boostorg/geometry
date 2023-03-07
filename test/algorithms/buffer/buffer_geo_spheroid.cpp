// Boost.Geometry
// Unit Test

// Copyright (c) 2022 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "test_buffer_geo.hpp"

namespace
{
    int const points_per_circle = 360;
    std::string const road = "LINESTRING(10.3966569 63.4276957,10.3998059 63.4279182,10.4000859 63.4283889,10.3982915 63.4284015,10.3980902 63.4288772,10.3987772 63.4288520)";
    std::string const torg = "POINT(10.3937759 63.4302323)";
}

template <typename Formula, bool Clockwise, typename Point, typename Spheroid>
void test_linestring(std::string const& label, Spheroid const& spheroid,
                     double expected_area_round, double expected_area_miter)
{
    using linestring = bg::model::linestring<Point>;
    using polygon = bg::model::polygon<Point, Clockwise>;

    ut_settings settings(0.1);

    bg::strategies::buffer::geographic<Formula, Spheroid> strategy(spheroid);
    bg::strategy::buffer::geographic_side_straight<Formula, Spheroid> side(spheroid);
    bg::strategy::buffer::geographic_join_miter<Formula, Spheroid> join_miter(spheroid);
    bg::strategy::buffer::geographic_join_round<Formula, Spheroid> join_round(spheroid, points_per_circle);
    bg::strategy::buffer::geographic_end_round<Formula, Spheroid> end_round(spheroid, points_per_circle);

    // Ignored for linear or areal features
    bg::strategy::buffer::geographic_point_circle<Formula, Spheroid> circle(spheroid, points_per_circle);

    test_one_geo<linestring, polygon>(label + "_round", road, strategy, side, circle, join_round, end_round, expected_area_round, 10.0, settings);
    test_one_geo<linestring, polygon>(label + "_miter", road, strategy, side, circle, join_miter, end_round, expected_area_miter, 10.0, settings);
}

template <typename Formula, bool Clockwise, typename Point, typename Spheroid>
void test_point(std::string const& label, Spheroid const& spheroid, double expected_area)
{
    using polygon = bg::model::polygon<Point, Clockwise>;

    ut_settings settings(0.01);

    bg::strategies::buffer::geographic<Formula, Spheroid> strategy(spheroid);
    bg::strategy::buffer::geographic_point_circle<Formula, Spheroid> circle(spheroid, points_per_circle);

    // All are ignored for points
    bg::strategy::buffer::geographic_side_straight<Formula, Spheroid> side(spheroid);
    bg::strategy::buffer::geographic_join_miter<Formula, Spheroid> join_miter(spheroid);
    bg::strategy::buffer::geographic_join_round<Formula, Spheroid> join_round(spheroid);
    bg::strategy::buffer::geographic_end_round<Formula, Spheroid> end_round(spheroid);

    test_one_geo<Point, polygon>(label, torg, strategy, side, circle, join_round, end_round, expected_area, 100.0, settings);
}

int test_main(int, char* [])
{
    BoostGeometryWriteTestConfiguration();

    using test_type = default_test_type;

    using point_t = bg::model::point<test_type, 2, bg::cs::geographic<bg::degree>>;

    // Use the default spheroid
    bg::srs::spheroid<test_type> def_spheroid;
    test_linestring<bg::strategy::andoyer, true, point_t>("line_def", def_spheroid, 7996.0, 8093.0);
    test_point<bg::strategy::andoyer, true, point_t>("point_def", def_spheroid, 31450.0);

    // Call it with a quite different spheroid (a near sphere), this changes internal geographic calculations
    // and should result in different areas. Using CSV creation, it's visible in QGis.
    bg::srs::spheroid<test_type> alt_spheroid(6378000.0, 6375000.0);
    test_linestring<bg::strategy::andoyer, true, point_t>("line_alt", alt_spheroid, 8097.0, 8115.3);
    test_point<bg::strategy::andoyer, true, point_t>("point_alt", alt_spheroid, 31409.0);

    return 0;
}
