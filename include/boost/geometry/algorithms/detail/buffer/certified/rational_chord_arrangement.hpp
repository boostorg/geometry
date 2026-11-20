// Boost.Geometry
// Copyright (c) 2026
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_RATIONAL_CHORD_ARRANGEMENT_HPP
#define BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_RATIONAL_CHORD_ARRANGEMENT_HPP

#include <algorithm>
#include <cstddef>
#include <set>
#include <utility>
#include <vector>

#include <boost/geometry/algorithms/detail/buffer/certified/linestring.hpp>

namespace boost { namespace geometry { namespace detail { namespace rational_chord_arrangement
{

namespace line = certified_linestring_buffer;
using line::cross;
using line::difference;
using line::dot;
using line::exact_number;
using line::piece;
using line::rational_xy;
using line::same_point;

struct bounds
{
    rational_xy low, high;

    void extend(rational_xy const& p)
    {
        if (p.x < low.x) { low.x = p.x; }
        if (p.y < low.y) { low.y = p.y; }
        if (p.x > high.x) { high.x = p.x; }
        if (p.y > high.y) { high.y = p.y; }
    }

    bool contains(rational_xy const& p) const
    {
        return low.x <= p.x && p.x <= high.x && low.y <= p.y && p.y <= high.y;
    }

    bool overlaps(bounds const& b) const
    {
        return low.x <= b.high.x && b.low.x <= high.x && low.y <= b.high.y && b.low.y <= high.y;
    }
};

struct edge
{
    edge(rational_xy const& a, rational_xy const& b)
        : from(a), to(b), direction(difference(b, a)), box{a, a}
    { box.extend(b); }

    rational_xy at(exact_number const& t) const
    { return {from.x + t * direction.x, from.y + t * direction.y}; }

    // p is on the supporting line and this is a nonzero edge.
    exact_number parameter(rational_xy const& p) const
    {
        return direction.x != 0 ? exact_number((p.x - from.x) / direction.x)
                                : exact_number((p.y - from.y) / direction.y);
    }

    rational_xy from, to, direction;
    bounds box;
};

struct ring
{
    explicit ring(piece vertices)
        : points(std::move(vertices)), box{points.front(), points.front()}
    {
        auto previous = points.end() - 1;
        for (auto current = points.begin(); current != points.end(); previous = current++)
        {
            box.extend(*current);
            area2 += cross(*previous, *current);
            edges.emplace_back(*previous, *current);
        }
    }

