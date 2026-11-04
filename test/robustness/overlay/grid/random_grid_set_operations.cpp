// Boost.Geometry (aka GGL, Generic Geometry Library)
// Robustness Test

// Copyright (c) 2026 Tinko Bartels, Shenzhen, China.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_GEOMETRY_NO_BOOST_TEST

#include "random_grid.hpp"
#include "random_grid_svg.hpp"

#include <boost/geometry/algorithms/difference.hpp>
#include <boost/geometry/algorithms/intersection.hpp>
#include <boost/geometry/algorithms/length.hpp>
#include <boost/geometry/algorithms/sym_difference.hpp>
#include <boost/geometry/algorithms/union.hpp>

using namespace random_grid;

template <typename Tag1, typename Tag2,
          typename Geometry1, typename Geometry2, typename Operation>
bool check_operation(char const* label, boolean_operation operation,
                     Geometry1 const& geometry1, Geometry2 const& geometry2,
                     grid const& grid, primitive_vectors const& primitives,
                     test_point_vectors const& test_points,
                     settings const& settings, int iteration,
                     bits const& bits1, bits const& bits2,
                     Operation const& apply,
                     std::map<std::string, int>& failures)
{
    output_tuple output;
    apply(geometry1, geometry2, output);
    output_tuple const expected
        = expected_operation<Tag1, Tag2>(grid, bits1, bits2, primitives, operation);

    bits const expected_points
        = geometry_to_bits(std::get<0>(expected), std::get<0>(test_points));
    bits const expected_lines
        = geometry_to_bits(std::get<1>(expected), std::get<1>(test_points));
    bits const expected_areas
        = geometry_to_bits(std::get<2>(expected), std::get<2>(test_points));

    output_tuple const detected = canonical_output(grid, primitives, test_points, output);
    bits const detected_points
        = geometry_to_bits(std::get<0>(detected), std::get<0>(test_points));
    bits const detected_lines
        = geometry_to_bits(std::get<1>(detected), std::get<1>(test_points));
    bits const detected_areas
        = geometry_to_bits(std::get<2>(detected), std::get<2>(test_points));
    bool ok = detected_points == expected_points
           && detected_lines == expected_lines
           && detected_areas == expected_areas;

    std::string reason;
    bool const area_valid = bg::is_valid(std::get<2>(output), reason);
    coordinate_type const expected_area = cell_area * count(expected_areas);
    auto const detected_area = bg::area(std::get<2>(output));
    int expected_length = 0;
    for (int edge = 0; edge < grid.edge_count(); ++edge)
    {
        if (! get(expected_lines, edge)) continue;
        switch (grid.family(edge))
        {
        case edge_family::horizontal: expected_length += 4 * scale; break;
        case edge_family::vertical: expected_length += 3 * scale; break;
        case edge_family::diagonal: expected_length += 5 * scale; break;
        }
    }
    auto const detected_length = bg::length(std::get<1>(output));
    ok = ok && area_valid && detected_area == expected_area
            && detected_length == expected_length;
    if (! ok)
    {
        std::cout << label << " mismatch\n"
                  << "  input bits: " << bit_string(bits1) << " and "
                  << bit_string(bits2) << '\n'
                  << "  expected: points=" << bit_string(expected_points)
                  << " lines=" << bit_string(expected_lines)
                  << " areas=" << bit_string(expected_areas) << '\n'
                  << "  detected: points=" << bit_string(detected_points)
                  << " lines=" << bit_string(detected_lines)
                  << " areas=" << bit_string(detected_areas) << '\n';
        if (! area_valid) std::cout << "  invalid area: " << reason << '\n';
        if (detected_area != expected_area)
            std::cout << "  area: expected=" << expected_area << " detected=" << detected_area << '\n';
        if (detected_length != expected_length)
            std::cout << "  length: expected=" << expected_length
                      << " detected=" << detected_length << '\n';
        if (settings.verbose)
        {
            std::cout << "  expected point: " << bg::wkt(std::get<0>(expected)) << '\n'
                      << "  expected line: " << bg::wkt(std::get<1>(expected)) << '\n'
                      << "  expected area: " << bg::wkt(std::get<2>(expected)) << '\n'
                      << "  point output: " << bg::wkt(std::get<0>(output)) << '\n'
                      << "  line output: " << bg::wkt(std::get<1>(output)) << '\n'
                      << "  area output: " << bg::wkt(std::get<2>(output)) << '\n';
        }
        if (svg_enabled(settings, failures))
        {
            write_failure_svg(settings, iteration, std::string("set_operations ") + label,
                grid, [&](failure_svg& svg)
            {
                svg.add(geometry1);
                svg.add(geometry2);
                svg.add(expected);
                svg.add(output);
                svg.record("A", geometry1);
                svg.record("B", geometry2);
                svg.record("Expected", expected);
                svg.record("Actual (raw)", output);
                svg.panel(0, "Inputs", [&] { svg.inputs(geometry1, geometry2); });
                svg.panel(1, "Expected result", [&] { svg.map(expected); });
                svg.panel(2, "Actual result (raw)", [&]
                {
                    svg.map(output);
                    svg.samples(expected_points, detected_points, std::get<0>(test_points));
                    svg.samples(expected_lines, detected_lines, std::get<1>(test_points));
                    svg.samples(expected_areas, detected_areas, std::get<2>(test_points));
                });
                std::ostringstream measures;
                measures << "Length: expected " << expected_length << ", actual " << detected_length
                         << "; area: expected " << expected_area << ", actual " << detected_area;
                svg.text(20, 585, measures.str());
                if (!area_valid) svg.text(20, 615, "Invalid area: " + reason, "#c32f4b");
            });
        }
        ++failures[label];
    }
    return ok;
}

