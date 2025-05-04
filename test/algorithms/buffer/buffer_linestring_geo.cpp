// Boost.Geometry
// Unit Test

// Copyright (c) 2018-2019 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "test_buffer_geo.hpp"
#include "aimes_cases.hpp"

// Road with one segment from west to east
static std::string const simplex = "LINESTRING(10.3965628 63.4276786,10.3953134 63.4299634)";

// Road from west to east and from south to north
static std::string const road = "LINESTRING(10.3966569 63.4276957,10.3998059 63.4279182,10.4003964 63.4288424)";

// Road with first a sharp angle to the right, then a rectangular angle to the left, then two obtuse angles
static std::string const sharp = "LINESTRING(10.3939684 63.4255808, 10.3948159 63.4263661, 10.3947354 63.4255918, 10.3950209 63.4255588, 10.3960570 63.4255423, 10.3973017 63.4255570)";

// Linestring forwards and backwards (close to 180 corner)
static std::string const opposite = "LINESTRING(10.4000988 63.4283885,10.4029694 63.4280726,10.4006988 63.4283397)";

namespace
{
    constexpr int points_per_circle = 360;
}

template <typename Formula, bool Clockwise, typename PointType>
void test_linestring()
{
    using linestring = bg::model::linestring<PointType>;
    using polygon = bg::model::polygon<PointType, Clockwise>;

    // Because areas can change significantly when another formula is used,
    // use a high tolerance.
    ut_settings settings(0.5);

#if defined(BOOST_GEOMETRY_TEST_FAILURES)
    bool const thomas_skip = false;
#else
    // TODO: some cases are missing one or more turns
    bool const thomas_skip = std::is_same<Formula, bg::strategy::thomas>::value;
#endif

    bg::strategies::buffer::geographic<Formula> strategy;
    bg::strategy::buffer::geographic_side_straight<Formula> side;
    bg::strategy::buffer::geographic_join_miter<Formula> join_miter;
    bg::strategy::buffer::geographic_join_miter<Formula> join_miter25(2.5);
    bg::strategy::buffer::geographic_join_round<Formula> join_round(points_per_circle);
    bg::strategy::buffer::geographic_end_round<Formula> end_round(points_per_circle);
    bg::strategy::buffer::geographic_point_circle<Formula> circle(points_per_circle);
    bg::strategy::buffer::end_flat end_flat;

    test_one_geo<linestring, polygon>("simplex_5_8", simplex, strategy, side, circle, join_round, end_flat, 2622.0, 5.0, settings);
    test_one_geo<linestring, polygon>("road_5_flat", road, strategy, side, circle, join_round, end_flat, 2644.0, 5.0, settings);
    test_one_geo<linestring, polygon>("road_5_25_round", road, strategy, side, circle, join_round, end_round, 2016.0, 5.0, settings, 2.5);
    test_one_geo<linestring, polygon>("sharp_5_round", sharp, strategy, side, circle, join_round, end_round, 3090.0, 5.0, settings);
    test_one_geo<linestring, polygon>("sharp_5_miter", sharp, strategy, side, circle, join_miter, end_round, 3181.0, 5.0, settings);
    test_one_geo<linestring, polygon>("sharp_5_miter25", sharp, strategy, side, circle, join_miter25, end_round, 3121.0, 5.0, settings);

#if defined(BOOST_GEOMETRY_TEST_FAILURES)
    if (! BOOST_GEOMETRY_CONDITION(thomas_skip))
    {
        // Misses an intersection point when using thomas
        test_one_geo<linestring, polygon>("opposite", opposite, strategy, side, circle, join_round, end_round, 1658.0, 5.0, settings);
    }
#endif

    {
        auto specific = settings;
        specific.fraction_buffered_points_too_close = 0.3;
        test_one_geo<linestring, polygon>("opposite_miter", opposite, strategy, side, circle, join_miter, end_flat, 1705.0, 5.0, specific);
        test_one_geo<linestring, polygon>("opposite_miter25", opposite, strategy, side, circle, join_miter25, end_flat, 1642.0, 5.0, specific);
    }
}

