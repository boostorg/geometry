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

void rational_and_mixed_inputs()
{
    using q = boost::multiprecision::cpp_rational;
    using qpoint = bg::model::d2::point_xy<q>;
    using qpolygon = bg::model::polygon<qpoint>;
    using qmulti = bg::model::multi_polygon<qpolygon>;
    sr::cartesian_exact<double, sr::nearest_output> const strategy;
    q const third(1, 3);
    bg::model::segment<qpoint> const a{{third, 0}, {third, 1}};
    bg::model::segment<qpoint> const b{{0, 2 * third}, {1, 2 * third}};
    bg::model::segment<point> const c{{0, 0.1}, {1, 0.1}};
    bg::model::multi_point<qpoint> hits;
    BOOST_TEST(bg::intersection(a, b, hits, strategy));
    BOOST_TEST_EQ(hits.size(), 1u);
    if (hits.size() == 1)
    {
        BOOST_TEST_EQ(hits.front().x(), third);
        BOOST_TEST_EQ(hits.front().y(), q(2, 3));
    }
    hits.clear();
    BOOST_TEST(bg::intersection(a, c, hits, strategy));
    BOOST_TEST_EQ(hits.size(), 1u);
    if (hits.size() == 1)
    {
        BOOST_TEST_EQ(hits.front().x(), third);
        BOOST_TEST_EQ(hits.front().y(), q(0.1));
    }

    qpolygon left;
    bg::model::polygon<point> right;
    bg::read_wkt("POLYGON((1/3 0,1/3 1,4/3 1,4/3 0,1/3 0))", left);
    bg::read_wkt("POLYGON((0.5 0,0.5 1,2 1,2 0,0.5 0))", right);
    BOOST_TEST_EQ(bg::relation(left, right, strategy).str(), "212111212");
    qmulti out;
    BOOST_TEST(bg::intersection(left, right, out, strategy));
    BOOST_TEST_EQ(bg::area(out), q(5, 6));
    out.clear();
    bg::union_(left, right, out, strategy);
    BOOST_TEST_EQ(bg::area(out), q(5, 3));
    out.clear();
    bg::difference(left, right, out, strategy);
    BOOST_TEST_EQ(bg::area(out), q(1, 6));
    out.clear();
    bg::sym_difference(left, right, out, strategy);
    BOOST_TEST_EQ(bg::area(out), q(5, 6));

    bg::model::box<qpoint> bounds;
    bg::envelope(left, bounds, strategy);
    BOOST_TEST_EQ((bg::get<bg::min_corner, 0>(bounds)), third);
    BOOST_TEST_EQ((bg::get<bg::max_corner, 0>(bounds)), q(4, 3));
    bg::model::box<point> disjoint_box{{0.5, 2}, {2, 3}};
    bg::model::box<qpoint> clipped{{0, -2}, {0, -1}};
    BOOST_TEST(!bg::intersection(bounds, disjoint_box, clipped, strategy));
    BOOST_TEST_EQ((bg::get<bg::min_corner, 0>(clipped)), q(1, 2));
    BOOST_TEST_EQ((bg::get<bg::max_corner, 0>(clipped)), q(4, 3));
    BOOST_TEST_EQ((bg::get<bg::min_corner, 1>(clipped)), q(-2));
    BOOST_TEST_EQ((bg::get<bg::max_corner, 1>(clipped)), q(-1));
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
    namespace filter = exact::ulp_filter;
    namespace recipe = bg::strategies::detail::linear_formula;
    using certificate = filter::producer_certificate<4>;
    namespace rules = bg::detail::semi_static_error;
    using native_product = decltype(rules::product(
        rules::difference<0, 0>(0., 0.), rules::difference<0, 0>(0., 0.)));
    constexpr double epsilon = std::numeric_limits<double>::epsilon();
    static_assert(rules::native_sign_coefficient(native_product{}, native_product{})
        == (1.5 + 4 * epsilon) * epsilon, "Preserve the native orient2d sign bound");

    filter::coordinates const start{{1, 2}}, end{{4, 8}};
    auto const edge = recipe::orientation::prepare(filter::detail::bilinear_ops<certificate>{},
        filter::detail::certified_point<false>{start}, filter::detail::certified_point<false>{end});
    // Preparation must preserve the bound and use the late operand's guarantee.
    for (filter::coordinates const p : {filter::coordinates{{2, 5}}, {{3, 6}}, {{3, 5}}})
    {
        exact::scalar_approximation direct;
        BOOST_TEST((filter::try_signed_determinant<certificate, false, false, true>(start, end, p, direct)));
        auto const prepared = edge.evaluate(filter::detail::certified_point<true>{p});
        BOOST_TEST_EQ(prepared.value, direct.value);
        BOOST_TEST_EQ(prepared.error, direct.error);
    }

    // With an uncertain ordinate, this almost-collinear turn can have
    // either sign. Treating the rounded leaf as exact would be unsound.
    int sign = 0;
    filter::coordinates const zero{{0, 0}}, one{{1, 1}};
    filter::coordinates const near{{1, std::nextafter(1.0, 2.0)}};
    BOOST_TEST((!filter::try_orientation<certificate, false, false, true>(zero, one, near, sign)));

    filter::coordinates const orthogonal{{2, 0}}, forward{{2, 2}};
    BOOST_TEST((!filter::try_dot<certificate, false, false, true>(zero, one, orthogonal, sign)));
    BOOST_TEST((filter::try_dot<certificate, false, false, true>(zero, one, forward, sign)));
    BOOST_TEST_EQ(sign, 1);
    exact::scalar_approximation square;
    BOOST_TEST((filter::try_squared_distance<certificate, false, true>(zero, one, square)));
    BOOST_TEST_EQ(square.value, 2.0);
    BOOST_TEST(square.error > 0 && square.error < square.value);

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
    rational_and_mixed_inputs();
    squared_distance_order();
    conservative_box_operations();
    return boost::report_errors();
}
