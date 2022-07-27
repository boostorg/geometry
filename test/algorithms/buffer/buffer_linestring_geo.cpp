// Boost.Geometry
// Unit Test

// Copyright (c) 2018-2019 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "test_buffer_geo.hpp"
#include "aimes_cases.hpp"

static std::string const simplex = "LINESTRING(10.3965628 63.4276786,10.3953134 63.4299634)";
// Simple road from west to east and from south to north
static std::string const road = "LINESTRING(10.3966569 63.4276957,10.3998059 63.4279182,10.4003964 63.4288424)";

template <bool Clockwise, typename PointType>
void test_linestring()
{
    using linestring = bg::model::linestring<PointType>;
    using polygon = bg::model::polygon<PointType, Clockwise>;

    ut_settings settings(0.1, false, 360);

    test_one_geo<linestring, polygon>("simplex_5_8", simplex,  2622.1129, 5.0, settings);
    test_one_geo<linestring, polygon>("road_5_8", road, 1972.6796, 5.0, settings, 2.5);

    settings.test_area = false;
    auto const n = sizeof(testcases_aimes) / sizeof(testcases_aimes[0]);

    // Cases (ouf of 197) where the guessed area estimations are not met.
    // If this needs to be changed, be sure to
    // inspect the result visually on SVG or CSV (QGis).

    // Cases which are curved and have some artefacts (same as for cartesian)
    std::set<int> const cases_with_artefacts{119, 142};
    // Cases which are curved such that the min area is smaller than expected.
    std::set<int> const curved_cases_min_area{86, 181};
    // Cases which are curved such that the max area is larger than expected.
    std::set<int> const curved_cases_max_area{5, 95, 119, 142, 192};
    // Cases which are rounded such that it results in a large area
    std::set<int> const round_cases_max_area{196};

    for (auto i = 0; i < n; i++)
    {
        settings.multiplier_min_area
            = curved_cases_min_area.count(i) > 0 ? 0.85
            : ut_settings().multiplier_min_area;

        settings.multiplier_max_area
            = curved_cases_max_area.count(i) > 0 ? 1.2
            : round_cases_max_area.count(i) > 0 ? 1.5
            : ut_settings().multiplier_max_area;

        settings.fraction_buffered_points_too_close
            = cases_with_artefacts.count(i) > 0 ? 0.15
            : ut_settings().fraction_buffered_points_too_close;

        test_one_geo<linestring, polygon>("aimes_" + std::to_string(i), testcases_aimes[i], -1, 25.0, settings);
    }
}

int test_main(int, char* [])
{
    BoostGeometryWriteTestConfiguration();

    test_linestring<true, bg::model::point<default_test_type, 2, bg::cs::geographic<bg::degree> > >();

#if ! defined(BOOST_GEOMETRY_TEST_ONLY_ONE_TYPE)
    test_linestring<true, bg::model::point<long double, 2, bg::cs::geographic<bg::degree> > >();
#endif

    return 0;
}
