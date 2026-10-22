// Boost.Geometry (aka GGL, Generic Geometry Library)
// Robustness Test

// Copyright (c) 2026 Tinko Bartels, Shenzhen, China.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_ROBUSTNESS_OVERLAY_GRID_RANDOM_GRID_HPP
#define BOOST_GEOMETRY_TEST_ROBUSTNESS_OVERLAY_GRID_RANDOM_GRID_HPP

#include <boost/geometry/algorithms/area.hpp>
#include <boost/geometry/algorithms/covered_by.hpp>
#include <boost/geometry/algorithms/is_valid.hpp>
#include <boost/geometry/algorithms/union.hpp>
#include <boost/geometry/algorithms/within.hpp>
#include <boost/geometry/core/assert.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/io/wkt/write.hpp>

#include <boost/program_options.hpp>

#include <algorithm>
#include <bitset>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#define BOOST_GEOMETRY_TEST_ONLY_ONE_TYPE
#ifndef BOOST_GEOMETRY_DEFAULT_TEST_TYPE
#define BOOST_GEOMETRY_DEFAULT_TEST_TYPE std::int_least64_t
#endif
#include <geometry_test_common.hpp>

namespace bg = boost::geometry;

namespace random_grid
{

constexpr int chunk_size = 64;
// H_j: y=15j, V_i: x=20i, D_k: 3x+4y=120k, E_k: 3x-4y=120k.
// Even tiles (i+j) have a rising diagonal, odd tiles a falling one.
constexpr int scale = 5;
constexpr int cell_area = 150;
using bits = std::vector<std::bitset<chunk_size>>;
using generator_type = std::mt19937_64;

using coordinate_type = BOOST_GEOMETRY_DEFAULT_TEST_TYPE;
using point = bg::model::d2::point_xy<coordinate_type>;
using linestring = bg::model::linestring<point>;
using ring = bg::model::ring<point, false>;
using polygon = bg::model::polygon<point, false>;
using multi_point = bg::model::multi_point<point>;
using multi_linestring = bg::model::multi_linestring<linestring>;
using multi_polygon = bg::model::multi_polygon<polygon>;
using output_tuple = std::tuple<multi_point, multi_linestring, multi_polygon>;
using primitive_vectors = std::tuple<std::vector<point>,
                                     std::vector<linestring>,
                                     std::vector<ring>>;
using test_point_vectors = std::tuple<std::vector<point>,
                                      std::vector<point>,
                                      std::vector<point>>;
using vertex_paths = std::vector<std::vector<int>>;

struct settings
{
    int width = 3;
    int height = 3;
    int count = 100;
    generator_type::result_type seed = generator_type::default_seed;
    bool verbose = false;
    bool stop_on_failure = false;
    std::string svg_directory;
    int svg_limit = 20;
    std::map<std::string, bits> bit_patterns;
};

inline bits make_bits(int size)
{
    return bits((size + chunk_size - 1) / chunk_size);
}

inline bool get(bits const& value, int index)
{
    return value[index / chunk_size][index % chunk_size];
}

inline void set(bits& value, int index, bool state = true)
{
    value[index / chunk_size].set(index % chunk_size, state);
}

inline void reset(bits& value, int index)
{
    value[index / chunk_size].reset(index % chunk_size);
}

inline int count(bits const& value)
{
    return std::accumulate(value.begin(), value.end(), 0,
        [](int result, std::bitset<chunk_size> const& chunk)
        {
            return result + static_cast<int>(chunk.count());
        });
}

inline bits random_bits(generator_type& generator, int size)
{
    bits result = make_bits(size);
    std::generate(result.begin(), result.end(), std::ref(generator));
    if (size % chunk_size != 0)
    {
        std::bitset<chunk_size> mask;
        mask.set();
        mask >>= chunk_size - size % chunk_size;
        result.back() &= mask;
    }
    return result;
}

inline std::string bit_string(bits const& value)
{
    std::ostringstream out;
    out << '{';
    char const* separator = "";
    for (auto const& chunk : value)
    {
        out << separator << chunk.to_ullong();
        separator = " ";
    }
    out << '}';
    return out.str();
}

inline void bit_pattern_error(std::string const& name, char const* reason)
{
    std::cerr << "--" << name << ": " << reason << '\n';
    std::exit(EXIT_FAILURE);
}

inline bits parse_bit_pattern(std::string const& text, std::string const& name)
{
    auto const first = text.find_first_not_of(" \t\r\n");
    auto const last = text.find_last_not_of(" \t\r\n");
    if (first == std::string::npos || text[first] != '{' || text[last] != '}')
        bit_pattern_error(name, "expected {decimal chunks}, least-significant chunk first");
    std::istringstream input(text.substr(first + 1, last - first - 1));
    bits result;
    std::string token;
    while (input >> token)
    {
        unsigned long long chunk;
        std::istringstream number(token);
        if (token.find_first_not_of("0123456789") != std::string::npos
            || !(number >> chunk) || !number.eof())
            bit_pattern_error(name, "each chunk must be an unsigned 64-bit decimal integer");
        result.emplace_back(chunk);
    }
    return result;
}

inline void add_bit_options(settings& value,
                            boost::program_options::options_description& description)
{
    for (char const* name : {"point1", "point2", "line1", "line2", "area1", "area2",
                            "line-vertices1", "line-vertices2", "area-vertices1", "area-vertices2"})
    {
        description.add_options()(name,
            boost::program_options::value<std::string>()->notifier(
                [&value, name](std::string const& text)
                {
                    value.bit_patterns[name] = parse_bit_pattern(text, name);
                }),
            "Exact {decimal chunks}; unspecified patterns are empty, vertices are removable");
    }
}

inline bits input_bits(generator_type& generator, int size,
                        settings const& settings, char const* name)
{
    if (settings.bit_patterns.empty()) return random_bits(generator, size);
    bits result = make_bits(size);
    auto const found = settings.bit_patterns.find(name);
    if (found == settings.bit_patterns.end()) return result;
    auto const& pattern = found->second;
    if (pattern.size() > result.size()
        || (pattern.size() == result.size() && size % chunk_size != 0
            && (pattern.back() >> (size % chunk_size)).any()))
        bit_pattern_error(name, "pattern exceeds this grid's bit count");
    std::copy(pattern.begin(), pattern.end(), result.begin());
    return result;
}

inline void print_replay(settings const& settings,
                         std::initializer_list<std::pair<char const*, bits const*>> patterns)
{
    std::cout << "replay: --width " << settings.width << " --height " << settings.height
              << " --count 1";
    for (auto const& pattern : patterns)
        std::cout << " --" << pattern.first << " '" << bit_string(*pattern.second) << '\'';
    std::cout << '\n';
}

inline int failure_count(std::map<std::string, int> const& failures)
{
    return std::accumulate(failures.begin(), failures.end(), 0,
        [](int result, std::pair<std::string const, int> const& failure)
        {
            return result + failure.second;
        });
}

inline bool parse_options(int argc, char** argv, char const* title, settings& value,
                          boost::program_options::options_description const& additional
                              = boost::program_options::options_description())
{
    namespace po = boost::program_options;
    po::options_description description(title);
    description.add_options()
        ("help", "Help message")
        ("seed", po::value<generator_type::result_type>(&value.seed)->default_value(value.seed),
         "Initialization seed")
        ("count", po::value<int>(&value.count)->default_value(value.count),
         "Number of iterations (-1 for an infinite run)")
        ("width", po::value<int>(&value.width)->default_value(value.width), "Tile columns")
        ("height", po::value<int>(&value.height)->default_value(value.height), "Tile rows")
        ("verbose", po::bool_switch(&value.verbose), "Print geometries on failure")
        ("svg", po::value<std::string>(&value.svg_directory),
         "Write failure SVGs to an existing directory")
        ("svg-limit", po::value<int>(&value.svg_limit)->default_value(value.svg_limit),
         "Maximum failure SVGs per run (0 for unlimited)")
        ("stop-on-failure", po::bool_switch(&value.stop_on_failure),
         "Stop after the first failing iteration");
    description.add(additional);

    po::variables_map variables;
    po::store(po::parse_command_line(argc, argv, description), variables);
    po::notify(variables);
    if (variables.count("help"))
    {
        std::cout << description << '\n';
        return false;
    }
    if (!value.bit_patterns.empty() && variables["count"].defaulted()) value.count = 1;
    if (value.width < 1 || value.height < 1 || value.count < -1)
    {
        std::cerr << "width and height must be positive; count must be at least -1\n";
        std::exit(EXIT_FAILURE);
    }
    if (value.svg_limit < 0)
    {
        std::cerr << "svg-limit must be nonnegative\n";
        std::exit(EXIT_FAILURE);
    }
    return true;
}

inline std::string report(std::chrono::high_resolution_clock::time_point start,
                          int iterations,
                          std::map<std::string, int> const& failures)
{
    auto const elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now() - start).count();
    std::ostringstream out;
    out << iterations << " iterations in " << elapsed << " ms";
    if (failures.empty())
    {
        out << ", no failures\n";
    }
    else
    {
        out << ", failures:";
        for (auto const& failure : failures)
        {
            out << "\n  " << failure.first << ": " << failure.second;
        }
        out << '\n';
    }
    return out.str();
}

