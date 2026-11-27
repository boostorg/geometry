// Boost.Geometry
// Distributed under the Boost Software License, Version 1.0.

#include <boost/core/lightweight_test.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/strategies/relate/cartesian_exact.hpp>
#include <array>
#include <cmath>
#include <cstdint>
#include <functional>
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

void squared_distance_order()
{
    point const origin(0, 0), a(1, 0), b(1, 0x1p-40);
    sr::cartesian_exact<double> const strategy;
    strategy.with_operands(a, b, [&](auto const&, auto const&, auto const& bound)
    {
        auto const topology = bound.topology();
        auto const near = topology.squared_distance(origin, a);
        auto const far = topology.squared_distance(origin, b);
        auto const tie = topology.squared_distance(a, origin);
        // The rounded squared distances coincide, but the exact distances do not.
        using bg::strategies::detail::compare_squared_distances;
        BOOST_TEST(compare_squared_distances(near, far, std::less<>{}));
        BOOST_TEST(!compare_squared_distances(far, near, std::less_equal<>{}));
        BOOST_TEST(compare_squared_distances(near, tie, std::less_equal<>{}));
        BOOST_TEST(compare_squared_distances(tie, near, std::greater_equal<>{}));
        BOOST_TEST_EQ(topology.side_measure_sign(origin, a, b), 1);
    });
}

void conservative_box_operations()
{
    namespace exact = bg::strategies::detail::exact_cartesian;
    sr::cartesian_exact<double> const strategy;
    double const infinity = std::numeric_limits<double>::infinity();
    exact::search_box empty;
    bg::envelope(line{}, empty, strategy);
    BOOST_TEST_EQ(empty.min_corner().x(), infinity);
    BOOST_TEST_EQ(empty.max_corner().x(), -infinity);

    exact::search_box bounds;
    bg::envelope(line{{1, 2}, {3, 4}}, bounds, strategy);
    bg::expand(bounds, empty, strategy);
    BOOST_TEST_EQ(bounds.min_corner().x(), 1.0);
    BOOST_TEST_EQ(bounds.max_corner().x(), 3.0);
    bg::expand(bounds, exact::search_box{point(-infinity, 0), point(-1, 1)}, strategy);
    BOOST_TEST_EQ(bounds.min_corner().x(), -infinity);
    BOOST_TEST_EQ(bounds.max_corner().x(), 3.0);
    BOOST_TEST_EQ(bounds.min_corner().y(), 0.0);
    BOOST_TEST_EQ(bounds.max_corner().y(), 4.0);

    using int_point = bg::model::d2::point_xy<std::int64_t>;
    std::int64_t const beyond_double = (std::int64_t(1) << 53) + 1;
    bg::envelope(int_point(beyond_double, -beyond_double), bounds,
                 sr::cartesian_exact<std::int64_t>{});
    BOOST_TEST(bounds.min_corner().x() <= 0x1p53);
    BOOST_TEST(bounds.max_corner().x() >= std::nextafter(0x1p53, infinity));
    BOOST_TEST(bounds.min_corner().y() <= std::nextafter(-0x1p53, -infinity));
    BOOST_TEST(bounds.max_corner().y() >= -0x1p53);
}

int main()
{
    namespace exact = bg::strategies::detail::exact_cartesian;
    namespace recipe = bg::strategies::detail::linear_formula;
    namespace rules = bg::detail::semi_static_error;
    using native_product = decltype(rules::product(
        rules::difference<0, 0>(0., 0.), rules::difference<0, 0>(0., 0.)));
    constexpr double epsilon = std::numeric_limits<double>::epsilon();
    static_assert(rules::native_sign_coefficient(native_product{}, native_product{})
        == (1.5 + 4 * epsilon) * epsilon, "Preserve the native orient2d sign bound");

    // The integer inputs and their products cannot all be exact in binary64.
    // Both exact supports meet at (1/2, 1/2), despite cancellation near 2^62.
    std::int64_t const m = std::int64_t(1) << 62;
    std::array<std::int64_t, 8> const wide{{-m, -m, m, m, -m, m + 1, m, 1 - m}};
    auto const fixed = exact::direct_native_homogeneous(wide);
    BOOST_TEST(fixed.W > 0 && fixed.X * 2 == fixed.W && fixed.Y * 2 == fixed.W);
    exact::forward_error::point_approximation enclosure;
    BOOST_TEST(exact::forward_error::try_intersection(wide, enclosure));
    for (std::size_t i = 0; i < enclosure.value.size(); ++i)
        BOOST_TEST(std::abs(enclosure.value[i] - 0.5) <= enclosure.error[i]);

    // Exact intersection from Cramer's rule; each integer fits exactly in binary64.
    line const a{{43,7}, {-24,81}}, b{{-78,22}, {63,95}};
    crossing(a, b, sr::approximate_output{});

    squared_distance_order();
    conservative_box_operations();
    return boost::report_errors();
}
