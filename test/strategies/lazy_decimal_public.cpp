// Boost.Geometry
// Distributed under the Boost Software License, Version 1.0.

#include <boost/geometry/strategies/relate/cartesian_decimal.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/adapted/c_array.hpp>
#include <boost/core/lightweight_test.hpp>
#include <sstream>

namespace bg = boost::geometry;
BOOST_GEOMETRY_REGISTER_C_ARRAY_CS(cs::cartesian)
using decimal = bg::exact_coordinate<std::string>;
using rational = boost::multiprecision::cpp_rational;
template <typename C> using pt = bg::model::point<C, 2, bg::cs::cartesian>;
template <typename C> using poly = bg::model::polygon<pt<C>, false>;
template <typename C> using line = bg::model::linestring<pt<C>>;
template <typename C> using polys = bg::model::multi_polygon<poly<C>>;
template <typename C> using pts = bg::model::multi_point<pt<C>>;
template <typename C> using lines = bg::model::multi_linestring<line<C>>;

bg::strategies::relate::cartesian_decimal const strategy;

template <typename G>
G read(char const* wkt)
{
    G g;
    bg::read_wkt(wkt, g);
    return g;
}

void areal()
{
    auto a = read<poly<decimal>>("POLYGON((0 0,3 0,0 1,0 0))");
    auto b = read<polys<decimal>>("MULTIPOLYGON(((1 0,4 0,4 1,1 1,1 0)))");
    BOOST_TEST_EQ(bg::relation(a, b, strategy).str(), "212111212");
    BOOST_TEST(bg::relate(a, b, bg::de9im::mask("T*T***T**"), strategy));
    BOOST_TEST(bg::overlaps(a, b, strategy));
    BOOST_TEST(bg::equals(a, a, strategy));
    polys<rational> common, either, rest, sym;
    BOOST_TEST(bg::intersection(a, b, common, strategy));
    bg::union_(a, b, either, strategy);
    bg::difference(a, b, rest, strategy);
    bg::sym_difference(a, b, sym, strategy);
    BOOST_TEST_EQ(bg::area(common), rational(2, 3));
    BOOST_TEST_EQ(bg::area(either), rational(23, 6));
    BOOST_TEST_EQ(bg::area(rest), rational(5, 6));
    BOOST_TEST_EQ(bg::area(sym), rational(19, 6));

    using open_poly = bg::model::polygon<pt<decimal>, true, false>;
    auto hole = read<open_poly>("POLYGON((0 0,0 4,4 4,4 0,-0 0),(1 1,2 1,2 2,1 2,1.0 1e0))");
    auto p = read<pt<decimal>>("POINT(1.5 1.5)");
    BOOST_TEST(bg::disjoint(p, hole, strategy));
    BOOST_TEST_EQ(bg::relation(hole, polys<decimal>{}, strategy).str(), "FF2FF1FF2");
    poly<decimal> empty;
    polys<rational> retained;
    bg::difference(hole, empty, retained, strategy);
    BOOST_TEST_EQ(bg::area(retained), 15);

    // The island inside the cut has its own hole, which must not be
    // assigned to the larger surviving polygon.
    auto const window = read<poly<decimal>>(
        "POLYGON((0 0,10 0,10 10,0 10,0 0),(4 4,4 5,5 5,5 4,4 4))");
    auto const cut = read<poly<decimal>>(
        "POLYGON((2 2,8 2,8 8,2 8,2 2),(3 3,3 6,6 6,6 3,3 3))");
    polys<rational> nested;
    bg::difference(window, cut, nested, strategy);
    BOOST_TEST_EQ(nested.size(), 2u);
    BOOST_TEST_EQ(bg::area(nested), 72);
    for (auto const& piece : nested) BOOST_TEST_EQ(piece.inners().size(), 1u);
}