enum class edge_family { horizontal, vertical, diagonal };
enum class location { interior = 0, boundary = 1, exterior = 2 };

struct grid
{
    int width;
    int height;

    explicit grid(int w, int h)
        : width(w), height(h)
    {}

    int vertex_count() const { return (width + 1) * (height + 1); }
    int horizontal_count() const { return width * (height + 1); }
    int vertical_count() const { return (width + 1) * height; }
    int diagonal_count() const { return width * height; }
    int edge_count() const { return horizontal_count() + vertical_count() + diagonal_count(); }
    int cell_count() const { return 2 * width * height; }

    int vertex_index(int i, int j) const { return j * (width + 1) + i; }
    int vertex_index(point const& p) const
    {
        int const i = static_cast<int>(p.x() / (4 * scale));
        int const j = static_cast<int>(p.y() / (3 * scale));
        BOOST_GEOMETRY_ASSERT(p.x() == coordinate_type(4 * scale) * i);
        BOOST_GEOMETRY_ASSERT(p.y() == coordinate_type(3 * scale) * j);
        BOOST_GEOMETRY_ASSERT(0 <= i && i <= width && 0 <= j && j <= height);
        return vertex_index(i, j);
    }
    int horizontal_index(int i, int j) const { return j * width + i; }
    int vertical_index(int i, int j) const
    {
        return horizontal_count() + j * (width + 1) + i;
    }
    int diagonal_index(int i, int j) const
    {
        return horizontal_count() + vertical_count() + j * width + i;
    }
    int cell_index(int i, int j, int upper) const
    {
        return 2 * (j * width + i) + upper;
    }

