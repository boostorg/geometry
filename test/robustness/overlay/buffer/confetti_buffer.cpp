// Boost.Geometry (aka GGL, Generic Geometry Library)
// Robustness Test - buffer of many "confetti" streamers

// Copyright (c) 2026 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Generates many random "confetti" streamers going from bottom to top, with
// varying length and extent, and buffers them all at once (as one
// multi_linestring). This stresses the buffer with many interacting pieces, and
// it produces a colorful SVG: each streamer's buffer is drawn as a translucent
// coloured ribbon, with the merged buffer outline on top.
//
// Made for the 15th anniversary of Boost.Geometry in Boost (July 11, 2011).

#define BOOST_GEOMETRY_NO_BOOST_TEST

#include <chrono>
#include <cmath>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/variant2/variant.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/adapted/boost_variant2.hpp>
#include <boost/geometry/strategies/buffer.hpp>
#include <boost/geometry/algorithms/merge_elements.hpp>

#include <robustness/common/make_random_generator.hpp>

#include "confetti_buffer_svg.hpp"

namespace bg = boost::geometry;

struct confetti_settings
{
    int confetti_count{100}; // number of streamers per test
    double distance{2.0};    // buffer distance
    int field_size{100};     // size of the field
    int points_per_circle{36};
    bool dark{true};         // dark background (colours pop)
    bool svg{true};
    bool verbose{false};
};

// Generate one confetti streamer going generally upwards (bottom to top), like a
// candle. It advances by small steps while smoothly wandering its heading (kept
// within +/- ~70 degrees of north, so it always progresses upward). Length and
// step vary per streamer.
template <typename Linestring, typename Generator>
Linestring make_confetti_line(Generator& unit, double field)
{
    using point_type = typename bg::point_type<Linestring>::type;
    double const half_pi = 2.0 * std::atan(1.0); // north (straight up)

    Linestring line;
    double x = field * unit();                  // start anywhere across the base
    double y = field * (0.30 * unit());         // start in the bottom third of the field
    int const segments = 10 + static_cast<int>(20 * unit()); // 10..30 vertices (varying length)
    double const step = field * (0.020 + 0.035 * unit());    // varying extent
    double heading = half_pi + (unit() - 0.5) * 0.8; // start roughly northward

    for (int i = 0; i < segments; i++)
    {
        point_type p;
        bg::set<0>(p, x);
        bg::set<1>(p, y);
        line.push_back(p);

        heading += (unit() - 0.5) * 0.9;                     // smooth wander
        if (heading > half_pi + 1.2) heading = half_pi + 1.2; // keep progressing upward
        if (heading < half_pi - 1.2) heading = half_pi - 1.2;
        x += step * std::cos(heading);
        y += step * std::sin(heading);
    }
    return line;
}