void linear()
{
    auto a = read<line<decimal>>("LINESTRING(0 0,3 1)");
    auto b = read<lines<decimal>>("MULTILINESTRING((1 0,1 1),(1.00000000000000000001 0,1.00000000000000000001 1))");
    BOOST_TEST_EQ(bg::relation(a, b, strategy).str(), "0F1FF0102");
    BOOST_TEST(bg::crosses(b, a, strategy));
    std::vector<pt<rational>> out(1); // must append, not replace
    BOOST_TEST(bg::intersection(b, a, out, strategy));
    BOOST_TEST_EQ(out.size(), 3u);
    if (out.size() == 3)
    {
        auto x = bg::get<0>(out[1]), y = bg::get<0>(out[2]);
        BOOST_TEST(x != y);
        BOOST_TEST_EQ((std::min)(x, y), 1);
        BOOST_TEST_EQ((std::max)(x, y), decimal("1.00000000000000000001").rational());
        BOOST_TEST_EQ(3 * bg::get<1>(out[1]), x);
        BOOST_TEST_EQ(3 * bg::get<1>(out[2]), y);
    }

    auto sliver = read<poly<decimal>>(
        "POLYGON((1 0,1.00000000000000000001 0,1.00000000000000000001 1,1 1,1 0))");
    lines<rational> clip, rest;
    BOOST_TEST(bg::intersection(a, sliver, clip, strategy));
    bg::difference(a, sliver, rest, strategy);
    BOOST_TEST_EQ(clip.size(), 1u);
    BOOST_TEST_EQ(rest.size(), 2u);
    if (clip.size() == 1 && !clip.front().empty())
    {
        BOOST_TEST_EQ(bg::get<0>(clip.front().front()), 1);
        BOOST_TEST_EQ(bg::get<0>(clip.front().back()), decimal("1.00000000000000000001").rational());
    }
    BOOST_TEST(bg::relate(sliver, a, bg::de9im::mask("1********"), strategy));
    auto c = read<line<decimal>>("LINESTRING(1.5 0.5,6 2)");
    lines<rational> common, either, sym;
    BOOST_TEST(bg::intersection(a, c, common, strategy));
    bg::union_(a, c, either, strategy);
    bg::sym_difference(a, c, sym, strategy);
    BOOST_TEST_EQ(common.size(), 1u);
    auto span = [](lines<rational> const& result)
    {
        rational dx = 0;
        for (auto const& part : result)
            for (std::size_t i = 1; i < part.size(); ++i)
                dx += abs(bg::get<0>(part[i]) - bg::get<0>(part[i - 1]));
        return dx;
    };
    BOOST_TEST_EQ(span(common), rational(3, 2));
    BOOST_TEST_EQ(span(either), 6);
    BOOST_TEST_EQ(sym.size(), 2u);
    BOOST_TEST_EQ(span(sym), rational(9, 2));
}

void open_ring_wkt()
{
    using open_poly = bg::model::polygon<pt<decimal>, true, false>;
    auto closed = read<open_poly>("POLYGON((0.1 0,0.1 2,2.1 2,1e-1 0))");
    auto distinct = read<open_poly>("POLYGON((0.1 0,0.1 2,2.1 2,0.10000000000000000001 0))");
    auto probe = read<pt<decimal>>("POINT(0.100000000000000000005 0)");
    BOOST_TEST_EQ(closed.outer().size(), 3u);
    BOOST_TEST_EQ(distinct.outer().size(), 4u);
    BOOST_TEST_EQ(bg::relation(probe, closed, strategy).str(), "FF0FFF212");
    BOOST_TEST_EQ(bg::relation(probe, distinct, strategy).str(), "F0FFFF212");

    // Native double closure still treats these adjacent binary64 values as equal.
    using binary_poly = bg::model::polygon<pt<double>, true, false>;
    auto binary = read<binary_poly>("POLYGON((1 0,1 2,3 2,1.0000000000000002 0))");
    BOOST_TEST_EQ(binary.outer().size(), 3u);
}

