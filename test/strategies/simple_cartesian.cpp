// Boost.Geometry
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#include <boost/core/lightweight_test.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/strategies/relate/simple_cartesian.hpp>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <type_traits>
#include <vector>

namespace bg = boost::geometry;
using point = bg::model::d2::point_xy<double>;
using polygon = bg::model::polygon<point>;
using multi = bg::model::multi_polygon<polygon>;
using line = bg::model::linestring<point>;
using segment = bg::model::segment<point>;
using box = bg::model::box<point>;
using strategy = bg::strategies::relate::simple_cartesian;
namespace services = bg::strategies::relate::services;
static_assert(!services::exact_predicates<strategy>::value, "No certified predicates");
static_assert(std::is_same<services::operation_scope<strategy>::type,
              services::empty_operation_scope>::value, "Use the default empty scope");

void check_set(multi const& actual, char const* wkt)
{
    multi expected;
    bg::read_wkt(wkt, expected);
    BOOST_TEST(bg::is_valid(actual));
    BOOST_TEST(actual.empty() || expected.empty() ? actual.empty() && expected.empty()
        : bg::relation(actual, expected).str() == "2FFF1FFF2");
}

void overlays()
{
    polygon a, b;
    bg::read_wkt("POLYGON((0 0,0 2,2 2,2 0,0 0))", a);
    bg::read_wkt("POLYGON((1 1,1 3,3 3,3 1,1 1))", b);
    strategy s;
    multi u, i, d, reverse, x;
    bg::union_(a, b, u, s); bg::intersection(a, b, i, s);
    bg::difference(a, b, d, s); bg::difference(b, a, reverse, s);
    bg::sym_difference(a, b, x, s);
    check_set(u, "MULTIPOLYGON(((0 0,0 2,1 2,1 3,3 3,3 1,2 1,2 0,0 0)))");
    check_set(i, "MULTIPOLYGON(((1 1,1 2,2 2,2 1,1 1)))");
    check_set(d, "MULTIPOLYGON(((0 0,0 2,1 2,1 1,2 1,2 0,0 0)))");
    check_set(reverse, "MULTIPOLYGON(((1 2,1 3,3 3,3 1,2 1,2 2,1 2)))");
    check_set(x, "MULTIPOLYGON(((0 0,0 2,1 2,1 1,2 1,2 0,0 0)),((1 2,1 3,3 3,3 1,2 1,2 2,1 2)))");
    BOOST_TEST_EQ(bg::relation(a, b, s).str(), "212101212");
    BOOST_TEST(bg::overlaps(a, b, s));
    BOOST_TEST(!bg::touches(a, b, s));
    d.clear(); bg::difference(a, a, d, s); check_set(d, "MULTIPOLYGON EMPTY");
    bg::model::polygon<point, false, false> open_a, open_b;
    bg::convert(a, open_a); bg::convert(b, open_b);
    multi opened;
    bg::intersection(open_a, open_b, opened, s);
    BOOST_TEST_EQ(bg::relation(opened, i).str(), "2FFF1FFF2");
}

void strict_predicates()
{
    strategy s;
    double const next = std::nextafter(1.0, 2.0);
    point const a(1, 1), b(next, 1), c(1, next);
    BOOST_TEST(!s.relate(a, b).apply(a, b));
    BOOST_TEST(!s.relate(a, c).apply(a, c));
    point const zero(0, -0.0), other_zero(-0.0, 0);
    BOOST_TEST(s.relate(zero, other_zero).apply(zero, other_zero));
    using less = strategy::compare_type<bg::strategy::compare::less, bg::strategy::compare::equals_epsilon>;
    BOOST_TEST(less::apply(point(1, 2), point(next, 0)));
    BOOST_TEST(less::apply(a, c)); BOOST_TEST(!less::apply(c, a));
    BOOST_TEST_EQ(s.side().apply(point(0, 1), point(2, 1), c), 1);
    BOOST_TEST_EQ(s.side().apply(point(2, 1), point(0, 1), c), -1);
    // Deliberately demonstrate ordinary-double cancellation, not robustness.
    std::int64_t const n = std::int64_t(1) << 27;
    BOOST_TEST_EQ(n * (n - 2) - (n - 1) * (n - 1), -1);
    BOOST_TEST_EQ(s.side().apply(point(0, 0), point(double(n), double(n - 1)),
                                point(double(n - 1), double(n - 2))), 0);
}

