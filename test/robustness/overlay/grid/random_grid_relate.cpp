// Boost.Geometry (aka GGL, Generic Geometry Library)
// Robustness Test

// Copyright (c) 2026 Tinko Bartels, Shenzhen, China.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_GEOMETRY_NO_BOOST_TEST

#include "random_grid.hpp"
#include "random_grid_svg.hpp"

#include <boost/geometry/algorithms/detail/relation/interface.hpp>
#include <boost/geometry/algorithms/relate.hpp>

using namespace random_grid;

template <typename Geometry>
bool check_construction(char const* label, bits const& expected, Geometry const& geometry,
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
                std::string("relate ") + label + " construction", grid,
                expected, geometry, test_points, error);
        ++failures[std::string(label) + " construction"];
    }
    return ok;
}

template <typename Tag1, typename Tag2, typename Geometry1, typename Geometry2>
bool check_relation(char const* label,
                    Geometry1 const& geometry1, Geometry2 const& geometry2,
                    bits const& bits1, bits const& bits2,
                    grid const& grid, settings const& settings, int iteration,
                    std::map<std::string, int>& failures)
{
    std::string const expected = expected_relation<Tag1, Tag2>(grid, bits1, bits2);
    std::string const detected = bg::relation(geometry1, geometry2).str();
    bool const matrix_ok = expected == detected;
    bool const mask_ok = bg::relate(geometry1, geometry2, bg::de9im::mask(expected));
    if (! matrix_ok || ! mask_ok)
    {
        std::cout << label << " mismatch expected=" << expected << " detected=" << detected
                  << " mask=" << mask_ok << '\n'
                  << "  input bits: " << bit_string(bits1) << " and "
                  << bit_string(bits2) << '\n';
        if (settings.verbose)
        {
            std::cout << "  geometry1: " << bg::wkt(geometry1) << '\n'
                      << "  geometry2: " << bg::wkt(geometry2) << '\n';
        }
        if (svg_enabled(settings, failures))
        {
            write_failure_svg(settings, iteration, std::string("relate ") + label,
                grid, [&](failure_svg& svg)
            {
                svg.add(geometry1);
                svg.add(geometry2);
                svg.record("A", geometry1);
                svg.record("B", geometry2);
                svg.panel(0, "Inputs", [&] { svg.inputs(geometry1, geometry2); });
                svg.matrix(1, "Expected DE-9IM", expected, detected);
                svg.matrix(2, "Actual DE-9IM", detected, expected);
                svg.text(20, 585, "I = interior; B = boundary; E = exterior. F = empty; 0 = points; 1 = curves; 2 = area.");
                svg.text(20, 615, std::string("relate(expected mask) = ") + (mask_ok ? "true" : "false"));
            });
        }
        ++failures[label];
    }
    return matrix_ok && mask_ok;
}

