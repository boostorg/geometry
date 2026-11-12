// Boost.Geometry
// Unit Test

// Copyright (c) 2026 Tinko Sebastian Bartels.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <numeric>
#include <string>
#include <vector>

#include <boost/geometry.hpp>
#include <boost/test/included/test_exec_monitor.hpp>

namespace bg = boost::geometry;

template <typename MultiPolygon>
void check_result(std::string const& name, MultiPolygon const& result,
                  MultiPolygon const& expected, std::size_t count, double area)
{
    BOOST_CHECK_MESSAGE(result.size() == count, name << ": component count " << result.size());
    BOOST_CHECK_MESSAGE(bg::area(result) == area, name << ": area " << bg::area(result));
    BOOST_CHECK_MESSAGE(bg::is_valid(result), name << ": invalid result");
    // Relate-based equality also checks support, independently of overlay traversal.
    BOOST_CHECK_MESSAGE(bg::equals(result, expected), name << ": wrong support");
}

template <typename MultiPolygon>
void check_lenses(MultiPolygon const& a, MultiPolygon const& b, std::string const& name)
{
    BOOST_CHECK(bg::is_valid(a));
    BOOST_CHECK(bg::is_valid(b));
    auto const count = a.size();
    double const area = 5 * count;
    MultiPolygon empty, expected = a, result;
    expected.insert(expected.end(), b.begin(), b.end());

    bg::difference(a, b, result);
    check_result(name + " a-b", result, a, count, area);
    result.clear();
    bg::difference(b, a, result);
    check_result(name + " b-a", result, b, 1, 10);

    for (bool reverse : {false, true})
    {
        auto const& first = reverse ? b : a;
        auto const& second = reverse ? a : b;
        result.clear();
        bg::intersection(first, second, result);
        check_result(name + " intersection", result, empty, 0, 0);
        result.clear();
        bg::union_(first, second, result);
        check_result(name + " union", result, expected, count + 1, area + 10);
        result.clear();
        bg::sym_difference(first, second, result);
        check_result(name + " symmetric difference", result, expected, count + 1, area + 10);
    }
}

template <typename MultiPolygon>
void rotate_ring_starts(MultiPolygon& geometry)
{
    for (auto& polygon : geometry)
    {
        auto& ring = bg::exterior_ring(polygon);
        bool const closed = bg::closure<MultiPolygon>::value == bg::closed;
        auto const end = ring.end() - (closed ? 1 : 0);
        std::rotate(ring.begin(), ring.begin() + 1, end);
        if (closed)
        {
            ring.back() = ring.front();
        }
    }
}

template <typename MultiPolygon>
void test_lenses()
{
    // All rings meet at two clustered graph nodes. Every upper lens has area 5;
    // the lower triangle has area 10 and touches the lenses only at those nodes.
    // Three parallel paths used to lose one complete lens after assigning their
    // shared biconnected component, although the incomplete output stayed valid.
    for (std::size_t count = 3; count <= 5; ++count)
    {
        MultiPolygon lenses, lower;
        for (std::size_t i = 0; i < count; ++i)
        {
            typename MultiPolygon::value_type polygon;
            bg::read_wkt("POLYGON((0 0,5 " + std::to_string(2 * i + 2)
                + ",10 0,5 " + std::to_string(2 * i + 1) + ",0 0))", polygon);
            bg::correct(polygon);
            lenses.push_back(polygon);
        }
        bg::read_wkt("MULTIPOLYGON(((0 0,10 0,5 -2,0 0)))", lower);
        bg::correct(lower);

        std::vector<std::size_t> order(count);
        std::iota(order.begin(), order.end(), 0);
        do
        {
            if (count == 5)
            {
                // Test cyclic member orders and their reversals for five lenses;
                // three and four lenses exercise every member permutation.
                bool forward = true, backward = true;
                for (std::size_t i = 1; i < count; ++i)
                {
                    forward = forward && order[i] == (order[0] + i) % count;
                    backward = backward && order[i] == (order[0] + count - i) % count;
                }
                if (!forward && !backward)
                {
                    continue;
                }
            }
            MultiPolygon a, b = lower;
            for (auto const index : order)
            {
                a.push_back(lenses[index]);
            }
            // Also move each ring's first vertex off the shared graph node.
            for (int start = 0; start < 4; ++start)
            {
                check_lenses(a, b, "parallel_lenses_" + std::to_string(count));
                rotate_ring_starts(a);
                rotate_ring_starts(b);
            }
        }
        while (std::next_permutation(order.begin(), order.end()));
    }
}

