// Boost.Geometry (aka GGL, Generic Geometry Library)
// Robustness Test

// Copyright (c) 2026 Tinko Bartels, Shenzhen, China.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_GEOMETRY_NO_BOOST_TEST

#include "random_grid.hpp"
#include "random_grid_svg.hpp"
#include "buffer_cell_helpers.hpp"

#include <boost/geometry/algorithms/buffer.hpp>
#include <boost/geometry/algorithms/convert.hpp>

#include <boost/geometry/strategies/agnostic/buffer_distance_symmetric.hpp>
#include <boost/geometry/strategies/cartesian/buffer_point_square.hpp>
#include <boost/geometry/strategies/cartesian/buffer_side_straight.hpp>
#include <boost/geometry/strategies/cartesian/buffer_join_miter.hpp>
#include <boost/geometry/strategies/cartesian/buffer_end_flat.hpp>

using namespace random_grid;

using buffer_point = bg::model::d2::point_xy<double>;
using buffer_linestring = bg::model::linestring<buffer_point>;
using buffer_polygon = bg::model::polygon<buffer_point, false>;
using buffer_multi_point = bg::model::multi_point<buffer_point>;
using buffer_multi_linestring = bg::model::multi_linestring<buffer_linestring>;
using buffer_multi_polygon = bg::model::multi_polygon<buffer_polygon>;
using margin_masks = std::tuple<bits, bits, bits>;

margin_masks make_margin_masks(grid const& g, int grid_margin)
{
    margin_masks result{make_bits(g.vertex_count()),
                        make_bits(g.edge_count()),
                        make_bits(g.cell_count())};
    bits& vertices = std::get<0>(result);
    bits& edges = std::get<1>(result);
    bits& cells = std::get<2>(result);
    int const max_i = g.width - grid_margin;
    int const max_j = g.height - grid_margin;

    for (int j = grid_margin; j <= max_j; ++j)
        for (int i = grid_margin; i <= max_i; ++i)
            set(vertices, g.vertex_index(i, j));

    // Linear buffers use only axis-aligned edges; diagonal bits remain clear.
    for (int j = grid_margin; j <= max_j; ++j)
        for (int i = grid_margin; i < max_i; ++i)
            set(edges, g.horizontal_index(i, j));
    for (int j = grid_margin; j < max_j; ++j)
        for (int i = grid_margin; i <= max_i; ++i)
            set(edges, g.vertical_index(i, j));
    for (int j = grid_margin; j < max_j; ++j)
    {
        for (int i = grid_margin; i < max_i; ++i)
        {
            set(cells, g.cell_index(i, j, 0));
            set(cells, g.cell_index(i, j, 1));
        }
    }
    return result;
}

bits random_masked_bits(generator_type& generator, int size, bits const& mask)
{
    bits result = random_bits(generator, size);
    BOOST_GEOMETRY_ASSERT(result.size() == mask.size());
    for (std::size_t chunk = 0; chunk < result.size(); ++chunk)
        result[chunk] &= mask[chunk];
    return result;
}

template <typename Geometry>
buffer_multi_polygon apply_buffer(Geometry const& geometry)
{
    bg::strategy::buffer::distance_symmetric<double> distance(buffer_distance);
    bg::strategy::buffer::side_straight side;
    bg::strategy::buffer::join_miter join(5.0);
    bg::strategy::buffer::end_flat end;
    bg::strategy::buffer::point_square point_strategy;
    buffer_multi_polygon result;
    bg::buffer(geometry, result, distance, side, join, end, point_strategy);
    return result;
}

template <typename BufferGeometry, typename Geometry>
bool check_buffer(char const* label, Geometry const& input,
                  bits const& input_bits, vertex_paths const& paths,
                  std::vector<point> const& input_test_points,
                  std::vector<point> const& cell_test_points,
                  grid const& g, settings const& settings, int iteration,
                  std::map<std::string, int>& failures)
{
    std::string input_error;
    if (! validate_geometry(input_bits, input, input_test_points, input_error))
    {
        std::cout << label << " construction mismatch input=" << bit_string(input_bits)
                  << ": " << input_error << '\n';
        if (settings.verbose) std::cout << "  input geometry: " << bg::wkt(input) << '\n';
        if (svg_enabled(settings, failures))
            write_construction_svg(settings, iteration,
                std::string("buffer ") + label + " construction",
                g, input_bits, input, input_test_points, input_error);
        ++failures[std::string(label) + " construction"];
        return false;
    }

    BufferGeometry buffer_input;
    bg::convert(input, buffer_input);
    bits const expected = buffer_cells(g, input_bits, bg::tag_t<Geometry>{}, paths);
    buffer_multi_polygon const output = apply_buffer(buffer_input);
    bits const detected = geometry_to_bits(output, cell_test_points);
    if (detected == expected) return true;

    std::cout << label << " buffer mismatch distance=" << buffer_distance
              << " input=" << bit_string(input_bits) << '\n'
              << "  expected=" << bit_string(expected)
              << " detected=" << bit_string(detected) << '\n';
    if (settings.verbose)
    {
        std::cout << "  input geometry: " << bg::wkt(input) << '\n'
                  << "  output geometry: " << bg::wkt(output) << '\n';
    }
    if (svg_enabled(settings, failures))
    {
        write_failure_svg(settings, iteration,
            std::string("buffer ") + label + " distance " + std::to_string(buffer_distance),
            g, [&](failure_svg& svg)
        {
            svg.add(input);
            svg.add(output);
            svg.record("Input", input);
            svg.record("Actual buffer", output);
            svg.panel(0, "Input", [&] { svg.map(input); });
            svg.panel(1, "Expected cells", [&] { svg.cells(expected); });
            svg.panel(2, "Actual buffer", [&]
            {
                svg.map(output);
                svg.samples(expected, detected, cell_test_points);
            });
            svg.text(20, 585, "Miter buffer, flat ends, square points; distance = "
                + std::to_string(buffer_distance)
                + ". Comparison uses cell sample membership only.");
        });
    }
    ++failures[std::string(label) + " buffer"];
    return false;
}