template <typename Formula, bool Clockwise, typename PointType>
void test_linestring_aimes()
{
    using coordinate_type = bg::coordinate_type_t<PointType>;
    using linestring = bg::model::linestring<PointType>;
    using polygon = bg::model::polygon<PointType, Clockwise>;

    constexpr coordinate_type distance_meters = 25.0;

    // Because areas can change significantly when another formula is used,
    // use a high tolerance.
    ut_settings settings(0.5);
    settings.test_area = false;

    bg::strategies::buffer::geographic<Formula> strategy;
    bg::strategy::buffer::geographic_side_straight<Formula> side;
    bg::strategy::buffer::geographic_join_miter<Formula> join_miter;
    bg::strategy::buffer::geographic_join_miter<Formula> join_miter25(2.5);
    bg::strategy::buffer::geographic_join_round<Formula> join_round(points_per_circle);
    bg::strategy::buffer::geographic_end_round<Formula> end_round(points_per_circle);
    bg::strategy::buffer::geographic_point_circle<Formula> circle(points_per_circle);
    bg::strategy::buffer::end_flat end_flat;

    // There are cases where some estimations are not met.
    // If this needs to be changed, be sure to
    // inspect the result visually on SVG or CSV (QGis).

    // Cases which are curved and have some artefacts (same as for cartesian)
    std::set<int> const cases_with_artefacts{119, 142};
    // Cases which are curved such that the min area is smaller than expected.
    std::set<int> const curved_cases_min_area{86, 181};
    // Cases which are curved such that the max area is larger than expected.
    std::set<int> const curved_cases_max_area{5, 95, 119, 142};

#if defined(BOOST_GEOMETRY_TEST_FAILURES)
    std::set<int> const skip_cases_round_round{};
    std::set<int> const skip_cases_round_flat{};
    std::set<int> const skip_cases_miter_flat{};
#else
    // Cases where the algorithm is still failing.
    // It might differ in debug/release mode.
    // It might also depend on the compiler and on the operating system.
    std::set<int> const skip_cases_round_round{17, 22, 38, 67, 75, 109, 163, 181, 143, 196};
    std::set<int> const skip_cases_round_flat{17, 22, 38, 67, 75, 103, 109, 196};
    std::set<int> const skip_cases_miter_flat{17, 18, 22, 38, 67, 75, 103, 109, 196};
#endif

    for (auto const& enumerated : bg::util::enumerate(testcases_aimes))
    {
        auto const index = enumerated.index;
        auto const& test_case = enumerated.value;

        settings.multiplier_min_area
            = curved_cases_min_area.count(index) > 0 ? 0.85
            : ut_settings().multiplier_min_area;

        settings.multiplier_max_area
            = curved_cases_max_area.count(index) > 0 ? 1.2
            : ut_settings().multiplier_max_area;

        settings.fraction_buffered_points_too_close
            = cases_with_artefacts.count(index) > 0 ? 0.20
            : ut_settings().fraction_buffered_points_too_close;

        // With miter/flat, the artefacts are more pronounced and there can
        // be more points close than expected.
        auto settings_miter_flat = settings;
        settings_miter_flat.fraction_buffered_points_too_close *= 4.0;

        // With rounded cases, both rounded ends overap, adapt the expected area
        auto settings_round_round = settings;
        settings_round_round.multiplier_min_area = settings.multiplier_min_area;

        std::string const caseid = "aimes_" + std::to_string(index);
        if (skip_cases_round_round.count(index) == 0)
        {
            test_one_geo<linestring, polygon>(caseid + "_rr", test_case,
                strategy, side, circle, join_round, end_round, -1, distance_meters, settings_round_round);
        }
        if (skip_cases_round_flat.count(index) == 0)
        {
            test_one_geo<linestring, polygon>(caseid + "_rf", test_case,
                strategy, side, circle, join_round, end_flat, -1, distance_meters, settings);
        }
        if (skip_cases_miter_flat.count(index) == 0)
        {
            test_one_geo<linestring, polygon>(caseid + "_mf", test_case,
                strategy, side, circle, join_miter, end_flat, -1, distance_meters, settings_miter_flat);
        }
    }
}

int test_main(int, char* [])
{
    BoostGeometryWriteTestConfiguration();

    test_linestring<bg::strategy::andoyer, true, bg::model::point<default_test_type, 2, bg::cs::geographic<bg::degree> > >();
    test_linestring<bg::strategy::thomas, true, bg::model::point<default_test_type, 2, bg::cs::geographic<bg::degree> > >();

    test_linestring_aimes<bg::strategy::andoyer, true, bg::model::point<default_test_type, 2, bg::cs::geographic<bg::degree> > >();

#if ! defined(BOOST_GEOMETRY_TEST_ONLY_ONE_TYPE)
    test_linestring<bg::strategy::andoyer, true, bg::model::point<long double, 2, bg::cs::geographic<bg::degree> > >();
#endif

    return 0;
}
