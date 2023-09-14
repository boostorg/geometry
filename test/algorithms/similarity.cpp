// Boost.Geometry
// (Unit) Test

// Copyright (c) 2023 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/geometry/algorithms/similarity.hpp>

#include <boost/geometry/io/wkt/read.hpp>
#include <boost/geometry/algorithms/simplify.hpp>

#include <geometry_test_common.hpp>

#if defined(TEST_WITH_GEOJSON)
#include <boost/geometry/io/geojson/geojson_writer.hpp>
#include <fstream>
#endif

namespace
{

// Two lines consistently 0.5 apart
std::string const simplex1 = "LINESTRING(1.0 1.0, 2.0 1.0)";
std::string const simplex2 = "LINESTRING(1.0 1.1, 2.0 1.1)";

// With extra point in between
std::string const simplex3 = "LINESTRING(1.0 1.1, 1.5 1.1,2.0 1.1)";

// With a "spike"
std::string const simplex4 = "LINESTRING(1.0 1.1, 1.49 1.1, 1.5 2.5, 1.51 1.1, 2.0 1.1)";

// Realistic cases
std::string const west1 = "LINESTRING(5.134673416614532 52.27074444293976,5.134786069393158 52.27136671543121,5.134778022766113 52.27139353752136,5.134756565093994 52.27141231298447,5.134694874286652 52.27144449949265,5.134069919586182 52.27149546146393,5.133965313434601 52.27146327495575,5.133906304836273 52.27141231298447,5.133863389492035 52.27112531661987,5.133847296237946 52.27108776569366,5.133809745311737 52.27106094360352,5.133764147758484 52.27100998163223,5.133707821369171 52.27053791284561,5.133721232414246 52.27049767971039,5.133863389492035 52.27035015821457)";
std::string const west2 = "LINESTRING(5.134670734405518 52.27073907852173,5.134778022766113 52.27136135101318,5.134778022766113 52.27139353752136,5.134756565093994 52.27140426635742,5.134692192077637 52.2714364528656,5.134069919586182 52.2714900970459,5.134016275405884 52.27147936820984,5.133962631225586 52.27145791053772,5.133898258209229 52.27140426635742,5.133844614028931 52.27108240127563,5.133801698684692 52.27106094360352,5.133758783340454 52.27100729942322,5.133705139160156 52.2705352306366,5.133715867996216 52.27049231529236,5.13385534286499 52.27034211158752)";
std::string const east1 = "LINESTRING(5.141247510910034 52.27364659309387,5.140657424926758 52.27396845817566,5.14061450958252 52.2740113735199,5.140560865402222 52.27447271347046,5.140872001647949 52.27483749389648,5.140936374664307 52.27486968040466,5.141386985778809 52.27483749389648)";
std::string const east2 = "LINESTRING(5.141247510910034 52.27364659309387,5.140665471553802 52.27397114038467,5.14061450958252 52.27401673793793,5.1405930519104 52.27407306432724,5.140560865402222 52.2744807600975,5.140606462955475 52.27453976869583,5.140874683856964 52.27483749389648,5.140941739082336 52.27486968040466,5.141386985778809 52.27483749389648)";
std::string const complete = "LINESTRING(5.135158896446228 52.26660043001175,5.134890675544739 52.2666272521019,5.134839713573456 52.26665943861008,5.13435423374176 52.26669162511826,5.134295225143433 52.26670503616333,5.13424426317215 52.26672381162643,5.134182572364807 52.26675063371658,5.134131610393524 52.26690083742142,5.134077966213226 52.26698398590088,5.134086012840271 52.26706176996231,5.134115517139435 52.26712614297867,5.134161114692688 52.26717710494995,5.134324729442596 52.26729512214661,5.134397149085999 52.26735413074493,5.134442746639252 52.26739972829819,5.134464204311371 52.26744264364243,5.134584903717041 52.2677618265152)";
std::string const part1 = "LINESTRING(5.134332776069641 52.26669698953629,5.134295225143433 52.26670503616333,5.13424426317215 52.26672381162643,5.134182572364807 52.26675063371658,5.134131610393524 52.26690083742142,5.134077966213226 52.26698398590088,5.134086012840271 52.26706176996231,5.134115517139435 52.26712614297867,5.134161114692688 52.26717710494995,5.134324729442596 52.26729512214661,5.134397149085999 52.26735413074493,5.134442746639252 52.26739972829819,5.134464204311371 52.26744264364243,5.134584903717041 52.2677618265152)";
std::string const part2 = "LINESTRING(5.135158896446228 52.26660043001175,5.134890675544739 52.2666272521019,5.134839713573456 52.26665943861008,5.13435423374176 52.26669162511826,5.134332776069641 52.26669698953629)";

auto reverse = [](auto const& g)
{
    auto result = g;
    std::reverse(result.begin(), result.end());
    return result;
};

auto simplify = [](auto const& g, auto const simplify_distance)
{
    std::decay_t<decltype(g)> result;
    boost::geometry::simplify(g, result, simplify_distance);
    return result;
};

#if defined(TEST_WITH_GEOJSON)
struct geojson_visitor
{
    geojson_visitor()
    {
        m_stream_coll << std::setprecision(20);
        m_stream_quad << std::setprecision(20);
    }
    template <typename T, typename I> void visit_quadrilateral(T const& ring,
        I const& a, I const& b, I const& c, I const& d) 
    {
        m_quad.feature(ring);
        m_quad.add_property("role", 7);
        m_quad.add_property("area", std::round(boost::geometry::area(ring)));
    }
    template <typename C> void visit_projections(int source, C const& collection) 
    {
        for (const auto& info : collection)
        {
            m_collection.feature(info.point);
            m_collection.add_property("source", source);
            m_collection.add_property("is_projection", info.is_projection ? 1 : 0);
            m_collection.add_property("distance", info.distance);
            m_collection.add_property("source_index", info.source_index);
            m_collection.add_property("segment_index", info.segment_index);
            m_collection.add_property("offset", info.offset);
        }
    }