    piece points;
    bounds box;
    exact_number area2 = 0;
    std::vector<edge> edges;
};

struct intersection
{
    // Parameter pairs at each end of the common part. Equal ends mean a
    // singleton contact; u0,u1 need not be in increasing order.
    exact_number t0, t1, u0, u1;
};

inline bool intersect(edge const& a, edge const& b, intersection& result)
{
    if (!a.box.overlaps(b.box)) { return false; }
    auto const offset = difference(b.from, a.from);
    exact_number const det = cross(a.direction, b.direction);
    if (det != 0)
    {
        exact_number const t = cross(offset, b.direction) / det;
        exact_number const u = cross(offset, a.direction) / det;
        if (t < 0 || t > 1 || u < 0 || u > 1) { return false; }
        result = {t, t, u, u};
        return true;
    }
    if (cross(offset, a.direction) != 0) { return false; }
    exact_number const t = a.parameter(b.from);
    exact_number const u = a.parameter(b.to);
    exact_number const lo = (std::max)(exact_number(0), (std::min)(t, u));
    exact_number const hi = (std::min)(exact_number(1), (std::max)(t, u));
    if (lo > hi) { return false; }
    result = {lo, hi, b.parameter(a.at(lo)), b.parameter(a.at(hi))};
    return true;
}

// -1 outside, 0 on the boundary, +1 inside the bounded side of this ring.
inline int locate(ring const& r, rational_xy const& p, edge const** facet = nullptr)
{
    if (!r.box.contains(p)) { return -1; }
    bool inside = false;
    for (auto const& e : r.edges)
    {
        bool const boundary_candidate = e.box.contains(p);
        bool const straddles_ray = (e.from.y > p.y) != (e.to.y > p.y);
        if (!boundary_candidate && !straddles_ray) { continue; }
        exact_number const side = cross(e.direction, difference(p, e.from));
        if (side == 0 && boundary_candidate)
        {
            if (facet != nullptr) { *facet = &e; }
            return 0;
        }
        if (straddles_ray && (side > 0) == (e.direction.y > 0)) { inside = !inside; }
    }
    return inside ? 1 : -1;
}

struct sides { bool left, right; };

// The query is in the open interior of a noded atom. Any incident polygon
// boundary therefore runs along the atom, not across it or into a vertex.
inline sides incident(ring const& r, rational_xy const& p, rational_xy const& direction)
{
    edge const* facet = nullptr;
    int const where = locate(r, p, &facet);
    if (where != 0) { return {where > 0, where > 0}; }
    bool const bounded_left = (r.area2 > 0) == (dot(facet->direction, direction) > 0);
    return {bounded_left, !bounded_left};
}

struct atom
{
    rational_xy from, to;
};

struct less_atom
{
    bool operator()(atom const& a, atom const& b) const
    {
        line::less_xy const less;
        if (!same_point(a.from, b.from)) { return less(a.from, b.from); }
        return less(a.to, b.to);
    }
};

constexpr std::size_t max_atoms = 4096;
constexpr std::size_t max_pair_cuts = 32768;

inline bool node(std::vector<edge const*> const& edges, std::set<atom, less_atom>& atoms)
{
    std::vector<std::vector<exact_number>> cuts(edges.size(), {0, 1});
    std::size_t pair_cuts = 0;
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        for (std::size_t j = i + 1; j < edges.size(); ++j)
        {
            intersection x;
            if (!intersect(*edges[i], *edges[j], x)) { continue; }
            cuts[i].push_back(x.t0);
            cuts[j].push_back(x.u0);
            pair_cuts += 2;
            if (x.t0 != x.t1)
            {
                cuts[i].push_back(x.t1);
                cuts[j].push_back(x.u1);
                pair_cuts += 2;
            }
            if (pair_cuts > max_pair_cuts) { return false; }
        }
    }
    line::less_xy const less;
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        auto& c = cuts[i];
        std::sort(c.begin(), c.end());
        c.erase(std::unique(c.begin(), c.end()), c.end());
        auto previous = edges[i]->at(c.front());
        for (auto t = c.begin() + 1; t != c.end(); ++t)
        {
            auto current = edges[i]->at(*t);
            atoms.insert(less(previous, current) ? atom{previous, current} : atom{current, previous});
            if (atoms.size() > max_atoms) { return false; }
            previous = std::move(current);
        }
    }
    return true;
}

template <typename Select>
inline bool assemble(std::set<atom, less_atom> const& atoms, Select const& select, std::vector<ring>& cycles)
{
    line::boundary_edges links;
    std::set<rational_xy, line::less_xy> incoming;
    for (auto const& a : atoms)
    {
        rational_xy const mid{(a.from.x + a.to.x) / 2, (a.from.y + a.to.y) / 2};
        sides const s = select(mid, difference(a.to, a.from));
        if (s.left == s.right) { continue; }
        auto const& from = s.left ? a.from : a.to;
        auto const& to = s.left ? a.to : a.from;
        if (!links.emplace(from, to).second || !incoming.insert(to).second)
        {
            return false; // No point-touching or branching output rings.
        }
    }
    while (!links.empty())
    {
        auto first = links.begin()->first;
        auto current = first;
        piece vertices;
        do
        {
            auto const it = links.find(current);
            if (it == links.end()) { return false; }
            vertices.push_back(std::move(current));
            current = it->second;
            links.erase(it);
        }
        while (!same_point(first, current));

        if (vertices.size() < 3) { return false; }
        cycles.emplace_back(std::move(vertices));
        if (cycles.back().area2 == 0) { return false; }
    }
    return true;
}

}}}} // namespace boost::geometry::detail::rational_chord_arrangement

#endif // BOOST_GEOMETRY_ALGORITHMS_DETAIL_BUFFER_CERTIFIED_RATIONAL_CHORD_ARRANGEMENT_HPP