    bool valid_tile(int i, int j) const
    {
        return 0 <= i && i < width && 0 <= j && j < height;
    }

    bool rising(int i, int j) const
    {
        return (i + j) % 2 == 0;
    }

    point vertex_point(int i, int j) const
    {
        return point(coordinate_type(4 * scale) * i,
                     coordinate_type(3 * scale) * j);
    }
    point cell_sample(int i, int j, int upper) const
    {
        point const p = vertex_point(i, j);
        int const x = (upper != rising(i, j) ? 3 : 1) * scale;
        return point(p.x() + x, p.y() + (upper ? 2 : 1) * scale);
    }

    edge_family family(int edge) const
    {
        if (edge < horizontal_count()) return edge_family::horizontal;
        if (edge < horizontal_count() + vertical_count()) return edge_family::vertical;
        return edge_family::diagonal;
    }

    void edge_position(int edge, int& i, int& j) const
    {
        switch (family(edge))
        {
        case edge_family::horizontal:
            i = edge % width;
            j = edge / width;
            break;
        case edge_family::vertical:
        {
            int const local = edge - horizontal_count();
            i = local % (width + 1);
            j = local / (width + 1);
            break;
        }
        case edge_family::diagonal:
        {
            int const local = edge - horizontal_count() - vertical_count();
            i = local % width;
            j = local / width;
            break;
        }
        }
    }