    std::ofstream m_stream_coll{"/tmp/collection.geojson"};
    std::ofstream m_stream_quad{"/tmp/quad.geojson"};
    boost::geometry::geojson_writer m_collection{m_stream_coll};
    boost::geometry::geojson_writer m_quad{m_stream_quad};
};
#endif


} // anonymous namespace

template <typename Geometry, typename T>
void test_ad(std::string const& case_id, Geometry const& p, Geometry const& q, T const expected)
{
    constexpr T test_epsilon = 0.0001;
    auto const detected = boost::geometry::average_distance(p, q);
    BOOST_CHECK_MESSAGE(std::abs(detected - expected) < test_epsilon,
        "Algorithm: average_distance"
        << " Case: " << case_id
        << " Detected: " << detected
        << " Expected: " << expected);
}

template <typename Geometry, typename Visitor, typename Result>
void test_sim(std::string const& case_id, Geometry const& p, Geometry const& q, Result const expected, Visitor& visitor)
{
    auto const detected = boost::geometry::similarity(p, q, visitor);
    BOOST_CHECK_MESSAGE(detected.is_reversed == expected.is_reversed,
        "Algorithm: similarity"
        << " Case: " << case_id
        << " Detected: " << detected.is_reversed
        << " Expected: " << expected.is_reversed);
}

template <typename T>
void test_average_distance_geographic()
{
    using point = bg::model::point<T, 2, boost::geometry::cs::geographic<boost::geometry::degree>>;
    using linestring = boost::geometry::model::linestring<point>;

    // Abbreviation, for readability
    auto make_ls = [](std::string const& wkt)
    {
        return boost::geometry::from_wkt<linestring>(wkt);
    };

    // Should be close
    test_ad("geo_west", make_ls(west1), make_ls(west2), 0.44457);
    test_ad("geo_east", make_ls(east1), make_ls(east2), 0.343435);

    // Should be far apart
    test_ad("geo_west_east1", make_ls(west1), make_ls(east1), 423.5956);
    test_ad("geo_west_east2", make_ls(west2), make_ls(east2), 419.2242);
}

