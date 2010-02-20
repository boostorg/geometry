// Boost.Geometry (aka GGL, Generic Geometry Library) test file
//
// Copyright Barend Gehrels 2007-2009, Geodan, Amsterdam, the Netherlands
// Copyright Bruno Lalande 2008, 2009
// Use, modification and distribution is subject to the Boost Software License,
// Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <geometry_test_common.hpp>

#include <boost/geometry/strategies/cartesian/distance_projected_point.hpp>
#include <boost/geometry/strategies/concepts/distance_concept.hpp>

#include <boost/geometry/extensions/gis/io/wkt/read_wkt.hpp>


#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/adapted/c_array_cartesian.hpp>
#include <boost/geometry/geometries/adapted/tuple_cartesian.hpp>
#include <test_common/test_point.hpp>


template <typename P1, typename P2>
void test_all_2d()
{
    P1 p;
    P2 sp1, sp2;
    boost::geometry::read_wkt("POINT(1 1)", p);
    boost::geometry::read_wkt("POINT(0 0)", sp1);
    boost::geometry::read_wkt("POINT(2 3)", sp2);

    typedef typename boost::geometry::strategy::distance::projected_point
        <
            P1,
            P2
        > strategy_type;

    BOOST_CONCEPT_ASSERT
        (
            (boost::geometry::concept::PointSegmentDistanceStrategy<strategy_type>)
        );


    strategy_type strategy;
    std::cout << strategy.apply(p, sp1, sp2) << std::endl;
}


template <typename P>
void test_all_2d()
{
    using boost::geometry::point;
    using boost::geometry::cs::cartesian;

    //test_all_2d<P, int[2]>();
    //test_all_2d<P, float[2]>();
    //test_all_2d<P, double[2]>();
    //test_all_2d<P, test::test_point>();
    //test_all_2d<P, point<int, 2, cartesian> >();
    //test_all_2d<P, point<float, 2, cartesian> >();
    test_all_2d<P, point<double, 2, cartesian> >();
}

int test_main(int, char* [])
{
    using boost::geometry::point;
    using boost::geometry::cs::cartesian;

#if ! defined(_MSC_VER)
    test_all_2d<int[2]>();
    test_all_2d<float[2]>();
    test_all_2d<double[2]>();
#endif
    //test_all_2d<test::test_point>();

#if ! defined(_MSC_VER)
    test_all_2d<point<int, 2, cartesian> >();
#endif
    test_all_2d<point<float, 2, cartesian> >();
    test_all_2d<point<double, 2, cartesian> >();


#if defined(HAVE_CLN)
    // combination of CLN with normal types
    typedef boost::numeric_adaptor::cln_value_type cln_type;
    typedef point<cln_type, 2, cartesian> cln_point;
    test_all_2d<cln_point>();

#endif
#if defined(HAVE_GMP)
    typedef boost::numeric_adaptor::gmp_value_type gmp_type;
    typedef point<gmp_type, 2, cartesian> gmp_point;
    test_all_2d<gmp_point>();

#endif

    return 0;
}
