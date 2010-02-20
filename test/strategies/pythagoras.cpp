// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <ggl_test_common.hpp>

#include <boost/geometry/algorithms/assign.hpp>
#include <boost/geometry/strategies/cartesian/distance_pythagoras.hpp>


#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>
#include <test_common/test_point.hpp>

template <typename P1, typename P2>
void test_null_distance_3d()
{
    typename boost::geometry::strategy::distance::pythagoras<P1, P2> pythagoras;

    P1 p1;
    boost::geometry::assign(p1, 1, 2, 3);
    P2 p2;
    boost::geometry::assign(p2, 1, 2, 3);
    BOOST_CHECK_EQUAL(double(
            typename boost::geometry::coordinate_type<P1>::type(pythagoras.apply(p1, p2))), 0);
}

template <typename P1, typename P2>
void test_axis_3d()
{
    boost::geometry::strategy::distance::pythagoras<P1, P2> pythagoras;

    P1 p1;
    boost::geometry::assign(p1, 0, 0, 0);

    P2 p2;
    boost::geometry::assign(p2, 1, 0, 0);
    BOOST_CHECK_EQUAL(double(
            typename boost::geometry::coordinate_type<P1>::type(pythagoras.apply(p1, p2))), 1);
    boost::geometry::assign(p2, 0, 1, 0);
    BOOST_CHECK_EQUAL(double(
            typename boost::geometry::coordinate_type<P1>::type(pythagoras.apply(p1, p2))), 1);
    boost::geometry::assign(p2, 0, 0, 1);
    BOOST_CHECK_EQUAL(double(
            typename boost::geometry::coordinate_type<P1>::type(pythagoras.apply(p1, p2))), 1);
}

template <typename P1, typename P2>
void test_arbitrary_3d()
{
    boost::geometry::strategy::distance::pythagoras<P1, P2> pythagoras;

    P1 p1;
    boost::geometry::assign(p1, 1, 2, 3);
    P2 p2;
    boost::geometry::assign(p2, 9, 8, 7);
    BOOST_CHECK_CLOSE(double(
            typename boost::geometry::coordinate_type<P1>::type(pythagoras.apply(p1, p2))),
            sqrt((double)116), 0.001);
}

template <typename CoordinateType, typename CalculationType, typename AssignType>
void test_big_2d_with(AssignType const& x1, AssignType const& y1,
                 AssignType const& x2, AssignType const& y2)
{
    typedef boost::geometry::point<CoordinateType, 2, boost::geometry::cs::cartesian> point_type;
    typedef boost::geometry::strategy::distance::pythagoras
        <
            point_type,
            point_type,
            CalculationType
        > pythagoras_type;

    pythagoras_type pythagoras;


    point_type p1, p2;
    boost::geometry::assign(p1, x1, y1);
    boost::geometry::assign(p2, x2, y2);
    typename pythagoras_type::return_type d1 = pythagoras.apply(p1, p2);

    /*
    std::cout << typeid(CalculationType).name()
        << " " << std::fixed << std::setprecision(20) << d1.squared_value()
        << std::endl << std::endl;
    */

    CalculationType d2 = d1;

    BOOST_CHECK_CLOSE((double) d2,
            1076554.5485833955678294387789057, 0.001);
}

template <typename CoordinateType, typename CalculationType>
void test_big_2d()
{
    test_big_2d_with<CoordinateType, CalculationType>
        (123456.78900001, 234567.89100001,
        987654.32100001, 876543.21900001);
}

template <typename CoordinateType, typename CalculationType>
void test_big_2d_string()
{
    test_big_2d_with<CoordinateType, CalculationType>
        ("123456.78900001", "234567.89100001",
        "987654.32100001", "876543.21900001");
}



template <typename P1, typename P2>
void test_all_3d()
{
return;
    test_null_distance_3d<P1, P2>();
    test_axis_3d<P1, P2>();
    test_arbitrary_3d<P1, P2>();
}

template <typename P>
void test_all_3d()
{
    using boost::geometry::point;
    using boost::geometry::cs::cartesian;

    test_all_3d<P, int[3]>();
    test_all_3d<P, float[3]>();
    test_all_3d<P, double[3]>();
    test_all_3d<P, test::test_point>();
    test_all_3d<P, point<int, 3, cartesian> >();
    test_all_3d<P, point<float, 3, cartesian> >();
    test_all_3d<P, point<double, 3, cartesian> >();
}

int test_main(int, char* [])
{
    using boost::geometry::point;
    using boost::geometry::cs::cartesian;

#if ! defined(_MSC_VER)
    test_all_3d<int[3]>();
#endif
    test_all_3d<float[3]>();
    test_all_3d<double[3]>();
    test_all_3d<test::test_point>();
#if ! defined(_MSC_VER)
    test_all_3d<point<int, 3, cartesian> >();
#endif
    test_all_3d<point<float, 3, cartesian> >();
    test_all_3d<point<double, 3, cartesian> >();

    test_big_2d<float, float>();
    test_big_2d<double, double>();
    test_big_2d<long double, long double>();
    test_big_2d<float, long double>();

/*
TODO: fix this, assign type should use boost::to etc

#if defined(HAVE_CLN)
    // combination of CLN with normal types
    typedef boost::numeric_adaptor::cln_value_type cln_type;
    typedef point<cln_type, 3, cartesian> cln_point;
    test_all_3d<cln_point>();
    test_all_3d<cln_point, cln_point>();

    test_big_2d<cln_type, cln_type>();
    test_big_2d_string<cln_type, cln_type>();

#endif
#if defined(HAVE_GMP)
    typedef boost::numeric_adaptor::gmp_value_type gmp_type;
    typedef point<gmp_type, 3, cartesian> gmp_point;
    test_all_3d<gmp_point>();
    test_all_3d<gmp_point, gmp_point>();

    test_big_2d<gmp_type, gmp_type>();
    test_big_2d_string<gmp_type, gmp_type>();
#endif
*/
    return 0;
}