template <typename T>
void test_average_distance_cartesian()
{
    using point = boost::geometry::model::d2::point_xy<T>;
    using linestring = boost::geometry::model::linestring<point>;

    auto make_ls = [](std::string const& wkt)
    {
        return boost::geometry::from_wkt<linestring>(wkt);
    };

    test_ad("simplex1", make_ls(simplex1), make_ls(simplex2), 0.1);
    test_ad("simplex2", make_ls(simplex1), make_ls(simplex3), 0.1);
    test_ad("simplex3", make_ls(simplex1), make_ls(simplex4), 0.114);

    // One meter is roughly 1.0e-5 degree. For readability and convenient omparison with the
    // cases in test_average_distance_geographic, this scale is used for all test cases using lat-lon coordinates.
    constexpr T scale = 1.0e-5;

    // Should be close
    test_ad("west", make_ls(west1), make_ls(west2), 0.561308 * scale);
    test_ad("east", make_ls(east1), make_ls(east2), 0.398684 * scale);

    // Should be far apart
    test_ad("west_east1", make_ls(west1), make_ls(east1), 492.539 * scale);
    test_ad("west_east2", make_ls(west2), make_ls(east2), 484.801 * scale);

    // Should be identical to close
    test_ad("west_rev", make_ls(west1), reverse(make_ls(west2)), 0.561308 * scale);
    test_ad("east_rev", make_ls(east1), reverse(make_ls(east2)), 0.398684 * scale);

    // Should become closer and closer
    test_ad("west_simp1", make_ls(west2), simplify(make_ls(west2), 50.0 * scale), 6.14674 * scale);
    test_ad("west_simp2", make_ls(west2), simplify(make_ls(west2), 10.0 * scale), 2.45124 * scale);
    test_ad("west_simp3", make_ls(west2), simplify(make_ls(west2), 1.0 * scale), 0.0733346 * scale);
    test_ad("west_simp4", make_ls(west2), simplify(make_ls(west2), 0.1 * scale), 0.0 * scale);

    test_ad("east_simp1", make_ls(east2), simplify(make_ls(east2), 50.0 * scale), 12.4301 * scale);
    test_ad("east_simp2", make_ls(east2), simplify(make_ls(east2), 10.0 * scale), 0.753191 * scale);
    test_ad("east_simp3", make_ls(east2), simplify(make_ls(east2), 1.0 * scale), 0.0511786 * scale);
    test_ad("east_simp4", make_ls(east2), simplify(make_ls(east2), 0.1 * scale), 0.0 * scale);

    // Parts are not yet finished
    test_ad("part1", make_ls(complete), make_ls(part1), 0.725765 * scale);
    test_ad("part2", make_ls(complete), make_ls(part2), 19.9363 * scale);

    // Identical cases return 0.0
    test_ad("identical_west", make_ls(west1), make_ls(west1), 0.0);
    test_ad("identical_east", make_ls(east1), make_ls(east1), 0.0);
    test_ad("identical_part1", make_ls(part1), make_ls(part1), 0.0);
    test_ad("identical_part2", make_ls(part2), make_ls(part2), 0.0);
}

template <typename T>
void test_similarity()
{
    using point = boost::geometry::model::d2::point_xy<T>;
    using linestring = boost::geometry::model::linestring<point>;

    auto make_expectation = [](bool is_reversed)
    {
        boost::geometry::similarity_info<T> result;
        result.is_reversed = is_reversed;
        return result;
    };

    auto const p = boost::geometry::from_wkt<linestring>(west1);
    auto const q = boost::geometry::from_wkt<linestring>(west2);

    boost::geometry::similarity_default_visitor default_visitor;
#if defined(TEST_WITH_GEOJSON)

    // Write the features to a geojson file, as linestring and as point,
    // and then, at visiting the algorithm, write debug-information.
    geojson_visitor visitor;

    {
        std::ofstream fout{"/tmp/west.geojson"};
        fout << std::setprecision(20);
        
        boost::geometry::geojson_writer writer(fout);

        writer.feature(p);
        writer.add_property("name", "p");
        
        writer.feature(q);
        writer.add_property("name", "q");

        for (const auto& pnt : p)
        {
            writer.feature(pnt);
            writer.add_property("name", "p");
        }
        for (const auto& pnt : q)
        {
            writer.feature(pnt);
            writer.add_property("name", "q");
        }
    }

#else
    auto& visitor = default_visitor;
#endif

    test_sim("west", p, q, make_expectation(false), visitor);
    test_sim("west_rev1", p, reverse(q), make_expectation(true), default_visitor);
    test_sim("west_rev2", reverse(p), q, make_expectation(true), default_visitor);
    test_sim("west_both", reverse(p), reverse(q), make_expectation(false), default_visitor);
}

int test_main(int, char* [])
{
    test_average_distance_cartesian<double>();
    test_average_distance_geographic<double>();
    test_similarity<double>();

    return 0;
}
