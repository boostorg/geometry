// Boost.Geometry
// Distributed under the Boost Software License, Version 1.0.

#include <boost/core/lightweight_test.hpp>
#include <boost/geometry.hpp>
#include <algorithm>
#include <cmath>
#include <tuple>
#include <type_traits>

int main()
{
#ifdef BOOST_GEOMETRY_DETAIL_HAS_CARTESIAN_EXACT_DEFAULT
    namespace bg = boost::geometry;
    namespace sr = bg::strategies::relate;
    using point = bg::model::d2::point_xy<double>;
    using polygon = bg::model::polygon<point>;
    using selected = bg::detail::cartesian_exact_default::strategy<polygon, polygon>;
    static_assert(std::is_same<selected, sr::cartesian_exact<double, sr::bounded_ulp_output<4>>>::value,
                  "The Cartesian default certifies four-ULP output");

    // An explicit strategy still takes precedence over the default.
    double const x = 1000000.0;
    point const a(x, 0), b(std::nextafter(x, x + 1), 0);
    BOOST_TEST_EQ(bg::relation(a, b).str(), "FF0FFF0F2");
    BOOST_TEST(bg::relation(a, b, sr::cartesian<>{}).str() != bg::relation(a, b).str());
    BOOST_TEST(!bg::covered_by(b, a));
    BOOST_TEST(bg::covered_by(b, a, sr::cartesian<>{}));
    BOOST_TEST(!bg::within(b, a));
    BOOST_TEST(!bg::equals(a, b));
    BOOST_TEST(bg::disjoint(a, b));
    BOOST_TEST(!bg::intersects(a, b));
    polygon square;
    square.outer() = {point(x - 1, -1), point(x - 1, 1), point(x, 1), point(x, -1), point(x - 1, -1)};
    BOOST_TEST(bg::covered_by(a, square));
    BOOST_TEST(!bg::covered_by(b, square));
    BOOST_TEST(!bg::within(a, square));
    auto rectangle = [](double left, double right)
    {
        polygon p;
        p.outer() = {{left, -1}, {left, 1}, {right, 1}, {right, -1}, {left, -1}};
        return p;
    };
    auto const touching_square = rectangle(x, x + 1);
    auto const separated_square = rectangle(b.x(), x + 1);
    auto const overlapping_square = rectangle(std::nextafter(x, x - 1), x + 1);
    BOOST_TEST(bg::touches(square, touching_square));
    BOOST_TEST(!bg::touches(square, separated_square));
    BOOST_TEST(bg::disjoint(square, separated_square));
    BOOST_TEST(bg::overlaps(square, overlapping_square));
    BOOST_TEST(!bg::touches(square, overlapping_square));
    BOOST_TEST(!bg::equals(square, rectangle(x - 1, b.x())));
    auto subdivided = square;
    subdivided.outer().insert(subdivided.outer().begin() + 1, point(x - 1, 0));
    BOOST_TEST(bg::equals(square, subdivided));
    auto rotated = square;
    std::rotate(rotated.outer().begin(), rotated.outer().begin() + 1, rotated.outer().end() - 1);
    rotated.outer().back() = rotated.outer().front();
    BOOST_TEST(bg::equals(square, rotated));
    using box = bg::model::box<point>;
    box const square_box({x - 1, -1}, {x, 1}), wider_box({x - 1, -1}, {b.x(), 1});
    BOOST_TEST(!bg::equals(square_box, wider_box));
    BOOST_TEST(bg::equals(square_box, subdivided));
    BOOST_TEST(!bg::equals(square, wider_box));
    BOOST_TEST(bg::covered_by(square_box, square));
    BOOST_TEST(!bg::covered_by(wider_box, square));
    BOOST_TEST(bg::touches(square_box, box({x, -1}, {x + 1, 1})));
    BOOST_TEST(!bg::touches(square_box, box({b.x(), -1}, {x + 1, 1})));
    BOOST_TEST(bg::overlaps(square_box, overlapping_square));
    polygon triangle, complement;
    triangle.outer() = {{x - 1, -1}, {x - 1, 1}, {x, 1}, {x - 1, -1}};
    complement.outer() = {{x - 1, -1}, {x, 1}, {x, -1}, {x - 1, -1}};
    auto const filter = selected::areal_equality();
    BOOST_TEST(filter.bounds_match(square, triangle));
    BOOST_TEST(!filter.area_can_match(square, triangle));
    BOOST_TEST(filter.area_can_match(triangle, complement));
    BOOST_TEST(!bg::equals(square, triangle));
    BOOST_TEST(!bg::equals(triangle, complement));
    BOOST_TEST(filter.area_can_match(square, subdivided));
    BOOST_TEST(!filter.bounds_match(square, rectangle(x - 1, x - 0.5)));
    boost::variant<point, polygon> const dynamic_a(a), dynamic_b(b);
    BOOST_TEST(bg::disjoint(dynamic_a, dynamic_b));
    BOOST_TEST(!bg::equals(dynamic_a, dynamic_b));
    bg::model::multi_point<point> output;
    bg::union_(a, b, output);
    BOOST_TEST_EQ(output.size(), 2u);

    using line = bg::model::linestring<point>;
    line const horizontal{{x - 1, 0}, {b.x(), 0}};
    line const vertical{{x, -1}, {x, 1}};
    BOOST_TEST(bg::crosses(horizontal, vertical));
    line const endpoint{{x - 1, 0}, {x, 0}};
    BOOST_TEST(!bg::crosses(endpoint, vertical));
    BOOST_TEST(bg::touches(endpoint, vertical));
    bg::model::segment<point> const segment({x - 1, 0}, a);
    BOOST_TEST(bg::touches(a, segment));
    BOOST_TEST(!bg::touches(b, segment));
    using lines = bg::model::multi_linestring<line>;
    using points = bg::model::multi_point<point>;
    bg::read_wkt("POLYGON((0 0,0 2,2 2,2 0,0 0))", square);
    lines touching;
    bg::read_wkt("MULTILINESTRING((-1 1,0 0,1 -1),(-1 3,0 2,1 3),(-1 0,0 0,0 -1))",
                 touching);
    std::tuple<points, lines> isolated;
    BOOST_TEST(bg::intersection(touching, square, isolated));
    auto const& emitted = std::get<0>(isolated);
    BOOST_TEST_EQ(emitted.size(), 2u);
    BOOST_TEST(std::get<1>(isolated).empty());
    if (emitted.size() == 2)
    {
        BOOST_TEST_EQ(bg::get<1>(emitted[0]), 0);
        BOOST_TEST_EQ(bg::get<1>(emitted[1]), 2);
    }
#endif
    return boost::report_errors();
}
