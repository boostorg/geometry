// Boost.Geometry
// Copyright (c) 2026
// Distributed under the Boost Software License, Version 1.0.

#ifndef BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_RADICAL_BUFFER_NODING_HPP
#define BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_RADICAL_BUFFER_NODING_HPP

#include <stdexcept>
#include <utility>

#include <boost/geometry/strategies/cartesian/exact/bounded_radical_field.hpp>
#include <boost/geometry/strategies/cartesian/exact/round_buffer.hpp>

namespace boost { namespace geometry { namespace strategies { namespace detail
{ namespace exact_cartesian { namespace round_buffer { namespace radical_noding
{

using field = bounded_radical_field;
using value = field::value;

struct point { value x, y; };
struct segment { point from, to; };

inline point lift(field const& f, rational_xy const& p)
{
    return {f.rational(p.x), f.rational(p.y)};
}

inline bool lift_offset(field& f, straight_offset const& support, segment& out)
{
    auto const a = support.start(), b = support.end();
    auto const& d = support.normal().squared_length();
    auto const& n = support.normal().numerator();
    value root;
    if (a.radius != 0 && !f.try_sqrt(d, root)) { return false; }
    auto const x = f.scale(root, a.radius * n.x / d);
    auto const y = f.scale(root, a.radius * n.y / d);
    out = {{f.add(f.rational(a.base.x), x), f.add(f.rational(a.base.y), y)},
           {f.add(f.rational(b.base.x), x), f.add(f.rational(b.base.y), y)}};
    return true;
}

inline point subtract(field const& f, point const& a, point const& b)
{
    return {f.subtract(a.x, b.x), f.subtract(a.y, b.y)};
}

inline value cross(field const& f, point const& a, point const& b)
{
    return f.subtract(f.multiply(a.x, b.y), f.multiply(a.y, b.x));
}

// Denominators are kept positive; equality and ordering do not require
// division in the field or conversion to an approximate coordinate.
struct parameter { value n, d; };

inline parameter ratio(field const& f, value const& n, value const& d)
{
    int const sign = f.sign(d);
    if (sign == 0) { throw std::domain_error("Zero radical noding denominator"); }
    return sign > 0 ? parameter{n, d} : parameter{f.scale(n, -1), f.scale(d, -1)};
}

inline int compare(field const& f, parameter const& a, parameter const& b)
{
    return f.sign(f.subtract(f.multiply(a.n, b.d), f.multiply(b.n, a.d)));
}

inline bool in_unit_interval(field const& f, parameter const& p)
{
    return f.sign(p.n) >= 0 && f.sign(f.subtract(p.n, p.d)) <= 0;
}

struct node { parameter x, y; };

inline node at(field const& f, segment const& s, parameter const& t)
{
    auto const v = subtract(f, s.to, s.from);
    return {{f.add(f.multiply(s.from.x, t.d), f.multiply(v.x, t.n)), t.d},
            {f.add(f.multiply(s.from.y, t.d), f.multiply(v.y, t.n)), t.d}};
}

enum class intersection_kind { disjoint, contact, overlap, degenerate };

struct intersection
{
    intersection_kind kind = intersection_kind::disjoint;
    parameter t0, t1, u0, u1;
};

// Covers a pair of nonzero closed segments. The overlap ends are ordered
// on a, with the corresponding (possibly reversed) parameters on b.
inline intersection intersect(field const& f, segment const& a, segment const& b)
{
    auto const v = subtract(f, a.to, a.from), w = subtract(f, b.to, b.from);
    bool const use_x = f.sign(v.x) != 0;
    if ((!use_x && f.sign(v.y) == 0) || (f.sign(w.x) == 0 && f.sign(w.y) == 0))
    {
        return {intersection_kind::degenerate, {}, {}, {}, {}};
    }
    auto const shift = subtract(f, b.from, a.from);
    auto const det = cross(f, v, w);
    if (f.sign(det) != 0)
    {
        auto const t = ratio(f, cross(f, shift, w), det);
        auto const u = ratio(f, cross(f, shift, v), det);
        if (!in_unit_interval(f, t) || !in_unit_interval(f, u)) { return {}; }
        return {intersection_kind::contact, t, t, u, u};
    }
    if (f.sign(cross(f, shift, v)) != 0) { return {}; }

    auto const axis = [&](point const& p) -> value const& { return use_x ? p.x : p.y; };
    auto const lo = parameter{f.rational(0), f.rational(1)};
    auto const hi = parameter{f.rational(1), f.rational(1)};
    auto t0 = ratio(f, axis(shift), axis(v));
    auto t1 = ratio(f, f.add(axis(shift), axis(w)), axis(v));
    if (compare(f, t0, t1) > 0) { std::swap(t0, t1); }
    if (compare(f, t0, lo) < 0) { t0 = lo; }
    if (compare(f, t1, hi) > 0) { t1 = hi; }
    int const order = compare(f, t0, t1);
    if (order > 0) { return {}; }
    auto const across = [&](parameter const& t)
    {
        return ratio(f, f.subtract(f.multiply(axis(v), t.n), f.multiply(axis(shift), t.d)),
                        f.multiply(axis(w), t.d));
    };
    return {order == 0 ? intersection_kind::contact : intersection_kind::overlap,
            t0, t1, across(t0), across(t1)};
}

}}}}}}} // namespace boost::geometry::strategies::detail::exact_cartesian::round_buffer::radical_noding

#endif // BOOST_GEOMETRY_STRATEGIES_CARTESIAN_EXACT_RADICAL_BUFFER_NODING_HPP
