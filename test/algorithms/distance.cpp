// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//#define TEST_ARRAY

#include <sstream>

#include <boost/mpl/if.hpp>
#include <geometry_test_common.hpp>


#include <boost/geometry/algorithms/distance.hpp>
#include <boost/geometry/strategies/strategies.hpp>

#ifndef TEST_ARRAY
#include <boost/geometry/algorithms/make.hpp>
#endif

#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/geometries/adapted/boost_array_as_linestring.hpp>
#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>
#include <test_common/test_point.hpp>


template <typename P>
void test_distance_result()
{
    typedef typename boost::geometry::distance_result<P, P>::type distance_type;

#ifndef TEST_ARRAY
    P p1 = boost::geometry::make<P>(0, 0);
    P p2 = boost::geometry::make<P>(3, 0);
    P p3 = boost::geometry::make<P>(0, 4);
#else
    P p1, p2, p3;
    boost::geometry::set<0>(p1, 0); boost::geometry::set<1>(p1, 0);
    boost::geometry::set<0>(p2, 3); boost::geometry::set<1>(p2, 0);
    boost::geometry::set<0>(p3, 0); boost::geometry::set<1>(p3, 4);
#endif

    distance_type dr12 = boost::geometry::distance(p1, p2);
    distance_type dr13 = boost::geometry::distance(p1, p3);
    distance_type dr23 = boost::geometry::distance(p2, p3);

    BOOST_CHECK_CLOSE(double(dr12), 3.000, 0.001);
    BOOST_CHECK_CLOSE(double(dr13), 4.000, 0.001);
    BOOST_CHECK_CLOSE(double(dr23), 5.000, 0.001);

    // COMPILATION TESTS
    distance_type comparable = boost::geometry::make_distance_result<distance_type>(3);
    //BOOST_CHECK_CLOSE(comparable.value(), 9.000, 0.001);


    // Question: how to check if the implemented operator is used, and not the auto-conversion to double?
    if (comparable == dr12) {};
    if (comparable < dr12) {};
    if (comparable > dr12) {};

    // Check streamability
    std::ostringstream s;
    s << comparable;

    // Check comparisons with plain double
    double d = 3.0;
    if (dr12 == d) {};
    if (dr12 < d) {};
    if (dr12 > d) {};

}

template <typename P>
void test_distance_point()
{
    P p1;
    boost::geometry::set<0>(p1, 1);
    boost::geometry::set<1>(p1, 1);

    P p2;
    boost::geometry::set<0>(p2, 2);
    boost::geometry::set<1>(p2, 2);

    double d = boost::geometry::distance(p1, p2);
    BOOST_CHECK_CLOSE(d, 1.4142135, 0.001);
}

template <typename P>
void test_distance_segment()
{
    typedef typename boost::geometry::coordinate_type<P>::type coordinate_type;

    P s1; boost::geometry::set<0>(s1, 2); boost::geometry::set<1>(s1, 2);
    P s2; boost::geometry::set<0>(s2, 3); boost::geometry::set<1>(s2, 3);

    // Check points left, right, projected-left, projected-right, on segment
    P p1; boost::geometry::set<0>(p1, 0); boost::geometry::set<1>(p1, 0);
    P p2; boost::geometry::set<0>(p2, 4); boost::geometry::set<1>(p2, 4);
    P p3; boost::geometry::set<0>(p3, coordinate_type(2.4)); boost::geometry::set<1>(p3, coordinate_type(2.6));
    P p4; boost::geometry::set<0>(p4, coordinate_type(2.6)); boost::geometry::set<1>(p4, coordinate_type(2.4));
    P p5; boost::geometry::set<0>(p5, 2.5); boost::geometry::set<1>(p5, 2.5);

    const boost::geometry::segment<const P> seg(s1, s2);

    double d1 = boost::geometry::distance(p1, seg); BOOST_CHECK_CLOSE(d1, 2.8284271, 0.001);
    double d2 = boost::geometry::distance(p2, seg); BOOST_CHECK_CLOSE(d2, 1.4142135, 0.001);
    double d3 = boost::geometry::distance(p3, seg); BOOST_CHECK_CLOSE(d3, 0.141421, 0.001);
    double d4 = boost::geometry::distance(p4, seg); BOOST_CHECK_CLOSE(d4, 0.141421, 0.001);
    double d5 = boost::geometry::distance(p5, seg); BOOST_CHECK_CLOSE(d5, 0.0, 0.001);

    // Reverse case
    double dr1 = boost::geometry::distance(seg, p1); BOOST_CHECK_CLOSE(dr1, d1, 0.001);
    double dr2 = boost::geometry::distance(seg, p2); BOOST_CHECK_CLOSE(dr2, d2, 0.001);
}

template <typename P>
void test_distance_linestring()
{
    typedef typename boost::geometry::coordinate_type<P>::type coordinate_type;

    boost::array<P, 2> points;
    boost::geometry::set<0>(points[0], 1);
    boost::geometry::set<1>(points[0], 1);
    boost::geometry::set<0>(points[1], 3);
    boost::geometry::set<1>(points[1], 3);

#ifndef TEST_ARRAY
    P p = boost::geometry::make<P>(2, 1);
#else
    P p;
    boost::geometry::set<0>(p, 2); boost::geometry::set<1>(p, 1);
#endif

    double d = boost::geometry::distance(p, points);
    BOOST_CHECK_CLOSE(d, 0.70710678, 0.001);

#ifndef TEST_ARRAY
    p = boost::geometry::make<P>(5, 5);
#else
    boost::geometry::set<0>(p, 5); boost::geometry::set<1>(p, 5);
#endif
    d = boost::geometry::distance(p, points);
    BOOST_CHECK_CLOSE(d, 2.828427, 0.001);


    boost::geometry::linestring<P> line;
#ifndef TEST_ARRAY
    line.push_back(boost::geometry::make<P>(1,1));
    line.push_back(boost::geometry::make<P>(2,2));
    line.push_back(boost::geometry::make<P>(3,3));
#else
    {
        P lp;
        boost::geometry::set<0>(lp, 1); boost::geometry::set<1>(lp, 1); line.push_back(lp);
        boost::geometry::set<0>(lp, 2); boost::geometry::set<1>(lp, 2); line.push_back(lp);
        boost::geometry::set<0>(lp, 3); boost::geometry::set<1>(lp, 3); line.push_back(lp);
    }
#endif

#ifndef TEST_ARRAY
    p = boost::geometry::make<P>(5, 5);
#else
    boost::geometry::set<0>(p, 5); boost::geometry::set<1>(p, 5);
#endif

    d = boost::geometry::distance(p, line);
    BOOST_CHECK_CLOSE(d, 2.828427, 0.001);

    // Reverse case
    d = boost::geometry::distance(line, p);
    BOOST_CHECK_CLOSE(d, 2.828427, 0.001);
}

template <typename P>
void test_all()
{
    test_distance_result<P>();
    test_distance_point<P>();
    test_distance_segment<P>();
#ifndef TEST_ARRAY
    test_distance_linestring<P>();
#endif
}

int test_main(int, char* [])
{
#ifdef TEST_ARRAY
    //test_all<int[2]>();
    test_all<float[2]>();
    test_all<double[2]>();
    test_all<test::test_point>(); // located here because of 3D
#endif

    //test_all<boost::geometry::point_xy<int> >();
    test_all<boost::tuple<float, float> >();
    test_all<boost::geometry::point_xy<float> >();
    test_all<boost::geometry::point_xy<double> >();

    return 0;
}