void pointlike()
{
    auto p = read<pt<decimal>>("POINT(1 0)");
    auto p2 = read<pt<decimal>>("POINT(1.00000000000000000001 0)");
    auto m = read<pts<decimal>>("MULTIPOINT((1 0),(1.00000000000000000001 0))");
    auto l = read<line<decimal>>("LINESTRING(0 0,1 0)");
    BOOST_TEST_EQ(bg::relation(m, l, strategy).str(), "F00FFF102");
    BOOST_TEST(bg::touches(m, l, strategy));
    BOOST_TEST(bg::disjoint(l, p2, strategy));
    BOOST_TEST(bg::covered_by(p, l, strategy));
    BOOST_TEST(!bg::within(p, l, strategy));
    BOOST_TEST(!bg::equals(p, p2, strategy));
    auto box = read<poly<decimal>>("POLYGON((0 -1,1 -1,1 1,0 1,0 -1))");
    BOOST_TEST_EQ(bg::relation(m, box, strategy).str(), "F00FFF212");
    BOOST_TEST_EQ(bg::relation(box, m, strategy).str(), "FF20F10F2");
    BOOST_TEST(bg::touches(p, box, strategy));
    BOOST_TEST(!bg::intersects(box, p2, strategy));
    pts<rational> common, rest, either, sym;
    BOOST_TEST(bg::intersection(m, l, common, strategy));
    bg::difference(m, l, rest, strategy);
    bg::union_(p, p2, either, strategy);
    bg::sym_difference(m, p, sym, strategy);
    BOOST_TEST_EQ(common.size(), 1u);
    BOOST_TEST_EQ(rest.size(), 1u);
    BOOST_TEST_EQ(either.size(), 2u);
    BOOST_TEST_EQ(sym.size(), 1u);
    if (rest.size() == 1 && sym.size() == 1)
    {
        BOOST_TEST_EQ(bg::get<0>(rest.front()), bg::get<0>(p2).rational());
        BOOST_TEST_EQ(bg::get<0>(sym.front()), bg::get<0>(p2).rational());
    }
}

void final_rounding()
{
    auto a = read<poly<decimal>>("POLYGON((0 0,1 0,1 1,0 1,0 0))");
    auto b = read<poly<decimal>>("POLYGON((1.00000000000000000001 0,2 0,2 1,1.00000000000000000001 1,1.00000000000000000001 0))");
    BOOST_TEST_EQ(bg::relation(a, b, strategy).str(), "FF2FF1212");
    polys<double> rounded;
    bg::sym_difference(a, b, rounded, strategy);
    BOOST_TEST_EQ(rounded.size(), 2u); // rounding before the final union would join them

    for (char const* e : {"e400", "e-400"})
    {
        auto const text = std::string("LINESTRING(0 0,3") + e + " 1)";
        auto l = read<line<decimal>>(text.c_str());
        auto const cut = std::string("LINESTRING(1") + e + " 0,1" + e + " 1)";
        auto r = read<line<decimal>>(cut.c_str());
        pts<rational> exact;
        pts<double> binary;
        BOOST_TEST(bg::intersection(l, r, exact, strategy));
        BOOST_TEST(bg::intersection(l, r, binary, strategy));
        BOOST_TEST_EQ(exact.size(), 1u);
        BOOST_TEST_EQ(binary.size(), 1u);
        if (exact.size() == 1 && binary.size() == 1)
        {
            BOOST_TEST_EQ(bg::get<0>(exact.front()), decimal(std::string("1") + e).rational());
            BOOST_TEST_EQ(bg::get<1>(exact.front()), rational(1, 3));
            BOOST_TEST_EQ(bg::get<1>(binary.front()), 1. / 3);
        }
    }
}

