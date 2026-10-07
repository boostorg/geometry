// Boost.Geometry (aka GGL, Generic Geometry Library)
// Robustness Test

// Copyright (c) 2026 Tinko Bartels, Shenzhen, China.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_ROBUSTNESS_OVERLAY_GRID_RANDOM_GRID_SVG_HPP
#define BOOST_GEOMETRY_TEST_ROBUSTNESS_OVERLAY_GRID_RANDOM_GRID_SVG_HPP

#include "random_grid.hpp"
#include <boost/geometry/core/topological_dimension.hpp>
#include <boost/geometry/io/svg/svg_mapper.hpp>

#include <fstream>
#include <iomanip>
#include <limits>
#include <type_traits>

namespace random_grid
{

inline bool svg_enabled(settings const& settings,
                        std::map<std::string, int> const& failures)
{
    return !settings.svg_directory.empty()
        && (settings.svg_limit == 0 || failure_count(failures) < settings.svg_limit);
}

inline std::string svg_escape(std::string const& text)
{
    std::string result;
    for (char c : text)
    {
        switch (c)
        {
        case '&': result += "&amp;"; break;
        case '<': result += "&lt;"; break;
        case '>': result += "&gt;"; break;
        default: result += c;
        }
    }
    return result;
}

struct failure_svg
{
    using svg_point = bg::model::d2::point_xy<double>;
    std::ostream& out;
    bg::svg_mapper<svg_point>& mapper;
    grid const& g;

    void text(int x, int y, std::string const& value, char const* color = "#243540",
              int size = 15)
    {
        out << "<text x=\"" << x << "\" y=\"" << y << "\" fill=\"" << color
            << "\" font-family=\"sans-serif\" font-size=\"" << size << "\">"
            << svg_escape(value) << "</text>\n";
    }

    template <typename Geometry>
    void add(Geometry const& geometry) { mapper.add(geometry); }

    void add(output_tuple const& geometry)
    {
        add(std::get<0>(geometry));
        add(std::get<1>(geometry));
        add(std::get<2>(geometry));
    }

    template <typename Geometry>
    void map(Geometry const& geometry, char const* color = "#2868b2")
    {
        using tag = bg::tag_cast_t<bg::tag_t<Geometry>, bg::areal_tag, bg::linear_tag>;
        std::string const fill = std::is_same<tag, bg::linear_tag>::value ? "none" : color;
        mapper.map(geometry, "fill:" + fill + ";fill-opacity:0.25;stroke:" + color
                            + ";stroke-width:2", 3);
    }

    void map(output_tuple const& geometry, char const* color = "#2868b2")
    {
        map(std::get<2>(geometry), color);
        map(std::get<1>(geometry), color);
        map(std::get<0>(geometry), color);
    }

    template <typename Geometry1, typename Geometry2>
    void inputs(Geometry1 const& a, Geometry2 const& b)
    {
        // Draw lower-dimensional inputs last so polygon fills cannot hide them.
        if (bg::topological_dimension<Geometry1>::value >= bg::topological_dimension<Geometry2>::value)
        {
            map(a);
            map(b, "#13836c");
        }
        else
        {
            map(b, "#13836c");
            map(a);
        }
    }

    template <typename Geometry>
    void record(char const* label, Geometry const& geometry)
    {
        std::ostringstream wkt;
        wkt << std::setprecision(std::numeric_limits<double>::max_digits10) << bg::wkt(geometry);
        out << "<desc>" << svg_escape(label) << ": " << svg_escape(wkt.str()) << "</desc>\n";
    }

    void record(char const* label, output_tuple const& geometry)
    {
        record(label, std::get<0>(geometry));
        record(label, std::get<1>(geometry));
        record(label, std::get<2>(geometry));
    }

    template <typename Draw>
    void panel(int index, char const* title, Draw const& draw)
    {
        text(20 + 440 * index, 115, title, "#243540", 18);
        out << "<g transform=\"translate(" << 20 + 440 * index << ",135)\">\n";
        for (int edge = 0; edge < g.edge_count(); ++edge)
        {
            auto const v = g.edge_vertices(edge);
            linestring const segment{
                g.vertex_point(v.first % (g.width + 1), v.first / (g.width + 1)),
                g.vertex_point(v.second % (g.width + 1), v.second / (g.width + 1))};
            mapper.map(segment, "fill:none;stroke:#e5eaee;stroke-width:0.6");
        }
        draw();
        out << "</g>\n";
    }

    void samples(bits const& expected, bits const& detected,
                 std::vector<point> const& points)
    {
        for (std::size_t i = 0; i < points.size(); ++i)
        {
            if (get(expected, static_cast<int>(i)) == get(detected, static_cast<int>(i))) continue;
            char const* color = get(expected, static_cast<int>(i)) ? "#d47a00" : "#c32f4b";
            out << "<g><title>Sample " << i << ": " << bg::wkt(points[i]) << "</title>\n";
            mapper.map(points[i], std::string("fill:white;stroke:") + color + ";stroke-width:2", 4);
            out << "</g>\n";
        }
    }