void segments()
{
    strategy s;
    auto less = [](point const& a, point const& b)
    { return bg::get<0>(a) < bg::get<0>(b) || (bg::get<0>(a) == bg::get<0>(b) && bg::get<1>(a) < bg::get<1>(b)); };
    auto check = [&](segment const& first, segment const& second, std::vector<point> expected)
    {
        std::sort(expected.begin(), expected.end(), less);
        for (unsigned mask = 0; mask != 8; ++mask)
        {
            segment a = first, b = second;
            if (mask & 1) std::swap(a.first, a.second);
            if (mask & 2) std::swap(b.first, b.second);
            if (mask & 4) std::swap(a, b);
            std::vector<point> actual;
            bg::intersection(a, b, actual, s);
            BOOST_TEST_EQ(bg::disjoint(a, b, s), expected.empty());
            BOOST_TEST_EQ(actual.size(), expected.size());
            std::sort(actual.begin(), actual.end(), less);
            for (std::size_t j = 0; j < actual.size() && j < expected.size(); ++j)
            { BOOST_TEST_EQ(bg::get<0>(actual[j]), bg::get<0>(expected[j])); BOOST_TEST_EQ(bg::get<1>(actual[j]), bg::get<1>(expected[j])); }
        }
    };
    check(segment(point(0, 0), point(4, 4)), segment(point(0, 4), point(4, 0)), {{2, 2}});
    segment const span(point(0, 0), point(4, 0));
    check(span, segment(point(2, 0), point(6, 0)), {{2, 0}, {4, 0}});
    check(span, segment(point(4, 0), point(5, 1)), {{4, 0}});
    check(span, segment(point(5, 0), point(7, 0)), {});
    double const positions[] = {-1, 0, 2, 4, 5};
    for (unsigned j = 0; j != 5; ++j)
    {
        point const p(positions[j], 0);
        BOOST_TEST_EQ(s.side().apply(span.first, span.second, p), 0);
        BOOST_TEST_EQ(s.segment_position().classify_collinear(span.first, span.second, p), int(j));
        BOOST_TEST_EQ(s.segment_position().classify_collinear(span.second, span.first, p), 4 - int(j));
        check(span, segment(p, p), j >= 1 && j <= 3 ? std::vector<point>{p} : std::vector<point>{});
        BOOST_TEST_EQ(bg::within(p, span, s), j == 2);
        BOOST_TEST_EQ(bg::covered_by(p, span, s), j >= 1 && j <= 3);
    }
}

void queries_and_boxes()
{
    strategy s;
    polygon p; bg::read_wkt("POLYGON((0 0,0 2,2 2,2 0,0 0))", p);
    point const boundary(0, 1), inside(1, 1), outside(std::nextafter(2.0, 3.0), 1);
    BOOST_TEST(!bg::within(boundary, p, s)); BOOST_TEST(bg::covered_by(boundary, p, s));
    BOOST_TEST(bg::within(inside, p, s)); BOOST_TEST(!bg::covered_by(outside, p, s));
    line const a{{-1, 1}, {3, 1}}, b{{1, -1}, {1, 3}};
    BOOST_TEST_EQ(bg::relation(a, b, s).str(), "0F1FF0102");
    BOOST_TEST_EQ(bg::relation(a, p, s).str(), "101FF0212");
    BOOST_TEST_EQ(bg::relation(p, a, s).str(), bg::relation(p, a).str());
    BOOST_TEST(bg::crosses(a, b, s)); BOOST_TEST(bg::crosses(a, p, s));
    box const bounds(point(0, 0), point(2, 2));
    BOOST_TEST(!bg::within(boundary, bounds, s)); BOOST_TEST(bg::covered_by(boundary, bounds, s));
    BOOST_TEST(!bg::covered_by(outside, bounds, s));
    BOOST_TEST(!bg::disjoint(segment(a.front(), a.back()), bounds, s));
    BOOST_TEST(bg::disjoint(segment(point(-1, 3), point(3, 3)), bounds, s));
    BOOST_TEST(bg::touches(bounds, box(point(2, 0), point(4, 2)), s));
    BOOST_TEST(bg::overlaps(bounds, box(point(1, 1), point(3, 3)), s));
    for (bool use_box : {false, true})
    {
        bg::model::multi_linestring<line> actual, expected;
        if (use_box) bg::intersection(a, bounds, actual, s); else bg::intersection(a, p, actual, s);
        bg::read_wkt("MULTILINESTRING((0 1,2 1))", expected);
        BOOST_TEST_EQ(bg::relation(actual, expected).str(), "1FFF0FFF2");
    }
    box envelope; bg::envelope(segment(a.front(), a.back()), envelope, s);
    BOOST_TEST_EQ((bg::get<bg::min_corner, 0>(envelope)), -1);
    BOOST_TEST_EQ((bg::get<bg::max_corner, 1>(envelope)), 1);
}

int main()
{
    overlays(); strict_predicates(); segments(); queries_and_boxes();
    return boost::report_errors();
}