template <typename Tag1, typename Tag2, typename Geometry1, typename Geometry2>
bool check_pair(char const* pair_label,
                Geometry1 const& geometry1, Geometry2 const& geometry2,
                grid const& grid, primitive_vectors const& primitives,
                test_point_vectors const& test_points,
                settings const& settings, int iteration,
                bits const& bits1, bits const& bits2,
                std::map<std::string, int>& failures)
{
    bool ok = true;
    auto check = [&](char const* operation_label, boolean_operation operation,
                     auto const& apply)
    {
        std::string const label = std::string(operation_label) + " " + pair_label;
        bool const operation_ok = check_operation<Tag1, Tag2>(label.c_str(), operation,
            geometry1, geometry2, grid, primitives, test_points,
            settings, iteration, bits1, bits2, apply, failures);
        ok = ok && operation_ok;
        return operation_ok || ! settings.stop_on_failure;
    };
    return check("union", boolean_operation::union_,
                 [](auto const& a, auto const& b, auto& out) { bg::union_(a, b, out); })
        && check("intersection", boolean_operation::intersection,
                 [](auto const& a, auto const& b, auto& out) { bg::intersection(a, b, out); })
        && check("difference", boolean_operation::difference,
                 [](auto const& a, auto const& b, auto& out) { bg::difference(a, b, out); })
        && check("sym_difference", boolean_operation::sym_difference,
                 [](auto const& a, auto const& b, auto& out) { bg::sym_difference(a, b, out); })
        && ok;
}

template <typename Geometry>
bool check_conversion(char const* label, bits const& expected, Geometry const& geometry,
                      std::vector<point> const& test_points,
                      grid const& grid, settings const& settings, int iteration,
                      std::map<std::string, int>& failures)
{
    std::string error;
    bool const ok = validate_geometry(expected, geometry, test_points, error);
    if (! ok)
    {
        std::cout << label << " construction mismatch: " << error << '\n';
        if (settings.verbose) std::cout << bg::wkt(geometry) << '\n';
        if (svg_enabled(settings, failures))
            write_construction_svg(settings, iteration,
                std::string("set_operations ") + label + " construction", grid,
                expected, geometry, test_points, error);
        ++failures[std::string(label) + " construction"];
    }
    return ok;
}