    std::pair<int, int> edge_vertices(int edge) const
    {
        int i, j;
        edge_position(edge, i, j);
        switch (family(edge))
        {
        case edge_family::horizontal:
            return {vertex_index(i, j), vertex_index(i + 1, j)};
        case edge_family::vertical:
            return {vertex_index(i, j), vertex_index(i, j + 1)};
        case edge_family::diagonal:
            if (rising(i, j))
                return {vertex_index(i, j), vertex_index(i + 1, j + 1)};
            return {vertex_index(i, j + 1), vertex_index(i + 1, j)};
        }
        BOOST_GEOMETRY_ASSERT(false);
        return {};
    }

    std::vector<int> edge_cells(int edge) const
    {
        int i, j;
        edge_position(edge, i, j);
        std::vector<int> result;
        switch (family(edge))
        {
        case edge_family::horizontal:
            if (valid_tile(i, j - 1)) result.push_back(cell_index(i, j - 1, 1));
            if (valid_tile(i, j)) result.push_back(cell_index(i, j, 0));
            break;
        case edge_family::vertical:
            if (valid_tile(i - 1, j))
                result.push_back(cell_index(i - 1, j, rising(i - 1, j) ? 0 : 1));
            if (valid_tile(i, j))
                result.push_back(cell_index(i, j, rising(i, j) ? 1 : 0));
            break;
        case edge_family::diagonal:
            result.push_back(cell_index(i, j, 0));
            result.push_back(cell_index(i, j, 1));
            break;
        }
        return result;
    }

    std::vector<int> cell_vertices(int cell) const
    {
        int const upper = cell % 2;
        int const tile = cell / 2;
        int const i = tile % width;
        int const j = tile / width;
        if (rising(i, j))
        {
            if (upper)
                return {vertex_index(i, j), vertex_index(i + 1, j + 1),
                        vertex_index(i, j + 1)};
            return {vertex_index(i, j), vertex_index(i + 1, j),
                    vertex_index(i + 1, j + 1)};
        }
        if (upper)
        {
            return {vertex_index(i + 1, j), vertex_index(i + 1, j + 1),
                    vertex_index(i, j + 1)};
        }
        return {vertex_index(i, j), vertex_index(i + 1, j), vertex_index(i, j + 1)};
    }

    std::vector<int> cell_edges(int cell) const
    {
        int const upper = cell % 2;
        int const tile = cell / 2;
        int const i = tile % width;
        int const j = tile / width;
        if (rising(i, j))
        {
            if (upper)
                return {diagonal_index(i, j), horizontal_index(i, j + 1),
                        vertical_index(i, j)};
            return {horizontal_index(i, j), vertical_index(i + 1, j),
                    diagonal_index(i, j)};
        }
        if (upper)
        {
            return {vertical_index(i + 1, j), horizontal_index(i, j + 1),
                    diagonal_index(i, j)};
        }
        return {horizontal_index(i, j), diagonal_index(i, j), vertical_index(i, j)};
    }

    std::vector<int> vertex_edges(int vertex) const
    {
        int const i = vertex % (width + 1);
        int const j = vertex / (width + 1);
        std::vector<int> result;
        if (i > 0) result.push_back(horizontal_index(i - 1, j));
        if (i < width) result.push_back(horizontal_index(i, j));
        if (j > 0) result.push_back(vertical_index(i, j - 1));
        if (j < height) result.push_back(vertical_index(i, j));
        if (rising(i, j))
        {
            for (int dj = -1; dj <= 0; ++dj)
                for (int di = -1; di <= 0; ++di)
                    if (valid_tile(i + di, j + dj))
                        result.push_back(diagonal_index(i + di, j + dj));
        }
        return result;
    }

    std::vector<int> vertex_cells(int vertex) const
    {
        int const i = vertex % (width + 1);
        int const j = vertex / (width + 1);
        std::vector<int> result;
        for (int dj = -1; dj <= 0; ++dj)
        {
            for (int di = -1; di <= 0; ++di)
            {
                if (! valid_tile(i + di, j + dj)) continue;
                int const upper = dj == -1 ? 1 : 0;
                result.push_back(cell_index(i + di, j + dj, upper));
                if (rising(i, j))
                    result.push_back(cell_index(i + di, j + dj, 1 - upper));
            }
        }
        return result;
    }
};