int test_confetti(confetti_settings const& settings, int seed, bool make_svg)
{
    using point_type = bg::model::d2::point_xy<double>;
    using linestring = bg::model::linestring<point_type>;
    using multi_linestring = bg::model::multi_linestring<linestring>;
    using polygon = bg::model::polygon<point_type>;
    using multi_polygon = bg::model::multi_polygon<polygon>;

    // A generator returning a real number in [0, 1).
    auto unit = make_real_generator(seed, 0.0, 1.0);

    // Generate the confetti streamers.
    multi_linestring lines;
    for (int i = 0; i < settings.confetti_count; i++)
    {
        lines.push_back(make_confetti_line<linestring>(unit, settings.field_size));
    }

    // The buffer strategies, reused for the merged buffer and the per-streamer ones.
    bg::strategy::buffer::distance_symmetric<double> const distance_strategy(settings.distance);
    bg::strategy::buffer::side_straight const side_strategy;
    bg::strategy::buffer::join_round const join_strategy(settings.points_per_circle);
    bg::strategy::buffer::end_round const end_strategy(settings.points_per_circle);
    bg::strategy::buffer::point_circle const circle_strategy(settings.points_per_circle);

    // Buffer all streamers at once (the operation under test).
    multi_polygon merged;
    int problems = 0;
    try
    {
        bg::buffer(lines, merged, distance_strategy, side_strategy,
            join_strategy, end_strategy, circle_strategy);
    }
    catch (std::exception const& e)
    {
        std::cout << "Exception: " << e.what() << std::endl;
        problems++;
    }

    // Buffer each streamer individually (reusing the same strategy), collected as
    // a vector of multi_polygons - used for the coloured ribbons and for reuse.
    std::vector<multi_polygon> ribbons;
    ribbons.reserve(lines.size());
    for (auto const& line : lines)
    {
        multi_polygon ribbon;
        bg::buffer(line, ribbon, distance_strategy, side_strategy,
            join_strategy, end_strategy, circle_strategy);
        ribbons.push_back(std::move(ribbon));
    }

    std::string message;
    bool const valid = bg::is_valid(merged, message);
    if (! valid)
    {
        std::cout << "INVALID buffer: " << message << std::endl;
        problems++;
    }

    // Cross-check: merging the individual ribbons (merge_elements, divide and
    // conquer) should give the same area as buffering all streamers at once.
    // merge_elements works on a geometry_collection whose element type spans the
    // whole geometry family (it forms multi_point / multi_linestring result types
    // from the alternatives). We only ever store the (areal) ribbons in it.
    using multi_point = bg::model::multi_point<point_type>;
    using ring = bg::model::ring<point_type>;
    using element_t = boost::variant2::variant
        <
            point_type, multi_point, linestring, multi_linestring, ring, polygon, multi_polygon
        >;
    using collection_t = bg::model::geometry_collection<element_t>;
    collection_t gc;
    for (auto const& ribbon : ribbons)
    {
        gc.push_back(ribbon);
    }
    collection_t merged_elements;
    bg::merge_elements(gc, merged_elements);
    double const area_merged = bg::area(merged);
    double const area_combined = bg::area(merged_elements);
    double const area_diff = std::abs(area_merged - area_combined);
    if (area_diff > 1.0e-6 * std::abs(area_merged) + 1.0e-9)
    {
        std::cout << "AREA MISMATCH: buffer-at-once " << area_merged
            << " vs merge_elements " << area_combined
            << " (diff " << area_diff << ")" << std::endl;
        problems++;
    }

    if (settings.verbose || problems > 0)
    {
        std::cout << "seed: " << seed
            << " streamers: " << settings.confetti_count
            << " distance: " << settings.distance
            << " field: " << settings.field_size
            << " buffer polygons: " << merged.size()
            << " buffer area: " << area_merged
            << " merge_elements area: " << area_combined
            << (valid ? " (valid)" : " (INVALID)")
            << std::endl;
    }

    if (make_svg || problems > 0)
    {
        std::ostringstream filename;
        filename << "confetti_buffer_" << seed << ".svg";
        create_svg(filename.str(), ribbons, merged, settings.dark, settings.distance);
    }

    return problems;
}

int main(int argc, char** argv)
{
    try
    {
        namespace po = boost::program_options;
        po::options_description description("=== confetti_buffer ===\nAllowed options");

        int seed = -1;
        int count = 1;
        confetti_settings settings;

        description.add_options()
            ("help", "Help message")
            ("seed", po::value<int>(&seed), "Initialization seed for random generator")
            ("count", po::value<int>(&count)->default_value(1), "Number of tests (each a different seed)")
            ("confetti_count", po::value<int>(&settings.confetti_count)->default_value(100), "Number of confetti streamers per test")
            ("distance", po::value<double>(&settings.distance)->default_value(2.0), "Buffer distance")
            ("size", po::value<int>(&settings.field_size)->default_value(100), "Size of the field")
            ("ppc", po::value<int>(&settings.points_per_circle)->default_value(36), "Points per circle")
            ("dark", po::value<bool>(&settings.dark)->default_value(true), "Dark background")
            ("verbose", po::value<bool>(&settings.verbose), "Verbose")
            ("svg", po::value<bool>(&settings.svg)->default_value(true), "Create an SVG (single test, or on failure)")
        ;

        po::variables_map varmap;
        po::store(po::parse_command_line(argc, argv, description), varmap);
        po::notify(varmap);

        if (varmap.count("help"))
        {
            std::cout << description << std::endl;
            return 1;
        }

        // Resolve a reproducible base seed, so that any failure among the tests
        // can be reproduced (each test uses base_seed + its index).
        int base_seed = seed;
        if (base_seed == -1)
        {
            base_seed = static_cast<int>(std::random_device{}() % 1000000);
            std::cout << "Using random base seed " << base_seed << std::endl;
        }

        auto const t0 = std::chrono::high_resolution_clock::now();
        int problems = 0;
        for (int i = 0; i < count; i++)
        {
            problems += test_confetti(settings, base_seed + i, settings.svg);
        }
        auto const t = std::chrono::high_resolution_clock::now();
        auto const elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t - t0).count();
        std::cout << "tests: " << count
            << " problems: " << problems
            << " time: " << elapsed_ms / 1000.0 << std::endl;

        return problems == 0 ? 0 : 1;
    }
    catch (std::exception const& e)
    {
        std::cout << "Exception " << e.what() << std::endl;
    }
    catch (...)
    {
        std::cout << "Other exception" << std::endl;
    }
    return 1;
}