bool test_multipoint(grid const& g,
                     bits const& margin_mask,
                     std::vector<point> const& primitives,
                     std::vector<point> const& test_points,
                     std::vector<point> const& cell_test_points,
                     settings const& settings, int iteration,
                     generator_type& generator,
                     std::map<std::string, int>& failures)
{
    bits const point_bits
        = random_masked_bits(generator, g.vertex_count(), margin_mask);
    multi_point const points
        = bits_to_geometry<multi_point>(point_bits, point_bits, g, primitives);
    return check_buffer<buffer_multi_point>("point", points, point_bits, {},
        test_points, cell_test_points, g, settings, iteration, failures);
}

bool test_multilinestring(grid const& g,
                          bits const& margin_mask,
                          std::vector<linestring> const& primitives,
                          std::vector<point> const& test_points,
                          std::vector<point> const& cell_test_points,
                          settings const& settings, int iteration,
                          generator_type& generator,
                          std::map<std::string, int>& failures)
{
    bits const line_bits
        = random_masked_bits(generator, g.edge_count(), margin_mask);
    bits const vertices = random_bits(generator, g.vertex_count());
    vertex_paths paths;
    multi_linestring const lines
        = bits_to_geometry<multi_linestring>(line_bits, vertices, g, primitives, &paths);
    return check_buffer<buffer_multi_linestring>("line", lines, line_bits, paths,
        test_points, cell_test_points, g, settings, iteration, failures);
}

bool test_multipolygon(grid const& g,
                       bits const& margin_mask,
                       std::vector<ring> const& primitives,
                       std::vector<point> const& cell_test_points,
                       settings const& settings, int iteration,
                       generator_type& generator,
                       std::map<std::string, int>& failures)
{
    bits const area_bits
        = random_masked_bits(generator, g.cell_count(), margin_mask);
    bits const vertices = random_bits(generator, g.vertex_count());
    vertex_paths paths;
    multi_polygon const areas
        = bits_to_geometry<multi_polygon>(area_bits, vertices, g, primitives, &paths);
    return check_buffer<buffer_multi_polygon>("area", areas, area_bits, paths,
        cell_test_points, cell_test_points, g, settings, iteration, failures);
}

int main(int argc, char** argv)
{
    BoostGeometryWriteTestConfiguration();
    try
    {
        settings settings;
        if (! parse_options(argc, argv, "=== random_grid_buffer ===", settings))
            return 0;
        // Miter coordinate displacements are at most 3d, or 24 tile rows.
        int const grid_margin = 24;
        grid const g(settings.width + 2 * grid_margin,
                     settings.height + 2 * grid_margin);
        primitive_vectors const primitives = make_primitives(g);
        margin_masks const masks = make_margin_masks(g, grid_margin);
        test_point_vectors const test_points = make_test_points(g, primitives);
        generator_type generator(settings.seed);
        std::map<std::string, int> failures;
        auto const start = std::chrono::high_resolution_clock::now();
        int iterations = 0;
        for (; iterations < settings.count || settings.count == -1; ++iterations)
        {
            bool ok = test_multipoint(
                g, std::get<0>(masks), std::get<0>(primitives),
                std::get<0>(test_points),
                std::get<2>(test_points), settings, iterations, generator, failures);
            if (ok || ! settings.stop_on_failure)
            {
                bool const linear_ok = test_multilinestring(
                    g, std::get<1>(masks), std::get<1>(primitives),
                    std::get<1>(test_points),
                    std::get<2>(test_points), settings, iterations, generator, failures);
                ok = linear_ok && ok;
            }
            if (ok || ! settings.stop_on_failure)
            {
                bool const areal_ok = test_multipolygon(
                    g, std::get<2>(masks), std::get<2>(primitives),
                    std::get<2>(test_points),
                    settings, iterations, generator, failures);
                ok = areal_ok && ok;
            }
            if (! ok)
                std::cout << "replay: --width " << settings.width << " --height " << settings.height
                          << " --seed " << settings.seed << " (iteration " << iterations << ")\n";
            if (! ok && settings.stop_on_failure) { ++iterations; break; }
        }
        std::cout << report(start, iterations, failures);
        return failure_count(failures) == 0 ? 0 : 1;
    }
    catch (std::exception const& error)
    {
        std::cout << "Exception: " << error.what() << '\n';
        return 1;
    }
}
