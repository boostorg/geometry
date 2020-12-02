// Boost.Geometry
// Unit Test

// Copyright (c) 2020 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <geometry_test_common.hpp>

#include <boost/geometry/algorithms/detail/overlay/sort_by_side.hpp>

#include <boost/geometry/strategies/strategies.hpp>
#include <boost/geometry/algorithms/disjoint.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/io/wkt/wkt.hpp>

template <typename T, bool Closed>
void test_basic(std::string const& case_id, int line,
                std::string const& wkt, bg::segment_identifier const& id,
                int offset, std::size_t expected_index)
{
    using point_type = bg::model::point<T, 2, bg::cs::cartesian>;
    using polygon_type = bg::model::polygon<point_type, true, Closed>;

    polygon_type polygon;
    bg::read_wkt(wkt, polygon);

    // Check the result
    auto const& ring = bg::exterior_ring(polygon);

    point_type point;
    bg::copy_segment_point<false>(polygon, id, offset, point);

    // Sanity check
    bool const expectation_in_range = expected_index < ring.size();
    BOOST_CHECK_MESSAGE(expectation_in_range, "Expectation out of range " << expected_index);
    if (! expectation_in_range)
    {
        return;
    }

    point_type const expected_point = ring[expected_index];

    bool const equal = ! bg::disjoint(point, expected_point);

    BOOST_CHECK_MESSAGE(equal, "copy_segment_point: " << case_id
                        << " line " << line << " at index " << expected_index
                        << " not as expected: "
                        << bg::wkt(point) << " vs " << bg::wkt(expected_point));
}

template <typename T,  bool Closed>
void test_all(std::string const& case_id, std::string const& wkt)
{
    // Check zero offset, all segment ids
    test_basic<T, Closed>(case_id, __LINE__, wkt, {0, 0, -1, 0}, 0, 0);
    test_basic<T, Closed>(case_id, __LINE__, wkt, {0, 0, -1, 1}, 0, 1);
    test_basic<T, Closed>(case_id, __LINE__, wkt, {0, 0, -1, 2}, 0, 2);
    test_basic<T, Closed>(case_id, __LINE__, wkt, {0, 0, -1, 3}, 0, 3);

    // Check positive offsets, it should endlessly loop around, regardless of direction or closure
    bg::segment_identifier const start{0, 0, -1, 0};
    test_basic<T, Closed>(case_id, __LINE__, wkt, start, 1, 1);
    test_basic<T, Closed>(case_id, __LINE__, wkt, start, 2, 2);
    test_basic<T, Closed>(case_id, __LINE__, wkt, start, 3, 3);
    test_basic<T, Closed>(case_id, __LINE__, wkt, start, 4, 0);
    test_basic<T, Closed>(case_id, __LINE__, wkt, start, 5, 1);
    test_basic<T, Closed>(case_id, __LINE__, wkt, start, 6, 2);
    test_basic<T, Closed>(case_id, __LINE__, wkt, start, 7, 3);

    // Check negative offsets
    test_basic<T, Closed>(case_id, __LINE__, wkt, start, -1, 3);
    test_basic<T, Closed>(case_id, __LINE__, wkt, start, -2, 2);
    test_basic<T, Closed>(case_id, __LINE__, wkt, start, -3, 1);
    test_basic<T, Closed>(case_id, __LINE__, wkt, start, -4, 0);
    test_basic<T, Closed>(case_id, __LINE__, wkt, start, -5, 3);
    test_basic<T, Closed>(case_id, __LINE__, wkt, start, -6, 2);
    test_basic<T, Closed>(case_id, __LINE__, wkt, start, -7, 1);
}

int test_main(int, char* [])
{
    test_all<double, true>("closed", "POLYGON((0 2,1 2,1 1,0 1,0 2))");
    test_all<double, false>("open", "POLYGON((0 2,1 2,1 1,0 1))");
    return 0;
}
