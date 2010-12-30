// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#define TEST_ARRAY

#include <sstream>

#include <algorithms/test_distance.hpp>

#include <boost/mpl/if.hpp>

#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/adapted/boost_array_as_linestring.hpp>
#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>

#include <test_common/test_point.hpp>
#include <test_geometries/custom_segment.hpp>
#include <test_geometries/wrapped_boost_array.hpp>


template <typename P>
void test_distance_point()
{
    namespace services = bg::strategy::distance::services;
    typedef typename bg::distance_result<P>::type return_type;

    // Basic, trivial test

    P p1;
    bg::set<0>(p1, 1);
    bg::set<1>(p1, 1);

    P p2;
    bg::set<0>(p2, 2);
    bg::set<1>(p2, 2);

    return_type d = bg::distance(p1, p2);
    BOOST_CHECK_CLOSE(d, return_type(1.4142135), 0.001);

    // Test specifying strategy manually
    typename services::default_strategy<bg::point_tag, P>::type strategy;

    d = bg::distance(p1, p2, strategy);
    BOOST_CHECK_CLOSE(d, return_type(1.4142135), 0.001);

    {
        // Test custom strategy
        BOOST_CONCEPT_ASSERT( (bg::concept::PointDistanceStrategy<taxicab_distance<P> >) );

        typedef typename services::return_type<taxicab_distance<P> >::type cab_return_type;
        BOOST_MPL_ASSERT((boost::is_same<cab_return_type, typename bg::coordinate_type<P>::type>));

        taxicab_distance<P> tcd;
        cab_return_type d = bg::distance(p1, p2, tcd);

        BOOST_CHECK( bg::math::abs(d - cab_return_type(2)) <= cab_return_type(0.01) );
    }

    {
        // test comparability

        typedef typename services::default_strategy<bg::point_tag, P>::type strategy_type;
        typedef typename services::comparable_type<strategy_type>::type comparable_strategy_type;

        strategy_type strategy;
        comparable_strategy_type comparable_strategy = services::get_comparable<strategy_type>::apply(strategy);
        return_type comparable = services::result_from_distance<comparable_strategy_type>::apply(comparable_strategy, 3);

        BOOST_CHECK_CLOSE(comparable, return_type(9), 0.001);
    }
}

template <typename P>
void test_distance_segment()
{
    typedef typename bg::distance_result<P>::type return_type;
    typedef typename bg::coordinate_type<P>::type coordinate_type;

    P s1; bg::set<0>(s1, 1); bg::set<1>(s1, 1);
    P s2; bg::set<0>(s2, 4); bg::set<1>(s2, 4);

    // Check points left, right, projected-left, projected-right, on segment
    P p1; bg::set<0>(p1, 0); bg::set<1>(p1, 1);
    P p2; bg::set<0>(p2, 1); bg::set<1>(p2, 0);
    P p3; bg::set<0>(p3, 3); bg::set<1>(p3, 1);
    P p4; bg::set<0>(p4, 1); bg::set<1>(p4, 3);
    P p5; bg::set<0>(p5, 3); bg::set<1>(p5, 3);

    bg::model::referring_segment<P const> const seg(s1, s2);

    return_type d1 = bg::distance(p1, seg);
    return_type d2 = bg::distance(p2, seg);
    return_type d3 = bg::distance(p3, seg);
    return_type d4 = bg::distance(p4, seg);
    return_type d5 = bg::distance(p5, seg);

    BOOST_CHECK_CLOSE(d1, return_type(1), 0.001);
    BOOST_CHECK_CLOSE(d2, return_type(1), 0.001);
    BOOST_CHECK_CLOSE(d3, return_type(1.4142135), 0.001);
    BOOST_CHECK_CLOSE(d4, return_type(1.4142135), 0.001);
    BOOST_CHECK_CLOSE(d5, return_type(0), 0.001);

    // Reverse case: segment/point instead of point/segment
    return_type dr1 = bg::distance(seg, p1);
    return_type dr2 = bg::distance(seg, p2);

    BOOST_CHECK_CLOSE(dr1, d1, 0.001);
    BOOST_CHECK_CLOSE(dr2, d2, 0.001);

    // Test specifying strategy manually:
    // 1) point-point-distance
    typename bg::strategy::distance::services::default_strategy<bg::point_tag, P>::type pp_strategy;
    d1 = bg::distance(p1, seg, pp_strategy);
    BOOST_CHECK_CLOSE(d1, return_type(1), 0.001);

    // 2) point-segment-distance
    typename bg::strategy::distance::services::default_strategy<bg::segment_tag, P>::type ps_strategy;
    d1 = bg::distance(p1, seg, ps_strategy);
    BOOST_CHECK_CLOSE(d1, return_type(1), 0.001);

    // 3) custom point strategy
    taxicab_distance<P> tcd;
    d1 = bg::distance(p1, seg, tcd);
    BOOST_CHECK_CLOSE(d1, return_type(1), 0.001);
}


