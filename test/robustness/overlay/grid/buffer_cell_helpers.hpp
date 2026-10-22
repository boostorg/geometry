// Boost.Geometry (aka GGL, Generic Geometry Library)
// Copyright (c) 2026 Tinko Bartels, Shenzhen, China.
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_BUFFER_CELL_HELPERS_HPP
#define BOOST_GEOMETRY_TEST_BUFFER_CELL_HELPERS_HPP

#include "random_grid.hpp"

namespace random_grid
{

constexpr int buffer_distance = 24 * scale;

namespace buffer_helper_data
{

// Fixed distance 120. Row bit 2*(di+4)+upper denotes cell (di,dj,upper)
// relative to an even-parity vertex. Reflections cover the other orientations.
// The six corner tables are N->NW, N->SW, NW->W, NE->W, NE->NW,
// NW->SW, followed by the rising edge's right-side strip. None of their
// boundaries contains a cell sample at this distance, so ordinary OR suffices.
struct row
{
    int dj;
    std::uint64_t cells;
};

static constexpr row rows[] = {
    // Corner 1.
    {0, 0xffd00ULL}, {1, 0xffc00ULL}, {2, 0xff800ULL},
    {3, 0xff000ULL}, {4, 0x7d000ULL}, {5, 0x1c000ULL},
    // Corner 2.
    {0, 0xfff80ULL}, {1, 0xfffc0ULL}, {2, 0xfffd0ULL},
    {3, 0xffff0ULL}, {4, 0xffff8ULL}, {5, 0xffffcULL},
    {6, 0xffffdULL}, {7, 0xffff4ULL}, {8, 0xfffd0ULL},
    {9, 0xfff40ULL}, {10, 0xffd00ULL}, {11, 0xff400ULL},
    {12, 0xfd000ULL}, {13, 0xf4000ULL}, {14, 0xd0000ULL},
    {15, 0x40000ULL},
    // Corner 3.
    {0, 0x200ULL}, {1, 0x300ULL}, {2, 0x700ULL},
    {3, 0xf00ULL}, {4, 0x2f00ULL}, {5, 0x3f00ULL},
    {6, 0x7f00ULL}, {7, 0x1f00ULL},
    // Corner 4.
    {-6, 0x2c000ULL}, {-5, 0xbe000ULL}, {-4, 0x2ff000ULL},
    {-3, 0xbff400ULL}, {-2, 0x2fffc00ULL}, {-1, 0xbfffe00ULL},
    {0, 0x2fffff00ULL}, {1, 0xbfffff00ULL}, {2, 0x2ffffff00ULL},
    {3, 0xbffffff00ULL}, {4, 0x2fffffff00ULL}, {5, 0xbfffffff00ULL},
    {6, 0x2ffffffff00ULL}, {7, 0xbffffffff00ULL},
    // Corner 5.
    {-6, 0x2c000ULL}, {-5, 0xbe000ULL}, {-4, 0x2ff000ULL},
    {-3, 0xbff400ULL}, {-2, 0x2fffc00ULL}, {-1, 0xbfffe00ULL},
    {0, 0x7fffd00ULL}, {1, 0x1fffc00ULL}, {2, 0x7ff800ULL},
    {3, 0x1ff000ULL}, {4, 0x7d000ULL}, {5, 0x1c000ULL},
    // Corner 6.
    {0, 0x280ULL}, {1, 0x3c0ULL}, {2, 0x7d0ULL},
    {3, 0xff0ULL}, {4, 0x2ff8ULL}, {5, 0x3ffcULL},
    {6, 0x7ffdULL}, {7, 0x1ff4ULL}, {8, 0x7d0ULL},
    {9, 0x140ULL},
    // Diagonal side.
    {-6, 0x2c000ULL}, {-5, 0xe000ULL}, {-4, 0x7000ULL},
    {-3, 0x3400ULL}, {-2, 0x2c00ULL}, {-1, 0xe00ULL},
    {0, 0x500ULL},
};

static constexpr int row_ranges[][2] = {
    {0, 6}, {6, 22}, {22, 30}, {30, 44}, {44, 56}, {56, 66}, {66, 73}
};

// Directions: E, NE, N, NW, W, SW, S, SE. Each entry selects
// 4*corner + reflections (bit 0: horizontal, bit 1: vertical).
// Corner 0 is a 6-by-8 rectangle; corners 1..6 use the row tables.
// Equal and opposite directions have no corner.
static constexpr int corners[8][8] = {
    {-1, 14, 2, 18, -1, 19, 3, 15},
    {14, -1, 6, 22, 16, -1, 11, 27},
    {2, 6, -1, 4, 0, 8, -1, 10},
    {18, 22, 4, -1, 12, 25, 9, -1},
    {-1, 16, 0, 12, -1, 13, 1, 17},
    {19, -1, 8, 25, 13, -1, 5, 23},
    {3, 11, -1, 9, 1, 5, -1, 7},
    {15, 27, 10, -1, 17, 23, 7, -1},
};

} // namespace buffer_helper_data

inline int grid_direction(grid const& g, int from, int to)
{
    int const di = to % (g.width + 1) - from % (g.width + 1);
    int const dj = to / (g.width + 1) - from / (g.width + 1);
    static constexpr int directions[3][3] = {{5, 6, 7}, {4, -1, 0}, {3, 2, 1}};
    BOOST_GEOMETRY_ASSERT(di != 0 || dj != 0);
    return directions[1 + (dj > 0) - (dj < 0)][1 + (di > 0) - (di < 0)];
}

struct buffer_cell_helpers
{
    grid const& g;
    bits cells;

    explicit buffer_cell_helpers(grid const& grid)
        : g(grid), cells(make_bits(g.cell_count()))
    {}

