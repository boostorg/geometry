// Boost.Geometry
// Unit Test

// Copyright (c) 2026 Moritz Fischer, Darmstadt, Germany.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <geometry_test_common.hpp>

#include <boost/geometry/util/math.hpp>

#include <cmath>
#include <limits>

namespace bgm = bg::math;

template <typename T>
void check(T const& x)
{
    T sinx, cosx;
    bgm::sin_cos_degrees(x, sinx, cosx);

    // Reference computed in high precision from the exact argument so that
    // only the rounding of sin_cos_degrees itself is measured.
    mp_test_type const rad = mp_test_type(x) * bgm::d2r<mp_test_type>();
    T const expected_sin = static_cast<T>(sin(rad));
    T const expected_cos = static_cast<T>(cos(rad));

    T const tolerance = T(8) * std::numeric_limits<T>::epsilon();

    BOOST_CHECK_SMALL(sinx - expected_sin, tolerance);
    BOOST_CHECK_SMALL(cosx - expected_cos, tolerance);
}

template <typename T>
void test_all()
{
    // Multiples of 45 degrees hit every branch of the quadrant selection,
    // including the negative quotients (x < -45) which must not be
    // converted to unsigned directly.
    for (int deg = -720; deg <= 720; deg += 45)
    {
        check(T(deg));
    }

    // Values close to the quadrant boundaries.
    for (int deg = -720; deg <= 720; deg += 90)
    {
        check(T(deg) - T(0.001));
        check(T(deg) + T(0.001));
    }

    // Exact results for quadrant multiples.
    T sinx, cosx;

    bgm::sin_cos_degrees(T(-90), sinx, cosx);
    BOOST_CHECK_EQUAL(sinx, T(-1));
    BOOST_CHECK_EQUAL(cosx, T(0));

    bgm::sin_cos_degrees(T(-180), sinx, cosx);
    BOOST_CHECK_EQUAL(sinx, T(0));
    BOOST_CHECK_EQUAL(cosx, T(-1));

    bgm::sin_cos_degrees(T(-270), sinx, cosx);
    BOOST_CHECK_EQUAL(sinx, T(1));
    BOOST_CHECK_EQUAL(cosx, T(0));

    // Elementary properties hold exactly (see documentation).
    T sin9, cos9, sin81, cos81;
    bgm::sin_cos_degrees(T(9), sin9, cos9);
    bgm::sin_cos_degrees(T(81), sin81, cos81);
    BOOST_CHECK_EQUAL(sin9, cos81);
    BOOST_CHECK_EQUAL(cos9, sin81);

    T sin_neg9, cos_neg9;
    bgm::sin_cos_degrees(T(-9), sin_neg9, cos_neg9);
    BOOST_CHECK_EQUAL(sin_neg9, -sin9);
    BOOST_CHECK_EQUAL(cos_neg9, cos9);

    T sin_neg99, cos_neg99;
    bgm::sin_cos_degrees(T(-99), sin_neg99, cos_neg99);
    BOOST_CHECK_EQUAL(sin_neg99, -cos9);
    BOOST_CHECK_EQUAL(cos_neg99, -sin9);

    // sin 9 deg = -sin 123456789 deg; requires 123456789 to be representable.
    if (std::numeric_limits<T>::digits >= 53)
    {
        T sin_large, cos_large;
        bgm::sin_cos_degrees(T(123456789), sin_large, cos_large);
        BOOST_CHECK_EQUAL(sin9, -sin_large);
    }

    // -0 is only returned for sin(-0)
    bgm::sin_cos_degrees(-T(0), sinx, cosx);
    BOOST_CHECK(std::signbit(sinx));
    BOOST_CHECK(! std::signbit(cosx));
}

int test_main(int, char* [])
{
    test_all<float>();
    test_all<double>();

    return 0;
}