bool test_iteration(grid const& grid, primitive_vectors const& primitives,
                    test_point_vectors const& test_points,
                    settings const& settings, int iteration,
                    generator_type& generator, std::map<std::string, int>& failures)
{
    bits const p1 = input_bits(generator, grid.vertex_count(), settings, "point1");
    bits const p2 = input_bits(generator, grid.vertex_count(), settings, "point2");
    bits const l1 = input_bits(generator, grid.edge_count(), settings, "line1");
    bits const l2 = input_bits(generator, grid.edge_count(), settings, "line2");
    bits const a1 = input_bits(generator, grid.cell_count(), settings, "area1");
    bits const a2 = input_bits(generator, grid.cell_count(), settings, "area2");
    bits const line_vertices1 = input_bits(generator, grid.vertex_count(), settings, "line-vertices1");
    bits const line_vertices2 = input_bits(generator, grid.vertex_count(), settings, "line-vertices2");
    bits const area_vertices1 = input_bits(generator, grid.vertex_count(), settings, "area-vertices1");
    bits const area_vertices2 = input_bits(generator, grid.vertex_count(), settings, "area-vertices2");
    auto finish = [&](bool ok)
    {
        if (!ok) print_replay(settings, {
            {"point1", &p1}, {"point2", &p2}, {"line1", &l1}, {"line2", &l2},
            {"area1", &a1}, {"area2", &a2},
            {"line-vertices1", &line_vertices1}, {"line-vertices2", &line_vertices2},
            {"area-vertices1", &area_vertices1}, {"area-vertices2", &area_vertices2}});
        return ok;
    };

    multi_point const gp1
        = bits_to_geometry<multi_point>(p1, p1, grid, std::get<0>(primitives));
    multi_point const gp2
        = bits_to_geometry<multi_point>(p2, p2, grid, std::get<0>(primitives));
    multi_linestring const gl1
        = bits_to_geometry<multi_linestring>(
            l1, line_vertices1, grid, std::get<1>(primitives));
    multi_linestring const gl2
        = bits_to_geometry<multi_linestring>(
            l2, line_vertices2, grid, std::get<1>(primitives));
    multi_polygon const ga1
        = bits_to_geometry<multi_polygon>(
            a1, area_vertices1, grid, std::get<2>(primitives));
    multi_polygon const ga2
        = bits_to_geometry<multi_polygon>(
            a2, area_vertices2, grid, std::get<2>(primitives));

    bool ok = check_conversion("point1", p1, gp1, std::get<0>(test_points),
                               grid, settings, iteration, failures)
           && check_conversion("point2", p2, gp2, std::get<0>(test_points),
                               grid, settings, iteration, failures)
           && check_conversion("line1", l1, gl1, std::get<1>(test_points),
                               grid, settings, iteration, failures)
           && check_conversion("line2", l2, gl2, std::get<1>(test_points),
                               grid, settings, iteration, failures)
           && check_conversion("area1", a1, ga1, std::get<2>(test_points),
                               grid, settings, iteration, failures)
           && check_conversion("area2", a2, ga2, std::get<2>(test_points),
                               grid, settings, iteration, failures);
    if (! ok) return finish(false);

#define CHECK_PAIR(label, g1, g2, tag1, tag2, b1, b2) \
    do { bool const pair_ok = check_pair<tag1, tag2>(label, g1, g2, grid, primitives, \
             test_points, settings, iteration, b1, b2, failures); \
         ok = ok && pair_ok; if (!pair_ok && settings.stop_on_failure) return finish(false); } while (false)

    CHECK_PAIR("point/point", gp1, gp2,
               bg::pointlike_tag, bg::pointlike_tag, p1, p2);
    CHECK_PAIR("point/point reversed", gp2, gp1,
               bg::pointlike_tag, bg::pointlike_tag, p2, p1);
    CHECK_PAIR("point/line", gp1, gl2,
               bg::pointlike_tag, bg::linear_tag, p1, l2);
    CHECK_PAIR("line/point", gl1, gp2,
               bg::linear_tag, bg::pointlike_tag, l1, p2);
    CHECK_PAIR("point/area", gp1, ga2,
               bg::pointlike_tag, bg::areal_tag, p1, a2);
    CHECK_PAIR("area/point", ga1, gp2,
               bg::areal_tag, bg::pointlike_tag, a1, p2);
    CHECK_PAIR("line/line", gl1, gl2,
               bg::linear_tag, bg::linear_tag, l1, l2);
    CHECK_PAIR("line/line reversed", gl2, gl1,
               bg::linear_tag, bg::linear_tag, l2, l1);
    CHECK_PAIR("line/area", gl1, ga2,
               bg::linear_tag, bg::areal_tag, l1, a2);
    CHECK_PAIR("area/line", ga1, gl2,
               bg::areal_tag, bg::linear_tag, a1, l2);
    CHECK_PAIR("area/area", ga1, ga2,
               bg::areal_tag, bg::areal_tag, a1, a2);
    CHECK_PAIR("area/area reversed", ga2, ga1,
               bg::areal_tag, bg::areal_tag, a2, a1);
#undef CHECK_PAIR
    return finish(ok);
}

int main(int argc, char** argv)
{
    BoostGeometryWriteTestConfiguration();
    try
    {
        settings settings;
        boost::program_options::options_description bit_options("Exact inputs");
        add_bit_options(settings, bit_options);
        if (! parse_options(argc, argv, "=== random_grid_set_operations ===", settings, bit_options))
            return 0;
        grid const grid(settings.width, settings.height);
        primitive_vectors const primitives = make_primitives(grid);
        test_point_vectors const test_points = make_test_points(grid, primitives);
        generator_type generator(settings.seed);
        std::map<std::string, int> failures;
        auto const start = std::chrono::high_resolution_clock::now();
        int iterations = 0;
        for (; iterations < settings.count || settings.count == -1; ++iterations)
        {
            bool const ok = test_iteration(grid, primitives, test_points,
                                           settings, iterations, generator, failures);
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