template <typename P>
void test_distance_array_as_linestring()
{
    typedef typename bg::distance_result<P>::type return_type;

    // Normal array does not have
    boost::array<P, 2> points;
    bg::set<0>(points[0], 1);
    bg::set<1>(points[0], 1);
    bg::set<0>(points[1], 3);
    bg::set<1>(points[1], 3);

    P p;
    bg::set<0>(p, 2);
    bg::set<1>(p, 1);

    return_type d = bg::distance(p, points);
    BOOST_CHECK_CLOSE(d, return_type(0.70710678), 0.001);

    bg::set<0>(p, 5); bg::set<1>(p, 5);
    d = bg::distance(p, points);
    BOOST_CHECK_CLOSE(d, return_type(2.828427), 0.001);
}




template <typename P>
void test_all()
{
    test_distance_point<P>();
    test_distance_segment<P>();
    test_distance_array_as_linestring<P>();

    test_geometry<P, bg::model::segment<P> >("POINT(1 3)", "LINESTRING(1 1,4 4)", sqrt(2.0));
    test_geometry<P, bg::model::segment<P> >("POINT(3 1)", "LINESTRING(1 1,4 4)", sqrt(2.0));

    test_geometry<P, P>("POINT(1 1)", "POINT(2 2)", sqrt(2.0));
    test_geometry<P, P>("POINT(0 0)", "POINT(0 3)", 3.0);
    test_geometry<P, P>("POINT(0 0)", "POINT(4 0)", 4.0);
    test_geometry<P, P>("POINT(0 3)", "POINT(4 0)", 5.0);
    test_geometry<P, bg::model::linestring<P> >("POINT(1 3)", "LINESTRING(1 1,4 4)", sqrt(2.0));
    test_geometry<P, bg::model::linestring<P> >("POINT(3 1)", "LINESTRING(1 1,4 4)", sqrt(2.0));
    test_geometry<bg::model::linestring<P>, P>("LINESTRING(1 1,4 4)", "POINT(1 3)", sqrt(2.0));

    // Rings
    test_geometry<P, bg::model::linear_ring<P> >("POINT(1 3)", "POLYGON((1 1,4 4,5 0,1 1))", sqrt(2.0));
    test_geometry<P, bg::model::linear_ring<P> >("POINT(3 1)", "POLYGON((1 1,4 4,5 0,1 1))", 0.0);
    // other way round
    test_geometry<bg::model::linear_ring<P>, P>("POLYGON((1 1,4 4,5 0,1 1))", "POINT(3 1)", 0.0);
    // open ring
    test_geometry<P, bg::model::linear_ring<P, true, false> >("POINT(1 3)", "POLYGON((4 4,5 0,1 1))", sqrt(2.0));

    // Polygons
    test_geometry<P, bg::model::polygon<P> >("POINT(1 3)", "POLYGON((1 1,4 4,5 0,1 1))", sqrt(2.0));
    test_geometry<P, bg::model::polygon<P> >("POINT(3 1)", "POLYGON((1 1,4 4,5 0,1 1))", 0.0);
    // other way round
    test_geometry<bg::model::polygon<P>, P>("POLYGON((1 1,4 4,5 0,1 1))", "POINT(3 1)", 0.0);
    // open polygon
    test_geometry<P, bg::model::polygon<P, true, false> >("POINT(1 3)", "POLYGON((4 4,5 0,1 1))", sqrt(2.0));

    // Polygons with holes
    std::string donut = "POLYGON ((0 0,1 9,8 1,0 0),(1 1,4 1,1 4,1 1))";
    test_geometry<P, bg::model::polygon<P> >("POINT(2 2)", donut, 0.5 * sqrt(2.0));
    test_geometry<P, bg::model::polygon<P> >("POINT(3 3)", donut, 0.0);
    // other way round
    test_geometry<bg::model::polygon<P>, P>(donut, "POINT(2 2)", 0.5 * sqrt(2.0));
    // open
    test_geometry<P, bg::model::polygon<P, true, false> >("POINT(2 2)", "POLYGON ((0 0,1 9,8 1),(1 1,4 1,1 4))", 0.5 * sqrt(2.0));


    // DOES NOT COMPILE - cannot do read_wkt (because boost::array is not variably sized)
    // test_geometry<P, boost::array<P, 2> >("POINT(3 1)", "LINESTRING(1 1,4 4)", sqrt(2.0));

    test_geometry<P, test::wrapped_boost_array<P, 2> >("POINT(3 1)", "LINESTRING(1 1,4 4)", sqrt(2.0));
}

int test_main(int, char* [])
{
#ifdef TEST_ARRAY
    //test_all<int[2]>();
    //test_all<float[2]>();
    //test_all<double[2]>();
    //test_all<test::test_point>(); // located here because of 3D
#endif

    test_all<bg::model::d2::point_xy<int> >();
    test_all<boost::tuple<float, float> >();
    test_all<bg::model::d2::point_xy<float> >();
    test_all<bg::model::d2::point_xy<double> >();

    return 0;
}