template <typename MultiPolygon>
void test_issues()
{
    // Exact inputs and expected support from boostorg/geometry issues 1420/1421.
    MultiPolygon a, b, expected, result;
    bg::read_wkt("MULTIPOLYGON(((4 6,2 6,2 4,0 4,0 8,4 8,6 8,6 6,6 4,4 4,4 6)),"
        "((2 2,2 4,4 4,4 2,2 2)),((2 2,2 0,0 0,0 2,2 2)),((4 2,6 2,6 0,4 0,4 2)))", a);
    bg::read_wkt("MULTIPOLYGON(((2 2,2 4,4 4,4 2,2 2)))", b);
    bg::correct(a);
    bg::correct(b);
    for (bool reverse : {false, true})
    {
        result.clear();
        bg::union_(reverse ? b : a, reverse ? a : b, result);
        check_result("issue_1420 union", result, a, 4, 32);
    }

    bg::read_wkt("MULTIPOLYGON(((8 8,8 6,8 4,6 4,6 2,6 0,0 0,0 6,2 6,2 8,8 8),"
        "(2 4,2 2,4 2,4 4,2 4),(6 6,4 6,4 4,6 4,6 6)))", a);
    bg::read_wkt("MULTIPOLYGON(((6 6,6 4,4 4,4 6,6 6)),((6 6,6 8,8 8,8 6,6 6)),"
        "((4 6,2 6,2 8,4 8,4 6)),((4 4,4 2,2 2,2 4,4 4)),((4 2,6 2,6 0,4 0,4 2)),"
        "((2 6,2 4,0 4,0 6,2 6)),((2 2,2 0,0 0,0 2,2 2)),((6 2,6 4,8 4,8 2,6 2)))", b);
    bg::correct(a);
    bg::correct(b);
    bg::read_wkt("MULTIPOLYGON(((8 6,8 4,6 4,6 6,8 6)),((4 0,2 0,2 2,4 2,4 0)),"
        "((0 4,2 4,2 2,0 2,0 4)),((6 8,6 6,4 6,4 8,6 8)),"
        "((6 4,6 2,4 2,4 4,6 4)),((2 6,4 6,4 4,2 4,2 6)))", expected);
    bg::correct(expected);
    result.clear();
    bg::difference(a, b, result);
    check_result("issue_1421 difference", result, expected, 6, 24);

    bg::read_wkt("MULTIPOLYGON(((8 2,4 2,4 0,2 0,2 2,0 2,0 4,2 4,2 6,4 6,4 8,6 8,6 6,8 6,8 2)))",
        expected);
    bg::correct(expected);
    for (bool reverse : {false, true})
    {
        result.clear();
        bg::sym_difference(reverse ? b : a, reverse ? a : b, result);
        check_result("issue_1421 symmetric difference", result, expected, 1, 36);
    }
}

template <typename Coordinate, bool Clockwise, bool Closed>
void test_all()
{
    using point = bg::model::d2::point_xy<Coordinate>;
    using polygon = bg::model::polygon<point, Clockwise, Closed>;
    using multi_polygon = bg::model::multi_polygon<polygon>;
    test_lenses<multi_polygon>();
    test_issues<multi_polygon>();
}

int test_main(int, char* [])
{
    test_all<double, true, true>();
    test_all<double, false, true>();
    test_all<int, false, false>();
    return 0;
}
