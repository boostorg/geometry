// Boost.Geometry
// Unit Test

// Copyright (c) 2016 Oracle and/or its affiliates.

// Contributed and/or modified by Adam Wulkiewicz, on behalf of Oracle

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GEOMETRY_TEST_FORMULA_HPP
#define BOOST_GEOMETRY_TEST_FORMULA_HPP

#include <geometry_test_common.hpp>

#include <boost/geometry/util/math.hpp>

void normalize_deg(double & deg)
{
    while (deg > 180.0)
        deg -= 360.0;
    while (deg <= -180.0)
        deg += 360.0;
}

void check_one(double result, double expected, double reference, double reference_error, bool normalize = false)
{
    if (normalize)
    {
        normalize_deg(result);
        normalize_deg(expected);
        normalize_deg(reference);
    }

    double res_max = (std::max)(bg::math::abs(result), bg::math::abs(expected));
    if (res_max > 100 * std::numeric_limits<double>::epsilon())
    {
        BOOST_CHECK_CLOSE(result, expected, 0.001);
    }
    else if (res_max > 10 * std::numeric_limits<double>::epsilon())
    {
        BOOST_CHECK_CLOSE(result, expected, 0.1);
    }
    else if (res_max > std::numeric_limits<double>::epsilon())
    {
        BOOST_CHECK_CLOSE(result, expected, 10);
    }

    // NOTE: in some cases it probably will be necessary to normalize
    //       the differences between the result and expected result
    double ref_diff = bg::math::abs(result - reference);
    double ref_max = (std::max)(bg::math::abs(result), bg::math::abs(reference));
    bool is_ref_close = ref_diff <= reference_error || ref_diff <= reference_error * ref_max;

    BOOST_CHECK_MESSAGE((is_ref_close), std::setprecision(16) << "{" << result << "} and {" << reference << "} not close enough.");
}

#endif // BOOST_GEOMETRY_TEST_FORMULA_HPP
