// Boost.Geometry
// Distributed under the Boost Software License, Version 1.0.

#include <boost/core/lightweight_test.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/strategies/relate/cartesian_exact.hpp>
#include <cmath>
#include <limits>

namespace bg = boost::geometry;
namespace sr = bg::strategies::relate;
using point = bg::model::d2::point_xy<double>;
using line = bg::model::linestring<point>;

template <typename Policy>
point crossing(line const& a, line const& b, Policy const&)
{
    sr::cartesian_exact<double, Policy> strategy;
    BOOST_TEST_EQ(bg::relation(a, b, strategy).str(), "0F1FF0102");
    bg::model::multi_point<point> output;
    bg::intersection(a, b, output, strategy);
    BOOST_TEST_EQ(output.size(), 1u);
    return output.empty() ? point(0, 0) : output.front();
}

bool within_four_steps(double x, double nearest)
{
    double lower = nearest, upper = nearest;
    for (int i = 0; i < 4; ++i)
    {
        lower = std::nextafter(lower, -std::numeric_limits<double>::infinity());
        upper = std::nextafter(upper, std::numeric_limits<double>::infinity());
    }
    return lower <= x && x <= upper;
}

int main()
{
    // Exact intersection from Cramer's rule; each integer fits exactly in binary64.
    line const a{{43,7}, {-24,81}}, b{{-78,22}, {63,95}};
    point const expected(-74541.0 / 15325.0, 917427.0 / 15325.0);
    auto const nearest = crossing(a, b, sr::nearest_output{});
    BOOST_TEST_EQ(nearest.x(), expected.x());
    BOOST_TEST_EQ(nearest.y(), expected.y());
    // With linear output the same isolated crossing is a degenerate linestring.
    bg::model::multi_linestring<line> isolated;
    bg::intersection(a, b, isolated, sr::cartesian_exact<double, sr::nearest_output>{});
#ifdef BOOST_GEOMETRY_INTERSECTION_DO_NOT_INCLUDE_ISOLATED_POINTS
    BOOST_TEST(isolated.empty());
#else
    BOOST_TEST_EQ(isolated.size(), 1u);
    if (isolated.size() == 1)
    {
#ifdef BOOST_GEOMETRY_ALLOW_ONE_POINT_LINESTRINGS
        BOOST_TEST_EQ(isolated.front().size(), 1u);
#else
        BOOST_TEST_EQ(isolated.front().size(), 2u);
#endif
        for (auto const& p : isolated.front())
        {
            BOOST_TEST_EQ(p.x(), expected.x());
            BOOST_TEST_EQ(p.y(), expected.y());
        }
    }
#endif
    auto const bounded = crossing(a, b, sr::bounded_ulp_output<4>{});
    BOOST_TEST(within_four_steps(bounded.x(), expected.x()));
    BOOST_TEST(within_four_steps(bounded.y(), expected.y()));
    crossing(a, b, sr::approximate_output{});

    // The crossing is exactly halfway between consecutive doubles, first
    // with the even significand below, then with the even significand above.
    double const odd = std::nextafter(1.0, 2.0);
    double const even = std::nextafter(odd, 2.0);
    line const horizontal{{0,1}, {2,1}};
    BOOST_TEST_EQ(crossing(line{{1,0}, {odd,2}}, horizontal, sr::nearest_output{}).x(), 1.0);
    BOOST_TEST_EQ(crossing(line{{odd,0}, {even,2}}, horizontal, sr::nearest_output{}).x(), even);
    return boost::report_errors();
}
