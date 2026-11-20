// Boost.Geometry
// Unit Test
// Copyright (c) 2026 Tinko Bartels.
// Distributed under the Boost Software License, Version 1.0.

#include <boost/geometry/algorithms/buffer_round_certified.hpp>

#include <cstddef>
#include <string>
#include <vector>

#include <boost/core/lightweight_test.hpp>
#include <boost/geometry/algorithms/within.hpp>
#include <boost/geometry/geometries/multi_point.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/io/wkt/read.hpp>

namespace bg = boost::geometry;
using decimal = bg::exact_coordinate<std::string>;
using rational = boost::multiprecision::cpp_rational;
using point = bg::model::point<decimal, 2, bg::cs::cartesian>;
using polygon = bg::model::polygon<point, false, false>;
using status = bg::certified_buffer_status;

template <typename Ring>
std::vector<std::string> tokens(Ring const& ring)
{
    std::vector<std::string> result;
    for (auto const& p : ring)
    {
        result.push_back(bg::get<0>(p).scalar());
        result.push_back(bg::get<1>(p).scalar());
    }
    return result;
}

void test_point()
{
    point center;
    bg::read_wkt("POINT(9007199254740992.1 -0.125)", center);
    polygon out;
    decimal const radius("0.25");
    BOOST_TEST(bg::buffer_round_certified(center, out, radius, decimal("0.20"), 0)
               == status::success);
    auto const& ring = out.outer();
    BOOST_TEST_EQ(ring.size(), 4u);
    if (ring.size() != 4)
    {
        return;
    }

    rational const cx = rational(9007199254740992LL) + rational(1) / 10;
    rational const cy = rational(-1) / 8;
    rational area2 = 0;
    for (std::size_t i = 0; i < ring.size(); ++i)
    {
        auto const& p = ring[i];
        auto const& q = ring[(i + 1) % ring.size()];
        rational const x = bg::get<0>(p).rational() - cx;
        rational const y = bg::get<1>(p).rational() - cy;
        BOOST_TEST(x*x + y*y == rational(1) / 16);
        area2 += x * (bg::get<1>(q).rational() - cy) - y * (bg::get<0>(q).rational() - cx);
    }
    BOOST_TEST(area2 == rational(1) / 4);
    auto const before = tokens(ring);
    // Finer chords require a vertex with a factor other than 2 or 5 in
    // the denominator. A large displacement budget cannot round it to text.
    BOOST_TEST(bg::buffer_round_certified(center, out, radius, decimal("0.001"), 10)
               == status::cannot_certify);
    BOOST_TEST(tokens(ring) == before);
    BOOST_TEST(bg::buffer_round_certified(center, out, radius, -1, 0) == status::invalid_input);
    BOOST_TEST(tokens(ring) == before);

    using native = bg::model::polygon<bg::model::point<double, 2, bg::cs::cartesian>>;
    native stored;
    stored.outer().emplace_back(7, 8);
    BOOST_TEST(bg::buffer_round_certified(center, stored, radius, decimal("0.20"), 0)
               == status::cannot_certify);
    BOOST_TEST_EQ(stored.outer().size(), 1u);
    if (!stored.outer().empty())
    {
        BOOST_TEST_EQ(bg::get<0>(stored.outer().front()), 7);
        BOOST_TEST_EQ(bg::get<1>(stored.outer().front()), 8);
    }
}

void test_trace()
{
    point const a(decimal("0.1"), decimal("0"));
    point const b(decimal("3.1"), decimal("4"));
    polygon out;
    BOOST_TEST(bg::buffer_round_certified(bg::model::segment<point>(a, b), out, 1, 0.8, 0)
               == status::success);
    auto const capsule = tokens(out.outer());
    BOOST_TEST_EQ(capsule.size(), 12u);
    bg::model::linestring<point> const trace{b, a, b};
    BOOST_TEST(bg::buffer_round_certified(trace, out, 1, 0.8, 0) == status::success);
    BOOST_TEST(tokens(out.outer()) == capsule);

    point const diagonal(decimal("1.1"), decimal("1"));
    BOOST_TEST(bg::buffer_round_certified(bg::model::segment<point>(a, diagonal), out, 1, 0.8, 1)
               == status::cannot_certify);
    BOOST_TEST(tokens(out.outer()) == capsule);
    bg::model::multi_point<point> const points{a, b};
    BOOST_TEST(bg::buffer_round_certified(points, out, 1, 0.8, 0) == status::cannot_certify);
    BOOST_TEST(tokens(out.outer()) == capsule);
    BOOST_TEST(bg::buffer_round_certified(points, out, -1, 0, 0) == status::success);
    BOOST_TEST(out.outer().empty());
}

void test_native_export()
{
    using native_point = bg::model::point<double, 2, bg::cs::cartesian>;
    using native_polygon = bg::model::polygon<native_point>;
    bg::model::multi_point<point> input;
    bg::read_wkt("MULTIPOINT((0.1 0),(3.1 0))", input);
    bg::model::multi_polygon<native_polygon> out;
    decimal const radius("0.25"), sagitta("0.20"), displacement("1e-12");
    BOOST_TEST(bg::buffer_round_certified(input, out, radius, sagitta, displacement)
               == status::success);
    BOOST_TEST_EQ(out.size(), 2u);
    BOOST_TEST(bg::within(native_point(0.1, 0), out));
    BOOST_TEST(bg::within(native_point(3.1, 0), out));
    BOOST_TEST(!bg::within(native_point(1, 0), out));

    native_polygon one;
    one.outer().emplace_back(9, 7);
    BOOST_TEST(bg::buffer_round_certified(input, one, radius, sagitta, displacement)
               == status::cannot_certify);
    BOOST_TEST_EQ(one.outer().size(), 1u);
    if (!one.outer().empty())
    {
        BOOST_TEST_EQ(bg::get<0>(one.outer().front()), 9);
        BOOST_TEST_EQ(bg::get<1>(one.outer().front()), 7);
    }
}

void test_polygon()
{
    using closed_polygon = bg::model::polygon<point>;
    closed_polygon square;
    bg::read_wkt("POLYGON((0.1 0,0.1 2,2.1 2,2.1 0,0.1 0))", square);
    polygon out;
    BOOST_TEST(bg::buffer_round_certified(square, out, decimal("-0.25"), 0, 0)
               == status::success);
    auto const& ring = out.outer();
    BOOST_TEST_EQ(ring.size(), 4u);
    for (auto const& p : ring)
    {
        auto const x = bg::get<0>(p).rational(), y = bg::get<1>(p).rational();
        BOOST_TEST(x == rational(7) / 20 || x == rational(37) / 20);
        BOOST_TEST(y == rational(1) / 4 || y == rational(7) / 4);
    }
    auto const before = tokens(ring);
    closed_polygon huge;
    bg::read_wkt("POLYGON((0 0,0 2,1e1500 2,1e1500 0,0 0))", huge);
    BOOST_TEST(bg::buffer_round_certified(huge, out, 1, 0.8, 0) == status::cannot_certify);
    BOOST_TEST(tokens(ring) == before);
    square.outer().pop_back();
    BOOST_TEST(bg::buffer_round_certified(square, out, 0, 0, 0) == status::invalid_input);
    BOOST_TEST(tokens(ring) == before);
}

int main()
{
    test_point();
    test_trace();
    test_native_export();
    test_polygon();
    return boost::report_errors();
}