bool test_iteration(grid const& grid, primitive_vectors const& primitives,
                    test_point_vectors const& test_points,
                    settings const& settings, int iteration,
                    generator_type& generator, std::map<std::string, int>& failures)
{
    bits const points1 = input_bits(generator, grid.vertex_count(), settings, "point1");
    bits const points2 = input_bits(generator, grid.vertex_count(), settings, "point2");
    bits const lines1 = input_bits(generator, grid.edge_count(), settings, "line1");
    bits const lines2 = input_bits(generator, grid.edge_count(), settings, "line2");
    bits const areas1 = input_bits(generator, grid.cell_count(), settings, "area1");
    bits const areas2 = input_bits(generator, grid.cell_count(), settings, "area2");
    bits const line_vertices1 = input_bits(generator, grid.vertex_count(), settings, "line-vertices1");
    bits const line_vertices2 = input_bits(generator, grid.vertex_count(), settings, "line-vertices2");
    bits const area_vertices1 = input_bits(generator, grid.vertex_count(), settings, "area-vertices1");
    bits const area_vertices2 = input_bits(generator, grid.vertex_count(), settings, "area-vertices2");
    auto finish = [&](bool ok)
    {
        if (!ok) print_replay(settings, {
            {"point1", &points1}, {"point2", &points2}, {"line1", &lines1}, {"line2", &lines2},
            {"area1", &areas1}, {"area2", &areas2},
            {"line-vertices1", &line_vertices1}, {"line-vertices2", &line_vertices2},
            {"area-vertices1", &area_vertices1}, {"area-vertices2", &area_vertices2}});
        return ok;
    };

    multi_point const gp1
        = bits_to_geometry<multi_point>(points1, points1, grid, std::get<0>(primitives));
    multi_point const gp2
        = bits_to_geometry<multi_point>(points2, points2, grid, std::get<0>(primitives));
    multi_linestring const gl1
        = bits_to_geometry<multi_linestring>(lines1, line_vertices1, grid,
                                             std::get<1>(primitives));
    multi_linestring const gl2
        = bits_to_geometry<multi_linestring>(lines2, line_vertices2, grid,
                                             std::get<1>(primitives));
    multi_polygon const ga1
        = bits_to_geometry<multi_polygon>(areas1, area_vertices1, grid,
                                          std::get<2>(primitives));
    multi_polygon const ga2
        = bits_to_geometry<multi_polygon>(areas2, area_vertices2, grid,
                                          std::get<2>(primitives));

    bool const inputs_ok
        = check_construction("point1", points1, gp1, std::get<0>(test_points),
                             grid, settings, iteration, failures)
       && check_construction("point2", points2, gp2, std::get<0>(test_points),
                             grid, settings, iteration, failures)
       && check_construction("line1", lines1, gl1, std::get<1>(test_points),
                             grid, settings, iteration, failures)
       && check_construction("line2", lines2, gl2, std::get<1>(test_points),
                             grid, settings, iteration, failures)
       && check_construction("area1", areas1, ga1, std::get<2>(test_points),
                             grid, settings, iteration, failures)
       && check_construction("area2", areas2, ga2, std::get<2>(test_points),
                             grid, settings, iteration, failures);
    if (! inputs_ok) return finish(false);

    bool ok = true;
#define CHECK_RELATION(label, g1, g2, tag1, b1, tag2, b2) \
    do { bool const relation_ok = check_relation<tag1, tag2>(label, g1, g2, b1, b2, \
             grid, settings, iteration, failures); ok = ok && relation_ok; \
         if (! relation_ok && settings.stop_on_failure) return finish(false); } while (false)

    CHECK_RELATION("point/point", gp1, gp2,
                   bg::pointlike_tag, points1, bg::pointlike_tag, points2);
    CHECK_RELATION("point/line", gp1, gl2,
                   bg::pointlike_tag, points1, bg::linear_tag, lines2);
    CHECK_RELATION("point/area", gp1, ga2,
                   bg::pointlike_tag, points1, bg::areal_tag, areas2);
    CHECK_RELATION("line/point", gl1, gp2,
                   bg::linear_tag, lines1, bg::pointlike_tag, points2);
    CHECK_RELATION("line/line", gl1, gl2,
                   bg::linear_tag, lines1, bg::linear_tag, lines2);
    CHECK_RELATION("line/area", gl1, ga2,
                   bg::linear_tag, lines1, bg::areal_tag, areas2);
    CHECK_RELATION("area/point", ga1, gp2,
                   bg::areal_tag, areas1, bg::pointlike_tag, points2);
    CHECK_RELATION("area/line", ga1, gl2,
                   bg::areal_tag, areas1, bg::linear_tag, lines2);
    CHECK_RELATION("area/area", ga1, ga2,
                   bg::areal_tag, areas1, bg::areal_tag, areas2);
#undef CHECK_RELATION
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
        if (! parse_options(argc, argv, "=== random_grid_relate ===", settings, bit_options)) return 0;
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
