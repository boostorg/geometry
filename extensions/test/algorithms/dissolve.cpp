// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2010-2025 Barend Gehrels, Amsterdam, the Netherlands.

// This file was modified by Oracle on 2022.
// Modifications copyright (c) 2022, Oracle and/or its affiliates.
// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#if defined(TEST_WITH_GEOJSON)
#define BOOST_GEOMETRY_DEBUG_SEGMENT_IDENTIFIER
#define BOOST_GEOMETRY_DEBUG_IDENTIFIER
#endif

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

#include "dissolve_overlay_cases.hpp"

#if defined(TEST_WITH_GEOJSON)
#include <boost/geometry/extensions/gis/io/geojson/geojson_writer.hpp>
#include "dissolve_geojson_visitor.hpp"
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
        ut_settings const& settings)
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

    //std::cout << bg::area(geometry) << std::endl;

    using coordinate_type = typename bg::coordinate_type<Geometry>::type;
    using multi_polygon = bg::model::multi_polygon<GeometryOut>;
    multi_polygon dissolved1;

#if defined(TEST_WITH_GEOJSON)
        geojson_visitor visitor(writer);
#else
        bg::detail::overlay::overlay_null_visitor visitor;
#endif        

    // Check dispatch::dissolve
    {
        using strategy_type = typename bg::strategies::relate::services::default_strategy
            <
                Geometry, Geometry
            >::type;

        strategy_type strategy;


        bg::dispatch::dissolve
            <
                Geometry,
                GeometryOut,
                false
            >::apply(geometry, std::back_inserter(dissolved1),
                     strategy, visitor);
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

    for (GeometryOut& dissolved : dissolved1)
    {
        length_or_area += bg::area(dissolved);

        holes += bg::num_interior_rings(dissolved);
    }

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

#if defined(TEST_WITH_GEOJSON)
    writer.feature(geometry);
    writer.add_property("type", "input");

    for (const auto& polygon : dissolved1)
    {
        writer.feature(polygon);
        writer.add_property("type", "dissolved");
    }
#endif

}

template <typename Geometry, typename GeometryOut>
void test_one(std::string caseid, std::string const& wkt,
        double expected_area,
        std::size_t expected_clip_count,
        std::size_t expected_hole_count,
        ut_settings const& settings)
{
    Geometry geometry;
    bg::read_wkt(wkt, geometry);

    // If defined as closed, it should be closed. The algorithm itself
    // cannot close it without making a copy.
    bg::correct_closure(geometry);

    test_dissolve<GeometryOut>(caseid, geometry,
        expected_area,
        expected_clip_count, expected_hole_count,
        settings);

    // Verify if reversed version is indeed identical (it should, because each
    // ring is now corrected within dissolve itself
    bg::reverse(geometry);

    caseid += "_rev";
    test_dissolve<GeometryOut>(caseid, geometry,
        expected_area,
        expected_clip_count, expected_hole_count,
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
                expected_clip_count, expected_hole_count, settings);
    } while (std::next_permutation(indices.begin(), indices.end()));
#endif

}

#define TEST_DISSOLVE(caseid, area, clips, holes, points_ignored) { \
    ut_settings settings; \
    (test_one<polygon, polygon>) ( #caseid, caseid, area, clips, holes, settings); }

#define TEST_DISSOLVE_WITH(caseid, area, clips, holes, points_ignored, settings) { \
    (test_one<polygon, polygon>) ( #caseid, caseid, area, clips, holes, settings); }

#define TEST_DISSOLVE_IGNORE(caseid, area, clips, holes, points_ignored) { \
    ut_settings settings; settings.test_validity = false; \
    (test_one<polygon, polygon>) ( #caseid, caseid, area, clips, holes, settings); }

#define TEST_MULTI(caseid, area, clips, holes, points_ignored) { \
    ut_settings settings; \
    (test_one<multi_polygon, polygon>) ( #caseid, caseid, area, clips, holes, settings); }

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

#if defined(BOOST_GEOMETRY_TEST_FAILURES)
    TEST_DISSOLVE(gitter_2013_04_a, 3043.9181, 3, 0, 21);
#endif

    TEST_DISSOLVE(gitter_2013_04_b, 31210.429356259738, 1, 0, 11);

    TEST_DISSOLVE(ggl_list_denis, 21123.3281, 2, 0, 22);

    TEST_DISSOLVE(dissolve_mail_2018_08_19, 26.211, 2, 1, 15);
}


int test_main(int, char* [])
{
    test_all<bg::model::d2::point_xy<double>, true >(ut_settings());
    // Counter clockwise input does not work correctly in all cases, it is
    // partly a problem of the test itself
    return 0;
}
