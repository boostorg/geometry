// Boost.Geometry
//
// Copyright (c) 2025 Barend Gehrels, Amsterdam, the Netherlands.

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// Official repository: https://github.com/boostorg/geometry
// Documentation: http://www.boost.org/libs/geometry

#if defined(TEST_WITH_GEOJSON)
#define BOOST_GEOMETRY_DEBUG_SEGMENT_IDENTIFIER
#define BOOST_GEOMETRY_DEBUG_IDENTIFIER
#endif

#include <geometry_test_common.hpp>

#include <boost/geometry/extensions/algorithms/dissolve_using_correct.hpp>

// To check results
#include <boost/geometry/algorithms/correct_closure.hpp>
#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/is_valid.hpp>

#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/point_xy.hpp>

#include <boost/geometry/strategies/strategies.hpp>

#include <boost/geometry/io/wkt/wkt.hpp>

#include "dissolve_overlay_cases.hpp"


#if defined(TEST_WITH_GEOJSON)
#include <boost/geometry/extensions/gis/io/geojson/geojson_writer.hpp>
#include "dissolve_geojson_visitor.hpp"
#endif

// Equivalent with BOOST_CHECK_CLOSE
// See also "expectation_limits.hpp" in the test directory
template <typename Settings>
bool is_equal_within_tolerance(Settings const& settings, double const value, double const expected)
{
    double const fraction = settings.percentage / 100.0;
    double const lower_limit = expected * (1.0 - fraction);
    double const upper_limit = expected * (1.0 + fraction);
    return value >= lower_limit && value <= upper_limit;
}

//! Unittest settings
struct ut_settings
{
    double remove_spike_threshold{1.0e-12};
    double percentage{0.01};
    bool test_validity{true};
};

template <typename Geometry>
std::string as_wkt(Geometry const& geometry)
{
    std::ostringstream out;
    out << bg::wkt(geometry);
    return out.str();
}

template <typename GeometryOut, typename Geometry>
void dissolve_alternative(GeometryOut& geometry_out, Geometry const& geometry,
        double remove_spike_threshold)
{
    boost::geometry::correct_non_zero(geometry, geometry_out, remove_spike_threshold);
}

template <typename GeometryOut, typename Geometry>
void test_dissolve(std::string const& caseid, Geometry const& geometry,
        double expected_area, ut_settings const& settings)
{
#if defined(TEST_WITH_GEOJSON)
    std::ostringstream filename;
    // For QGis, it is usually convenient to always write to the same geojson file.
    filename << "/tmp/"
        // << caseid << "_"
        << "dissolve.geojson";
    std::ofstream geojson_file(filename.str().c_str());

    boost::geometry::geojson_writer writer(geojson_file);
#endif    

    using coordinate_type = typename bg::coordinate_type<Geometry>::type;
    using multi_polygon = bg::model::multi_polygon<GeometryOut>;
    multi_polygon dissolved;

    dissolve_alternative(dissolved, geometry, settings.remove_spike_threshold);

#if defined(TEST_WITH_GEOJSON)
        geojson_visitor visitor(writer);
#else
        bg::detail::overlay::overlay_null_visitor visitor;
#endif        

    if (settings.test_validity)
    {
        std::string message;
        bool const valid = bg::is_valid(dissolved, message);
        BOOST_CHECK_MESSAGE(valid,
                "dissolve: " << caseid
                << " geometry is not valid: " << message);
    }

    auto const detected_area = bg::area(dissolved);

    BOOST_CHECK_MESSAGE(is_equal_within_tolerance(settings, detected_area, expected_area),
            "dissolve: " << caseid
            << " #area expected: " << expected_area
            << " detected: " << detected_area
            );

#if defined(TEST_WITH_GEOJSON)
    writer.feature(geometry);
    writer.add_property("type", "input");

    for (const auto& polygon : dissolved)
    {
        writer.feature(polygon);
        writer.add_property("type", "dissolved");
    }
#endif

}