    void cells(bits const& selected)
    {
        for (int cell = 0; cell < g.cell_count(); ++cell)
        {
            if (!get(selected, cell)) continue;
            ring triangle;
            for (int v : g.cell_vertices(cell))
                triangle.push_back(g.vertex_point(v % (g.width + 1), v / (g.width + 1)));
            triangle.push_back(triangle.front());
            map(triangle);
        }
    }

    void matrix(int index, char const* title, std::string const& value,
                std::string const& other)
    {
        int const x = 20 + 440 * index;
        text(x, 115, title, "#243540", 18);
        char const* row[] = {"I(A)", "B(A)", "E(A)"};
        char const* col[] = {"I(B)", "B(B)", "E(B)"};
        for (int i = 0; i < 3; ++i)
        {
            text(x + 105 + 85 * i, 180, col[i]);
            text(x + 20, 234 + 75 * i, row[i]);
            for (int j = 0; j < 3; ++j)
            {
                bool const different = value[3 * i + j] != other[3 * i + j];
                out << "<rect x=\"" << x + 85 + j * 85 << "\" y=\"" << 195 + i * 75
                    << "\" width=\"80\" height=\"70\" fill=\""
                    << (different ? "#ffe9ee" : "#f3f6f8") << "\"/>\n";
                text(x + 113 + j * 85, 240 + i * 75, value.substr(3 * i + j, 1),
                     different ? "#c32f4b" : "#243540", 28);
            }
        }
        text(x + 85, 465, value, "#243540", 20);
    }
};

template <typename Draw>
void write_failure_svg(settings const& settings, int iteration,
                       std::string const& label, grid const& g, Draw const& draw,
                       std::string const& extra_options = "")
{
    std::string name = label;
    for (char& c : name)
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
            || (c >= '0' && c <= '9'))) c = '_';
    std::string const filename = settings.svg_directory + '/' + name + '_'
        + std::to_string(settings.width) + 'x' + std::to_string(settings.height)
        + "_seed" + std::to_string(settings.seed) + "_iteration" + std::to_string(iteration) + ".svg";
    std::ofstream out(filename);
    if (!out)
    {
        std::cerr << "Cannot write SVG: " << filename << '\n';
        std::exit(EXIT_FAILURE);
    }
    {
        bg::svg_mapper<failure_svg::svg_point> mapper(out, 400, 400, 0.95,
            "width=\"1320\" height=\"660\" viewBox=\"0 0 1320 660\"");
        failure_svg svg{out, mapper, g};
        out << "<title>" << svg_escape(label) << "</title>\n";
        if (!settings.bit_patterns.empty())
        {
            out << "<desc>Exact replay: --width " << settings.width
                << " --height " << settings.height << " --count 1";
            for (auto const& pattern : settings.bit_patterns)
                out << " --" << pattern.first << " '" << bit_string(pattern.second) << '\'';
            out << "</desc>\n";
        }
        // A nondegenerate frame also covers empty, point and horizontal/vertical inputs.
        mapper.add(g.vertex_point(0, 0));
        mapper.add(g.vertex_point(g.width, g.height));
        out << "<rect width=\"1320\" height=\"660\" fill=\"white\"/>\n";
        svg.text(20, 30, label, "#243540", 22);
        svg.text(20, 58, "--width " + std::to_string(settings.width) + " --height "
            + std::to_string(settings.height) + " --seed " + std::to_string(settings.seed)
            + extra_options + " --count " + std::to_string(static_cast<long long>(iteration) + 1)
            + " (iteration " + std::to_string(iteration) + ", zero-based)"
            + (settings.bit_patterns.empty() ? "" : " [exact inputs in SVG description]"));
        svg.text(20, 82, "Inputs: A blue, B green. Sample markers: orange missing, red unexpected.");
        draw(svg);
    }
    out.close();
    if (!out)
    {
        std::cerr << "Failed writing SVG: " << filename << '\n';
        std::exit(EXIT_FAILURE);
    }
    std::cout << "  SVG: " << filename << '\n';
}

template <typename Geometry>
void write_construction_svg(settings const& settings, int iteration,
                            std::string const& label, grid const& g, bits const& expected,
                            Geometry const& geometry, std::vector<point> const& points,
                            std::string const& error, std::string const& extra_options = "")
{
    write_failure_svg(settings, iteration, label, g, [&](failure_svg& svg)
    {
        svg.add(geometry);
        svg.record("Constructed", geometry);
        svg.panel(0, "Expected selected samples", [&]
        {
            for (std::size_t i = 0; i < points.size(); ++i)
                if (get(expected, static_cast<int>(i))) svg.map(points[i]);
        });
        svg.panel(1, "Constructed geometry", [&] { svg.map(geometry); });
        svg.panel(2, "Sampling discrepancies", [&]
        {
            svg.map(geometry);
            if (bg::is_valid(geometry))
                svg.samples(expected, geometry_to_bits(geometry, points), points);
        });
        svg.out << "<desc>Construction mismatch: " << svg_escape(error) << "</desc>\n";
        svg.text(20, 590, error.size() <= 140 ? error : error.substr(0, 137) + "...");
    }, extra_options);
}

} // namespace random_grid

#endif