inline primitive_vectors make_primitives(grid const& g)
{
    std::vector<point> points;
    points.reserve(g.vertex_count());
    for (int vertex = 0; vertex < g.vertex_count(); ++vertex)
    {
        points.push_back(g.vertex_point(vertex % (g.width + 1),
                                        vertex / (g.width + 1)));
    }

    std::vector<linestring> lines;
    lines.reserve(g.edge_count());
    for (int edge = 0; edge < g.edge_count(); ++edge)
    {
        auto const endpoints = g.edge_vertices(edge);
        lines.push_back(linestring{
            points[endpoints.first], points[endpoints.second]});
    }

    std::vector<ring> rings;
    rings.reserve(g.cell_count());
    for (int cell = 0; cell < g.cell_count(); ++cell)
    {
        ring primitive;
        for (int vertex : g.cell_vertices(cell))
            primitive.push_back(points[vertex]);
        primitive.push_back(primitive.front());
        rings.push_back(std::move(primitive));
    }
    return std::make_tuple(std::move(points), std::move(lines), std::move(rings));
}

inline test_point_vectors make_test_points(grid const& g,
                                           primitive_vectors const& primitives)
{
    std::vector<point> edge_points;
    edge_points.reserve(g.edge_count());
    for (linestring const& edge : std::get<1>(primitives))
    {
        edge_points.emplace_back(
            edge.front().x() + (edge.back().x() - edge.front().x()) / scale,
            edge.front().y() + (edge.back().y() - edge.front().y()) / scale);
    }

    std::vector<point> cell_points;
    cell_points.reserve(g.cell_count());
    for (int cell = 0; cell < g.cell_count(); ++cell)
    {
        int const tile = cell / 2;
        cell_points.push_back(g.cell_sample(tile % g.width, tile / g.width, cell % 2));
    }

    return std::make_tuple(std::get<0>(primitives),
                           std::move(edge_points), std::move(cell_points));
}

template <typename MultiGeometry>
inline void connect_members(grid const&, MultiGeometry&)
{}

inline void connect_members(grid const& g, multi_linestring& geometry)
{
    std::vector<std::vector<std::size_t>> incident(g.vertex_count());
    for (std::size_t index = 0; index < geometry.size(); ++index)
    {
        BOOST_GEOMETRY_ASSERT(geometry[index].size() >= 2);
        incident[g.vertex_index(geometry[index].front())].push_back(index);
        incident[g.vertex_index(geometry[index].back())].push_back(index);
    }

    multi_linestring result;
    std::vector<bool> used(geometry.size(), false);
    auto follow = [&](int vertex)
    {
        linestring line;
        while (! incident[vertex].empty())
        {
            std::size_t const index = incident[vertex].back();
            incident[vertex].pop_back();
            if (used[index]) continue;
            used[index] = true;
            linestring const& member = geometry[index];
            if (g.vertex_index(member.front()) == vertex)
            {
                line.insert(line.end(), member.begin() + (line.empty() ? 0 : 1),
                            member.end());
                vertex = g.vertex_index(member.back());
            }
            else
            {
                line.insert(line.end(), member.rbegin() + (line.empty() ? 0 : 1),
                            member.rend());
                vertex = g.vertex_index(member.front());
            }
        }
        if (! line.empty()) result.push_back(std::move(line));
    };

    // Start open trails at odd-degree vertices, then consume any remaining cycles.
    std::vector<int> starts;
    for (int vertex = 0; vertex < g.vertex_count(); ++vertex)
    {
        if (incident[vertex].size() % 2 != 0) starts.push_back(vertex);
    }
    for (int vertex : starts) follow(vertex);
    for (int vertex = 0; vertex < g.vertex_count(); ++vertex) follow(vertex);
    geometry = std::move(result);
}

inline bool removable_vertex(grid const& g, bits const& vertices,
                             point const& previous, point const& current,
                             point const& next)
{
    return ! get(vertices, g.vertex_index(current))
        && bg::covered_by(current, bg::model::segment<point>(previous, next));
}

inline void remove_vertices(grid const& g, bits const& vertices,
                            linestring& line)
{
    for (std::size_t index = 1; index + 1 < line.size();)
    {
        if (removable_vertex(g, vertices,
                             line[index - 1], line[index], line[index + 1]))
        {
            line.erase(line.begin() + index);
        }
        else
        {
            ++index;
        }
    }
}