void boxes_and_collections()
{
    using box = bg::model::box<pt<decimal>>;
    using seg = bg::model::segment<pt<decimal>>;
    auto l = read<line<decimal>>("LINESTRING(0 0,3 1)");
    box bounds(read<pt<decimal>>("POINT(1 0)"), read<pt<decimal>>("POINT(2 1)"));
    lines<rational> clipped;
    BOOST_TEST(bg::intersection(l, bounds, clipped, strategy));
    BOOST_TEST_EQ(clipped.size(), 1u);
    if (clipped.size() == 1 && !clipped.front().empty())
        BOOST_TEST_EQ(bg::get<1>(clipped.front().front()), rational(1, 3));
    seg s(l.front(), l.back()), t(read<pt<decimal>>("POINT(1 0)"), read<pt<decimal>>("POINT(1 1)"));
    pts<rational> crossing;
    BOOST_TEST(bg::intersection(s, t, crossing, strategy));
    BOOST_TEST_EQ(crossing.size(), 1u);
    if (crossing.size() == 1)
    {
        BOOST_TEST_EQ(bg::get<1>(crossing.front()), rational(1, 3));
        // Rational output prints the fraction, not a rounded decimal spelling.
        std::ostringstream text;
        text << bg::wkt(crossing.front());
        BOOST_TEST_EQ(text.str(), "POINT(1 1/3)");
        BOOST_TEST_THROWS(read<pt<decimal>>(text.str().c_str()), bg::read_wkt_exception);
    }

    auto a = read<poly<decimal>>("POLYGON((0 0,3 0,0 1,0 0))");
    auto b = read<poly<decimal>>("POLYGON((1 0,4 0,4 1,1 1,1 0))");
    using gc = bg::model::geometry_collection<boost::variant<poly<decimal>>>;
    gc first{a}, second{b};
    BOOST_TEST_EQ(bg::relation(first, second, strategy).str(), "212111212");
    using out = bg::model::geometry_collection<boost::variant<pts<rational>, lines<rational>, polys<rational>>>;
    out result;
    BOOST_TEST(bg::intersection(first, second, result, strategy));
    BOOST_TEST_EQ(result.size(), 1u);
    if (result.size() == 1)
        BOOST_TEST_EQ(bg::area(boost::get<polys<rational>>(result.front())), rational(2, 3));
    bg::difference(first, second, result, strategy);
    bg::sym_difference(first, second, result, strategy);
    bg::union_(first, second, result, strategy);
    BOOST_TEST_EQ(result.size(), 4u);
    if (result.size() == 4)
    {
        BOOST_TEST_EQ(bg::area(boost::get<polys<rational>>(result[1])), rational(5, 6));
        BOOST_TEST_EQ(bg::area(boost::get<polys<rational>>(result[2])), rational(19, 6));
        BOOST_TEST_EQ(bg::area(boost::get<polys<rational>>(result[3])), rational(23, 6));
    }

    std::tuple<pts<rational>, lines<rational>, polys<rational>> tuple;
    BOOST_TEST(bg::intersection(a, b, tuple, strategy));
    BOOST_TEST_EQ(bg::area(std::get<2>(tuple)), rational(2, 3));
}

void cached_rational()
{
    using cached = bg::exact_coordinate<rational>;
    auto point_at = [](rational const& x)
    {
        pt<cached> p;
        bg::set<0>(p, cached(x));
        return p;
    };
    rational const a(1, 10);
    rational const b = decimal("0.10000000000000000001").rational();
    pts<cached> points{point_at(a), point_at(b)};
    auto const other = point_at(rational(1, 3));
    BOOST_TEST_EQ(bg::get<0>(points[0]).binary64(), bg::get<0>(points[1]).binary64());
    BOOST_TEST(!bg::equals(points[0], points[1], strategy));
    pts<rational> expected;
    bg::union_(points, other, expected, strategy);
    BOOST_TEST_EQ(expected.size(), 3u);

    std::tuple<pts<rational>, lines<rational>, polys<rational>> tuple;
    BOOST_TEST(bg::intersection(points, points, tuple, strategy));
    BOOST_TEST_EQ(std::get<0>(tuple).size(), 2u);

    pts<cached> result{point_at(rational(1))};
    bg::union_(points, other, result, strategy);
    BOOST_TEST_EQ(result.size(), expected.size() + 1);
    if (result.size() == expected.size() + 1)
        for (std::size_t i = 0; i < expected.size(); ++i)
            BOOST_TEST_EQ(bg::get<0>(result[i + 1]).rational(), bg::get<0>(expected[i]));
}

int main()
{
    areal();
    open_ring_wkt();
    linear();
    pointlike();
    final_rounding();
    boxes_and_collections();
    cached_rational();
    // Operand rebinding must leave adapted C array points intact for other strategies.
    double a[2] = {1, 2}, b[2] = {1, 2};
    BOOST_TEST(bg::equals(a, b, bg::strategies::relate::cartesian<>()));
    return boost::report_errors();
}
