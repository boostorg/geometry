// Boost.Geometry
// Distributed under the Boost Software License, Version 1.0.

#include <boost/core/lightweight_test.hpp>
#include <boost/geometry.hpp>
#include <cmath>
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
    bg::model::multi_point<point> output;
    bg::union_(a, b, output);
    BOOST_TEST_EQ(output.size(), 2u);
#endif
    return boost::report_errors();
}