inline void remove_vertices(grid const& g, bits const& vertices,
                            ring& boundary)
{
    bool const closed = bg::closure<ring>::value == bg::closed;
    std::vector<point> points(boundary.begin(),
        boundary.end() - (closed && ! boundary.empty() ? 1 : 0));
    for (std::size_t index = 0; points.size() > 3 && index < points.size();)
    {
        std::size_t const previous = index == 0 ? points.size() - 1 : index - 1;
        std::size_t const next = index + 1 == points.size() ? 0 : index + 1;
        if (removable_vertex(g, vertices,
                             points[previous], points[index], points[next]))
        {
            points.erase(points.begin() + index);
            if (index == points.size()) index = 0;
        }
        else
        {
            ++index;
        }
    }

    boundary.assign(points.begin(), points.end());
    if (closed && ! boundary.empty()) boundary.push_back(boundary.front());
}

inline void remove_vertices(grid const&, bits const&, multi_point&)
{}

inline void remove_vertices(grid const& g, bits const& vertices,
                            multi_linestring& geometry)
{
    for (linestring& line : geometry) remove_vertices(g, vertices, line);
}

inline void remove_vertices(grid const& g, bits const& vertices,
                            multi_polygon& geometry)
{
    for (polygon& area : geometry)
    {
        remove_vertices(g, vertices, bg::exterior_ring(area));
        for (ring& interior : bg::interior_rings(area))
            remove_vertices(g, vertices, interior);
    }
}

template <typename Range>
inline void append_vertex_path(grid const& g, Range const& range,
                               vertex_paths& paths, bool closed)
{
    paths.emplace_back();
    std::size_t const size = range.size() - (closed && !range.empty() ? 1 : 0);
    for (std::size_t index = 0; index < size; ++index)
        paths.back().push_back(g.vertex_index(range[index]));
}

inline void record_vertex_paths(grid const&, multi_point const&, vertex_paths&)
{}

inline void record_vertex_paths(grid const& g, multi_linestring const& geometry,
                                vertex_paths& paths)
{
    for (auto const& line : geometry) append_vertex_path(g, line, paths, false);
}

inline void record_vertex_paths(grid const& g, multi_polygon const& geometry,
                                vertex_paths& paths)
{
    bool const closed = bg::closure<ring>::value == bg::closed;
    for (auto const& area : geometry)
    {
        append_vertex_path(g, bg::exterior_ring(area), paths, closed);
        for (auto const& hole : bg::interior_rings(area))
            append_vertex_path(g, hole, paths, closed);
    }
}

template <typename MultiGeometry, typename Primitive>
inline MultiGeometry bits_to_geometry(bits const& selected,
                                      bits const& vertices,
                                      grid const& g,
                                      std::vector<Primitive> const& primitives,
                                      vertex_paths* paths = nullptr)
{
    MultiGeometry result;
    for (std::size_t index = 0; index < primitives.size(); ++index)
    {
        if (! get(selected, static_cast<int>(index))) continue;
        MultiGeometry next;
        bg::union_(result, primitives[index], next);
        result = std::move(next);
    }
    connect_members(g, result);
    if (paths)
    {
        paths->clear();
        record_vertex_paths(g, result, *paths);
    }
    remove_vertices(g, vertices, result);
    return result;
}

enum class boolean_operation { union_, intersection, difference, sym_difference };

inline bool apply_boolean(bool lhs, bool rhs, boolean_operation operation)
{
    switch (operation)
    {
    case boolean_operation::union_: return lhs || rhs;
    case boolean_operation::intersection: return lhs && rhs;
    case boolean_operation::difference: return lhs && ! rhs;
    case boolean_operation::sym_difference: return lhs != rhs;
    }
    BOOST_GEOMETRY_ASSERT(false);
    return false;
}

template <typename Geometry>
inline bits geometry_to_bits(Geometry const& geometry,
                             std::vector<point> const& test_points)
{
    bits result = make_bits(static_cast<int>(test_points.size()));
    for (std::size_t index = 0; index < test_points.size(); ++index)
    {
        if (bg::within(test_points[index], geometry))
            set(result, static_cast<int>(index));
    }
    return result;
}