    void rectangle(int min_i, int max_i, int min_j, int max_j)
    {
        for (int j = (std::max)(min_j, 0); j < (std::min)(max_j, g.height); ++j)
            for (int i = (std::max)(min_i, 0); i < (std::min)(max_i, g.width); ++i)
                for (int upper = 0; upper < 2; ++upper)
                    random_grid::set(cells, g.cell_index(i, j, upper));
    }

    void set(int vertex, int helper, int reflections)
    {
        int const i = vertex % (g.width + 1), j = vertex / (g.width + 1);
        BOOST_GEOMETRY_ASSERT(g.rising(i, j));
        auto const& range = buffer_helper_data::row_ranges[helper];
        for (int r = range[0]; r < range[1]; ++r)
        {
            auto mask = buffer_helper_data::rows[r].cells;
            for (int bit = 0; mask != 0; ++bit, mask >>= 1)
            {
                if (!(mask & 1)) continue;
                int di = bit / 2 - 4, dj = buffer_helper_data::rows[r].dj;
                int upper = bit % 2;
                if (reflections & 1) di = -di - 1;
                if (reflections & 2) { dj = -dj - 1; upper = 1 - upper; }
                if (g.valid_tile(i + di, j + dj))
                    random_grid::set(cells, g.cell_index(i + di, j + dj, upper));
            }
        }
    }

    void side(int vertex, int direction)
    {
        int const i = vertex % (g.width + 1), j = vertex / (g.width + 1);
        switch (direction)
        {
        case 0: rectangle(i, i + 1, j - 8, j); break;
        case 2: rectangle(i, i + 6, j, j + 1); break;
        case 4: rectangle(i - 1, i, j, j + 8); break;
        case 6: rectangle(i - 6, i, j - 1, j); break;
        case 1: set(vertex, 6, 0); break;
        case 3: set(g.vertex_index(i - 1, j + 1), 6, 2); break;
        case 5: set(vertex, 6, 3); break;
        case 7: set(g.vertex_index(i + 1, j - 1), 6, 1); break;
        default: BOOST_GEOMETRY_ASSERT(false);
        }
    }

    void corner(int vertex, int incoming, int outgoing, bool linear)
    {
        int const turn = (outgoing - incoming + 8) % 8;
        if (turn == 0 || turn == 4 || (!linear && turn > 4)) return;
        if (turn > 4)
        {
            incoming = (incoming + 4) % 8;
            outgoing = (outgoing + 4) % 8;
        }
        int const code = buffer_helper_data::corners[incoming][outgoing];
        BOOST_GEOMETRY_ASSERT(code >= 0);
        int const helper = code / 4, reflections = code % 4;
        if (helper == 0)
        {
            int const i = vertex % (g.width + 1), j = vertex / (g.width + 1);
            int const di = reflections & 1 ? -6 : 0;
            int const dj = reflections & 2 ? -8 : 0;
            rectangle(i + di, i + di + 6, j + dj, j + dj + 8);
        }
        else set(vertex, helper - 1, reflections);
    }
};

inline bits buffer_cells(grid const& g, bits const& selected, bg::multi_point_tag,
                         vertex_paths const& = vertex_paths())
{
    buffer_cell_helpers result(g);
    for (int vertex = 0; vertex < g.vertex_count(); ++vertex)
    {
        if (!get(selected, vertex)) continue;
        int const i = vertex % (g.width + 1), j = vertex / (g.width + 1);
        result.rectangle(i - 6, i + 6, j - 8, j + 8);
    }
    return std::move(result.cells);
}

inline bits buffer_cells(grid const& g, bits const& selected,
                         bg::multi_linestring_tag, vertex_paths const& paths)
{
    buffer_cell_helpers result(g);
    for (int edge = 0; edge < g.edge_count(); ++edge)
    {
        if (!get(selected, edge)) continue;
        BOOST_GEOMETRY_ASSERT(g.family(edge) != edge_family::diagonal);
        auto const ends = g.edge_vertices(edge);
        int const direction = grid_direction(g, ends.first, ends.second);
        result.side(ends.first, direction);
        result.side(ends.second, (direction + 4) % 8);
    }
    for (auto const& path : paths)
        for (std::size_t k = 1; k + 1 < path.size(); ++k)
            result.corner(path[k], grid_direction(g, path[k - 1], path[k]),
                          grid_direction(g, path[k], path[k + 1]), true);
    return std::move(result.cells);
}

inline bits buffer_cells(grid const& g, bits const& selected,
                         bg::multi_polygon_tag, vertex_paths const& paths)
{
    buffer_cell_helpers result(g);
    result.cells = selected;
    for (int cell = 0; cell < g.cell_count(); ++cell)
    {
        if (!get(selected, cell)) continue;
        auto const edges = g.cell_edges(cell);
        auto const vertices = g.cell_vertices(cell);
        for (int k = 0; k < 3; ++k)
        {
            bool boundary = true;
            for (int neighbor : g.edge_cells(edges[k]))
                if (neighbor != cell && get(selected, neighbor)) boundary = false;
            if (!boundary) continue;
            int const from = vertices[k], to = vertices[(k + 1) % 3];
            int const direction = grid_direction(g, from, to);
            result.side(from, direction);
        }
    }
    for (auto const& path : paths)
    {
        for (std::size_t k = 0; k < path.size(); ++k)
        {
            int const previous = path[k == 0 ? path.size() - 1 : k - 1];
            int const next = path[(k + 1) % path.size()];
            result.corner(path[k], grid_direction(g, previous, path[k]),
                          grid_direction(g, path[k], next), false);
        }
    }
    return std::move(result.cells);
}

} // namespace random_grid

#endif
