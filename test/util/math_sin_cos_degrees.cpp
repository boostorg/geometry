// Boost.Geometry
// Unit Test

// Copyright (c) 2026 Pulk17

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <cmath>
#include <limits>

#include <geometry_test_common.hpp>

#include <boost/geometry/util/math.hpp>

namespace bgm = bg::math;

void test_multiples_of_90()
{
    double const sin_expected[] = { 0, 1, 0, -1 };
    double const cos_expected[] = { 1, 0, -1, 0 };

    for (int k = -12; k <= 12; ++k)
    {
        double s, c;
        bgm::sin_cos_degrees(90.0 * k, s, c);
        int const q = ((k % 4) + 4) % 4;
        BOOST_CHECK_EQUAL(s, sin_expected[q]);
        BOOST_CHECK_EQUAL(c, cos_expected[q]);
    }
}

void test_against_std()
{
    // Negative angles below -45 have a negative quotient, which selected
    // the wrong quadrant on platforms that saturate the conversion to
    // unsigned, such as AArch64
    for (double x = -1080; x <= 1080; x += 7.5)
    {
        double s, c;
        bgm::sin_cos_degrees(x, s, c);
        double const r = x * bgm::d2r<double>();
        BOOST_CHECK_SMALL(s - std::sin(r), 1e-14);
        BOOST_CHECK_SMALL(c - std::cos(r), 1e-14);
    }
}

void test_symmetry()
{
    // From the documentation: sin 9 = cos 81 = -sin 123456789
    double s9, c9, s81, c81, s, c;
    bgm::sin_cos_degrees(9.0, s9, c9);
    bgm::sin_cos_degrees(81.0, s81, c81);
    bgm::sin_cos_degrees(123456789.0, s, c);
    BOOST_CHECK_SMALL(s9 - c81, 1e-15);
    BOOST_CHECK_SMALL(s9 + s, 1e-15);
    bgm::sin_cos_degrees(-123456789.0, s, c);
    BOOST_CHECK_SMALL(s9 - s, 1e-15);
    BOOST_CHECK_SMALL(c9 + c, 1e-15);
}

void test_non_finite()
{
    double const values[] = { std::numeric_limits<double>::quiet_NaN(),
                              std::numeric_limits<double>::infinity(),
                              -std::numeric_limits<double>::infinity() };
    for (double x : values)
    {
        double s, c;
        bgm::sin_cos_degrees(x, s, c);
        BOOST_CHECK(std::isnan(s));
        BOOST_CHECK(std::isnan(c));
    }
}

int test_main(int, char* [])
{
    test_multiples_of_90();
    test_against_std();
    test_symmetry();
    test_non_finite();

    return 0;
}