template <typename Geometry>
inline bool validate_geometry(bits const& expected, Geometry const& geometry,
                              std::vector<point> const& test_points,
                              std::string& error)
{
    std::string validity_error;
    if (! bg::is_valid(geometry, validity_error))
    {
        error = "invalid=" + validity_error;
        return false;
    }

    bits const detected = geometry_to_bits(geometry, test_points);
    if (detected == expected) return true;

    std::ostringstream out;
    out << "expected=" << bit_string(expected)
        << " detected=" << bit_string(detected);
    error = out.str();
    return false;
}

inline output_tuple canonical_output(grid const& g, primitive_vectors const& primitives,
                                     test_point_vectors const& test_points,
                                     output_tuple const& output)
{
    bits vertices = geometry_to_bits(std::get<0>(output), std::get<0>(test_points));
    bits edges = geometry_to_bits(std::get<1>(output), std::get<1>(test_points));
    bits const cells = geometry_to_bits(std::get<2>(output), std::get<2>(test_points));
    bits covered_vertices = make_bits(g.vertex_count());
    bits covered_edges = make_bits(g.edge_count());

    for (int cell = 0; cell < g.cell_count(); ++cell)
    {
        if (! get(cells, cell)) continue;
        for (int edge : g.cell_edges(cell)) set(covered_edges, edge);
        for (int vertex : g.cell_vertices(cell)) set(covered_vertices, vertex);
    }
    for (int edge = 0; edge < g.edge_count(); ++edge)
    {
        if (! get(edges, edge)) continue;
        if (get(covered_edges, edge))
        {
            reset(edges, edge);
            continue;
        }
        auto const endpoints = g.edge_vertices(edge);
        set(covered_vertices, endpoints.first);
        set(covered_vertices, endpoints.second);
    }
    for (int vertex = 0; vertex < g.vertex_count(); ++vertex)
    {
        if (get(covered_vertices, vertex)) reset(vertices, vertex);
    }

    return std::make_tuple(bits_to_geometry<multi_point>(
                               vertices, covered_vertices, g, std::get<0>(primitives)),
                           bits_to_geometry<multi_linestring>(
                               edges, covered_vertices, g, std::get<1>(primitives)),
                           std::get<2>(output));
}

inline location point_location(grid const& g, bits const& selected, int vertex)
{
    (void) g;
    return get(selected, vertex) ? location::interior : location::exterior;
}

inline location line_vertex_location(grid const& g, bits const& selected, int vertex)
{
    int degree = 0;
    for (int edge : g.vertex_edges(vertex)) degree += get(selected, edge) ? 1 : 0;
    if (degree == 0) return location::exterior;
    return degree % 2 == 0 ? location::interior : location::boundary;
}

inline location area_edge_location(grid const& g, bits const& selected, int edge)
{
    int adjacent = 0;
    for (int cell : g.edge_cells(edge)) adjacent += get(selected, cell) ? 1 : 0;
    if (adjacent == 0) return location::exterior;
    return adjacent == 2 ? location::interior : location::boundary;
}

inline location area_vertex_location(grid const& g, bits const& selected, int vertex)
{
    bool has_cell = false;
    for (int cell : g.vertex_cells(vertex)) has_cell = has_cell || get(selected, cell);
    if (! has_cell) return location::exterior;
    for (int edge : g.vertex_edges(vertex))
    {
        if (area_edge_location(g, selected, edge) == location::boundary)
            return location::boundary;
    }
    return location::interior;
}

inline location element_location(grid const& g, bits const& selected,
                                  int dimension, int index, bg::pointlike_tag)
{
    return dimension == 0 ? point_location(g, selected, index) : location::exterior;
}

inline location element_location(grid const& g, bits const& selected,
                                  int dimension, int index, bg::linear_tag)
{
    if (dimension == 1)
        return get(selected, index) ? location::interior : location::exterior;
    if (dimension == 0) return line_vertex_location(g, selected, index);
    return location::exterior;
}

inline location element_location(grid const& g, bits const& selected,
                                  int dimension, int index, bg::areal_tag)
{
    if (dimension == 2)
        return get(selected, index) ? location::interior : location::exterior;
    if (dimension == 1) return area_edge_location(g, selected, index);
    return area_vertex_location(g, selected, index);
}

