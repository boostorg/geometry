// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2021 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#define BOOST_GEOMETRY_NO_ROBUSTNESS

#include <geometry_test_common.hpp>
#include <algorithms/overlay/multi_overlay_cases.hpp>

#include <boost/geometry/algorithms/correct.hpp>
#include <boost/geometry/algorithms/union.hpp>
#include <boost/geometry/io/wkt/wkt.hpp>
#include <boost/geometry/geometries/geometries.hpp>

#include <algorithms/overlay/multi_overlay_cases.hpp>

#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/multi_polygon.hpp>

#include <boost/geometry/util/rational.hpp>

template <typename Geometry>
void test_one(std::string const& case_id,
              std::string const& wkt1, std::string const& wkt2,
              typename bg::coordinate_type<Geometry>::type const& expected_area)
{
    Geometry g1;
    bg::read_wkt(wkt1, g1);

    Geometry g2;
    bg::read_wkt(wkt2, g2);

    bg::correct(g1);
    bg::correct(g2);

    Geometry clip;

    bg::union_(g1, g2, clip);

    auto const area = bg::area(clip);

    BOOST_CHECK_MESSAGE(bg::math::equals(expected_area, area),
            "union: " << case_id
            << " #area expected: " << expected_area
            << " detected: " << area
            << " type: " << (string_from_type<typename bg::coordinate_type<Geometry>::type>::name()));

}

template <typename Point>
void test_areal()
{
    using polygon = bg::model::polygon<Point>;
    using multi_polygon = bg::model::multi_polygon<polygon>;

    // Intended tests: only 3:
    // - one simple case (rectangular)
    // - (TODO) one test with diagonals (but all intersection points should be on the integer grid)
    // - (TODO) one test currently often failing because of FP problems

    test_one<multi_polygon>("case_multi_rectangular",
        case_multi_rectangular[0], case_multi_rectangular[1], 33125);
}


int test_main(int, char* [])
{
    namespace bm = boost::multiprecision;

    using bg::model::d2::point_xy;

    // Our reference types
    test_areal<point_xy<long int>>();
    test_areal<point_xy<long double>>();

    // Boost multi precision (int)
    test_areal<point_xy<bm::int128_t>>();

    // Boost multi precision (float)
    test_areal<point_xy<bm::number<bm::cpp_bin_float<5> >>>();
    test_areal<point_xy<bm::number<bm::cpp_bin_float<10> >>>();
    test_areal<point_xy<bm::number<bm::cpp_bin_float<50> >>>();
    test_areal<point_xy<bm::number<bm::cpp_bin_float<100> >>>();

    // Boost rational compiles, and is correct
    // (because it's rectangular, some other input is currently incorrect)
    test_areal<point_xy<boost::rational<int>>>();

    return 0;
}
