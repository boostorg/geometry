// Boost.Geometry (aka GGL, Generic Geometry Library)
// Unit Test

// Copyright (c) 2012-2014 Barend Gehrels, Amsterdam, the Netherlands.

// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <test_buffer.hpp>

#include <boost/geometry/multi/geometries/multi_geometries.hpp>

static std::string const simplex = "MULTIPOINT((5 5),(7 7))";
static std::string const three = "MULTIPOINT((5 8),(9 8),(7 11))";

// Generated error (extra polygon on top of rest) at distance 14.0:
static std::string const multipoint_a = "MULTIPOINT((39 44),(38 37),(41 29),(15 33),(58 39))";

// Just one with holes at distance ~ 15
static std::string const multipoint_b = "MULTIPOINT((5 56),(98 67),(20 7),(58 60),(10 4),(75 68),(61 68),(75 62),(92 26),(74 6),(67 54),(20 43),(63 30),(45 7))";

// Grid, U-form, generates error for square point at 0.54 (top cells to control rescale)
static std::string const grid_a = "MULTIPOINT(5 0,6 0,7 0,  5 1,7 1,  0 13,8 13)";

template <bool Clockwise, typename P>
void test_all()
{
    typedef bg::model::polygon<P, Clockwise> polygon;
    typedef bg::model::multi_point<P> multi_point_type;

    bg::strategy::buffer::join_miter join_miter;
    bg::strategy::buffer::end_flat end_flat;

    double const pi = boost::geometry::math::pi<double>();

    test_one<multi_point_type, polygon>("simplex1", simplex, join_miter, end_flat, 2.0 * pi, 1.0, 1.0);
    test_one<multi_point_type, polygon>("simplex2", simplex, join_miter, end_flat, 22.8372, 2.0, 2.0);
    test_one<multi_point_type, polygon>("simplex3", simplex, join_miter, end_flat, 44.5692, 3.0, 3.0);

    test_one<multi_point_type, polygon>("three1", three, join_miter, end_flat, 3.0 * pi, 1.0, 1.0);
    test_one<multi_point_type, polygon>("three2", three, join_miter, end_flat, 36.7592, 2.0, 2.0);
    test_one<multi_point_type, polygon>("three19", three, join_miter, end_flat, 33.6914, 1.9, 1.9);
    test_one<multi_point_type, polygon>("three21", three, join_miter, end_flat, 39.6394, 2.1, 2.1);
    test_one<multi_point_type, polygon>("three3", three, join_miter, end_flat, 65.533, 3.0, 3.0);

    test_one<multi_point_type, polygon>("multipoint_a", multipoint_a, join_miter, end_flat, 2049.98, 14.0, 14.0);
    test_one<multi_point_type, polygon>("multipoint_b", multipoint_b, join_miter, end_flat, 7109.88, 15.0, 15.0);
    test_one<multi_point_type, polygon>("multipoint_b1", multipoint_b, join_miter, end_flat, 6911.89, 14.7, 14.7);
    test_one<multi_point_type, polygon>("multipoint_b2", multipoint_b, join_miter, end_flat, 7174.79, 15.1, 15.1);


    // Grid tests
    {
        bg::strategy::buffer::point_square point_strategy;
        bg::strategy::buffer::side_straight side_strategy;

        typedef bg::strategy::buffer::distance_symmetric
        <
            typename bg::coordinate_type<P>::type
        > distance_strategy;

        test_with_custom_strategies<multi_point_type, polygon>("grid_a50",
                grid_a, join_miter, end_flat,
                distance_strategy(0.5), side_strategy, point_strategy, 7.0);
#if defined(BOOST_GEOMETRY_BUFFER_INCLUDE_FAILING_TESTS)
        test_with_custom_strategies<multi_point_type, polygon>("grid_a54",
                grid_a, join_miter, end_flat,
                distance_strategy(0.54), side_strategy, point_strategy, 99);
#endif
    }
}

int test_main(int, char* [])
{
    test_all<true, bg::model::point<double, 2, bg::cs::cartesian> >();
    test_all<false, bg::model::point<double, 2, bg::cs::cartesian> >();

    return 0;
}