template <typename Tag>
inline location element_location(grid const& g, bits const& selected,
                                  int dimension, int index)
{
    return element_location(g, selected, dimension, index, Tag{});
}

template <typename LhsTag, typename RhsTag>
inline output_tuple expected_operation(grid const& g, bits const& lhs, bits const& rhs,
                                       primitive_vectors const& primitives,
                                       boolean_operation operation)
{
    bits vertices = make_bits(g.vertex_count());
    bits edges = make_bits(g.edge_count());
    bits cells = make_bits(g.cell_count());
    bits covered_vertices = make_bits(g.vertex_count());
    bits covered_edges = make_bits(g.edge_count());

    for (int cell = 0; cell < g.cell_count(); ++cell)
    {
        bool const lhs_member
            = element_location<LhsTag>(g, lhs, 2, cell) != location::exterior;
        bool const rhs_member
            = element_location<RhsTag>(g, rhs, 2, cell) != location::exterior;
        if (! apply_boolean(lhs_member, rhs_member, operation)) continue;
        set(cells, cell);
        for (int edge : g.cell_edges(cell)) set(covered_edges, edge);
        for (int vertex : g.cell_vertices(cell)) set(covered_vertices, vertex);
    }
    for (int edge = 0; edge < g.edge_count(); ++edge)
    {
        bool const lhs_member
            = element_location<LhsTag>(g, lhs, 1, edge) != location::exterior;
        bool const rhs_member
            = element_location<RhsTag>(g, rhs, 1, edge) != location::exterior;
        if (! apply_boolean(lhs_member, rhs_member, operation)
            || get(covered_edges, edge))
        {
            continue;
        }
        set(edges, edge);
        auto const endpoints = g.edge_vertices(edge);
        set(covered_vertices, endpoints.first);
        set(covered_vertices, endpoints.second);
    }
    for (int vertex = 0; vertex < g.vertex_count(); ++vertex)
    {
        bool const lhs_member
            = element_location<LhsTag>(g, lhs, 0, vertex) != location::exterior;
        bool const rhs_member
            = element_location<RhsTag>(g, rhs, 0, vertex) != location::exterior;
        if (apply_boolean(lhs_member, rhs_member, operation)
            && ! get(covered_vertices, vertex))
        {
            set(vertices, vertex);
        }
    }

    return std::make_tuple(bits_to_geometry<multi_point>(
                               vertices, covered_vertices, g, std::get<0>(primitives)),
                           bits_to_geometry<multi_linestring>(
                               edges, covered_vertices, g, std::get<1>(primitives)),
                           bits_to_geometry<multi_polygon>(
                               cells, covered_vertices, g, std::get<2>(primitives)));
}

inline char dimension_char(int dimension)
{
    return dimension < 0 ? 'F' : static_cast<char>('0' + dimension);
}

template <typename LhsTag, typename RhsTag>
inline std::string expected_relation(grid const& g, bits const& lhs, bits const& rhs)
{
    int matrix[3][3] = {{-1, -1, -1}, {-1, -1, -1}, {-1, -1, -1}};
    auto update = [&](location l, location r, int dimension)
    {
        int& value = matrix[static_cast<int>(l)][static_cast<int>(r)];
        value = (std::max)(value, dimension);
    };
    update(location::exterior, location::exterior, 2);
    for (int cell = 0; cell < g.cell_count(); ++cell)
        update(element_location<LhsTag>(g, lhs, 2, cell),
               element_location<RhsTag>(g, rhs, 2, cell), 2);
    for (int edge = 0; edge < g.edge_count(); ++edge)
        update(element_location<LhsTag>(g, lhs, 1, edge),
               element_location<RhsTag>(g, rhs, 1, edge), 1);
    for (int vertex = 0; vertex < g.vertex_count(); ++vertex)
        update(element_location<LhsTag>(g, lhs, 0, vertex),
               element_location<RhsTag>(g, rhs, 0, vertex), 0);

    std::string result;
    for (auto const& row : matrix)
        for (int value : row) result.push_back(dimension_char(value));
    return result;
}

} // namespace random_grid

#endif