template <typename Geometry, typename GeometryOut>
void test_one(std::string caseid, std::string const& wkt,
        double expected_area, ut_settings const& settings)
{
    Geometry geometry;
    bg::read_wkt(wkt, geometry);

    // If defined as closed, it should be closed. The algorithm itself
    // cannot close it without making a copy.
    bg::correct_closure(geometry);

    test_dissolve<GeometryOut>(caseid, geometry,
        expected_area,
        settings);

    // Verify if reversed version is identical
    bg::reverse(geometry);

    caseid += "_rev";
    test_dissolve<GeometryOut>(caseid, geometry,
        expected_area,
        settings);
}

#define TEST_DISSOLVE(caseid, area, clips_ignored, holes_ignored, points_ignored) { \
    ut_settings settings; \
    (test_one<polygon, polygon>) ( #caseid, caseid, area, settings); }

#define TEST_DISSOLVE_WITH(caseid, area, clips_ignored, holes_ignored, points_ignored, settings) { \
    (test_one<polygon, polygon>) ( #caseid, caseid, area, settings); }

#define TEST_DISSOLVE_IGNORE(caseid, area, clips_ignored, holes_ignored, points_ignored) { \
    ut_settings settings; settings.test_validity = false; \
    (test_one<polygon, polygon>) ( #caseid, caseid, area, settings); }

#define TEST_MULTI(caseid, area, clips_ignored, holes_ignored, points_ignored) { \
    ut_settings settings; \
    (test_one<multi_polygon, polygon>) ( #caseid, caseid, area, settings); }

template <typename P, bool Clockwise>
void test_all()
{
    typedef bg::model::polygon<P, Clockwise> polygon;
    typedef bg::model::multi_polygon<polygon> multi_polygon;

    TEST_DISSOLVE(dissolve_1, 8.0, 1, 0, 4);
    TEST_DISSOLVE(dissolve_2, 7.9296875, 1, 1, 12);
    TEST_DISSOLVE(dissolve_3, 4.0, 2, 0, 8);
    TEST_DISSOLVE(dissolve_4, 8.0, 2, 0, 8);
    TEST_DISSOLVE_IGNORE(dissolve_5, 12.0, 2, 0, 8);
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
    TEST_DISSOLVE(dissolve_16, 8.1333, 8, 0, 38);

    TEST_DISSOLVE(dissolve_17, 14.5, 2, 0, 11);
    TEST_DISSOLVE(dissolve_18, 15.0, 3, 0, 15);

    TEST_DISSOLVE(dissolve_d1, 8.0, 1, 0, 4);
    TEST_DISSOLVE(dissolve_d2, 16.0, 1, 0, 5);

    TEST_DISSOLVE(dissolve_h1_a, 14.0, 1, 1, 9);
    TEST_DISSOLVE(dissolve_h1_b, 14.0, 1, 1, 9);
    TEST_DISSOLVE(dissolve_h2, 12.25, 2, 0, 13);
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
    TEST_DISSOLVE(dissolve_mail_2017_09_24_f, 0.000361308800, 5, 0, 69);
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
    TEST_DISSOLVE(dissolve_ticket17, 0.00920834633689, 1, 1, 228);
    TEST_DISSOLVE(dissolve_reallife, 91756.916526794434, 1, 0, 25);

    TEST_DISSOLVE(gitter_2013_04_a, 2829.7832, 3, 0, 21);
    TEST_DISSOLVE(gitter_2013_04_b, 31210.429356259738, 1, 0, 11);

    TEST_DISSOLVE(ggl_list_denis, 21123.3281, 2, 0, 22);

    TEST_DISSOLVE(dissolve_mail_2018_08_19, 30.711696, 2, 1, 15);
}


int test_main(int, char* [])
{
    test_all<bg::model::d2::point_xy<double>, true >();
    return 0;
}
